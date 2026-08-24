/*
    ql_extract.c

    Sinclair QL / QDOS QL5A / QL5B filesystem extractor

    This file is part of ZEsarUX.

    ZEsarUX is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    Parts of this file were developed with assistance from ChatGPT.

    The implementation is an original implementation and is not copied
    from a third-party QL filesystem implementation.


  Uso:

      ql_extract disco.img
      ql_extract disco.img directorio_salida
      ql_extract -l disco.img

      -l       solo listar

  Compilar:

      gcc -O2 -Wall -Wextra -std=c99 -o ql_extract ql_extract.c

  QL5A:
      Usa la tabla logical -> physical y sector offset.

  QL5B:
      Mismo filesystem QDOS, pero sin traducción
      físico/lógica. La imagen se interpreta como una
      secuencia lineal de sectores de 512 bytes.

  Referencia:
      QDOS/SMS Reference Manual, sección 8.1.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include "ql_extract.h"
#include "debug.h"

#ifdef _WIN32
#include <direct.h>
#define MKDIR(x) _mkdir(x)
#define PATH_SEP "\\"
#else
#include <sys/stat.h>
#include <sys/types.h>
#define MKDIR(x) mkdir(x, 0755)
#define PATH_SEP "/"
#endif


/* ------------------------------------------------------------ */
/* Constantes                                                    */
/* ------------------------------------------------------------ */

#define SECTOR_SIZE             512U

#define QL5A_HEADER_SIZE         96U
#define MAP_OFFSET               96U
#define MAP_ENTRY_SIZE            3U

#define FILE_HEADER_SIZE          64U
#define DIR_ENTRY_SIZE            64U

/*
 * El file number del mapa QDOS ocupa 12 bits.
 * Reservamos 4096 entradas.
 */
#define MAX_FILE_ID            4096U
#define MAX_QDOS_FILE_ID       4095U

#define MAX_FILENAME_LENGTH     256U
#define MAX_OUTPUT_PATH        4096U


/* ------------------------------------------------------------ */
/* Estructuras                                                   */
/* ------------------------------------------------------------ */

typedef struct {
    int *seq;
    size_t count;
    size_t capacity;
} FileBlocks;


typedef struct {
    uint8_t *image;
    size_t image_size;

    char format_id[5];
    char medium_name[11];

    uint16_t free_sectors;
    uint16_t good_sectors;
    uint16_t total_sectors;

    uint16_t sectors_per_track;
    uint16_t sectors_per_cylinder;
    uint16_t tracks;

    uint16_t allocation_size;

    uint32_t directory_eof;
    uint16_t sector_offset;

    uint8_t logical_to_physical[18];
    uint8_t physical_to_logical[18];

    uint32_t block_count;

    FileBlocks files[MAX_FILE_ID];

} QLDisk;


typedef struct {
    uint32_t length;
    uint16_t name_length;
    char name[MAX_FILENAME_LENGTH];
    uint32_t date;
} DirEntry;


/* ------------------------------------------------------------ */
/* Big endian helpers                                            */
/* ------------------------------------------------------------ */

static uint16_t be16(const uint8_t *p)
{
    return ((uint16_t)p[0] << 8) |
           (uint16_t)p[1];
}


static uint32_t be32(const uint8_t *p)
{
    return ((uint32_t)p[0] << 24) |
           ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) |
           (uint32_t)p[3];
}


/* ------------------------------------------------------------ */
/* Safe arithmetic                                               */
/* ------------------------------------------------------------ */

static int size_mul_ok(size_t a,
                        size_t b,
                        size_t *result)
{
    if (b != 0 && a > SIZE_MAX / b)
        return 0;

    *result = a * b;
    return 1;
}


static int size_add_ok(size_t a,
                        size_t b,
                        size_t *result)
{
    if (a > SIZE_MAX - b)
        return 0;

    *result = a + b;
    return 1;
}


/* ------------------------------------------------------------ */
/* File size                                                      */
/* ------------------------------------------------------------ */

static int get_file_size(FILE *f,
                          size_t *out_size)
{
    long pos;

    if (fseek(f, 0, SEEK_END) != 0)
        return 0;

    pos = ftell(f);

    if (pos < 0)
        return 0;

    if ((unsigned long)pos > SIZE_MAX)
        return 0;

    if (fseek(f, 0, SEEK_SET) != 0)
        return 0;

    *out_size = (size_t)pos;

    return 1;
}


/* ------------------------------------------------------------ */
/* Load image                                                     */
/* ------------------------------------------------------------ */

static int load_image(const char *filename,
                      QLDisk *d)
{
    FILE *f;
    size_t size;

    f = fopen(filename, "rb");

    if (!f) {
        fprintf(stderr,
                "No puedo abrir %s: %s\n",
                filename,
                strerror(errno));
        return 0;
    }

    if (!get_file_size(f, &size)) {
        fprintf(stderr,
                "No puedo determinar el tamaño de %s\n",
                filename);
        fclose(f);
        return 0;
    }

    if (size == 0) {
        fprintf(stderr,
                "Imagen vacía\n");
        fclose(f);
        return 0;
    }

    d->image =
        (uint8_t *)malloc(size);

    if (!d->image) {
        fprintf(stderr,
                "Sin memoria para %zu bytes\n",
                size);
        fclose(f);
        return 0;
    }

    d->image_size = size;

    if (fread(d->image,
              1,
              size,
              f) != size) {

        fprintf(stderr,
                "Error leyendo imagen\n");

        fclose(f);
        free(d->image);
        d->image = NULL;
        d->image_size = 0;

        return 0;
    }

    fclose(f);

    return 1;
}


