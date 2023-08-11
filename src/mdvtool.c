/*
    mdvtool.c: Tool from MIST emulator to extract contents of a .mdv qlay file format. File should be 174930 bytes in size
    (c) 2015 by Till Harbaum
    https://github.com/mist-devel

    Copyright (c) 2015 Till Harbaum <till@harbaum.org>

    ZEsarUX  ZX Second-Emulator And Released for UniX
    Copyright (C) 2013 Cesar Hernandez Bano

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


*/

/* mdvtool */
/* another quick'n dirty tool to deal with microdrive images */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
//#include <zip.h>
#include <ctype.h>

#if defined(__APPLE__)
        #include <sys/syslimits.h>
#endif

#include "mdvtool.h"
#include "debug.h"
#include "utils.h"
#include "ay38912.h"

FILE *mdv = NULL;

#define SWAP16(a)   ((((a)&0xff)<<8)|(((a)&0xff00)>>8))
#define SWAP32(a)   ((((a)&0xff)<<24)|(((a)&0xff00)<<8)|(((a)&0xff0000)>>8)|(((a)&0xff000000)>>24))

void mdvtool_hexdump(void *data, int size) {
  unsigned char i, b2c;
  int n=0;
  char *ptr = data;

  if(!size) return;

  while(size>0) {
    printf("%04x: ", n);

    b2c = (size>16)?16:size;
    for(i=0;i<b2c;i++)      printf("%02x ", 0xff&ptr[i]);
    printf("  ");
    for(i=0;i<(16-b2c);i++) printf("   ");
    for(i=0;i<b2c;i++)      printf("%c", isprint(ptr[i])?ptr[i]:'.');
    printf("\n");
    ptr  += b2c;
    size -= b2c;
    n    += b2c;
  }
}
// buffer for mdv file
mdv_entry_t buffer[MDVTOOL_MAX_SECTORS];
unsigned char mdvtool_sector_table[MDVTOOL_MAX_SECTORS];
char mdvtool_medium_name[10];
unsigned char mdvtool_files[256][256];

int mdvtool_isbyte(unsigned char *c, unsigned char byte, int len) {
  while(len--)
    if(*c++ != byte)
      return 0;

  return 1;
}

unsigned short sum(unsigned char *p, int len) {
  unsigned short v = 0x0f0f;
  while(len--)
    v += *p++;

  return v;
}

unsigned short mdvtool_get_index(int s) {
  int i;
  for(i=0;i<MDVTOOL_MAX_SECTORS;i++)
    if(mdvtool_sector_table[i] == s)
      return i;

  // not found
  return 0xffff;
}

int mdvtool_check_preamble(unsigned char *p, int zeros) {
  if(!mdvtool_isbyte(p, 0, zeros))
    return -1;

  if(!mdvtool_isbyte(p+zeros, 0xff, 2))
    return -1;

  return 0;
}

// get entry from mapping table in sector 0
int mdvtool_get_mapping_entry(int i) {
  return 256*buffer[0].sec.data[2*i] + buffer[0].sec.data[2*i+1];
}

// compare mapping table with sector number of sector header
void mdv_check_mapping(void) {
  int i;
  for(i=0;i<255;i++) {
    // mapping entry from sector 0
    int me = mdvtool_get_mapping_entry(i);

    // check only used entries
    if(me != 0xff00) {
      // file/block entry as stored inside block header
      unsigned short phys = mdvtool_get_index(i);

      // check for valid physical entry
      // TODO:

      int me_bh = 256*buffer[phys].sec.file + buffer[phys].sec.block;

      if(me != me_bh)
	printf("%3d: %04x / %04x\n",  i, me, me_bh);
    } else {
      // this sector must not be used at all
      unsigned short phys = mdvtool_get_index(i);

      if(phys != 0xffff) {
	int me_bh = 256*buffer[phys].sec.file + buffer[phys].sec.block;
	printf("index = %d (%x/%x)\n", phys, me, me_bh);
      }
    }
  }
}

void mdvtool_file_dump_chain(int f) {
  int j;

  // dump block chain
  for(j=0;j<256;j++) {
    if(mdvtool_files[f][j] != 255)
      printf("%s%d", j?", ":"", mdvtool_files[f][j]);
  }
  printf("\n");
}

