#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char z80_byte;

long int tamanyo_archivo;
z80_byte *hilow_memoria;
z80_byte *hilow_ddh;

#define HILOW_SECTOR_SIZE 2048
//#define HILOW_SECTOR_SIZE 1024

/*
una cinta recien formateada acaba los ultimos bytes "útiles" (los de la tabla de sectores libres) hacia la dirección 4E8 hexadecimal
por tanto podria definir el tamaño total de directorio como 500 hexadecimal
*/
#define HILOW_DIRECTORY_TABLE_SIZE 0x500

//Deduzco por la tabla de sectores libre y como se modifica que el sector mayor es F5H (245)
#define HILOW_MAX_SECTORS 246
//#define HILOW_MAX_SECTORS 256

#define HILOW_DEVICE_SIZE (HILOW_SECTOR_SIZE*HILOW_MAX_SECTORS)

#define HILOW_MAX_SECTORS_PER_FILE 25

#define HILOW_MAX_FILES_DIRECTORY 22

//Lo que ocupa cada entrada de directorio
#define HILOW_DIRECTORY_ENTRY_SIZE 45

int lee_byte_memoria(int posicion)
{
    if (posicion<0 || posicion>=tamanyo_archivo) {
        //TODO: mejorar esto, no finalizar sino retornar fin de memoria a la rutina que llama
        printf("fuera de rango %d\n",posicion);
        return -1;
    }

    return hilow_memoria[posicion];
}

int filtro_ruido=2;

//Dice la duracion de una onda, asumiendo:
//subimos - bajamos - y empezamos a subir
int new_duracion_onda(int posicion,int *duracion_flanco_bajada)
{
    z80_byte valor_anterior=lee_byte_memoria(posicion);
    int direccion=+1;

    int salir=0;
    int duracion=0;

    *duracion_flanco_bajada=0;

    do {
        //printf("%d ",direccion);
        int valor_leido=lee_byte_memoria(posicion);
        printf("V(%d)%d ",direccion,valor_leido);
        if (valor_leido==-1) {
            //fin de archivo
            return -1;
        }

        //si variacion es muy poca, no actuar
        int delta=valor_leido-valor_anterior;

        int nosigndelta=delta;
        if (nosigndelta<0) nosigndelta=-nosigndelta;

        //temp 
        nosigndelta=99; 

        if (nosigndelta<filtro_ruido) {
            //nada
        }
        
        else {
            if (direccion==+1) {
                //subimos. vemos si bajamos
                if (delta<0) direccion=-1;
            }
            else {
                //bajamos. ver si subimos y por tanto finalizamos
                if (delta>0) {
                    //printf("\n");
                    return duracion;
                }

                
            }

            valor_anterior=valor_leido;

        }

        if (direccion==-1) (*duracion_flanco_bajada)++;
        duracion++;
        posicion++;

    } while (!salir);
}

int util_get_absolute(int valor)
{
        if (valor<0) valor=-valor;

        return valor;
}

//Dice la duracion de una onda, asumiendo:
//subimos - bajamos - y empezamos a subir
int duracion_onda(int posicion,int *duracion_flanco_bajada)
{
    z80_byte valor_anterior=lee_byte_memoria(posicion);
    int direccion=+1;

    int salir=0;
    int duracion=0;

    *duracion_flanco_bajada=0;

    do {
        //printf("%d ",direccion);
        int valor_leido=lee_byte_memoria(posicion);
        //printf("V%d ",valor_leido);
        if (valor_leido==-1) {
            //fin de archivo
            return -1;
        }


        if (direccion==+1) {
            //subimos. vemos si bajamos
            if (valor_leido<valor_anterior) {
                //bajamos
                if (valor_leido+filtro_ruido<valor_anterior) {
                    //Baja lo suficiente que cambia direccion
                    direccion=-1;
                    valor_anterior=valor_leido;
                }

                else {
                    //No baja lo suficiente
                    //valor_anterior=valor_leido;
                }
            }

            else {
                //subimos
                valor_anterior=valor_leido;
            }
        }
        else {
            //bajamos. ver si subimos y por tanto finalizamos
            if (valor_leido>valor_anterior) {
                //subimos
                if (valor_leido-filtro_ruido>valor_anterior) {
                    //Sube lo suficiente que cambia direccion
                    //printf("\n");
                    return duracion;
                }
                else {
                    //No sube lo suficiente
                    valor_anterior=valor_leido;
                }


            }
            else {
                //Bajamos
                valor_anterior=valor_leido;
            }

            (*duracion_flanco_bajada)++;
        }

        
        duracion++;
        posicion++;

    } while (!salir);
}