/* ------------------------------------------------------------ */
/* Header detection                                               */
/* ------------------------------------------------------------ */

static int find_qdos_header(const QLDisk *d,
                            size_t *offset)
{
    size_t pos;

    if (d->image_size < 4)
        return 0;

    /*
     * Normalmente el header está en el primer sector.
     *
     * También permitimos localizarlo en un límite de
     * sector de 512 bytes, pero posteriormente solamente
     * aceptamos una imagen desplazada si podemos demostrar
     * que el sector lógico 0 puede ser interpretado.
     */
    for (pos = 0;
         pos + 4 <= d->image_size;
         pos += SECTOR_SIZE) {

        if (memcmp(d->image + pos,
                   "QL5A",
                   4) == 0 ||
            memcmp(d->image + pos,
                   "QL5B",
                   4) == 0) {

            *offset = pos;
            return 1;
        }
    }

    return 0;
}


/* ------------------------------------------------------------ */
/* Parse header                                                   */
/* ------------------------------------------------------------ */

static int parse_header(QLDisk *d)
{
    const uint8_t *h;
    size_t expected_size;

    if (d->image_size < SECTOR_SIZE) {
        fprintf(stderr,
                "Imagen demasiado pequeña\n");
        return 0;
    }

    h = d->image;

    if (memcmp(h, "QL5A", 4) != 0 &&
        memcmp(h, "QL5B", 4) != 0) {

        fprintf(stderr,
                "La imagen no empieza con QL5A/QL5B\n");

        return 0;
    }

    memcpy(d->format_id,
           h,
           4);

    d->format_id[4] = '\0';

    memcpy(d->medium_name,
           h + 0x04,
           10);

    d->medium_name[10] = '\0';

    d->free_sectors =
        be16(h + 0x14);

    d->good_sectors =
        be16(h + 0x16);

    d->total_sectors =
        be16(h + 0x18);

    d->sectors_per_track =
        be16(h + 0x1a);

    d->sectors_per_cylinder =
        be16(h + 0x1c);

    d->tracks =
        be16(h + 0x1e);

    d->allocation_size =
        be16(h + 0x20);

    d->directory_eof =
        be32(h + 0x22);

    d->sector_offset =
        be16(h + 0x26);

    memcpy(d->logical_to_physical,
           h + 0x28,
           18);

    memcpy(d->physical_to_logical,
           h + 0x3a,
           18);


    /*
     * La documentación QDOS indica <=9 para el formato
     * floppy estándar descrito originalmente.
     *
     * QL5B se utiliza también para HD y puede presentar
     * 18 sectores por pista en imágenes de 1.44 MB.
     *
     * Por ello no imponemos el antiguo límite <=9.
     */
    if (d->sectors_per_track == 0) {
        fprintf(stderr,
                "Número de sectores/pista inválido: %u\n",
                d->sectors_per_track);
        return 0;
    }

    if (d->sectors_per_cylinder == 0) {
        fprintf(stderr,
                "Sectores/cilindro inválido: %u\n",
                d->sectors_per_cylinder);
        return 0;
    }

    if (d->tracks == 0) {
        fprintf(stderr,
                "Número de pistas inválido: %u\n",
                d->tracks);
        return 0;
    }

    if (d->allocation_size == 0) {
        fprintf(stderr,
                "Allocation size inválido\n");
        return 0;
    }


    /*
     * En un formato de dos caras:
     *
     *     sectors_per_cylinder =
     *         2 * sectors_per_track
     *
     * En algunos formatos no estándar puede ser diferente,
     * por lo que solamente exigimos que sea múltiplo entero
     * de sectors_per_track.
     */
    if (d->sectors_per_cylinder %
            d->sectors_per_track != 0) {

        fprintf(stderr,
                "Geometría inválida: "
                "sectores/cilindro=%u, "
                "sectores/pista=%u\n",
                d->sectors_per_cylinder,
                d->sectors_per_track);

        return 0;
    }


    /*
     * total_sectors debe corresponder a la geometría.
     *
     * Algunas imágenes pueden contener menos sectores de
     * los indicados en la geometría física, pero no debemos
     * aceptar una imagen que ni siquiera pueda contener los
     * sectores lógicos declarados.
     */
    if (!size_mul_ok((size_t)d->total_sectors,
                     SECTOR_SIZE,
                     &expected_size)) {

        fprintf(stderr,
                "Tamaño de imagen fuera de rango\n");

        return 0;
    }

    if (expected_size > d->image_size) {

        fprintf(stderr,
                "La imagen es demasiado pequeña\n"
                "Esperado al menos: %zu bytes\n"
                "Imagen:              %zu bytes\n",
                expected_size,
                d->image_size);

        return 0;
    }


    /*
     * El número de grupos se obtiene a partir del número
     * de sectores y del allocation size.
     */
    d->block_count =
        d->total_sectors /
        d->allocation_size;

    if (d->block_count == 0) {
        fprintf(stderr,
                "Número de bloques inválido\n");
        return 0;
    }

    if (d->total_sectors %
            d->allocation_size != 0) {

        fprintf(stderr,
                "El número de sectores (%u) "
                "no es múltiplo del allocation size (%u)\n",
                d->total_sectors,
                d->allocation_size);

        return 0;
    }


    /*
     * El allocation map tiene 3 bytes por grupo.
     */
    {
        size_t map_bytes;

        if (!size_mul_ok((size_t)d->block_count,
                         MAP_ENTRY_SIZE,
                         &map_bytes)) {

            fprintf(stderr,
                    "Allocation map demasiado grande\n");

            return 0;
        }

        if (map_bytes > 0x1000000U) {
            fprintf(stderr,
                    "Allocation map fuera de rango\n");
            return 0;
        }
    }

    return 1;
}


