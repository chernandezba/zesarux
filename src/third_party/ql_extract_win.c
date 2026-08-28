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
 * ql_extract_win.c
 *
 * Extractor de imagenes .WIN con filesystem QLWA / QXL.WIN.
 *
 * Entrada para uso desde otro programa:
 *
 *   int main_ql_extract_win(int argc, char **argv);
 *
 * Uso:
 *
 *   ql_extract_win imagen.win
 *   ql_extract_win imagen.win directorio_salida
 *   ql_extract_win -l imagen.win
 *   ql_extract_win -v imagen.win
 *   ql_extract_win -z imagen.win
 *
 * Opciones:
 *
 *   -l   solo listar
 *   -v   diagnostico detallado
 *   -z   extraer tambien ficheros de 0 bytes
 *
 * Formato QLWA:
 *
 *   +00  long   "QLWA"
 *   +04  word   longitud del nombre del dispositivo
 *   +06  20     nombre del dispositivo
 *   +22  word   sectores de 512 bytes por grupo
 *   +2A  word   numero de grupos
 *   +2C  word   grupos libres
 *   +2E  word   sectores por mapa
 *   +30  word   numero de mapas
 *   +32  word   primer grupo libre
 *   +34  word   grupo del directorio raiz
 *   +36  long   longitud del directorio raiz, incluyendo header
 *   +40  words  mapa enlazado de grupos
 *
 * Cada entrada del mapa contiene el siguiente grupo de la cadena,
 * o 0000 para indicar final.
 *
 * Los directorios y ficheros tienen un header de 64 bytes.
 * En el primer grupo se saltan esos 64 bytes para obtener el contenido.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include "ql_extract_win.h"

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

#define SECTOR_SIZE          512U
#define QLWA_HEADER_SIZE      64U
#define FILE_HEADER_SIZE      64U
#define DIR_ENTRY_SIZE        64U
#define QLWA_MAP_OFFSET     0x40U
#define QLWA_NAME_MAX         20U
#define QDOS_NAME_MAX         36U

#define HDR_FLEN            0x00U
#define HDR_TYPE            0x05U
#define HDR_NAME_LEN        0x0eU
#define HDR_NAME            0x10U
#define HDR_DATE            0x34U
#define HDR_FILE_GROUP      0x3aU

#define QDOS_TYPE_DIRECTORY 0xffU

typedef struct
{
    uint8_t *image;
    size_t image_size;

    char device_name[QLWA_NAME_MAX + 1];

    uint16_t sectors_per_group;
    uint16_t group_count;
    uint16_t free_groups;
    uint16_t sectors_per_map;
    uint16_t map_count;
    uint16_t first_free_group;

    uint16_t root_group;
    uint32_t root_length;

    size_t group_size;
    size_t map_bytes_available;

    int verbose;
} QLWin;

typedef struct
{
    uint32_t length;
    uint8_t type;
    uint16_t name_length;
    char name[QDOS_NAME_MAX + 1];
    uint32_t date;
    uint16_t first_group;
} WinDirEntry;

static uint16_t read_be16(const uint8_t *p)
{
    return ((uint16_t)p[0] << 8) |
           ((uint16_t)p[1]);
}

static uint32_t read_be32(const uint8_t *p)
{
    return ((uint32_t)p[0] << 24) |
           ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) |
           ((uint32_t)p[3]);
}