//a 44100 Hz
#define LONGITUD_ONDA_INICIO_BITS 367
#define LONGITUD_ONDA_INICIO_BITS_FLANCO_BAJADA 180
#define LONGITUD_ONDA_INICIO_BITS_MARGEN 40
int buscar_onda_inicio_bits(int posicion)
{
    int salir=0;

    int duracion_flanco_bajada;

    do {

        int duracion=duracion_onda(posicion,&duracion_flanco_bajada);

        //printf("duracion %d flanco bajada %d pos_antes %d\n",duracion,duracion_flanco_bajada,posicion);

        if (duracion_flanco_bajada>=LONGITUD_ONDA_INICIO_BITS_FLANCO_BAJADA-LONGITUD_ONDA_INICIO_BITS_MARGEN &&
            duracion_flanco_bajada<=LONGITUD_ONDA_INICIO_BITS_FLANCO_BAJADA+LONGITUD_ONDA_INICIO_BITS_MARGEN) {
                //TODO: puede ser -1??
                return posicion+duracion;
            }

        posicion +=duracion;
        
    } while (!salir && posicion!=-1);
}

int buscar_dos_sync_bits(int posicion)
{
    
    
    //Buscar dos marcas consecutivas primero

    do {
    
    printf("1 posicion %d\n",posicion);
    posicion=buscar_onda_inicio_bits(posicion);
    if (posicion==-1) return -1;
    //Estamos al final de la primera
    

    int posicion0=posicion;

    printf("2 posicion %d\n",posicion);
    //sleep(5);

    //TODO: la segunda no se detecta bien. asumir posicion
    //no la detecta por variaciones muy tenues en la segunda onda
    printf("final posicion %d\n",posicion+LONGITUD_ONDA_INICIO_BITS);
    return posicion+LONGITUD_ONDA_INICIO_BITS;

    posicion=buscar_onda_inicio_bits(posicion);
    if (posicion==-1) return -1;


    printf("3 posicion %d\n",posicion);
    sleep(5);

    //Estamos al final de la segunda

    

    //Ver si la segunda acaba en donde acaba la primera + el tiempo de onda
    int delta=posicion-posicion0;

    printf("delta %d esperado %d\n",delta,LONGITUD_ONDA_INICIO_BITS);

    sleep(5);

    if (delta>=LONGITUD_ONDA_INICIO_BITS-LONGITUD_ONDA_INICIO_BITS_MARGEN &&
            delta<=LONGITUD_ONDA_INICIO_BITS+LONGITUD_ONDA_INICIO_BITS_MARGEN) {
                printf("Dos sync consecutivos en %d\n",posicion);
                //sleep(5);
                return posicion;
            }

    } while (posicion!=-1);
}

int buscar_inicio_sector(int posicion)
{
    //Buscar 3 veces las dos marcas consecutivas de inicio de bits
    int i;

    for (i=0;i<3;i++) {
        posicion=buscar_dos_sync_bits(posicion);
        if (posicion==-1) return -1;
    }
    //sleep(2);

    return posicion;
}

int esperar_inicio_sincronismo(int posicion)
{

    z80_byte valor_anterior=lee_byte_memoria(posicion);
    do {
        //printf("%d ",direccion);
        int valor_leido=lee_byte_memoria(posicion);
        if (valor_leido==-1) {
            //fin de archivo
            return -1;
        }
        int delta=valor_leido-valor_anterior;
        if (delta<0) delta=-delta;

        if (delta>1) return posicion;

        valor_anterior=valor_leido;
        posicion++;
    } while(1);
}