/* ------------------------------------------------------------ */
/* Print disk information                                        */
/* ------------------------------------------------------------ */

static void print_disk_info(const QLDisk *d)
{
    char name[11];
    int i;

    memcpy(name,
           d->medium_name,
           10);

    name[10] = '\0';

    for (i = 9; i >= 0; --i) {
        if (name[i] == ' ')
            name[i] = '\0';
        else
            break;
    }

    printf("\n");
    printf("Formato             : %s\n",
           d->format_id);

    printf("Nombre medio        : \"%s\"\n",
           name);

    printf("Sectores libres     : %u\n",
           d->free_sectors);

    printf("Sectores buenos     : %u\n",
           d->good_sectors);

    printf("Sectores totales    : %u\n",
           d->total_sectors);

    printf("Sectores/pista      : %u\n",
           d->sectors_per_track);

    printf("Sectores/cilindro   : %u\n",
           d->sectors_per_cylinder);

    printf("Pistas              : %u\n",
           d->tracks);

    printf("Allocation size     : %u sectores\n",
           d->allocation_size);

    printf("Grupos              : %u\n",
           d->block_count);

    printf("Directory EOF       : block=%u byte=%u\n",
           (unsigned)(d->directory_eof >> 16),
           (unsigned)(d->directory_eof & 0xffff));

    printf("Sector offset       : %u\n",
           d->sector_offset);

    if (strcmp(d->format_id, "QL5A") == 0) {

        printf("\nLogical -> physical:\n");

        for (i = 0; i < 18; ++i) {

            int side =
                (d->logical_to_physical[i] & 0x80)
                ? 1 : 0;

            int sector =
                d->logical_to_physical[i] & 0x7f;

            printf("%2d:%d/%d  ",
                   i,
                   side,
                   sector);

            if ((i & 5) == 5)
                printf("\n");
        }

        printf("\n");
    }
    else {
        printf("\nQL5B: acceso lineal de sectores\n");
    }

    printf("\n");
}


/* ------------------------------------------------------------ */
/* Read logical sector                                            */
/* ------------------------------------------------------------ */

static int read_logical_sector(const QLDisk *d,
                               uint32_t logical_sector,
                               uint8_t *buffer)
{
    uint64_t index;

    if (!buffer)
        return 0;

    if (logical_sector >= d->total_sectors)
        return 0;


    /*
     * --------------------------------------------------------
     * QL5B
     * --------------------------------------------------------
     *
     * QL5B es QL5A sin traducción físico/lógica.
     *
     * Por tanto:
     *
     *     logical sector N -> byte N * 512
     *
     * Esto es especialmente importante para imágenes HD
     * de 1.44 MB.
     */
    if (strcmp(d->format_id, "QL5B") == 0) {

        index =
            (uint64_t)logical_sector *
            SECTOR_SIZE;

        if (index + SECTOR_SIZE >
            d->image_size) {

            return 0;
        }

        memcpy(buffer,
               d->image + index,
               SECTOR_SIZE);

        return 1;
    }


    /*
     * --------------------------------------------------------
     * QL5A
     * --------------------------------------------------------
     */

    {
        uint32_t track;
        uint32_t within_cylinder;

        uint32_t sides;
        uint32_t side;

        uint32_t sector;
        uint32_t physical_sector;

        uint64_t physical_sector_index;


        track =
            logical_sector /
            d->sectors_per_cylinder;

        within_cylinder =
            logical_sector %
            d->sectors_per_cylinder;


        /*
         * Normalmente son 18 sectores/cilindro:
         * 9 por cara.
         *
         * Para formatos con otro número de caras,
         * lo obtenemos de la geometría.
         */
        sides =
            d->sectors_per_cylinder /
            d->sectors_per_track;

        if (sides == 0 || sides > 18)
            return 0;


        /*
         * La tabla estándar solamente contiene 18 entradas.
         */
        if (within_cylinder >= 18)
            return 0;


        {
            uint8_t trans =
                d->logical_to_physical[within_cylinder];

            side =
                (trans & 0x80)
                ? 1U
                : 0U;

            sector =
                trans & 0x7fU;
        }


        if (side >= sides)
            return 0;

        if (sector >= d->sectors_per_track)
            return 0;


        /*
         * Sector offset por pista:
         *
         * physical =
         *     (translated + track * offset)
         *     % sectors_per_track
         */
        physical_sector =
            (sector +
             (track * d->sector_offset))
            % d->sectors_per_track;


        /*
         * Imagen lineal convencional:
         *
         * track 0 side 0
         * track 0 side 1
         * track 1 side 0
         * track 1 side 1
         *
         * Para geometrías de más de dos caras, la fórmula
         * general sigue siendo:
         *
         *     track * sides * sectors_per_track
         *       + side * sectors_per_track
         *       + physical_sector
         */
        physical_sector_index =
            ((uint64_t)track *
             sides *
             d->sectors_per_track)
            +
            ((uint64_t)side *
             d->sectors_per_track)
            +
            physical_sector;


        index =
            physical_sector_index *
            SECTOR_SIZE;


        if (index + SECTOR_SIZE >
            d->image_size) {

            return 0;
        }

        memcpy(buffer,
               d->image + index,
               SECTOR_SIZE);

        return 1;
    }
}