static int load_image(QLWin *win, const char *filename)
{
    FILE *f;
    long length;

    f = fopen(filename, "rb");
    if (f == NULL)
    {
        fprintf(stderr, "ERROR: no puedo abrir '%s': %s\n",
                filename, strerror(errno));
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

    win->image_size = (size_t)length;
    win->image = (uint8_t *)malloc(win->image_size);

    if (win->image == NULL)
    {
        fprintf(stderr, "ERROR: sin memoria para %zu bytes\n",
                win->image_size);
        fclose(f);
        return 0;
    }

    if (fread(win->image, 1, win->image_size, f) != win->image_size)
    {
        fprintf(stderr, "ERROR leyendo '%s'\n", filename);
        free(win->image);
        win->image = NULL;
        fclose(f);
        return 0;
    }

    fclose(f);
    return 1;
}

static int parse_header(QLWin *win)
{
    const uint8_t *h;
    uint16_t name_length;
    uint64_t required_size;
    uint64_t map_capacity;

    if (win->image_size < QLWA_HEADER_SIZE)
    {
        fprintf(stderr, "ERROR: imagen demasiado pequena\n");
        return 0;
    }

    h = win->image;

    if (memcmp(h, "QLWA", 4) != 0)
    {
        fprintf(stderr, "ERROR: la imagen no empieza por QLWA\n");
        return 0;
    }

    name_length = read_be16(h + 0x04);
    if (name_length > QLWA_NAME_MAX)
        name_length = QLWA_NAME_MAX;

    memcpy(win->device_name, h + 0x06, name_length);
    win->device_name[name_length] = '\0';

    while (name_length > 0 &&
           win->device_name[name_length - 1] == ' ')
    {
        win->device_name[--name_length] = '\0';
    }

    win->sectors_per_group = read_be16(h + 0x22);
    win->group_count       = read_be16(h + 0x2a);
    win->free_groups       = read_be16(h + 0x2c);
    win->sectors_per_map   = read_be16(h + 0x2e);
    win->map_count         = read_be16(h + 0x30);
    win->first_free_group  = read_be16(h + 0x32);
    win->root_group        = read_be16(h + 0x34);
    win->root_length       = read_be32(h + 0x36);

    if (win->sectors_per_group == 0 ||
        win->group_count == 0)
    {
        fprintf(stderr, "ERROR: cabecera QLWA invalida\n");
        return 0;
    }

    win->group_size =
        (size_t)win->sectors_per_group * SECTOR_SIZE;

    required_size =
        (uint64_t)win->group_count *
        (uint64_t)win->group_size;

    if (required_size > win->image_size)
    {
        fprintf(stderr,
                "ERROR: QLWA requiere al menos %llu bytes "
                "pero la imagen tiene %zu\n",
                (unsigned long long)required_size,
                win->image_size);
        return 0;
    }

    map_capacity =
        (uint64_t)win->sectors_per_map *
        (uint64_t)(win->map_count ? win->map_count : 1) *
        SECTOR_SIZE;

    if (map_capacity == 0 ||
        map_capacity > win->image_size)
    {
        map_capacity = win->image_size;
    }

    win->map_bytes_available = (size_t)map_capacity;

    if ((uint64_t)QLWA_MAP_OFFSET +
        (uint64_t)win->group_count * 2ULL >
        (uint64_t)win->map_bytes_available)
    {
        fprintf(stderr,
                "ERROR: el mapa QLWA no tiene espacio para %u grupos\n",
                win->group_count);
        return 0;
    }

    if (win->root_group >= win->group_count)
    {
        fprintf(stderr, "ERROR: root group %u fuera de rango\n",
                win->root_group);
        return 0;
    }

    return 1;
}

static void print_header(const QLWin *win)
{
    printf("\n");
    printf("Formato             : QLWA\n");
    printf("Nombre dispositivo  : \"%s\"\n", win->device_name);
    printf("Sectores/grupo      : %u\n", win->sectors_per_group);
    printf("Tamano grupo        : %zu bytes\n", win->group_size);
    printf("Grupos totales      : %u\n", win->group_count);
    printf("Grupos libres       : %u\n", win->free_groups);
    printf("Sectores/mapa       : %u\n", win->sectors_per_map);
    printf("Numero de mapas     : %u\n", win->map_count);
    printf("Primer grupo libre  : %u\n", win->first_free_group);
    printf("Root group          : %u\n", win->root_group);
    printf("Root length         : %u bytes\n", win->root_length);
    printf("\n");
}

static int get_next_group(const QLWin *win,
                          uint16_t group,
                          uint16_t *next)
{
    size_t offset;

    if (group >= win->group_count)
        return 0;

    offset =
        QLWA_MAP_OFFSET +
        (size_t)group * 2U;

    if (offset + 2U > win->image_size ||
        offset + 2U > win->map_bytes_available)
    {
        return 0;
    }

    *next = read_be16(win->image + offset);
    return 1;
}

static int group_offset(const QLWin *win,
                        uint16_t group,
                        size_t *offset)
{
    uint64_t off;

    if (group >= win->group_count)
        return 0;

    off =
        (uint64_t)group *
        (uint64_t)win->group_size;

    if (off + win->group_size > win->image_size)
        return 0;

    *offset = (size_t)off;
    return 1;
}

static uint8_t *read_chain(const QLWin *win,
                           uint16_t first_group,
                           uint32_t length)
{
    uint8_t *result;
    uint8_t *visited;
    size_t copied = 0;
    uint16_t group;
    size_t groups_seen = 0;

    if (length == 0)
        return (uint8_t *)calloc(1, 1);

    if (first_group == 0 ||
        first_group >= win->group_count)
    {
        fprintf(stderr,
                "ERROR: primer grupo invalido: %u\n",
                first_group);
        return NULL;
    }

    result = (uint8_t *)malloc(length);
    if (result == NULL)
        return NULL;

    visited =
        (uint8_t *)calloc(win->group_count, 1);

    if (visited == NULL)
    {
        free(result);
        return NULL;
    }

    group = first_group;

    while (copied < length)
    {
        size_t offset;
        size_t amount;
        uint16_t next;

        if (group == 0 ||
            group >= win->group_count)
        {
            fprintf(stderr,
                    "ERROR: cadena termina antes de tiempo "
                    "(grupo %u)\n",
                    group);
            free(visited);
            free(result);
            return NULL;
        }

        if (visited[group])
        {
            fprintf(stderr,
                    "ERROR: bucle en cadena de grupos, grupo %u\n",
                    group);
            free(visited);
            free(result);
            return NULL;
        }

        visited[group] = 1;
        groups_seen++;

        if (groups_seen > win->group_count)
        {
            free(visited);
            free(result);
            return NULL;
        }

        if (!group_offset(win, group, &offset))
        {
            fprintf(stderr,
                    "ERROR: grupo %u fuera de la imagen\n",
                    group);
            free(visited);
            free(result);
            return NULL;
        }

        amount = length - copied;
        if (amount > win->group_size)
            amount = win->group_size;

        memcpy(result + copied,
               win->image + offset,
               amount);

        copied += amount;

        if (copied >= length)
            break;

        if (!get_next_group(win, group, &next))
        {
            fprintf(stderr,
                    "ERROR: no puedo leer enlace del grupo %u\n",
                    group);
            free(visited);
            free(result);
            return NULL;
        }

        if (win->verbose)
        {
            printf("    chain: %u -> %u\n",
                   group, next);
        }

        if (next == 0)
        {
            fprintf(stderr,
                    "ERROR: fin de cadena prematuro en grupo %u\n",
                    group);
            free(visited);
            free(result);
            return NULL;
        }

        group = next;
    }

    free(visited);
    return result;
}

static void sanitize_name(const uint8_t *src,
                          size_t length,
                          char *dst,
                          size_t dst_size)
{
    size_t out = 0;

    if (dst_size == 0)
        return;

    for (size_t i = 0;
         i < length && out + 1 < dst_size;
         ++i)
    {
        unsigned char c = src[i];

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
            dst[out++] = '_';
        }
        else if (c < 32 || c == 127)
        {
            dst[out++] = '_';
        }
        else
        {
            dst[out++] = (char)c;
        }
    }

    while (out > 0 &&
           (dst[out - 1] == ' ' ||
            dst[out - 1] == '.'))
    {
        --out;
    }

    if (out == 0)
    {
        strncpy(dst, "unnamed", dst_size - 1);
        dst[dst_size - 1] = '\0';
    }
    else
    {
        dst[out] = '\0';
    }
}