void mdvtool_create_label(char *medium_name,char *dest_dir)
{
    //printf("medium name: %s dest_dir: %s\n",medium_name,dest_dir);

    char buffer_nombre[PATH_MAX];
    /*
    Evitar caracteres tipo / o \
    */

   int i;

   for (i=0;medium_name[i];i++) {
       char c=medium_name[i];

       if (c=='/' || c=='\\') c='.';

       buffer_nombre[i]=c;
   }

   buffer_nombre[i]=0;

   char nombre_final[PATH_MAX];

   sprintf(nombre_final,"%s/LABEL-%s",dest_dir,buffer_nombre);

   debug_printf(VERBOSE_INFO,"Creating label file %s",nombre_final);

   char *buffer_contenido="Dummy file just to generate a file label";

   util_save_file((z80_byte *)buffer_contenido,strlen(buffer_contenido),nombre_final);


}

int mdv_load(char *name,char *dest_dir) {

  printf("Loading %s ...\n", name);

  memset(mdvtool_medium_name, 0, sizeof(mdvtool_medium_name));
  memset(mdvtool_sector_table, 0xff, sizeof(mdvtool_sector_table));
  memset(mdvtool_files, 0xff, sizeof(mdvtool_files));

  mdv = fopen(name, "rb");
  if(!mdv) {
    fprintf(stderr, "Unable to open %s\n", name);
    return -1;
  }

  // check file size
  fseek(mdv, 0, SEEK_END);
  int size = ftell(mdv);
  fseek(mdv, 0, SEEK_SET);

  if(size == sizeof(buffer)) {
    // load qlay format file
    if(fread(buffer, sizeof(mdv_entry_t), MDVTOOL_MAX_SECTORS, mdv) != MDVTOOL_MAX_SECTORS) {
      perror("fread()");
      return -1;
    }
  } else {
    fprintf(stderr, "Uexpected file size\n");

    // check if it's a qemulator image and load and convert it


    return -1;
  }

  // check all chunks
  int i, free=0;
  int used = 0;
  for(i=0;i<MDVTOOL_MAX_SECTORS;i++) {
    /* -------------------- header checks ---------------- */
    hdr_t *hdr = &buffer[i].hdr;

    // check preamble
    if(mdvtool_check_preamble(hdr->preamble, 10) != 0) {
      fprintf(stderr, "Header @%d: Preamble check failed\n", i);

      //algun juego da error aqui
      //return -1;
    }

    if(hdr->ff != 0xff) {
      // silently ignore unused entries
      //      fprintf(stderr, "Header %d ff check failed\n", i);
    } else {
      if(hdr->csum != sum(&hdr->ff, 14)) {
	printf("Header @%d: checksum failed\n", i);
    //algun juego da error aqui
	//return -1;
      }

      if(!mdvtool_medium_name[0]) {
	memcpy(mdvtool_medium_name, hdr->name, 10);
      } else {
	if(memcmp(mdvtool_medium_name, hdr->name, 10) != 0) {
	  fprintf(stderr, "Header @%d: Medium name mismatch "
		  "(\"%.10s\" != \"%.10s\")\n", i, hdr->name, mdvtool_medium_name);
	  return -1;
	}
      }

      if(mdvtool_sector_table[i] != 0xff) {
	fprintf(stderr, "Header @%d: Multiple sector number %d\n",
		i, hdr->snum);

        //algun juego da error aqui
	    //return -1;
      }
      mdvtool_sector_table[i] = hdr->snum;

      /* -------------------- sector checks ---------------- */
      sector_t *sec = &buffer[i].sec;

      // check preamble
      if(mdvtool_check_preamble(sec->bh_preamble, 10) != 0) {
	fprintf(stderr, "Sector @%d: Block header preamble check failed\n", i);
	return -1;
      }

      if(sec->bh_csum != sum(&sec->file, 2)) {
	printf("WARNING: Sector @%d(%d): Block header checksum failed\n", i, hdr->snum);
      }

      if(sec->data_csum != sum(sec->data, 512)) {
	printf("WARNING: Sector @%d(%d): Data checksum failed %x != %x\n",
	       i, hdr->snum, sec->data_csum, sum(sec->data, 512));
      }

      // save the file index if it's not a free sector (file == 253)
      if(sec->file == 253)
	free++;
      else {
	if(mdvtool_files[sec->file][sec->block] != 255) {
	  fprintf(stderr, "Sector @%d: Multiple file/block %d/%d\n",
		  i, sec->file, sec->block);
          //algun juego da error aqui
	  //return -1;
	}

	used++;
	mdvtool_files[sec->file][sec->block] = hdr->snum;
      }
    }
  }

  printf("Medium name: \"%.10s\"\n", mdvtool_medium_name);
  //crear archivo con nombre igual que el label
  mdvtool_create_label(mdvtool_medium_name,dest_dir);

  // check if we are having gaps in the sector list
  for(i=0;i<MDVTOOL_MAX_SECTORS;i++) {
    if(mdvtool_sector_table[i] != 255) {
      // for every sector != 0 the previous sector must also exist
      if(mdvtool_sector_table[i] > 0) {
	     if(mdvtool_get_index(mdvtool_sector_table[i]-1) == 0xffff)
	       fprintf(stderr, "WARNING: Missing sector %d\n", mdvtool_sector_table[i]-1);
      }
    }
  }

  printf("Total sectors: %d (%d bytes)\n", used+free, (used+free)*512);
  printf("Sectors used: %d (%d bytes)\n", used, used*512);
  printf("Sectors free: %d (%d bytes)\n", free, free*512);

  return 0;
}