/* ------------------------------------------------------------ */
/* Read logical group/block                                      */
/* ------------------------------------------------------------ */

static int read_block(const QLDisk *d,
                      uint32_t block,
                      uint8_t *buffer)
{
    uint32_t s;
    size_t block_size;

    if (!buffer)
        return 0;

    if (block >= d->block_count)
        return 0;

    if (!size_mul_ok(d->allocation_size,
                     SECTOR_SIZE,
                     &block_size)) {

        return 0;
    }

    /*
     * allocation_size puede ser mayor que el antiguo
     * BLOCK_SIZE fijo. Ya no asumimos 1536 bytes.
     */
    for (s = 0;
         s < d->allocation_size;
         ++s) {

        uint32_t logical_sector =
            block * d->allocation_size + s;

        if (!read_logical_sector(
                d,
                logical_sector,
                buffer +
                (size_t)s * SECTOR_SIZE)) {

            return 0;
        }
    }

    (void)block_size;

    return 1;
}


/* ------------------------------------------------------------ */
/* Add block to file                                             */
/* ------------------------------------------------------------ */

static int add_file_block(QLDisk *d,
                          uint32_t file_id,
                          uint32_t seq,
                          uint32_t block)
{
    FileBlocks *f;

    if (file_id >= MAX_FILE_ID)
        return 0;

    f = &d->files[file_id];

    if (f->count == f->capacity) {

        size_t new_capacity;
        int *new_seq;

        if (f->capacity == 0)
            new_capacity = 8;
        else {
            if (f->capacity >
                SIZE_MAX / 2)
                return 0;

            new_capacity =
                f->capacity * 2;
        }

        if (new_capacity >
            SIZE_MAX / sizeof(int))
            return 0;

        new_seq =
            (int *)realloc(
                f->seq,
                new_capacity *
                sizeof(int));

        if (!new_seq)
            return 0;

        f->seq = new_seq;
        f->capacity = new_capacity;
    }


    /*
     * No empaquetamos seq y block en un int.
     *
     * En la versión original se utilizaban 20 bits para el
     * número de grupo y el resto para la secuencia.
     *
     * Eso era innecesario y podía provocar problemas si los
     * valores crecían.
     *
     * Como las imágenes QL normales tienen un número pequeño
     * de grupos, seguimos utilizando int, pero guardamos:
     *
     *     seq << 20 | block
     *
     * y validamos previamente ambos valores.
     */
    if (seq > 0xfffU)
        return 0;

    if (block > 0xfffffU)
        return 0;

    f->seq[f->count++] =
        ((int)seq << 20) |
        (int)block;

    return 1;
}


/* ------------------------------------------------------------ */
/* Unpack map entry                                               */
/* ------------------------------------------------------------ */

static uint32_t packed_seq(int value)
{
    return ((uint32_t)value >> 20) & 0x0fffU;
}


static uint32_t packed_block(int value)
{
    return (uint32_t)value & 0x000fffffU;
}


/* ------------------------------------------------------------ */
/* Compare blocks                                                */
/* ------------------------------------------------------------ */

static int compare_blocks(const void *a,
                          const void *b)
{
    int x = *(const int *)a;
    int y = *(const int *)b;

    uint32_t sx = packed_seq(x);
    uint32_t sy = packed_seq(y);

    if (sx < sy)
        return -1;

    if (sx > sy)
        return 1;

    return 0;
}


/* ------------------------------------------------------------ */
/* Read allocation map                                           */
/* ------------------------------------------------------------ */

static int build_map(QLDisk *d)
{
    size_t map_bytes;
    size_t total_needed;
    uint8_t *map;
    uint8_t *tmp;
    size_t sector_count;
    size_t sec;
    uint32_t i;


    if (!size_mul_ok(
            (size_t)d->block_count,
            MAP_ENTRY_SIZE,
            &map_bytes)) {

        fprintf(stderr,
                "Allocation map demasiado grande\n");

        return 0;
    }


    if (!size_add_ok(MAP_OFFSET,
                     map_bytes,
                     &total_needed)) {

        fprintf(stderr,
                "Allocation map fuera de rango\n");

        return 0;
    }


    /*
     * El mapa comienza en 0x60 dentro del primer sector.
     */
    sector_count =
        (total_needed +
         SECTOR_SIZE - 1) /
        SECTOR_SIZE;


    if (sector_count > d->total_sectors) {

        fprintf(stderr,
                "El allocation map excede la imagen\n");

        return 0;
    }


    tmp =
        (uint8_t *)malloc(total_needed);

    if (!tmp) {

        fprintf(stderr,
                "Sin memoria para leer "
                "el allocation map\n");

        return 0;
    }


    /*
     * Leemos desde el sector lógico 0.
     *
     * Importante: esto funciona tanto para QL5A como QL5B,
     * porque read_logical_sector() se encarga de la
     * traducción correspondiente.
     */
    for (sec = 0;
         sec < sector_count;
         ++sec) {

        size_t dst =
            sec * SECTOR_SIZE;

        if (!read_logical_sector(
                d,
                (uint32_t)sec,
                tmp + dst)) {

            fprintf(stderr,
                    "No puedo leer el sector %zu "
                    "del allocation map\n",
                    sec);

            free(tmp);
            return 0;
        }
    }


    map =
        tmp + MAP_OFFSET;


    /*
     * Cada entrada ocupa tres bytes:
     *
     *     byte 0
     *     byte 1
     *     byte 2
     *
     * 12 bits de file number
     * 12 bits de group number
     *
     * El manual denomina el primer campo "(file id-1)".
     *
     * Para el mapa:
     *
     *     file_number = ((b0 << 4) | b1 >> 4)
     *
     *     group_number =
     *         ((b1 & 0x0f) << 8) | b2
     */
    for (i = 0;
         i < d->block_count;
         ++i) {

        const uint8_t *e =
            map +
            (size_t)i *
            MAP_ENTRY_SIZE;

        uint32_t file_number =
            ((uint32_t)e[0] << 4) |
            ((uint32_t)e[1] >> 4);

        uint32_t group_number =
            (((uint32_t)e[1] & 0x0fU) << 8) |
            (uint32_t)e[2];


        /*
         * Valores especiales del file number:
         *
         *   0x000..0xeff  -> ficheros
         *   0xf80..       -> información especial/libre/etc.
         *
         * No incorporamos entradas especiales.
         */
        if (file_number >= MAX_FILE_ID)
            continue;

        if (file_number >= 0xF80U)
            continue;


        /*
         * El valor almacenado en el mapa es el file number
         * utilizado por QDOS. Lo mantenemos como índice.
         */
        if (!add_file_block(
                d,
                file_number,
                group_number,
                i)) {

            fprintf(stderr,
                    "No puedo guardar la entrada "
                    "del allocation map %u\n",
                    i);

            free(tmp);
            return 0;
        }
    }


    free(tmp);

    return 1;
}