static int parse_dir_entry(const uint8_t *p,
                           WinDirEntry *entry)
{
    uint16_t name_length;

    memset(entry, 0, sizeof(*entry));

    entry->length =
        read_be32(p + HDR_FLEN);

    entry->type =
        p[HDR_TYPE];

    name_length =
        read_be16(p + HDR_NAME_LEN);

    if (entry->length == 0 &&
        name_length == 0)
    {
        return 0;
    }

    if (name_length == 0 ||
        name_length > QDOS_NAME_MAX)
    {
        return -1;
    }

    entry->name_length = name_length;

    sanitize_name(p + HDR_NAME,
                  name_length,
                  entry->name,
                  sizeof(entry->name));

    entry->date =
        read_be32(p + HDR_DATE);

    entry->first_group =
        read_be16(p + HDR_FILE_GROUP);

    return 1;
}

static int make_directory(const char *path)
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

static int create_label_file(const char *output_dir,
                             const char *device_name)
{
    char filename[4096];
    size_t label_length;
    int n;
    FILE *f;

    if (device_name[0] == '\0')
        return 1;

    n = snprintf(filename,
                 sizeof(filename),
                 "%s%cLABEL",
                 output_dir,
                 PATH_SEPARATOR);

    if (n < 0 || (size_t)n >= sizeof(filename))
    {
        fprintf(stderr,
                "ERROR: ruta demasiado larga para el archivo LABEL\n");
        return 0;
    }

    f = fopen(filename, "wb");
    if (f == NULL)
    {
        fprintf(stderr,
                "ERROR: no puedo crear '%s': %s\n",
                filename,
                strerror(errno));
        return 0;
    }

    label_length = strlen(device_name);
    if (fwrite(device_name, 1, label_length, f) != label_length)
    {
        fprintf(stderr,
                "ERROR escribiendo '%s'\n",
                filename);
        fclose(f);
        return 0;
    }

    if (fclose(f) != 0)
    {
        fprintf(stderr,
                "ERROR cerrando '%s': %s\n",
                filename,
                strerror(errno));
        return 0;
    }

    printf("Archivo LABEL creado: %s\n", device_name);
    return 1;
}

