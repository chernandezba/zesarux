/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>


//Para evitar warnings
#ifndef GCC_UNUSED

#ifdef __GNUC__
#  define GCC_UNUSED __attribute__((unused))
#else
#  define GCC_UNUSED
#endif

#endif


/* Definitions of physical drive number for each drive */
#define DEV_MMC		0	/* Example: Map MMC/SD card to physical drive 0 */
#define DEV_RAM		1	/* Example: Map Ramdisk to physical drive 1 */
#define DEV_USB		2	/* Example: Map USB MSD to physical drive 2 */


FILE *ptr_fatfs_disk_zero_file;

//Puntero que apunta a la memoria donde hemos leido nuestro archivo 
BYTE *fatfs_disk_zero_memory=NULL;

//Tamanyo del archivo
long long int fatfs_disk_zero_tamanyo=0;

int debug_diskio=0;

int diskio_syncing_flag=0;

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	//DSTATUS stat;
	//int result;

    if (debug_diskio) printf("FatFs llamado disk status para physical drive: %d\n",pdrv);

	switch (pdrv) {
    /*
	case DEV_RAM :
		//result = RAM_disk_status();

		// translate the reslut code here

		return stat;
    */
	case DEV_MMC :
		//result = MMC_disk_status();

		// translate the result code here

        //TODO: de momento ok
        if (debug_diskio) printf("FatFs llamado disk status mmc\n");
        return 0;
    break;


    /*
	case DEV_USB :
		//result = USB_disk_status();

		// translate the reslut code here

		return stat;
    */
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	//DSTATUS stat;
	//int result;
    
    if (debug_diskio) printf("FatFs llamado disk_initialize para drive %d\n",pdrv);

	switch (pdrv) {
    /*
	case DEV_RAM :
		//result = RAM_disk_initialize();

		// translate the reslut code here

		return stat;
    */
	case DEV_MMC :
		//result = MMC_disk_initialize();

		// translate the reslut code here

        //Abrimos el archivo

        ptr_fatfs_disk_zero_file=fopen(fatfs_disk_zero_path,"rb");

        if (ptr_fatfs_disk_zero_file==NULL) {
            if (debug_diskio) printf("FatFs error abriendo archivo %s\n",fatfs_disk_zero_path);
            return STA_NOINIT;
        }


        //Tamanyo del archivo
        fatfs_disk_zero_tamanyo=get_file_size(fatfs_disk_zero_path);

        if (fatfs_disk_zero_tamanyo<=0) {
            if (debug_diskio) printf("FatFs error leyendo longitud archivo %s\n",fatfs_disk_zero_path);
            return STA_NOINIT;
        }

        //asignar memoria. Liberar si existia antes
        if (fatfs_disk_zero_memory!=NULL) {
            if (debug_diskio) printf("FatFs freeing previous ram cache\n");
            free(fatfs_disk_zero_memory);
        }
        fatfs_disk_zero_memory=malloc(fatfs_disk_zero_tamanyo);

        if (fatfs_disk_zero_memory==NULL) {
            if (debug_diskio) printf("FatFs error asignando memoria para archivo %s\n",fatfs_disk_zero_path);
            return STA_NOINIT;            
        }

        //Y leerlo entero
        long long int leidos=fread(fatfs_disk_zero_memory,1,fatfs_disk_zero_tamanyo,ptr_fatfs_disk_zero_file);

        if (leidos<fatfs_disk_zero_tamanyo) {
            if (debug_diskio) printf("FatFs error leyendo archivo %s en memoria\n",fatfs_disk_zero_path);
            return STA_NOINIT;               
        }

        //Y ya se puede cerrar
        fclose(ptr_fatfs_disk_zero_file);

            
		return 0;
    break;

    /*
    case DEV_USB :
		//result = USB_disk_initialize();

		// translate the reslut code here

		return stat;
    */
	}
    
	return STA_NOINIT;
}

//leer un byte del archivo mmc (en memoria) controlando offsets
BYTE diskio_lee_byte(long long int posicion)
{
    if (posicion>=fatfs_disk_zero_tamanyo || posicion<0) {
        if (debug_diskio) printf("FatFs error reading beyond mmc size (total %lld, trying %lld)\n",fatfs_disk_zero_tamanyo,posicion);
        return 0;
    }

    else {
        return fatfs_disk_zero_memory[posicion];
    }
}