//Retorna posicion
int lee_byte(int posicion,z80_byte *byte_salida)
{
    //Averiguar primero duracion pulso
    /*

    Marca inicio byte: 71 samples. Flanco bajada: 40
    Bit a 1: 56 samples -> 0.788 de la marca. 1.4 del flanco de bajada
             28 samples flanco bajada. 0.7 del flanco de bajada

    Bit a 0: 37 samples -> 0.521 de la marca. 0.925 del flanco de bajada
             11 samples flanco bajada. 0.275 del flanco de bajada
    */

   //Saltar zona de señal plana hasta que realmente empieza el sincronismo
   /*printf("pos antes inicio sync: %d\n",posicion);
   posicion=esperar_inicio_sincronismo(posicion);
   printf("pos en inicio sync: %d\n",posicion);

   if (posicion==-1) {
       //fin
       return -1;       
   }
   */

   z80_byte byte_final=0;

    int duracion_flanco_bajada;
   int duracion_sincronismo_byte=duracion_onda(posicion,&duracion_flanco_bajada);

   if (duracion_sincronismo_byte==-1) {
       //fin
       return -1;
   }
   posicion +=duracion_sincronismo_byte;

   //int duracion_uno=(duracion_sincronismo_byte*79)/100;
   //int duracion_cero=(duracion_sincronismo_byte*40)/100;

   //int duracion_uno=(duracion_flanco_bajada*140)/100;
   //int duracion_cero=(duracion_flanco_bajada*92)/100;   

   int duracion_uno=(duracion_flanco_bajada*70)/100;
   int duracion_cero=(duracion_flanco_bajada*27)/100;   


   //int duracion_uno=(duracion_sincronismo_byte*80)/100;
   //int duracion_cero=(duracion_sincronismo_byte*40)/100;
   //int duracion_cero=duracion_uno/2;



   int diferencia_cero_uno=duracion_uno-duracion_cero;

   int dif_umbral=(diferencia_cero_uno)/2;
   

   //Umbrales entre uno y otro
   int umbral_cero_uno=duracion_cero+dif_umbral;

   printf("Sync %d Bajada %d Zero %d One %d Umbral %d\n",duracion_sincronismo_byte,duracion_flanco_bajada,duracion_cero,duracion_uno,umbral_cero_uno);

   int i;

   for (i=0;i<8;i++) {
       int duracion_flanco_bajada;
       int duracion_bit=duracion_onda(posicion,&duracion_flanco_bajada);
       //printf("L%d ",duracion_bit);
       printf("L%d ",duracion_flanco_bajada);
       if (duracion_bit==-1) {
           //fin
           *byte_salida=byte_final;
           printf("\n");
           return posicion;
       }
       posicion +=duracion_bit;

        //if (duracion_bit<umbral_cero_uno) {
        if (duracion_flanco_bajada<umbral_cero_uno) {
            //Es un 0
            printf(" -0- ");
        }
        else {
            //Es un 1
            byte_final |=1;
            printf(" -1- ");
        }
        printf("\n");
       
       if (i!=7) byte_final=byte_final<<1;
    }

    printf("final: (%d) \n",byte_final);
   *byte_salida=byte_final;
   return posicion;
}

z80_byte buffer_result[HILOW_SECTOR_SIZE+1];


void lee_sector(int posicion)
{
    int total=HILOW_SECTOR_SIZE+1; //2049; //2049; //byte de numero de sector + 2048 del sector
    //int posicion=0;

    int i;

    
    for (i=0;i<total && posicion!=-1;i++) {
        printf("\nPos %d %d\n",i,posicion);
        z80_byte byte_leido;

        posicion=lee_byte(posicion,&byte_leido);
        if (posicion!=-1) {
            printf("Byte leido: %d (%02XH) (%c)\n",byte_leido,byte_leido,(byte_leido>=32 && byte_leido<=126 ? byte_leido : '.') );
        }

        buffer_result[i]=byte_leido;
    }

    //dump total ascii+hexa
    int colwidth=40;

    int sector=buffer_result[0];

    printf("Sector: %d\n",sector);

    for (i=1;i<total /*&& posicion!=-1*/;i+=colwidth) {
        int col;

        printf("%08X ",i-1);

        for (col=0;col<colwidth && i+col<HILOW_SECTOR_SIZE+1;col++) {
            z80_byte byte_leido=buffer_result[i+col];

            printf("%02X",byte_leido);
        }        

        printf(" ");

        for (col=0;col<colwidth && i+col<HILOW_SECTOR_SIZE+1;col++) {

            z80_byte byte_leido=buffer_result[i+col];

            printf("%c",(byte_leido>=32 && byte_leido<=126 ? byte_leido : '.'));
        }

        printf("\n");

    }    

    

    printf("\n");

    printf("Correcto? (s/n)");

    char buffer_pregunta[100];

    scanf("%s",buffer_pregunta);

    if (buffer_pregunta[0]!='s') {
        printf("Aborting\n");
        return;
    }


    //TODO: en emulador usamos sector 0 y 1 para directorio, aunque parece que en real es 1 y 2
    if (sector==2 || sector==1) sector--;

    //Copiar a memoria ddh
    int offset_destino=sector*HILOW_SECTOR_SIZE;    

    printf("offset_destino: %d\n",offset_destino);

    printf("puntero: %p\n",hilow_ddh);
    /*for (i=0;i<HILOW_SECTOR_SIZE;i++) {
        printf("%d\n",i);
        hilow_ddh[offset_destino+i]=buffer_result[i+1];
    }*/

    //por si acaso sector fuera de rango
    if (sector<HILOW_MAX_SECTORS) {
        memcpy(&hilow_ddh[offset_destino],&buffer_result[1],HILOW_SECTOR_SIZE);
    }

}