/* ------------------------------------------------------------ */
/* Read complete QDOS file                                      */
/* ------------------------------------------------------------ */

static uint8_t *read_qdos_file(const QLDisk *d,
                               uint32_t file_id,
                               size_t *out_size,
                               int include_header)
{
    const FileBlocks *fb;
    int *blocks;
    size_t capacity;
    uint8_t *result;
    size_t pos;
    size_t group_size;
    size_t i;


    if (!out_size)
        return NULL;

    *out_size = 0;


    if (file_id >= MAX_FILE_ID)
        return NULL;


    fb =
        &d->files[file_id];


    if (fb->count == 0)
        return NULL;


    if (!size_mul_ok(
            (size_t)d->allocation_size,
            SECTOR_SIZE,
            &group_size)) {

        return NULL;
    }


    if (!size_mul_ok(
            fb->count,
            group_size,
            &capacity)) {

        return NULL;
    }


    blocks =
        (int *)malloc(
            fb->count *
            sizeof(int));

    if (!blocks)
        return NULL;


    memcpy(blocks,
           fb->seq,
           fb->count *
           sizeof(int));


    qsort(blocks,
          fb->count,
          sizeof(int),
          compare_blocks);


    result =
        (uint8_t *)malloc(capacity);

    if (!result) {
        free(blocks);
        return NULL;
    }


    pos = 0;


    for (i = 0;
         i < fb->count;
         ++i) {

        uint32_t block_number =
            packed_block(blocks[i]);

        uint8_t *group =
            (uint8_t *)malloc(group_size);


        if (!group) {
            free(blocks);
            free(result);
            return NULL;
        }


        if (!read_block(
                d,
                block_number,
                group)) {

            free(group);
            free(blocks);
            free(result);

            return NULL;
        }


        memcpy(result + pos,
               group,
               group_size);

        pos += group_size;

        free(group);
    }


    free(blocks);


    /*
     * Los ficheros normales llevan 64 bytes de header.
     * El directorio (file 0) no se procesa aquí normalmente.
     */
    if (!include_header) {

        if (pos < FILE_HEADER_SIZE) {
            free(result);
            return NULL;
        }


        memmove(result,
                result + FILE_HEADER_SIZE,
                pos - FILE_HEADER_SIZE);

        pos -= FILE_HEADER_SIZE;
    }


    *out_size = pos;

    return result;
}


/* ------------------------------------------------------------ */
/* Sanitize filename                                             */
/* ------------------------------------------------------------ */

static void sanitize_filename(
    const uint8_t *src,
    size_t len,
    char *dst,
    size_t dst_size)
{
    size_t i;
    size_t p = 0;


    if (!dst ||
        dst_size == 0)
        return;


    for (i = 0;
         i < len &&
         p + 1 < dst_size;
         ++i) {

        unsigned char c =
            src[i];


        if (c == 0)
            break;


        /*
         * Impedimos:
         *
         *   /
         *   \
         *   :
         *   *
         *   ?
         *   "
         *   <
         *   >
         *   |
         *
         * para evitar que un nombre del QL cree una ruta
         * inesperada en el sistema anfitrión.
         */
        if (c == '/' ||
            c == '\\' ||
            c == ':' ||
            c == '*' ||
            c == '?' ||
            c == '"' ||
            c == '<' ||
            c == '>' ||
            c == '|') {

            dst[p++] = '_';
        }
        else if (c < 32) {
            dst[p++] = '_';
        }
        else {
            dst[p++] = (char)c;
        }
    }


    /*
     * Eliminar espacios finales.
     */
    while (p > 0 &&
           dst[p - 1] == ' ') {

        --p;
    }


    /*
     * Evitar nombres "." y "..".
     */
    if (p == 0 ||
        (p == 1 && dst[0] == '.') ||
        (p == 2 &&
         dst[0] == '.' &&
         dst[1] == '.')) {

        strcpy(dst,
               "unnamed");

        return;
    }


    dst[p] = '\0';
}


/* ------------------------------------------------------------ */
/* Parse directory                                               */
/* ------------------------------------------------------------ */

