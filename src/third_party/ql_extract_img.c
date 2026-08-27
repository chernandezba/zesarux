/*

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

 */

/*
 * ql_extract_img.c
 *
 * Extractor de imagenes de floppy Sinclair QL / QDOS.
 *
 * Soporta imagenes QL5A y QL5B de sectores de 512 bytes.
 *
 * Entrada para uso desde otro programa:
 *
 *   int main_ql_extract_img(int argc, char **argv);
 *
 * Uso:
 *
 *   ql_extract_img imagen.img
 *   ql_extract_img imagen.img directorio_salida
 *   ql_extract_img -l imagen.img
 *   ql_extract_img -v imagen.img
 *   ql_extract_img -z imagen.img
 *
 * Opciones:
 *
 *   -l   solo listar
 *   -v   diagnostico del allocation map
 *   -z   extraer tambien entradas con 0 bytes de datos
 *
 * Notas QDOS:
 *
 * - sectores de 512 bytes
 * - QL5A contiene tablas logico <-> fisico
 * - allocation map comienza en offset logico 0x60
 * - cada entrada del mapa ocupa 3 bytes:
 *
 *       12 bits: file ID
 *       12 bits: sequence
 *
 * - file ID 000 = master directory
 * - entrada N del master directory = file ID N
 * - cada entrada del directorio ocupa 64 bytes
 * - los ficheros normales contienen 64 bytes iniciales de header
 * - la longitud almacenada incluye esos 64 bytes
 * - Directory EOF usa bloques de 512 bytes, NO allocation groups
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include "ql_extract_img.h"

#ifdef _WIN32

#include <direct.h>

#define MKDIR(path) _mkdir(path)
#define PATH_SEPARATOR '\\'

#else

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define MKDIR(path) mkdir(path, 0755)
#define PATH_SEPARATOR '/'

#endif


#define SECTOR_SIZE          512
#define FILE_HEADER_SIZE      64

#define MAP_OFFSET          0x60
#define MAP_ENTRY_SIZE         3

#define MAX_FILE_ID        0x1000
#define FILE_SPECIAL_MIN   0x0f80


typedef struct
{
    uint16_t sequence;
    uint16_t disk_group;

} AllocationEntry;


typedef struct
{
    AllocationEntry *entry;

    size_t count;
    size_t capacity;

} FileAllocation;


typedef struct
{
    uint8_t *image;
    size_t image_size;

    char format[5];
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

    uint32_t group_count;

    FileAllocation file[MAX_FILE_ID];

    int verbose;

} QLDisk;


typedef struct
{
    uint16_t file_id;

    uint32_t qdos_length;

    uint16_t name_length;

    char name[64];

    uint32_t modification_date;

} DirectoryEntry;


/* --------------------------------------------------------- */
/* Big endian                                                */
/* --------------------------------------------------------- */

static uint16_t
read_be16(const uint8_t *p)
{
    return
        ((uint16_t)p[0] << 8) |
        ((uint16_t)p[1]);
}


static uint32_t
read_be32(const uint8_t *p)
{
    return
        ((uint32_t)p[0] << 24) |
        ((uint32_t)p[1] << 16) |
        ((uint32_t)p[2] << 8) |
        ((uint32_t)p[3]);
}


/* --------------------------------------------------------- */
/* Load image                                                */
/* --------------------------------------------------------- */

static int
load_image(QLDisk *disk, const char *filename)
{
    FILE *f;
    long length;

    f = fopen(filename, "rb");

    if (f == NULL)
    {
        fprintf(stderr,
                "ERROR: no puedo abrir '%s': %s\n",
                filename,
                strerror(errno));

        return 0;
    }

    if (fseek(f, 0, SEEK_END) != 0)
    {
        fclose(f);
        return 0;
    }

    length = ftell(f);

    if (length < 0)
    {
        fclose(f);
        return 0;
    }

    if (fseek(f, 0, SEEK_SET) != 0)
    {
        fclose(f);
        return 0;
    }

    disk->image_size = (size_t)length;

    disk->image =
        (uint8_t *)malloc(disk->image_size);

    if (disk->image == NULL)
    {
        fprintf(stderr,
                "ERROR: sin memoria para %zu bytes\n",
                disk->image_size);

        fclose(f);
        return 0;
    }

    if (fread(disk->image,
              1,
              disk->image_size,
              f) != disk->image_size)
    {
        fprintf(stderr,
                "ERROR leyendo '%s'\n",
                filename);

        free(disk->image);
        disk->image = NULL;

        fclose(f);

        return 0;
    }

    fclose(f);

    return 1;
}


/* --------------------------------------------------------- */
/* Header QDOS                                               */
/* --------------------------------------------------------- */