sector_t *mdvtool_file_get_sector(int file, int block) {
  if(mdvtool_files[file][block] == 255)
    return NULL;

  return &buffer[mdvtool_get_index(mdvtool_files[file][block])].sec;
}

void mdv_files_check() {
  // check all files
  int i, used = 0;
  for(i=0;i<256;i++) {
    // check length of block chain and whether its continuous
    int j, bused = 0;
    for(j=0;j<256;j++) {
      if(mdvtool_files[i][j] != 255) {
	bused++;

	if((j > 0) && (mdvtool_files[i][j-1] == 255))
	  printf("File %d: Missing entry for block %d\n", i, j-1);
      }
    }

    if(bused)
      if((i > 0)&&(i < 128))
	used++;
  }

  printf("Number of regular files: %d\n", used);

  // print infos from some special files
  if(mdvtool_files[249][0] != 0xff) {
    printf("List of defect sectors: ");
    mdvtool_file_dump_chain(249);
  }
}

file_t *mdvtool_file_get_entry(int i) {
  // each sector can hold 8 directory entries
  sector_t *s = mdvtool_file_get_sector(0,i/8);
  if(!s) {
    printf("Missing directory sector %d\n", i/8);
    return NULL;
  }

  return (file_t*)(s->data+sizeof(file_t)*(i&7));
}

int mdvtool_file_size(int i) {
  file_t *f = mdvtool_file_get_entry(i);
  if(!f) return -1;

  return SWAP32(f->length);
}

char *mdvtool_file_name(int i) {
  file_t *f = mdvtool_file_get_entry(i);
  if(!f) return NULL;

  return f->name;
}

int mdvtool_file_open(char *name) {
  int entries = mdvtool_file_size(0)/sizeof(file_t);
  if(entries < 2) return -1;

  // scan all entries
  int j;
  for(j=1;j<entries;j++) {
    if(strcmp(mdvtool_file_name(j), name) == 0)
      return j;
  }
  return -1;
}

void mdvtool_file_export_dest(char *name,char *destination_name) {
  int f = mdvtool_file_open(name);
  if(f < 0) {
    printf("File %s not found\n", name);
    return;
  }

  int size = mdvtool_file_size(f);
  printf("Exporting %d bytes to '%s' ... ", size, name);

  FILE *out = fopen(destination_name, "wb");
  if(!out) {
    printf("\nERROR: Unable to open %s for writing\n", name);
    return;
  }

  int block = 0;
  while(size) {
    sector_t *s = mdvtool_file_get_sector(f, block);
    if(!s) {
      printf("\nERROR: File %s is missing block %d\n", name, block);
      return;
    }

    // block 0 contains a file header which we need to skip
    int offset = (!block)?sizeof(file_t):0;
    int bytes2copy = (size>512-offset)?(512-offset):size;

    if(fwrite(s->data+offset, 1l, bytes2copy, out) != (size_t) bytes2copy) {
      printf("\nERROR: Writing %s\n", name);
      fclose(out);
      return;
    }
    block++;
    size -= bytes2copy;
  }

  fclose(out);
  printf("ok!\n");
}