static int parse_directory(
    const QLDisk *d,
    DirEntry **out_entries,
    size_t *out_count)
{
    uint32_t eof_block;
    uint32_t eof_byte;

    size_t useful_size;
    size_t dir_size;

    uint8_t *dir;

    const FileBlocks *fb;

    int *blocks;

    size_t i;
    size_t pos;

    size_t max_entries;

    DirEntry *entries;

    size_t count = 0;


    if (!out_entries ||
        !out_count)
        return 0;


    *out_entries = NULL;
    *out_count = 0;


    eof_block =
        d->directory_eof >> 16;

    eof_byte =
        d->directory_eof & 0xffffU;


    /*
     * El byte offset debe estar dentro de un grupo.
     *
     * El manual define el byte como 0..1ff.
     */
    if (eof_byte >= SECTOR_SIZE) {

        fprintf(stderr,
                "Directory EOF inválido: "
                "byte=%u\n",
                eof_byte);

        return 0;
    }


    /*
     * EOF = block / byte.
     *
     * El byte apunta al siguiente byte después del final.
     *
     * Para leer hasta ese punto necesitamos:
     *
     *     eof_block * group_size + eof_byte
     */
    {
        size_t group_size;

        if (!size_mul_ok(
                d->allocation_size,
                SECTOR_SIZE,
                &group_size)) {

            return 0;
        }


        if (!size_mul_ok(
                (size_t)eof_block,
                group_size,
                &useful_size)) {

            return 0;
        }


        if (!size_add_ok(
                useful_size,
                eof_byte,
                &useful_size)) {

            return 0;
        }
    }


    /*
     * Si EOF es cero, no hay directorio.
     */
    if (useful_size == 0) {

        fprintf(stderr,
                "Directorio vacío\n");

        return 0;
    }


    /*
     * Redondeamos al tamaño de grupo para leer el último
     * grupo completo.
     */
    {
        size_t group_size;
        size_t groups_needed;

        if (!size_mul_ok(
                d->allocation_size,
                SECTOR_SIZE,
                &group_size)) {

            return 0;
        }


        groups_needed =
            (useful_size +
             group_size - 1) /
            group_size;


        if (!size_mul_ok(
                groups_needed,
                group_size,
                &dir_size)) {

            return 0;
        }
    }


    dir =
        (uint8_t *)calloc(
            1,
            dir_size);

    if (!dir)
        return 0;


    /*
     * File 0 = directorio.
     */
    fb =
        &d->files[0];


    if (fb->count == 0) {

        fprintf(stderr,
                "No encuentro el directorio "
                "(file 0)\n");

        free(dir);
        return 0;
    }


    blocks =
        (int *)malloc(
            fb->count *
            sizeof(int));

    if (!blocks) {
        free(dir);
        return 0;
    }


    memcpy(blocks,
           fb->seq,
           fb->count *
           sizeof(int));


    qsort(blocks,
          fb->count,
          sizeof(int),
          compare_blocks);


    pos = 0;


    {
        size_t group_size;

        if (!size_mul_ok(
                d->allocation_size,
                SECTOR_SIZE,
                &group_size)) {

            free(blocks);
            free(dir);
            return 0;
        }


        for (i = 0;
             i < fb->count &&
             pos < dir_size;
             ++i) {

            uint32_t bn =
                packed_block(blocks[i]);

            uint8_t *group =
                (uint8_t *)malloc(
                    group_size);

            size_t copy;


            if (!group) {
                free(blocks);
                free(dir);
                return 0;
            }


            if (!read_block(
                    d,
                    bn,
                    group)) {

                free(group);
                free(blocks);
                free(dir);

                return 0;
            }


            copy =
                dir_size - pos;

            if (copy > group_size)
                copy = group_size;


            memcpy(dir + pos,
                   group,
                   copy);

            pos += copy;


            free(group);
        }
    }


    free(blocks);


    /*
     * No necesitamos conservar bytes después del EOF.
     */
    if (useful_size < dir_size)
        dir_size = useful_size;


    max_entries =
        dir_size / DIR_ENTRY_SIZE;


    if (max_entries == 0) {

        free(dir);
        return 0;
    }


    if (max_entries >
        SIZE_MAX / sizeof(DirEntry)) {

        free(dir);
        return 0;
    }


    entries =
        (DirEntry *)calloc(
            max_entries,
            sizeof(DirEntry));

    if (!entries) {

        free(dir);
        return 0;
    }


    for (i = 0;
         i < max_entries;
         ++i) {

        const uint8_t *e =
            dir +
            i * DIR_ENTRY_SIZE;

        uint32_t length =
            be32(e + 0);

        uint16_t name_length =
            be16(e + 0x0e);


        /*
         * Registro vacío.
         */
        if (length == 0 ||
            name_length == 0)
            continue;


        /*
         * El formato clásico reserva hasta 24 bytes
         * para el nombre en el header.
         */
        if (name_length >
            24)
            name_length = 24;


        entries[count].length =
            length;

        entries[count].name_length =
            name_length;


        sanitize_filename(
            e + 0x10,
            name_length,
            entries[count].name,
            sizeof(entries[count].name));


        entries[count].date =
            be32(e + 0x34);


        ++count;
    }


    free(dir);


    *out_entries =
        entries;

    *out_count =
        count;


    return 1;
}


/* ------------------------------------------------------------ */
/* Make output directory                                         */
/* ------------------------------------------------------------ */