static int
parse_header(QLDisk *disk)
{
    const uint8_t *h;

    if (disk->image_size < SECTOR_SIZE)
    {
        fprintf(stderr,
                "ERROR: imagen demasiado pequena\n");

        return 0;
    }

    h = disk->image;

    if (memcmp(h, "QL5A", 4) != 0 &&
        memcmp(h, "QL5B", 4) != 0)
    {
        fprintf(stderr,
                "ERROR: no encuentro QL5A/QL5B "
                "al principio de la imagen\n");

        return 0;
    }

    memcpy(disk->format, h, 4);
    disk->format[4] = '\0';

    memcpy(disk->medium_name,
           h + 4,
           10);

    disk->medium_name[10] = '\0';

    disk->free_sectors =
        read_be16(h + 0x14);

    disk->good_sectors =
        read_be16(h + 0x16);

    disk->total_sectors =
        read_be16(h + 0x18);

    disk->sectors_per_track =
        read_be16(h + 0x1a);

    disk->sectors_per_cylinder =
        read_be16(h + 0x1c);

    disk->tracks =
        read_be16(h + 0x1e);

    disk->allocation_size =
        read_be16(h + 0x20);

    disk->directory_eof =
        read_be32(h + 0x22);

    disk->sector_offset =
        read_be16(h + 0x26);

    memcpy(disk->logical_to_physical,
           h + 0x28,
           sizeof(disk->logical_to_physical));

    memcpy(disk->physical_to_logical,
           h + 0x3a,
           sizeof(disk->physical_to_logical));

    if (disk->total_sectors == 0 ||
        disk->allocation_size == 0 ||
        disk->sectors_per_track == 0 ||
        disk->sectors_per_cylinder == 0)
    {
        fprintf(stderr,
                "ERROR: geometria QDOS invalida\n");

        return 0;
    }

    if (disk->sectors_per_cylinder > 18)
    {
        fprintf(stderr,
                "ERROR: sectors/cylinder=%u; "
                "este extractor admite hasta 18\n",
                disk->sectors_per_cylinder);

        return 0;
    }

    if (disk->sectors_per_cylinder %
        disk->sectors_per_track != 0)
    {
        fprintf(stderr,
                "ERROR: geometria inconsistente\n");

        return 0;
    }

    if ((uint64_t)disk->total_sectors *
            SECTOR_SIZE >
        disk->image_size)
    {
        fprintf(stderr,
                "ERROR: header indica %u sectores "
                "(%u bytes), pero la imagen tiene %zu\n",
                disk->total_sectors,
                disk->total_sectors * SECTOR_SIZE,
                disk->image_size);

        return 0;
    }

    disk->group_count =
        disk->total_sectors /
        disk->allocation_size;

    if (disk->group_count == 0 ||
        disk->group_count > 65535)
    {
        fprintf(stderr,
                "ERROR: numero de grupos invalido\n");

        return 0;
    }

    return 1;
}


/* --------------------------------------------------------- */
/* Información del disco                                     */
/* --------------------------------------------------------- */

static void
print_disk_info(const QLDisk *disk)
{
    char name[11];
    unsigned i;

    memcpy(name,
           disk->medium_name,
           sizeof(name));

    name[10] = '\0';

    for (i = 10; i > 0; --i)
    {
        if (name[i - 1] == ' ' ||
            name[i - 1] == '\0')
        {
            name[i - 1] = '\0';
        }
        else
        {
            break;
        }
    }

    printf("\n");

    printf("Formato             : %s\n",
           disk->format);

    printf("Nombre medio        : \"%s\"\n",
           name);

    printf("Sectores libres     : %u\n",
           disk->free_sectors);

    printf("Sectores buenos     : %u\n",
           disk->good_sectors);

    printf("Sectores totales    : %u\n",
           disk->total_sectors);

    printf("Sectores/pista      : %u\n",
           disk->sectors_per_track);

    printf("Sectores/cilindro   : %u\n",
           disk->sectors_per_cylinder);

    printf("Pistas              : %u\n",
           disk->tracks);

    printf("Allocation size     : %u sectores\n",
           disk->allocation_size);

    printf("Grupos              : %u\n",
           disk->group_count);

    printf("Directory EOF       : block=%u byte=%u\n",
           (unsigned)(disk->directory_eof >> 16),
           (unsigned)(disk->directory_eof & 0xffff));

    printf("Sector offset       : %u\n",
           disk->sector_offset);

    {
        uint32_t block =
            disk->directory_eof >> 16;

        uint32_t byte =
            disk->directory_eof & 0xffff;

        uint64_t bytes =
            (uint64_t)block *
            SECTOR_SIZE +
            byte;

        printf("Directory bytes     : %llu\n",
               (unsigned long long)bytes);
    }

    if (strcmp(disk->format,
               "QL5A") == 0)
    {
        printf("\nLogical -> physical:\n");

        for (i = 0;
             i < disk->sectors_per_cylinder;
             ++i)
        {
            uint8_t t =
                disk->logical_to_physical[i];

            unsigned side =
                (t & 0x80) != 0;

            unsigned sector =
                t & 0x7f;

            printf("%2u:%u/%u  ",
                   i,
                   side,
                   sector);

            if ((i % 6) == 5)
                printf("\n");
        }

        if ((i % 6) != 0)
            printf("\n");
    }

    printf("\n");
}