static int host_file_exists(const char *filename)
{
    FILE *f = fopen(filename, "rb");

    if (f == NULL)
        return 0;

    fclose(f);
    return 1;
}

static int build_output_path(const char *parent,
                             const char *name,
                             uint16_t first_group,
                             int is_directory,
                             char *result,
                             size_t result_size)
{
    int n;

    n = snprintf(result,
                 result_size,
                 "%s%c%s",
                 parent,
                 PATH_SEPARATOR,
                 name);

    if (n < 0 || (size_t)n >= result_size)
        return 0;

    if (is_directory || !host_file_exists(result))
        return 1;

    n = snprintf(result,
                 result_size,
                 "%s%c%s__%04X",
                 parent,
                 PATH_SEPARATOR,
                 name,
                 first_group);

    if (n < 0 || (size_t)n >= result_size)
        return 0;

    return 1;
}

static int extract_regular_file(const QLWin *win,
                                const WinDirEntry *entry,
                                const char *output_dir,
                                int extract_zero_length)
{
    uint8_t *raw;
    size_t payload;
    char filename[4096];
    FILE *f;

    if (entry->length < FILE_HEADER_SIZE)
    {
        fprintf(stderr,
                "WARNING: '%s' tiene longitud QDOS invalida %u\n",
                entry->name,
                entry->length);
        return 0;
    }

    payload =
        (size_t)entry->length -
        FILE_HEADER_SIZE;

    if (payload == 0 &&
        !extract_zero_length)
    {
        if (win->verbose)
        {
            printf("  %-36s [omitido: 0 bytes]\n",
                   entry->name);
        }
        return 2;
    }

    raw =
        read_chain(win,
                   entry->first_group,
                   entry->length);

    if (raw == NULL)
    {
        fprintf(stderr,
                "ERROR: no puedo reconstruir '%s'\n",
                entry->name);
        return 0;
    }

    if (!build_output_path(output_dir,
                           entry->name,
                           entry->first_group,
                           0,
                           filename,
                           sizeof(filename)))
    {
        fprintf(stderr,
                "ERROR: ruta demasiado larga para '%s'\n",
                entry->name);
        free(raw);
        return 0;
    }

    f = fopen(filename, "wb");
    if (f == NULL)
    {
        fprintf(stderr,
                "ERROR: no puedo crear '%s': %s\n",
                filename,
                strerror(errno));
        free(raw);
        return 0;
    }

    if (payload > 0 &&
        fwrite(raw + FILE_HEADER_SIZE,
               1,
               payload,
               f) != payload)
    {
        fprintf(stderr,
                "ERROR escribiendo '%s'\n",
                filename);
        fclose(f);
        free(raw);
        return 0;
    }

    fclose(f);
    free(raw);

    printf("  %-36s %10zu bytes\n",
           entry->name,
           payload);

    return 1;
}