static int make_output_directory(
    const char *path)
{
    if (MKDIR(path) == 0)
        return 1;


    if (errno == EEXIST)
        return 1;


    fprintf(stderr,
            "No puedo crear directorio %s: %s\n",
            path,
            strerror(errno));


    return 0;
}


/* ------------------------------------------------------------ */
/* Build output path                                              */
/* ------------------------------------------------------------ */

static int build_output_path(
    const char *outdir,
    const char *filename,
    char *path,
    size_t path_size)
{
    int n;


    if (!outdir ||
        !filename ||
        !path ||
        path_size == 0)
        return 0;


    n =
        snprintf(path,
                 path_size,
                 "%s%s%s",
                 outdir,
                 PATH_SEP,
                 filename);


    if (n < 0 ||
        (size_t)n >= path_size) {

        fprintf(stderr,
                "Ruta de salida demasiado larga:\n"
                "  %s%s%s\n",
                outdir,
                PATH_SEP,
                filename);

        return 0;
    }


    return 1;
}


/* ------------------------------------------------------------ */
/* Extract one file                                              */
/* ------------------------------------------------------------ */

static int extract_file(
    const QLDisk *d,
    uint32_t file_id,
    const DirEntry *entry,
    const char *outdir)
{
    size_t size;
    uint8_t *data;

    FILE *f;

    char path[MAX_OUTPUT_PATH];


    data =
        read_qdos_file(
            d,
            file_id,
            &size,
            0);


    if (!data) {

        fprintf(stderr,
                "  ERROR leyendo fichero %u (%s)\n",
                file_id,
                entry->name);

        return 0;
    }


    /*
     * El tamaño real declarado por el directorio limita
     * el número de bytes que exportamos.
     */
    if ((size_t)entry->length < size)
        size = entry->length;


    if (!build_output_path(
            outdir,
            entry->name,
            path,
            sizeof(path))) {

        free(data);
        return 0;
    }


    f =
        fopen(path, "wb");


    if (!f) {

        fprintf(stderr,
                "  ERROR creando %s: %s\n",
                path,
                strerror(errno));

        free(data);

        return 0;
    }


    if (size > 0 &&
        fwrite(data,
               1,
               size,
               f) != size) {

        fprintf(stderr,
                "  ERROR escribiendo %s\n",
                path);

        fclose(f);
        free(data);

        return 0;
    }


    if (fclose(f) != 0) {

        fprintf(stderr,
                "  ERROR cerrando %s\n",
                path);

        free(data);

        return 0;
    }


    free(data);


    printf("  %-30s %8zu bytes\n",
           entry->name,
           size);


    return 1;
}


/* ------------------------------------------------------------ */
/* Find directory entry for file                                */
/* ------------------------------------------------------------ */

static int find_directory_entry(
    const QLDisk *d,
    uint32_t file_id,
    const DirEntry *entries,
    size_t entry_count,
    size_t *entry_index)
{
    size_t file_size;
    uint8_t *file_data;

    uint32_t file_length;

    uint16_t name_length;

    char file_name[MAX_FILENAME_LENGTH];

    size_t i;


    file_data =
        read_qdos_file(
            d,
            file_id,
            &file_size,
            1);


    if (!file_data)
        return 0;


    if (file_size < FILE_HEADER_SIZE) {

        free(file_data);
        return 0;
    }


    /*
     * El primer long del header es la longitud del fichero.
     */
    file_length =
        be32(file_data);


    name_length =
        be16(file_data + 0x0e);


    if (name_length > 24)
        name_length = 24;


    sanitize_filename(
        file_data + 0x10,
        name_length,
        file_name,
        sizeof(file_name));


    /*
     * Primero intentamos nombre + longitud.
     */
    for (i = 0;
         i < entry_count;
         ++i) {

        if (entries[i].length != file_length)
            continue;


        if (strcmp(entries[i].name,
                   file_name) == 0) {

            *entry_index = i;

            free(file_data);

            return 1;
        }
    }


    /*
     * Si no coincide, intentamos solamente por nombre.
     *
     * Esto permite recuperar imágenes donde el campo de
     * longitud del header y el de directorio no coinciden
     * debido a herramientas antiguas.
     */
    for (i = 0;
         i < entry_count;
         ++i) {

        if (strcmp(entries[i].name,
                   file_name) == 0) {

            *entry_index = i;

            free(file_data);

            return 1;
        }
    }


    free(file_data);

    return 0;
}


/* ------------------------------------------------------------ */
/* Free disk                                                      */
/* ------------------------------------------------------------ */

static void free_disk(QLDisk *d)
{
    uint32_t i;


    for (i = 0;
         i < MAX_FILE_ID;
         ++i) {

        free(d->files[i].seq);

        d->files[i].seq = NULL;
        d->files[i].count = 0;
        d->files[i].capacity = 0;
    }


    free(d->image);

    d->image = NULL;
    d->image_size = 0;
}


/* ------------------------------------------------------------ */
/* Usage                                                          */
/* ------------------------------------------------------------ */

static void print_usage(
    const char *program)
{
    fprintf(stderr,
            "Uso:\n"
            "  %s [-l] imagen.img [directorio]\n\n"
            "Opciones:\n"
            "  -l    solo listar\n\n"
            "Ejemplos:\n"
            "  %s disco.img\n"
            "  %s disco.img salida\n"
            "  %s -l disco.img\n",
            program,
            program,
            program,
            program);
}


/* ------------------------------------------------------------ */
/* Main                                                           */
/* ------------------------------------------------------------ */