/* --------------------------------------------------------- */
/* Sector lógico -> offset IMG                               */
/* --------------------------------------------------------- */

static int
logical_sector_to_offset(
        const QLDisk *disk,
        uint32_t logical_sector,
        size_t *image_offset)
{
    uint64_t physical_lba;

    if (logical_sector >=
        disk->total_sectors)
    {
        return 0;
    }

    if (strcmp(disk->format,
               "QL5B") == 0)
    {
        physical_lba =
            logical_sector;
    }
    else
    {
        uint32_t track;
        uint32_t within_cylinder;
        uint8_t translated;
        uint32_t side;
        uint32_t sector;
        uint32_t physical_sector;
        uint32_t sides;

        track =
            logical_sector /
            disk->sectors_per_cylinder;

        within_cylinder =
            logical_sector %
            disk->sectors_per_cylinder;

        translated =
            disk->logical_to_physical[
                within_cylinder];

        side =
            (translated & 0x80) ?
            1U :
            0U;

        sector =
            translated & 0x7f;

        sides =
            disk->sectors_per_cylinder /
            disk->sectors_per_track;

        if (side >= sides ||
            sector >= disk->sectors_per_track)
        {
            return 0;
        }

        physical_sector =
            (sector +
             track * disk->sector_offset) %
            disk->sectors_per_track;

        physical_lba =
            (uint64_t)track *
            disk->sectors_per_cylinder;

        physical_lba +=
            (uint64_t)side *
            disk->sectors_per_track;

        physical_lba +=
            physical_sector;
    }

    if ((physical_lba + 1) *
            SECTOR_SIZE >
        disk->image_size)
    {
        return 0;
    }

    *image_offset =
        (size_t)(
            physical_lba *
            SECTOR_SIZE);

    return 1;
}


/* --------------------------------------------------------- */
/* Leer sector lógico                                        */
/* --------------------------------------------------------- */

static int
read_logical_sector(
        const QLDisk *disk,
        uint32_t logical_sector,
        uint8_t *buffer)
{
    size_t offset;

    if (!logical_sector_to_offset(
            disk,
            logical_sector,
            &offset))
    {
        return 0;
    }

    memcpy(buffer,
           disk->image + offset,
           SECTOR_SIZE);

    return 1;
}


/* --------------------------------------------------------- */
/* Leer bytes del espacio lógico del disco                    */
/* --------------------------------------------------------- */

static int
read_logical_bytes(
        const QLDisk *disk,
        uint32_t byte_offset,
        uint8_t *output,
        size_t length)
{
    uint8_t sector[SECTOR_SIZE];

    while (length > 0)
    {
        uint32_t sector_number =
            byte_offset /
            SECTOR_SIZE;

        uint32_t sector_offset =
            byte_offset %
            SECTOR_SIZE;

        size_t count =
            SECTOR_SIZE -
            sector_offset;

        if (count > length)
            count = length;

        if (!read_logical_sector(
                disk,
                sector_number,
                sector))
        {
            return 0;
        }

        memcpy(output,
               sector + sector_offset,
               count);

        output += count;

        byte_offset +=
            (uint32_t)count;

        length -= count;
    }

    return 1;
}


/* --------------------------------------------------------- */
/* Leer allocation group                                     */
/* --------------------------------------------------------- */

static int
read_allocation_group(
        const QLDisk *disk,
        uint16_t group_number,
        uint8_t *buffer)
{
    uint32_t first_sector;
    unsigned i;

    if (group_number >=
        disk->group_count)
    {
        return 0;
    }

    first_sector =
        (uint32_t)group_number *
        disk->allocation_size;

    for (i = 0;
         i < disk->allocation_size;
         ++i)
    {
        if (!read_logical_sector(
                disk,
                first_sector + i,
                buffer +
                    i * SECTOR_SIZE))
        {
            return 0;
        }
    }

    return 1;
}