void mdvtool_file_export(char *name) {
	mdvtool_file_export_dest(name,name);
}

int mdvtool_file_exists(int i) {
  return(mdvtool_files[i][0] != 0xff);
}

void mdv_files_list_chain(int f) {
  printf("Sectors: ");
  mdvtool_file_dump_chain(f);
}

void show_file_entry(file_t *f) {
  printf("%16s %5d %s, V:%x, U:%d, B:%d", f->name,
	 SWAP32(f->length), (f->type<=1)?(f->type?"EXEC":"DATA"):"????",
	 SWAP32(f->version), SWAP32(f->last_update), SWAP32(f->last_backup));

  if(f->type == 1)
    printf(" info: %d/%d", SWAP32(f->info[0]), SWAP32(f->info[1]));

  printf("\n");
}

void mdv_files_list_chains() {
  printf("File chains:\n");

  int f;
  for(f=0;f<256;f++) {
    sector_t *s = mdvtool_file_get_sector(f,0);
    if(s) {
      printf("=== file %d ===\n", f);
      if((f>0) && (f<128)) {
	printf("Directory entry: ");
	show_file_entry((file_t*)s->data);
      }
      mdv_files_list_chain(f);
    }
  }
}

void mdv_close() {
  if(mdv)
    fclose(mdv);
}

void mdv_show_sector_mapping() {
  int i;

  printf("File offset -> sector number\n");
  for(i=0;i<MDVTOOL_MAX_SECTORS;i++)
    if(mdvtool_sector_table[i] != 255)
      printf("%3d -> %3d\n", i, mdvtool_sector_table[i]);
}

void mdv_dir() {

  int entries = mdvtool_file_size(0)/sizeof(file_t);
  if(entries >= 1) {
    printf("DIR listing from directory file:\n");

    // scan all entries
    int j;
    for(j=1;j<entries;j++) {
      file_t *f = mdvtool_file_get_entry(j);
      if(f) show_file_entry(f);
    }
  } else
    printf("ERROR: directory file does not exist\n");

  // by scanning for files

  printf("DIR listing from file headers:\n");

  // valid files from 1 to 127
  int i;
  for(i=1;i<127;i++) {
    sector_t *s = mdvtool_file_get_sector(i, 0);
    if(s) show_file_entry((file_t*)s->data);
  }

  printf("Special:\n");
  for(i=128;i<255;i++) {
    if(mdvtool_file_exists(i)) {
      printf("<%02x> ", i);
      mdv_files_list_chain(i);
    }
  }
}

void mdvtool_mdvtool_file_export_all(char *dest_dir)
{
int entries = mdvtool_file_size(0)/sizeof(file_t);
  if(entries >= 1) {
    printf("Extracting list from directory file:\n");

    // scan all entries
    int j;
    for(j=1;j<entries;j++) {
      file_t *f = mdvtool_file_get_entry(j);
      if(f) {
	//show_file_entry(f);
	printf("%s\n",f->name);

	char finalpath[PATH_MAX];

  //Cambiar las _ por . del nombre
  char finalname[PATH_MAX];
  strcpy(finalname,f->name);
  util_string_replace_char(finalname,'_','.');

	sprintf(finalpath,"%s/%s",dest_dir,finalname);
  printf("Extracting %s to %s\n",finalname,finalpath);
	mdvtool_file_export_dest(f->name,finalpath);

	}
    }
  } else
    printf("ERROR: directory file does not exist\n");


}