int main_ql_extract(int argc,
         char **argv)
{
    QLDisk disk;

    const char *image_name;
    const char *outdir =
        "ql_extract";

    int list_only = 0;

    DirEntry *entries = NULL;
    size_t entry_count = 0;

    size_t i;


    memset(&disk,
           0,
           sizeof(disk));


    if (argc < 2) {

        print_usage(argv[0]);

        return EXIT_FAILURE;
    }


    /*
     * -l imagen [directorio]
     */
    if (strcmp(argv[1], "-l") == 0) {

        list_only = 1;


        if (argc < 3) {

            fprintf(stderr,
                    "Falta la imagen IMG\n");

            return EXIT_FAILURE;
        }


        image_name =
            argv[2];


        if (argc >= 4)
            outdir =
                argv[3];
    }
    else {

        image_name =
            argv[1];


        if (argc >= 3)
            outdir =
                argv[2];
    }


    /*
     * Cargar imagen.
     */
    if (!load_image(
            image_name,
            &disk)) {

        return EXIT_FAILURE;
    }


    /*
     * El extractor espera una imagen donde el sector lógico
     * 0 sea el primer sector de 512 bytes.
     *
     * Buscamos el header para proporcionar un diagnóstico
     * útil si no lo encontramos ahí.
     */
    if (disk.image_size < SECTOR_SIZE ||
        (memcmp(disk.image,
                "QL5A",
                4) != 0 &&
         memcmp(disk.image,
                "QL5B",
                4) != 0)) {

        size_t header_offset;


        if (!find_qdos_header(
                &disk,
                &header_offset)) {

            fprintf(stderr,
                    "No encuentro una cabecera "
                    "QL5A/QL5B\n");

            free_disk(&disk);

            return EXIT_FAILURE;
        }


        if (header_offset != 0) {

            fprintf(stderr,
                    "Encuentro una cabecera QL5%c "
                    "en offset 0x%zx,\n"
                    "pero esta versión espera que "
                    "el sector lógico 0\n"
                    "comience en el offset 0.\n",
                    disk.image[header_offset + 3] == 'A'
                        ? 'A'
                        : 'B',
                    header_offset);

            fprintf(stderr,
                    "La imagen parece ser un dump "
                    "físico con un layout\n"
                    "que requiere una capa adicional "
                    "de traducción.\n");

            free_disk(&disk);

            return EXIT_FAILURE;
        }
    }


    /*
     * Cabecera.
     */
    if (!parse_header(
            &disk)) {

        free_disk(&disk);

        return EXIT_FAILURE;
    }


    print_disk_info(
        &disk);


    /*
     * Allocation map.
     */
    if (!build_map(
            &disk)) {

        fprintf(stderr,
                "No puedo interpretar el "
                "allocation map\n");

        free_disk(&disk);

        return EXIT_FAILURE;
    }


    /*
     * Ordenamos las listas de grupos.
     */
    for (i = 0;
         i < MAX_FILE_ID;
         ++i) {

        if (disk.files[i].count > 1) {

            qsort(
                disk.files[i].seq,
                disk.files[i].count,
                sizeof(int),
                compare_blocks);
        }
    }


    /*
     * Directorio.
     */
    if (!parse_directory(
            &disk,
            &entries,
            &entry_count)) {

        fprintf(stderr,
                "No puedo leer el "
                "directorio QDOS\n");

        free_disk(&disk);

        return EXIT_FAILURE;
    }


    printf("\nArchivos: %zu\n\n",
           entry_count);


    for (i = 0;
         i < entry_count;
         ++i) {

        printf("%4zu  %-30s %8u bytes\n",
               i + 1,
               entries[i].name,
               entries[i].length);
    }


    /*
     * Solo listar.
     */
    if (list_only) {

        free(entries);
        free_disk(&disk);

        return EXIT_SUCCESS;
    }


    /*
     * Crear directorio de salida.
     */
    printf("\nExtrayendo a: %s\n\n",
           outdir);


    if (!make_output_directory(
            outdir)) {

        free(entries);
        free_disk(&disk);

        return EXIT_FAILURE;
    }


    /*
     * Recorremos los file IDs.
     *
     * El file ID no tiene por qué coincidir con el índice
     * de la entrada que mostramos al usuario.
     */
    {
        uint32_t file_id;

        for (file_id = 1;
             file_id < MAX_FILE_ID;
             ++file_id) {

            FileBlocks *fb =
                &disk.files[file_id];

            size_t entry_index;


            if (fb->count == 0)
                continue;


            /*
             * Solamente consideramos ficheros cuyo primer
             * grupo sea el grupo 0.
             */
            {
                size_t j;
                int has_zero = 0;

                for (j = 0;
                     j < fb->count;
                     ++j) {

                    if (packed_seq(
                            fb->seq[j]) == 0) {

                        has_zero = 1;
                        break;
                    }
                }


                if (!has_zero)
                    continue;
            }


            /*
             * Buscar el nombre correspondiente en el
             * directorio.
             */
            if (!find_directory_entry(
                    &disk,
                    file_id,
                    entries,
                    entry_count,
                    &entry_index)) {

                /*
                 * No abortamos toda la extracción por un
                 * fichero que no podamos asociar.
                 */
                fprintf(stderr,
                        "  Aviso: no puedo asociar "
                        "file ID %u con una entrada "
                        "del directorio\n",
                        file_id);

                continue;
            }


            extract_file(
                &disk,
                file_id,
                &entries[entry_index],
                outdir);
        }
    }


    free(entries);
    free_disk(&disk);


    return EXIT_SUCCESS;
}