/* --------------------------------------------------------- */
/* Allocation list                                           */
/* --------------------------------------------------------- */

static int
add_allocation(
        QLDisk *disk,
        uint16_t file_id,
        uint16_t sequence,
        uint16_t disk_group)
{
    FileAllocation *f;

    if (file_id >= MAX_FILE_ID)
        return 0;

    f =
        &disk->file[file_id];

    for (size_t i = 0;
         i < f->count;
         ++i)
    {
        if (f->entry[i].sequence ==
            sequence)
        {
            fprintf(stderr,
                    "WARNING: file %03X "
                    "sequence %u duplicada "
                    "(groups %u y %u)\n",
                    file_id,
                    sequence,
                    f->entry[i].disk_group,
                    disk_group);

            return 1;
        }
    }

    if (f->count ==
        f->capacity)
    {
        size_t new_capacity =
            f->capacity ?
            f->capacity * 2 :
            8;

        AllocationEntry *p =
            (AllocationEntry *)realloc(
                f->entry,
                new_capacity *
                sizeof(*p));

        if (p == NULL)
            return 0;

        f->entry =
            p;

        f->capacity =
            new_capacity;
    }

    f->entry[f->count].sequence =
        sequence;

    f->entry[f->count].disk_group =
        disk_group;

    f->count++;

    return 1;
}


/* --------------------------------------------------------- */
/* Buscar grupo por file ID + sequence                        */
/* --------------------------------------------------------- */

static int
find_allocation_group(
        const QLDisk *disk,
        uint16_t file_id,
        uint16_t sequence,
        uint16_t *group)
{
    const FileAllocation *f;

    if (file_id >= MAX_FILE_ID)
        return 0;

    f =
        &disk->file[file_id];

    for (size_t i = 0;
         i < f->count;
         ++i)
    {
        if (f->entry[i].sequence ==
            sequence)
        {
            *group =
                f->entry[i].disk_group;

            return 1;
        }
    }

    return 0;
}


/* --------------------------------------------------------- */
/* Tipo especial del allocation map                           */
/* --------------------------------------------------------- */

static const char *
special_group_name(uint16_t file_id)
{
    if (file_id >= 0xf80 &&
        file_id <= 0xf8f)
    {
        return "MAP";
    }

    if (file_id >= 0xfd0 &&
        file_id <= 0xfdf)
    {
        return "FREE";
    }

    if (file_id >= 0xfe0 &&
        file_id <= 0xfef)
    {
        return "BAD";
    }

    if (file_id >= 0xff0 &&
        file_id <= 0xfff)
    {
        return "NONEXIST";
    }

    return "SPECIAL";
}


/* --------------------------------------------------------- */
/* Construir allocation map                                   */
/* --------------------------------------------------------- */

static int
build_allocation_map(QLDisk *disk)
{
    size_t map_bytes;
    uint8_t *map;

    map_bytes =
        (size_t)disk->group_count *
        MAP_ENTRY_SIZE;

    map =
        (uint8_t *)malloc(map_bytes);

    if (map == NULL)
    {
        fprintf(stderr,
                "ERROR: sin memoria para allocation map\n");

        return 0;
    }

    if (!read_logical_bytes(
            disk,
            MAP_OFFSET,
            map,
            map_bytes))
    {
        fprintf(stderr,
                "ERROR: no puedo leer allocation map\n");

        free(map);

        return 0;
    }

    if (disk->verbose)
    {
        printf("Allocation map:\n\n");
    }

    for (uint32_t disk_group = 0;
         disk_group < disk->group_count;
         ++disk_group)
    {
        const uint8_t *p =
            map +
            disk_group *
            MAP_ENTRY_SIZE;

        uint16_t file_id =
            ((uint16_t)p[0] << 4) |
            ((uint16_t)p[1] >> 4);

        uint16_t sequence =
            ((uint16_t)(p[1] & 0x0f) << 8) |
            (uint16_t)p[2];

        if (file_id <
            FILE_SPECIAL_MIN)
        {
            if (!add_allocation(
                    disk,
                    file_id,
                    sequence,
                    (uint16_t)disk_group))
            {
                fprintf(stderr,
                        "ERROR creando allocation map\n");

                free(map);

                return 0;
            }

            if (disk->verbose)
            {
                printf("  group=%03u "
                       "file=%03X "
                       "seq=%03X\n",
                       disk_group,
                       file_id,
                       sequence);
            }
        }
        else
        {
            if (disk->verbose)
            {
                printf("  group=%03u "
                       "%s=%03X "
                       "seq=%03X\n",
                       disk_group,
                       special_group_name(file_id),
                       file_id,
                       sequence);
            }
        }
    }

    free(map);

    return 1;
}