long int get_file_size(char *nombre)
{
        struct stat buf_stat;

                if (stat(nombre, &buf_stat)!=0) {
                        printf("Unable to get status of file %s\n",nombre);
                        return 0;
                }

                else {
                        //printf ("file size: %ld\n",buf_stat.st_size);
                        return buf_stat.st_size;
                }
}


z80_byte *read_hilow_audio_file(char *archivo)
{
    z80_byte *puntero;


   

    //Asignar memoria
    int tamanyo=get_file_size(archivo);
    puntero=malloc(tamanyo);

    if (puntero==NULL) {
        printf("Can not allocate memory for hilow audio file");
        return NULL;
    }


    //cargarlo en memoria
    FILE *ptr_bmpfile;
    ptr_bmpfile=fopen(archivo,"rb");

    if (!ptr_bmpfile) {
            printf("Unable to open bmp file %s\n",archivo);
            return NULL;
    }

    fread(puntero,1,tamanyo,ptr_bmpfile);
    fclose(ptr_bmpfile);



    return puntero;
}


void *read_hilow_ddh_file(char *archivo)
{
    //z80_byte *puntero;


    //Leer archivo ddh
    //Asignar memoria
    int tamanyo=HILOW_DEVICE_SIZE;
    hilow_ddh=malloc(tamanyo);

    if (hilow_ddh==NULL) {
        printf("Can not allocate memory for hilow ddh file");
        return NULL;
    }


    //cargarlo en memoria
    FILE *ptr_ddhfile;
    ptr_ddhfile=fopen(archivo,"rb");

    if (!ptr_ddhfile) {
            printf("Unable to open ddh file %s\n",archivo);
            return NULL;
    }

    fread(hilow_ddh,1,tamanyo,ptr_ddhfile);
    fclose(ptr_ddhfile);    

 

}

void *write_hilow_ddh_file(char *archivo)
{
    z80_byte *puntero;

    int tamanyo=HILOW_DEVICE_SIZE;


    FILE *ptr_ddhfile;
    ptr_ddhfile=fopen(archivo,"wb");

    if (!ptr_ddhfile) {
            printf("Unable to open ddh file %s\n",archivo);
            return NULL;
    }

    fwrite(hilow_ddh,1,tamanyo,ptr_ddhfile);
    fclose(ptr_ddhfile);    

}


int main(int argc,char *argv[])
{
    

    char *archivo;

    //44100hz, unsigned 8 bits

    archivo=argv[1];


    char *archivo_ddh;
    archivo_ddh=argv[2];

    tamanyo_archivo=get_file_size(archivo);


    hilow_memoria=read_hilow_audio_file(archivo);

    read_hilow_ddh_file(archivo_ddh);
    printf("puntero: %p\n",hilow_ddh);
    //sleep(2);

    int posicion=0;

    while (1) {
    
    posicion=buscar_inicio_sector(posicion);

    printf("Posicion inicio bits: %d\n",posicion);

    //sleep(5);
    

    lee_sector(posicion);

    write_hilow_ddh_file(archivo_ddh);

    }


    free(hilow_memoria);

    free(hilow_ddh);


    return 0;
}