static void print_indent(unsigned depth)
{
    for (unsigned i = 0; i < depth; ++i)
        printf("  ");
}

static int process_directory(const QLWin *win,
                             uint16_t first_group,
                             uint32_t length,
                             const char *display_name,
                             const char *output_dir,
                             unsigned depth,
                             int list_only,
                             int extract_zero_length,
                             uint8_t *directory_stack,
                             size_t *files_ok,
                             size_t *dirs_ok,
                             size_t *skipped,
                             size_t *failed)
{
    uint8_t *raw;
    size_t record_count;

    if (length < FILE_HEADER_SIZE)
    {
        fprintf(stderr,
                "ERROR: directorio '%s' demasiado pequeno (%u)\n",
                display_name,
                length);
        return 0;
    }

    if (first_group == 0 ||
        first_group >= win->group_count)
    {
        fprintf(stderr,
                "ERROR: grupo de directorio invalido: %u\n",
                first_group);
        return 0;
    }

    if (directory_stack[first_group])
    {
        fprintf(stderr,
                "ERROR: recursion circular de directorios en grupo %u\n",
                first_group);
        return 0;
    }

    directory_stack[first_group] = 1;

    raw =
        read_chain(win,
                   first_group,
                   length);

    if (raw == NULL)
    {
        directory_stack[first_group] = 0;
        return 0;
    }

    record_count =
        length / DIR_ENTRY_SIZE;

    if (depth == 0)
    {
        printf("Directorio raiz: %s\n\n",
               display_name);
    }

    /*
     * Los primeros 64 bytes del directorio son el header
     * reservado/no usado. Las entradas empiezan en record 1.
     */
    for (size_t index = 1;
         index < record_count;
         ++index)
    {
        WinDirEntry entry;
        const uint8_t *p =
            raw + index * DIR_ENTRY_SIZE;

        int parsed =
            parse_dir_entry(p, &entry);

        if (parsed == 0)
            continue;

        if (parsed < 0)
        {
            if (win->verbose)
            {
                print_indent(depth);
                printf("[entrada %zu invalida]\n", index);
            }
            continue;
        }

        print_indent(depth);

        if (entry.type == QDOS_TYPE_DIRECTORY)
        {
            printf("[DIR] %-36s "
                   "group=%u len=%u\n",
                   entry.name,
                   entry.first_group,
                   entry.length);

            if (!list_only)
            {
                char subdir[4096];

                if (!build_output_path(output_dir,
                                       entry.name,
                                       entry.first_group,
                                       1,
                                       subdir,
                                       sizeof(subdir)))
                {
                    fprintf(stderr,
                            "ERROR: ruta demasiado larga para directorio '%s'\n",
                            entry.name);
                    (*failed)++;
                    continue;
                }

                if (!make_directory(subdir))
                {
                    (*failed)++;
                    continue;
                }

                if (process_directory(win,
                                      entry.first_group,
                                      entry.length,
                                      entry.name,
                                      subdir,
                                      depth + 1,
                                      list_only,
                                      extract_zero_length,
                                      directory_stack,
                                      files_ok,
                                      dirs_ok,
                                      skipped,
                                      failed))
                {
                    (*dirs_ok)++;
                }
                else
                {
                    (*failed)++;
                }
            }
            else
            {
                if (process_directory(win,
                                      entry.first_group,
                                      entry.length,
                                      entry.name,
                                      output_dir,
                                      depth + 1,
                                      list_only,
                                      extract_zero_length,
                                      directory_stack,
                                      files_ok,
                                      dirs_ok,
                                      skipped,
                                      failed))
                {
                    (*dirs_ok)++;
                }
                else
                {
                    (*failed)++;
                }
            }
        }
        else
        {
            size_t payload =
                entry.length >= FILE_HEADER_SIZE ?
                (size_t)entry.length - FILE_HEADER_SIZE :
                0;

            printf("%-42s %10zu bytes "
                   "type=%u group=%u\n",
                   entry.name,
                   payload,
                   entry.type,
                   entry.first_group);

            if (!list_only)
            {
                int rc =
                    extract_regular_file(win,
                                         &entry,
                                         output_dir,
                                         extract_zero_length);

                if (rc == 1)
                    (*files_ok)++;
                else if (rc == 2)
                    (*skipped)++;
                else
                    (*failed)++;
            }
        }
    }

    free(raw);
    directory_stack[first_group] = 0;
    return 1;
}