/* --------------------------------------------------------- */
/* Tamaño del directorio                                     */
/* --------------------------------------------------------- */

static int
get_directory_size(
        const QLDisk *disk,
        size_t *directory_size)
{
    uint32_t block;
    uint32_t byte;
    uint64_t size;

    block =
        disk->directory_eof >>
        16;

    byte =
        disk->directory_eof &
        0xffff;

    /*
     * Directory EOF:
     *
     *     size = block * 512 + byte
     *
     * byte puede ser exactamente 512.
     *
     * Por ejemplo:
     *
     *     block=0 byte=512
     *
     * equivale a:
     *
     *     block=1 byte=0
     */
    if (byte > SECTOR_SIZE)
    {
        fprintf(stderr,
                "ERROR: Directory EOF byte=%u "
                "(debe ser <=512)\n",
                byte);

        return 0;
    }

    size =
        (uint64_t)block *
        SECTOR_SIZE +
        byte;

    if (size > SIZE_MAX)
        return 0;

    *directory_size =
        (size_t)size;

    return 1;
}


/* --------------------------------------------------------- */
/* Reconstruir un file ID                                     */
/* --------------------------------------------------------- */

static uint8_t *
reconstruct_file(
        const QLDisk *disk,
        uint16_t file_id,
        size_t qdos_length)
{
    size_t allocation_bytes;
    size_t groups_needed;
    size_t allocated_bytes;
    uint8_t *data;

    allocation_bytes =
        (size_t)disk->allocation_size *
        SECTOR_SIZE;

    if (allocation_bytes == 0)
        return NULL;

    groups_needed =
        (qdos_length +
         allocation_bytes - 1) /
        allocation_bytes;

    if (groups_needed == 0)
    {
        return
            (uint8_t *)calloc(1, 1);
    }

    if (groups_needed >
        SIZE_MAX /
        allocation_bytes)
    {
        return NULL;
    }

    allocated_bytes =
        groups_needed *
        allocation_bytes;

    data =
        (uint8_t *)malloc(
            allocated_bytes);

    if (data == NULL)
        return NULL;

    for (size_t sequence = 0;
         sequence < groups_needed;
         ++sequence)
    {
        uint16_t disk_group;

        if (sequence > 0xfff)
        {
            free(data);
            return NULL;
        }

        if (!find_allocation_group(
                disk,
                file_id,
                (uint16_t)sequence,
                &disk_group))
        {
            fprintf(stderr,
                    "ERROR: file %03X: "
                    "falta sequence %zu\n",
                    file_id,
                    sequence);

            free(data);

            return NULL;
        }

        if (!read_allocation_group(
                disk,
                disk_group,
                data +
                    sequence *
                    allocation_bytes))
        {
            fprintf(stderr,
                    "ERROR: file %03X: "
                    "no puedo leer group %u\n",
                    file_id,
                    disk_group);

            free(data);

            return NULL;
        }
    }

    return data;
}


/* --------------------------------------------------------- */
/* Nombre QDOS -> nombre host                                 */
/* --------------------------------------------------------- */

static void
sanitize_filename(
        const uint8_t *source,
        size_t source_length,
        char *destination,
        size_t destination_size)
{
    size_t out = 0;

    if (destination_size == 0)
        return;

    for (size_t i = 0;
         i < source_length &&
         out + 1 < destination_size;
         ++i)
    {
        unsigned char c =
            source[i];

        if (c == 0)
            break;

        if (c == '/'  ||
            c == '\\' ||
            c == ':'  ||
            c == '*'  ||
            c == '?'  ||
            c == '"'  ||
            c == '<'  ||
            c == '>'  ||
            c == '|')
        {
            destination[out++] =
                '_';
        }
        else if (c < 32 ||
                 c == 127)
        {
            destination[out++] =
                '_';
        }
        else
        {
            destination[out++] =
                (char)c;
        }
    }

    while (out > 0 &&
           (destination[out - 1] == ' ' ||
            destination[out - 1] == '.'))
    {
        --out;
    }

    if (out == 0)
    {
        strncpy(destination,
                "unnamed",
                destination_size - 1);

        destination[
            destination_size - 1] =
            '\0';
    }
    else
    {
        destination[out] =
            '\0';
    }
}


/* --------------------------------------------------------- */
/* Leer master directory                                     */
/* --------------------------------------------------------- */