void mdvtool_file_write(file_t *file, char *data) {
  printf("Writing file '%s' with %d bytes to mdv image ...\n",
	 file->name, SWAP32(file->length));

  // many programs have been modified to run from floppy
  // Change them to run from microdrive

  int i, replace = 0;
  for(i=0;(unsigned int) i<SWAP32(file->length)-5;i++) {
    if(memcmp(data+i, "flp1_", 5) == 0) {
      memcpy(data+i, "mdv1_", 5);
      replace++;
    }
  }

  if(replace)
    printf("!!!!INFO: Replaced %d occurances of flp1_ by mdv1_\n", replace);

  // check if file exists
  if(mdvtool_file_open(file->name) >= 0) {
    printf("file already exists!\n");
    return;
  }

  // search for a free directory entry
  int file_index = -1;
  int entries = mdvtool_file_size(0)/sizeof(file_t);

  // check if we need to extend the directory file
  if((entries & 7) == 7) {
    printf("ERROR: Directory file extension not supported yet\n");
    return;
  }

  // write directory entry
  file_index = entries;
  file_t *new_entry = mdvtool_file_get_entry(file_index);
  if(!new_entry) {
    fprintf(stderr, "ERROR: Locating new entry\n");
    return;
  }

  // write new entry
  memcpy(new_entry, file, sizeof(file_t));

  // update directory file length
  mdvtool_file_get_entry(0)->length = SWAP32((entries+1)*sizeof(file_t));

  //  printf("Using file %d\n", file_index);

  // write all sectors
  int last_block = 0;
  int block = 0;
  int size = SWAP32(file->length);
  while(size) {
    int blk_size = block?512:(512-sizeof(file_t));
    if(blk_size > size) blk_size = size;

    //    printf("Writing block %d with %d bytes\n", block, blk_size);

    // get a free block
    int i, s;
    for(i=0;i<MDVTOOL_MAX_SECTORS;i++) {
      s = last_block - 13 - i;
      if(s < 0) s += MDVTOOL_MAX_SECTORS;

      if((mdvtool_get_mapping_entry(s) &0xff00) == 0xfd00)
	break;
    }

    if(i == MDVTOOL_MAX_SECTORS) {
      printf("Image full\n");
      return;
    }

    // set new mapping entry
    buffer[0].sec.data[2*s] = file_index;
    buffer[0].sec.data[2*s+1] = block;
    mdvtool_files[file_index][block] = s;

    sector_t *sec = mdvtool_file_get_sector(file_index, block);

    // update mapping entry and fill sector
    if(!block) {
      memcpy(sec->data, file, sizeof(file_t));
      memcpy(sec->data + sizeof(file_t), data, blk_size);
    } else
      memcpy(sec->data, data, blk_size);

    // adjust headers
    //unsigned short phys = mdvtool_get_index(s);

    mdvtool_get_index(s);

    sec->file = file_index;
    sec->block = block;
    sec->bh_csum = sum(&sec->file, 2);

    block++;
    size -= blk_size;
    data += blk_size;

    last_block = s;
  }
}


void mdvtool_file_import(char *name) {
  FILE *in = fopen(name, "rb");
  if(!in) {
    fprintf(stderr, "Unable to open input file %s\n", name);
    return;
  }

  // check file size
  fseek(mdv, 0, SEEK_END);
  int size = ftell(in);
  fseek(mdv, 0, SEEK_SET);

  char *buffer = malloc(size);
  if(fread(buffer, 1, size, in) != (size_t) size) {
    perror("fread()");
    free(buffer);
    fclose(in);
    return;
  }

  fclose(in);

  file_t f;
  memset(&f, 0, sizeof(f));
  f.length = SWAP32(size);
  f.name_len = SWAP16(strlen(name));
  strcpy(f.name, name);

  mdvtool_file_write(&f, buffer);

  free(buffer);
}

void mdv_write(char *name) {
  printf("Writing mdv %s\n", name);

  // adjust checksums
  int i;
  for(i=0;i<MDVTOOL_MAX_SECTORS;i++) {
    if(buffer[i].hdr.ff == 0xff)
      buffer[i].sec.data_csum = sum(buffer[i].sec.data, 512);
  }

  FILE *out = fopen(name, "wb");
  if(!out) {
    fprintf(stderr, "Error opening output file %s\n", name);
    return;
  }

  if(fwrite(buffer, sizeof(mdv_entry_t), MDVTOOL_MAX_SECTORS, out) != MDVTOOL_MAX_SECTORS) {
    perror("fwrite()");
    fclose(out);
    return;
  }

  fclose(out);
}

void mdv_erase(void) {
  printf("Erasing MDV image ...\n");

  // mark all sectors as free
  int i;
  for(i=0;i<MDVTOOL_MAX_SECTORS;i++) {
    //unsigned short phys = mdvtool_get_index(i);

    mdvtool_get_index(i);

    // set new mapping entry

    int file = buffer[0].sec.data[2*i];
    int block = buffer[0].sec.data[2*i+1];
    sector_t *sec = mdvtool_file_get_sector(file, block);

    if(sec) {
      if(file) {
	//	printf("erasing file %d, block %d\n", file, block);

	mdvtool_files[file][block] = 0xff;
	buffer[0].sec.data[2*i] = 0xfd;
	buffer[0].sec.data[2*i+1] = 0x00;

	// adjust headers
	sec->file = 0xfd;
	sec->block = 0x00;
	sec->bh_csum = sum(&sec->file, 2);
      } else {
	// leave directory intact
	//	printf("keeping dir block %d\n", block);

	// but erase it
	memset(sec->data, 0, 512);
      }
    }
  }

  // finally set the directory file length to 1 (dir file only)
  mdvtool_file_get_entry(0)->length = SWAP32(1*sizeof(file_t));
}