//escribir un byte en el archivo mmc (en memoria) controlando offsets
void diskio_escribe_byte(long long int posicion,BYTE valor)
{
    if (posicion>=fatfs_disk_zero_tamanyo || posicion<0) {
        if (debug_diskio) printf("FatFs error writing beyond mmc size (total %lld, trying %lld)\n",fatfs_disk_zero_tamanyo,posicion);
        return;
    }

    else {
        //printf("Escribiendo byte %d (%c)\n",valor,
        //    (valor>=32 && valor<=127 ? valor : '?'));

        fatfs_disk_zero_memory[posicion]=valor;
    }
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	//DRESULT res;
	//int result;

    long long int offset;

    if (debug_diskio) printf("FatFs llamado disk_read para drive %d\n",pdrv);

	switch (pdrv) {
        /*
	case DEV_RAM :
		// translate the arguments here

		//result = RAM_disk_read(buff, sector, count);

		// translate the reslut code here

		return res;
        */

	case DEV_MMC :
		// translate the arguments here

		//result = MMC_disk_read(buff, sector, count);

		// translate the reslut code here

        //TODO: no se si realmente seria leer FF_MIN_SS o FF_MAX_SS

        offset=sector*FF_MIN_SS;

        long long int total_leer=count*FF_MIN_SS;

        for (;total_leer>0;total_leer--) {
            *buff=diskio_lee_byte(offset);

            buff++;
            offset++;
        }


		return RES_OK;
    break;

    /*

	case DEV_USB :
		// translate the arguments here

		//result = USB_disk_read(buff, sector, count);

		// translate the reslut code here

		return res;
    */

	}

	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	//DRESULT res;
	//int result;

    long long int offset;   

    if (debug_diskio) printf("FatFs llamado disk_write para drive %d sector %d count %d\n",pdrv,sector,count);

	switch (pdrv) {
        /*
	case DEV_RAM :
		// translate the arguments here

		//result = RAM_disk_write(buff, sector, count);

		// translate the reslut code here

		return res;
        */

	case DEV_MMC :
		// translate the arguments here

		//result = MMC_disk_write(buff, sector, count);

		// translate the reslut code here

        //TODO: no se si realmente seria leer FF_MIN_SS o FF_MAX_SS

        offset=sector*FF_MIN_SS;

        long long int total_leer=count*FF_MIN_SS;

        for (;total_leer>0;total_leer--) {
            diskio_escribe_byte(offset,*buff);

            buff++;
            offset++;
        }


		return RES_OK;        

    break;

    /*
	case DEV_USB :
		// translate the arguments here

		//result = USB_disk_write(buff, sector, count);

		// translate the reslut code here

		return res;
    */
	}

	return RES_PARERR;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff GCC_UNUSED		/* Buffer to send/receive control data */
)
{
	//DRESULT res;
	//int result;

    if (debug_diskio) printf("FatFs llamado disk_ioctl para drive %d\n",pdrv);

	switch (pdrv) {
        /*
	case DEV_RAM :

		// Process of the command for the RAM drive

		return res;
        */

	case DEV_MMC :

		// Process of the command for the MMC/SD card

        //TODO. sync por ejemplo, para hacer flush a filesystem.
        switch(cmd) {
            case CTRL_SYNC:
                //if (debug_diskio) printf("FatFs llamado disk_ioctl CTRL_SYNC\n");
            break;
        } 
        return RES_OK;

	break;

    /*

	case DEV_USB :

		// Process of the command the USB drive

		return res;
    */
	}

	return RES_PARERR;
}



DWORD get_fattime (void)
{
    /*
Return Value
Currnet local time shall be returned as bit-fields packed into a DWORD value. The bit fields are as follows:
bit31:25
Year origin from the 1980 (0..127, e.g. 37 for 2017)

bit24:21
Month (1..12)

bit20:16
Day of the month (1..31)

bit15:11
Hour (0..23)

bit10:5
Minute (0..59)

bit4:0
Second / 2 (0..29, e.g. 25 for 50)    
    */

    //fecha grabacion
    time_t tiempo = time(NULL);
    struct tm tm = *localtime(&tiempo);

    //printf("now: %d-%d-%d %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);


    return (tm.tm_sec) | (tm.tm_min<<5) | (tm.tm_hour<<11) | (tm.tm_mday<<16) | ((tm.tm_mon+1)<<21) | ((tm.tm_year-80)<<25); 
}

//Flush cambios a disco
/*
Solo permito hacerlo mediante esta llamada, que se puede invocar desde menu,
pero no permito mediante disk_ioctl comando CTRL_SYNC
Asi el usuario puede jugar con cambios en la imagen fat, sin peligro, hasta que decide aplicar los cambios (si es que quiere)
*/
int diskio_sync(void)
{
    //printf("Flushing changes to FatFS image %s\n",fatfs_disk_zero_path);



        ptr_fatfs_disk_zero_file=fopen(fatfs_disk_zero_path,"wb");

        if (ptr_fatfs_disk_zero_file==NULL) {
            if (debug_diskio) printf("FatFs error abriendo archivo %s\n",fatfs_disk_zero_path);
            return 1;
        }

    //Activamos un flag para que se pueda consultar desde menu
    diskio_syncing_flag=1;

        //Escribirlo entero
        fwrite(fatfs_disk_zero_memory,1,fatfs_disk_zero_tamanyo,ptr_fatfs_disk_zero_file);

 
        //Y ya se puede cerrar
        fclose(ptr_fatfs_disk_zero_file);

        //sleep(3);

    diskio_syncing_flag=0;        

        return 0;

}