static int
read_master_directory(
        const QLDisk *disk,
        DirectoryEntry **result,
        size_t *result_count)
{
    size_t directory_size;
    size_t record_count;
    uint8_t *raw;
    DirectoryEntry *directory;
    size_t valid_count = 0;

    if (!get_directory_size(
            disk,
            &directory_size))
    {
        return 0;
    }

    if (directory_size == 0)
    {
        fprintf(stderr,
                "ERROR: directorio de tamano cero\n");

        return 0;
    }

    if (directory_size %
        FILE_HEADER_SIZE != 0)
    {
        fprintf(stderr,
                "WARNING: directorio tiene %zu bytes, "
                "no es multiplo de 64\n",
                directory_size);
    }

    record_count =
        directory_size /
        FILE_HEADER_SIZE;

    raw =
        reconstruct_file(
            disk,
            0,
            directory_size);

    if (raw == NULL)
    {
        fprintf(stderr,
                "ERROR: no puedo reconstruir "
                "master directory\n");

        return 0;
    }

    directory =
        (DirectoryEntry *)calloc(
            record_count,
            sizeof(*directory));

    if (directory == NULL)
    {
        free(raw);
        return 0;
    }

    for (size_t file_id = 1;
         file_id < record_count;
         ++file_id)
    {
        const uint8_t *entry =
            raw +
            file_id *
            FILE_HEADER_SIZE;

        uint32_t length =
            read_be32(entry + 0x00);

        uint16_t name_length =
            read_be16(entry + 0x0e);

        if (length == 0 &&
            name_length == 0)
        {
            continue;
        }

        if (name_length == 0 ||
            name_length > 36)
        {
            if (disk->verbose)
            {
                fprintf(stderr,
                        "WARNING: directory file ID %zu: "
                        "name length invalida %u; "
                        "entrada ignorada\n",
                        file_id,
                        name_length);
            }

            continue;
        }

        if (length < FILE_HEADER_SIZE)
        {
            if (disk->verbose)
            {
                fprintf(stderr,
                        "WARNING: directory file ID %zu: "
                        "length=%u (<64); "
                        "entrada ignorada\n",
                        file_id,
                        length);
            }

            continue;
        }

        if ((uint64_t)length >
            (uint64_t)disk->image_size +
            FILE_HEADER_SIZE)
        {
            if (disk->verbose)
            {
                fprintf(stderr,
                        "WARNING: directory file ID %zu: "
                        "length absurdo %u; ignorado\n",
                        file_id,
                        length);
            }

            continue;
        }

        directory[valid_count].file_id =
            (uint16_t)file_id;

        directory[valid_count].qdos_length =
            length;

        directory[valid_count].name_length =
            name_length;

        sanitize_filename(
            entry + 0x10,
            name_length,
            directory[valid_count].name,
            sizeof(
                directory[valid_count].name));

        directory[valid_count].
            modification_date =
            read_be32(entry + 0x34);

        valid_count++;
    }

    free(raw);

    *result =
        directory;

    *result_count =
        valid_count;

    return 1;
}


/* --------------------------------------------------------- */
/* Tamaño real del contenido                                 */
/* --------------------------------------------------------- */

static size_t
payload_size(const DirectoryEntry *entry)
{
    if (entry->qdos_length <
        FILE_HEADER_SIZE)
    {
        return 0;
    }

    return
        (size_t)entry->qdos_length -
        FILE_HEADER_SIZE;
}


/* --------------------------------------------------------- */
/* Comprobar allocation                                      */
/* --------------------------------------------------------- */

static int
check_file_allocation(
        const QLDisk *disk,
        const DirectoryEntry *entry)
{
    size_t allocation_bytes =
        (size_t)disk->allocation_size *
        SECTOR_SIZE;

    size_t groups_needed =
        ((size_t)entry->qdos_length +
         allocation_bytes - 1) /
        allocation_bytes;

    int ok = 1;

    for (size_t sequence = 0;
         sequence < groups_needed;
         ++sequence)
    {
        uint16_t ignored;

        if (sequence > 0xfff ||
            !find_allocation_group(
                disk,
                entry->file_id,
                (uint16_t)sequence,
                &ignored))
        {
            fprintf(stderr,
                    "WARNING: file %03X %-36s: "
                    "falta sequence %zu\n",
                    entry->file_id,
                    entry->name,
                    sequence);

            ok = 0;
        }
    }

    return ok;
}


/* --------------------------------------------------------- */
/* Listar                                                    */
/* --------------------------------------------------------- */