z80_int mdv_get_random(void)
{
  ay_randomize(0);

  //randomize_noise es valor de 16 bits
  return randomize_noise[0];
}


void mdv_rename(char *name) {
  char lname[11];
  strncpy(lname, name, 11);

  // expand name to 10 characters
  lname[10] = 0;
  while(strlen(lname) < 10)
    lname[strlen(lname)] = ' ';

  printf("Setting name: '%s'\n", lname);

  int i;
  unsigned short rnd = mdv_get_random();
  for(i=0;i<MDVTOOL_MAX_SECTORS;i++) {
    if(buffer[i].hdr.ff == 0xff) {
      memcpy(buffer[i].hdr.name, lname, 10);
      buffer[i].hdr.rnd = rnd;
      buffer[i].hdr.csum = sum(&buffer[i].hdr.ff, 14);
    }
  }
}

int main_mdvtool(int argc, char **argv) {

  if(argc < 3) {
    printf("Usage: mdvtool <mdv> commands\n");
    printf("Commands:\n");
    printf("   dir                  - list MDV contents\n");
    printf("   check_files          - check file integrity\n");
    printf("   file_chains          - list chain of sectors for each file\n");
    printf("   check_mapping        - check the sector mapping\n");
    printf("   show_mapping         - show physical/loginal sector mapping\n");
    printf("   export file_name     - export a file from the MDV image\n");
    printf("   export_all dest_dir  - export all files from the MDV image\n");
    printf("   erase                - erase the MDV image\n");
    printf("   name image_name      - rename the MDV image\n");
    printf("   import file_name     - import a file to the MDV image\n");
    printf("   write file_name      - write the MDV image\n");
    return 0;
  }

  assert(sizeof(hdr_t) == 28);
  assert(sizeof(sector_t) == 658);
  assert(sizeof(file_t) == 64);

    //enviamos tambien directorio destino
  if(mdv_load(argv[1],argv[3]) < 0) {
    mdv_close();
    return -1;
  }

  int c = 2;
  while(c < argc) {
    puts("");

    if(!strcasecmp(argv[c], "dir"))
      mdv_dir();

    else if(!strcasecmp(argv[c], "export")) {
      if(++c >= argc) {
	printf("export needs a file name as parameter\n");
	return 0;
      }

      mdvtool_file_export(argv[c]);
    }

    else if(!strcasecmp(argv[c], "export_all")) {
	if(++c >= argc) {
        printf("export_all needs a path name as parameter\n");
        return 0;
      }

      mdvtool_mdvtool_file_export_all(argv[c]);
    }


    else if(!strcasecmp(argv[c], "import")) {
      if(++c >= argc) {
	printf("import needs a file name as parameter\n");
	return 0;
      }

      mdvtool_file_import(argv[c]);
    }

    else if(!strcasecmp(argv[c], "name")) {
      if(++c >= argc) {
	printf("name needs an image name as parameter\n");
	return 0;
      }

      mdv_rename(argv[c]);
    }


    else if(!strcasecmp(argv[c], "write")) {
      if(++c >= argc) {
	printf("write needs a file name as parameter\n");
	return 0;
      }

      mdv_write(argv[c]);
    }

    else if(!strcasecmp(argv[c], "check_files"))
      mdv_files_check();

    else if(!strcasecmp(argv[c], "erase"))
      mdv_erase();

    else if(!strcasecmp(argv[c], "file_chains"))
      mdv_files_list_chains();

    else if(!strcasecmp(argv[c], "check_mapping"))
      mdv_check_mapping();

    else if(!strcasecmp(argv[c], "show_mapping"))
      mdv_show_sector_mapping();

    else
      printf("Unknown command %s\n", argv[c]);

    c++;
  }

  mdv_close();

  return 0;
}
