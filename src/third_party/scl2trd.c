/*
 The MIT License (MIT)

Copyright (c) 2019 Alexander Sharikhin

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), 
to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
IN THE SOFTWARE.
*/

//https://github.com/nihirash/esxdos-scl2trd

//More info:
//http://www.zx-modules.de/fileformats/sclformat.html

//#include <input.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "debug.h"

//Para usar definiciones de PATH_MAX
#include "utils.h"
#include "zvfs.h"

//#include "lib/textUtils.h"
//#include "lib/esxdos.h"

//#define PATH_SIZE ( 200 )
//uint8_t filePath[ PATH_SIZE ];

char scl_inputfile[PATH_MAX];
char scl_outputfile[PATH_MAX];


//uint16_t drive;
FILE *iStream;
FILE *oStream;
z80_byte buff[256];
unsigned freeTrack = 1;
unsigned freeSec = 0;
unsigned char count;
unsigned char isFull = 0;
int totalFreeSect = 2544;


//Soporte para FatFS
FIL scl2trd_fil_input;        
int scl2trd_in_fatfs_input;

FIL scl2trd_fil_output;        
int scl2trd_in_fatfs_output;


size_t scl2trd_fread(void *ptr, size_t nitems)
{
    //return fread(ptr,1,nitems,iStream);

    return zvfs_fread(scl2trd_in_fatfs_input,ptr,nitems,iStream,&scl2trd_fil_input);
}

size_t scl2trd_fwrite(const void *ptr, size_t nitems)
{
    //return fwrite(ptr,1,nitems,oStream);

    return zvfs_fwrite(scl2trd_in_fatfs_output,(z80_byte *)ptr,nitems,oStream,&scl2trd_fil_output);
}

void cleanBuffer()
{
  int i;
    for (i=0;i<256;buff[i++] = 0); 
}

void showMessage(char *e) 
{
    debug_printf(VERBOSE_ERR,e);
} 

void writeDiskData()
{

    int r = scl2trd_fread(&buff,256);
    while (r == 256) {

        scl2trd_fwrite(&buff,r);

        r = scl2trd_fread(&buff,256);
    }
    
    if (isFull) {
        cleanBuffer();
        for (r=0;r<totalFreeSect;r++)

            scl2trd_fwrite(&buff,256);
    }
    
    zvfs_fclose(scl2trd_in_fatfs_input,iStream,&scl2trd_fil_input);
    //fclose(iStream);

    zvfs_fclose(scl2trd_in_fatfs_output,oStream,&scl2trd_fil_output);
    //fclose(oStream);

   debug_printf (VERBOSE_INFO,"All scl to trd data written");
}

void writeDiskInfo()
{
    cleanBuffer();
    buff[0xe3] = 0x16; // IMPORTANT! 80 track double sided
    buff[0xe4] = count;
    buff[0xe1] = freeSec;
    buff[0xe2] = freeTrack;
    
    if (isFull) {
        buff[0xe6] = totalFreeSect / 256;
        buff[0xe5] = totalFreeSect & 255;
    }

    buff[0xe7] = 0x10;
    buff[0xf5] = 's';
    buff[0xf6] = 'c';
    buff[0xf7] = 'l';
    buff[0xf8] = '2';
    buff[0xf9] = 't';
    buff[0xfa] = 'r';
    buff[0xfb] = 'd';

    scl2trd_fwrite(&buff,256);

    char dirt_data[1792];
    scl2trd_fwrite(&dirt_data,1792); // Any dirt is ok


    writeDiskData();
}

void writeCatalog()
{
    int i;
    totalFreeSect = 2544;
    freeTrack = 1;
    freeSec = 0;
    count = 0;

    if (zvfs_fopen_write(scl_outputfile,&scl2trd_in_fatfs_output,&oStream,&scl2trd_fil_output)<0) {
        showMessage("Can't open output file");
        return ;
    }    
    
    /*
    oStream = fopen(scl_outputfile,"wb");
    if  (oStream==NULL) {
        showMessage("Can't open output file");
        return ;
    }
    */

    scl2trd_fread(&count,1);
    for (i=0;i<count; i++) {

        scl2trd_fread(&buff,14);
        buff[14] = freeSec;
        buff[15] = freeTrack;
        freeSec += buff[0xd];
        freeTrack += freeSec / 16;
        totalFreeSect -= (int) buff[0xd];
        freeSec = freeSec % 16;

        scl2trd_fwrite(&buff,16);
    }
    cleanBuffer();

    for (i = count;i<128;i++) {

        scl2trd_fwrite(&buff,16);
    }

    writeDiskInfo();
}


void validateScl()
{
    char *expected = "SINCLAIR";

    if (zvfs_fopen_read(scl_inputfile,&scl2trd_in_fatfs_input,&iStream,&scl2trd_fil_input)<0) {
        showMessage("Can't open input file");
        return;
    }    

    /*
    iStream = fopen(scl_inputfile,"rb");
    if (iStream==NULL) {
        showMessage("Can't open input file");
        return ;
    }
    */

    cleanBuffer();

    scl2trd_fread(&buff,8);
    if (strcmp(expected, (char *)&buff)) {
        showMessage("Wrong file! Select only SCL files");
        return;
    }
    //sprintf(strstr(filePath, ".SCL"), ".TRD");
    //textUtils_println(" * File is valid SCL");
    writeCatalog();
}


void scl2trd_main(char *input,char *output)
{
    strcpy(scl_inputfile,input);
    strcpy(scl_outputfile,output);
    validateScl();
}