static void
print_directory(
        const QLDisk *disk,
        const DirectoryEntry *directory,
        size_t count)
{
    printf("Archivos: %zu\n\n",
           count);

    printf(" ID   QDOS BYTES  DATOS      GRUPOS  NOMBRE\n");

    printf("----  ----------  ----------  ------  "
           "------------------------------------\n");

    for (size_t i = 0;
         i < count;
         ++i)
    {
        const DirectoryEntry *entry =
            &directory[i];

        size_t allocation_bytes =
            (size_t)disk->allocation_size *
            SECTOR_SIZE;

        size_t required =
            ((size_t)entry->qdos_length +
             allocation_bytes - 1) /
            allocation_bytes;

        printf("%03X   %10u  %10zu  %3zu/%-2zu  %s",
               entry->file_id,
               entry->qdos_length,
               payload_size(entry),
               required,
               disk->file[
                   entry->file_id].count,
               entry->name);

        if (payload_size(entry) == 0)
        {
            printf("  [sin datos]");
        }

        printf("\n");
    }

    printf("\n");
}


/* --------------------------------------------------------- */
/* Crear directorio                                          */
/* --------------------------------------------------------- */

static int
make_directory(const char *path)
{
    if (MKDIR(path) == 0)
        return 1;

    if (errno == EEXIST)
        return 1;

    fprintf(stderr,
            "ERROR: no puedo crear '%s': %s\n",
            path,
            strerror(errno));

    return 0;
}


/* --------------------------------------------------------- */
/* Existe fichero host                                       */
/* --------------------------------------------------------- */

static int
host_file_exists(const char *filename)
{
    FILE *f =
        fopen(filename, "rb");

    if (f == NULL)
        return 0;

    fclose(f);

    return 1;
}


/* --------------------------------------------------------- */
/* Nombre de salida                                          */
/* --------------------------------------------------------- */

static int
make_output_filename(
        const char *directory,
        const DirectoryEntry *entry,
        char *result,
        size_t result_size)
{
    int n;

    n =
        snprintf(result,
                 result_size,
                 "%s%c%s",
                 directory,
                 PATH_SEPARATOR,
                 entry->name);

    if (n < 0 ||
        (size_t)n >= result_size)
    {
        return 0;
    }

    if (!host_file_exists(result))
        return 1;

    n =
        snprintf(result,
                 result_size,
                 "%s%c%s__%03X",
                 directory,
                 PATH_SEPARATOR,
                 entry->name,
                 entry->file_id);

    if (n < 0 ||
        (size_t)n >= result_size)
    {
        return 0;
    }

    return 1;
}


/* --------------------------------------------------------- */
/* Extraer fichero                                           */
/* --------------------------------------------------------- */

static int
extract_file(
        const QLDisk *disk,
        const DirectoryEntry *entry,
        const char *output_directory)
{
    uint8_t *data;
    size_t data_size;
    char filename[4096];
    FILE *f;

    data =
        reconstruct_file(
            disk,
            entry->file_id,
            entry->qdos_length);

    if (data == NULL)
    {
        fprintf(stderr,
                "ERROR: no puedo extraer file %03X '%s'\n",
                entry->file_id,
                entry->name);

        return 0;
    }

    data_size =
        payload_size(entry);

    if (!make_output_filename(
            output_directory,
            entry,
            filename,
            sizeof(filename)))
    {
        fprintf(stderr,
                "ERROR: nombre demasiado largo para '%s'\n",
                entry->name);

        free(data);

        return 0;
    }

    f =
        fopen(filename, "wb");

    if (f == NULL)
    {
        fprintf(stderr,
                "ERROR: no puedo crear '%s': %s\n",
                filename,
                strerror(errno));

        free(data);

        return 0;
    }

    if (data_size > 0)
    {
        if (fwrite(
                data + FILE_HEADER_SIZE,
                1,
                data_size,
                f) != data_size)
        {
            fprintf(stderr,
                    "ERROR escribiendo '%s'\n",
                    filename);

            fclose(f);

            free(data);

            return 0;
        }
    }

    fclose(f);

    free(data);

    printf("  %03X  %-36s %10zu bytes\n",
           entry->file_id,
           entry->name,
           data_size);

    return 1;
}


/* --------------------------------------------------------- */
/* Liberar                                                   */
/* --------------------------------------------------------- */

static void
free_disk(QLDisk *disk)
{
    for (unsigned i = 0;
         i < MAX_FILE_ID;
         ++i)
    {
        free(disk->file[i].entry);

        disk->file[i].entry = NULL;
        disk->file[i].count = 0;
        disk->file[i].capacity = 0;
    }

    free(disk->image);

    disk->image = NULL;
}


/* ========================================================= */
/* main_ql_extract_img                                           */
/* ========================================================= */