static void free_win(QLWin *win)
{
    free(win->image);
    win->image = NULL;
}

int main_ql_extract_win(int argc, char **argv)
{
    QLWin win;

    const char *image_filename = NULL;
    const char *output_directory = "ql_extract_win";

    int output_directory_given = 0;
    int list_only = 0;
    int verbose = 0;
    int extract_zero_length = 0;

    uint8_t *directory_stack = NULL;

    size_t files_ok = 0;
    size_t dirs_ok = 0;
    size_t skipped = 0;
    size_t failed = 0;

    int exit_status = EXIT_FAILURE;

    memset(&win, 0, sizeof(win));

    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "-l") == 0)
        {
            list_only = 1;
        }
        else if (strcmp(argv[i], "-v") == 0)
        {
            verbose = 1;
        }
        else if (strcmp(argv[i], "-z") == 0)
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
            image_filename = argv[i];
        }
        else if (!output_directory_given)
        {
            output_directory = argv[i];
            output_directory_given = 1;
        }
        else
        {
            fprintf(stderr, "Demasiados argumentos\n");
            goto done;
        }
    }

    if (image_filename == NULL)
    {
        fprintf(stderr,
                "Uso:\n"
                "  %s [-l] [-v] [-z] imagen.win [salida]\n\n"
                "  -l  solo listar\n"
                "  -v  diagnostico detallado\n"
                "  -z  extraer tambien ficheros de 0 bytes\n",
                argv[0]);
        goto done;
    }

    win.verbose = verbose;

    if (!load_image(&win, image_filename))
        goto done;

    if (!parse_header(&win))
        goto done;

    print_header(&win);

    directory_stack =
        (uint8_t *)calloc(win.group_count, 1);

    if (directory_stack == NULL)
    {
        fprintf(stderr,
                "ERROR: sin memoria para control de directorios\n");
        goto done;
    }

    if (!list_only)
    {
        if (!make_directory(output_directory))
            goto done;

        printf("Extrayendo a: %s\n\n",
               output_directory);
    }

    if (!process_directory(&win,
                           win.root_group,
                           win.root_length,
                           win.device_name[0] ?
                               win.device_name :
                               "<root>",
                           output_directory,
                           0,
                           list_only,
                           extract_zero_length,
                           directory_stack,
                           &files_ok,
                           &dirs_ok,
                           &skipped,
                           &failed))
    {
        failed++;
    }

    if (!list_only &&
        !create_label_file(output_directory, win.device_name))
    {
        failed++;
    }

    printf("\n");
    if (!list_only)
    {
        printf("Ficheros extraidos : %zu\n", files_ok);
        printf("Directorios        : %zu\n", dirs_ok);
        printf("Omitidos            : %zu\n", skipped);
    }
    printf("Fallidos            : %zu\n", failed);
    printf("\n");

    exit_status =
        (failed == 0) ?
        EXIT_SUCCESS :
        EXIT_FAILURE;

done:
    free(directory_stack);
    free_win(&win);
    return exit_status;
}