int
main_ql_extract_img(int argc, char **argv)
{
    QLDisk disk;

    DirectoryEntry *directory =
        NULL;

    size_t directory_count =
        0;

    const char *image_filename =
        NULL;

    const char *output_directory =
        "ql_extract_img";

    int output_directory_given =
        0;

    int list_only =
        0;

    int verbose =
        0;

    int extract_zero_length =
        0;

    int exit_status =
        EXIT_FAILURE;

    memset(&disk,
           0,
           sizeof(disk));


    for (int i = 1;
         i < argc;
         ++i)
    {
        if (strcmp(argv[i],
                   "-l") == 0)
        {
            list_only = 1;
        }
        else if (strcmp(argv[i],
                        "-v") == 0)
        {
            verbose = 1;
        }
        else if (strcmp(argv[i],
                        "-z") == 0)
        {
            extract_zero_length = 1;
        }
        else if (argv[i][0] == '-')
        {
            fprintf(stderr,
                    "Opcion desconocida: %s\n",
                    argv[i]);

            goto done;
        }
        else if (image_filename == NULL)
        {
            image_filename =
                argv[i];
        }
        else if (!output_directory_given)
        {
            output_directory =
                argv[i];

            output_directory_given =
                1;
        }
        else
        {
            fprintf(stderr,
                    "Demasiados argumentos\n");

            goto done;
        }
    }


    if (image_filename == NULL)
    {
        fprintf(stderr,
                "Uso:\n"
                "  %s [-l] [-v] [-z] "
                "imagen.img [salida]\n\n"
                "  -l  solo listar\n"
                "  -v  diagnostico detallado\n"
                "  -z  extraer tambien entradas "
                "de 0 bytes\n",
                argv[0]);

        goto done;
    }


    disk.verbose =
        verbose;


    if (!load_image(
            &disk,
            image_filename))
    {
        goto done;
    }


    if (!parse_header(&disk))
        goto done;


    print_disk_info(&disk);


    if (!build_allocation_map(
            &disk))
    {
        goto done;
    }


    {
        size_t directory_size;

        size_t allocation_bytes =
            (size_t)disk.allocation_size *
            SECTOR_SIZE;

        if (!get_directory_size(
                &disk,
                &directory_size))
        {
            goto done;
        }

        size_t required_groups =
            (directory_size +
             allocation_bytes - 1) /
            allocation_bytes;

        printf("Master directory:\n");

        printf("  EOF bytes          : %zu\n",
               directory_size);

        printf("  Entradas posibles  : %zu\n",
               directory_size /
               FILE_HEADER_SIZE);

        printf("  Grupos necesarios  : %zu\n",
               required_groups);

        printf("  Grupos en mapa     : %zu\n",
               disk.file[0].count);

        printf("\n");

        if (disk.file[0].count <
            required_groups)
        {
            fprintf(stderr,
                    "ERROR: faltan grupos del "
                    "master directory\n");

            goto done;
        }
    }


    if (!read_master_directory(
            &disk,
            &directory,
            &directory_count))
    {
        goto done;
    }


    print_directory(
        &disk,
        directory,
        directory_count);


    {
        size_t good = 0;
        size_t bad = 0;

        printf("Comprobando allocation map:\n");

        for (size_t i = 0;
             i < directory_count;
             ++i)
        {
            if (check_file_allocation(
                    &disk,
                    &directory[i]))
            {
                good++;
            }
            else
            {
                bad++;
            }
        }

        printf("  Correctos  : %zu\n",
               good);

        printf("  Con errores: %zu\n\n",
               bad);
    }


    if (list_only)
    {
        exit_status =
            EXIT_SUCCESS;

        goto done;
    }


    if (!make_directory(
            output_directory))
    {
        goto done;
    }


    printf("Extrayendo a: %s\n\n",
           output_directory);


    {
        size_t extracted = 0;
        size_t skipped = 0;
        size_t failed = 0;

        for (size_t i = 0;
             i < directory_count;
             ++i)
        {
            size_t size =
                payload_size(
                    &directory[i]);

            if (size == 0 &&
                !extract_zero_length)
            {
                skipped++;

                if (verbose)
                {
                    printf("  %03X  %-36s "
                           "[omitido: 0 bytes]\n",
                           directory[i].file_id,
                           directory[i].name);
                }

                continue;
            }

            if (extract_file(
                    &disk,
                    &directory[i],
                    output_directory))
            {
                extracted++;
            }
            else
            {
                failed++;
            }
        }


        printf("\n");

        printf("Extraidos : %zu\n",
               extracted);

        printf("Omitidos  : %zu\n",
               skipped);

        printf("Fallidos  : %zu\n",
               failed);

        printf("\n");


        if (failed == 0)
        {
            exit_status =
                EXIT_SUCCESS;
        }
        else
        {
            exit_status =
                EXIT_FAILURE;
        }
    }


done:

    free(directory);

    free_disk(&disk);

    return exit_status;
}