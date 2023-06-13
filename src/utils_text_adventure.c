/*
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

/*
   Utilities functions for Text Adventure Games
*/

//para strcasestr
#ifndef _GNU_SOURCE
    #define _GNU_SOURCE
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifndef MINGW
	//Para waitpid
	#include <sys/wait.h>
#endif




//Para spawn en windows
#ifdef MINGW
	#include <process.h>


#endif


#include "utils_text_adventure.h"
#include "cpu.h"
#include "utils.h"
#include "debug.h"
#include "compileoptions.h"
#include "operaciones.h"
#include "cpc.h"
#include "screen.h"
#include "menu_items.h"

z80_byte daad_peek(z80_int dir)
{

        if (MACHINE_IS_CPC) {
                z80_byte *start=cpc_ram_mem_table[0];
                z80_byte *p=&start[dir];
                return *p;
        }


        //Spectrum
        else {
                return peek_byte_no_time(dir);
        }
}


z80_int daad_peek_word(z80_int dir)
{

        z80_byte low,high;
        low=daad_peek(dir);
        dir++;
        high=daad_peek(dir);

        return value_8_to_16(high,low);
}

void daad_poke(z80_int dir,z80_byte value)
{

        if (MACHINE_IS_CPC) {
                z80_byte *start=cpc_ram_mem_table[0];
                z80_byte *p=&start[dir];
                *p=value;
        }

        //Spectrum
        else {
                poke_byte_no_time(dir,value);
        }
}

//Usado en unpaws, ungac etc para agregar letra de hotkey automaticamente
#define TOTAL_UNPAWSGAC_HOTKEYS 26
int util_unpawsgac_hotkeys[TOTAL_UNPAWSGAC_HOTKEYS]; //De la A a la Z

void util_init_unpawsgac_hotkeys(void)
{
        int i;

        for (i=0;i<TOTAL_UNPAWSGAC_HOTKEYS;i++) {
             util_unpawsgac_hotkeys[i]=0;   
        }
}


//util_add_text_adventure_kdb(buffer_palabra);
void util_unpawsgac_add_word_kb(char *palabra)
{

        //Buffer de maximo tamanyo y agregando dos ~~ al principio
        char buffer_palabra_destino[MAX_OSD_ADV_KEYB_TEXT_LENGTH+2];
        //De momento sin hotkey
        sprintf (buffer_palabra_destino,"%s",palabra);        

        //Agregamos la palabra metiendo hotkey si conviene
        char inicial=letra_minuscula(*palabra);

        if (inicial>='a' && inicial<'z') {
                int indice=inicial-'a';

                if (util_unpawsgac_hotkeys[indice]==0) {
                        //Meterla con hotkey
                        util_unpawsgac_hotkeys[indice]=1;

                        sprintf (buffer_palabra_destino,"~~%s",palabra);
                }
        }

        util_add_text_adventure_kdb(buffer_palabra_destino);
}

char *quillversions_strings[]={
        "Paws",
        "Quill.A",
        "Unkown version 2",
        "Quill.C"
};


void util_unpaws_get_maintop_mainattr(z80_int *final_maintop,z80_int *final_mainattr,int *final_quillversion)
{
        int quillversion=0;

        z80_int MainTop=daad_peek_word(65533);
        z80_int MainAttr=MainTop+311;

        if   (MainTop<=(65535-321) 
   && (MainTop>=(16384-311))
   && (daad_peek(MainAttr) == 16)
   && (daad_peek(MainAttr+2) == 17)
   && (daad_peek(MainAttr+4) == 18)
   && (daad_peek(MainAttr+6) == 19)
   && (daad_peek(MainAttr+8) == 20)
   && (daad_peek(MainAttr+10) == 21) 
        ) {
                //debug_printf (VERBOSE_DEBUG,"PAW signature found");
        }

  else {
       MainTop=26931;
      MainAttr=MainTop+977;
      if  (  (daad_peek(MainAttr) == 16)
        && (daad_peek(MainAttr+2) == 17)
        && (daad_peek(MainAttr+4) == 18)
        && (daad_peek(MainAttr+6) == 19)
        && (daad_peek(MainAttr+8) == 20)
        && (daad_peek(MainAttr+10) == 21)
      ) {
          //debug_printf (VERBOSE_DEBUG,"Quill.A signature found");
          quillversion=1;
      }

      else {
              MainTop=27356;
           MainAttr=MainTop+169;
           if (    (daad_peek(MainAttr) == 16)
             && (daad_peek(MainAttr+2) == 17)
             && (daad_peek(MainAttr+4) == 18)
             && (daad_peek(MainAttr+6) == 19)
             && (daad_peek(MainAttr+8) == 20)
             && (daad_peek(MainAttr+10) == 21)
           ) {
               //debug_printf (VERBOSE_DEBUG,"Quill.C signature found");
               quillversion=3;
           }

           else {
                   quillversion=-1;
           }
     
      }
  }

  *final_maintop=MainTop;
  *final_mainattr=MainAttr;
  *final_quillversion=quillversion;

  //printf ("quill version: %d\n",quillversion);

}

z80_int util_unpaws_get_maintop(void)
{
        z80_int final_maintop;
        z80_int final_mainattr;
        int quillversion;

        util_unpaws_get_maintop_mainattr(&final_maintop,&final_mainattr,&quillversion);

        return final_maintop;
}

z80_int util_unpaws_get_mainattr(void)
{
        z80_int final_maintop;
        z80_int final_mainattr;
        int quillversion;

        util_unpaws_get_maintop_mainattr(&final_maintop,&final_mainattr,&quillversion);

        return final_mainattr;
}

int util_unpaws_get_version(void)
{
        z80_int final_maintop;
        z80_int final_mainattr;
        int quillversion;

        util_unpaws_get_maintop_mainattr(&final_maintop,&final_mainattr,&quillversion);

        return quillversion;
}


char *util_unpaws_const_parser_paws="Paws";
char *util_unpaws_const_parser_quill="Quill";
char *util_unpaws_const_parser_daad="Daad";
char *util_unpaws_const_parser_gac="Gac";

char *util_unpaws_get_parser_name(void)
{
        if (util_unpaws_get_version()==0) return util_unpaws_const_parser_paws;
        else return util_unpaws_const_parser_quill;
}

int util_paws_quill_is_quill(void)
{
         if (util_unpaws_get_version()==0) return 0; //paws             

        return 1; //quill      
}

int util_undaad_unpaws_is_quill(void)
{
        if (util_daad_detect()) return 0; //daad

        return util_paws_quill_is_quill();

}

char *util_undaad_unpaws_ungac_get_parser_name(void)
{

    if (util_gac_detect() ) return util_unpaws_const_parser_gac;

        if (util_daad_detect()) return util_unpaws_const_parser_daad;
	else return util_unpaws_get_parser_name();
}

int util_textadventure_is_daad_quill_paws(void)
{
    if (util_daad_detect()) return 1;
    if (util_textadv_detect_paws_quill()) return 1;

    return 0;
}

//Variables que se inicializan desde util_unpaws_init_parameters

z80_byte util_unpaws_NumMsg;
z80_int util_unpaws_OffMsg;
z80_int util_unpaws_OffSys;
z80_byte util_unpaws_NumSys;
z80_byte util_unpaws_NumLoc;
z80_int util_unpaws_OffLoc;
z80_int util_unpaws_OffObj;
z80_byte util_unpaws_NumObj;
z80_byte util_unpaws_NumPro;
z80_int util_unpaws_OffPro;
z80_int util_unpaws_OffAbreviations;
z80_byte util_unpaws_NumFonts;
z80_int util_unpaws_OffFont;
z80_int util_unpaws_OffGraph;
z80_int util_unpaws_OffGraphAttr;
z80_byte util_unpaws_compressed;
z80_int util_unpaws_tOffs;
z80_byte util_unpaws_Patched;
z80_int util_unpaws_OffResp;
z80_byte util_unpaws_GraphCount;
z80_int util_unpaws_OffVoc;
z80_int util_unpaws_OffCon;
z80_int util_unpaws_vocptr;


void util_unpaws_init_parameters(void)
{

        z80_int maintop;
        z80_int mainattr;
        int quillversion;

        util_unpaws_get_maintop_mainattr(&maintop,&mainattr,&quillversion);







//(* Global data *)
 if (quillversion==0) {
   util_unpaws_NumMsg=daad_peek(maintop+326);
   util_unpaws_OffMsg=daad_peek_word(65503);
   util_unpaws_OffSys=daad_peek_word(65505);
   util_unpaws_NumSys=daad_peek(maintop+327);
   util_unpaws_NumLoc=daad_peek(maintop+325);
   util_unpaws_OffLoc=daad_peek_word(65501);
   util_unpaws_OffObj = daad_peek_word(65499);
   util_unpaws_NumObj = daad_peek(maintop+324);
   util_unpaws_NumPro = daad_peek(maintop+328) ;
   util_unpaws_OffPro = daad_peek_word(65497) ;
   util_unpaws_OffAbreviations = daad_peek_word(maintop+332) ;
   util_unpaws_NumFonts = daad_peek(maintop+329);
   util_unpaws_OffFont = daad_peek_word(maintop+330);
   util_unpaws_OffGraph = daad_peek_word(65521);
   util_unpaws_OffGraphAttr=daad_peek_word(65523);
   util_unpaws_compressed=daad_peek(util_unpaws_OffAbreviations); //=0;

     util_unpaws_vocptr = daad_peek_word(65509);
     util_unpaws_OffVoc = daad_peek_word(65509);
     util_unpaws_OffCon = daad_peek_word(65507);

}
 else
 {
   util_unpaws_tOffs=mainattr+13;
   util_unpaws_NumMsg=daad_peek(util_unpaws_tOffs+3);
   if (quillversion==1) 
   {
    util_unpaws_OffSys=maintop+168;
    util_unpaws_NumSys=32;
    util_unpaws_OffMsg=daad_peek_word(mainattr+25);
    util_unpaws_OffLoc=daad_peek_word(mainattr+23);
    util_unpaws_vocptr = daad_peek_word(mainattr+29);
    util_unpaws_OffCon = daad_peek_word(mainattr+27);
    }
   else
   {
    util_unpaws_OffSys=daad_peek_word(util_unpaws_tOffs+15);
    util_unpaws_NumSys=daad_peek(util_unpaws_tOffs+4);
    util_unpaws_OffMsg=daad_peek_word(mainattr+26);
    util_unpaws_OffLoc=daad_peek_word(mainattr+24);
    util_unpaws_vocptr = daad_peek_word(mainattr+32);
    util_unpaws_OffCon = daad_peek_word(mainattr+30);
   }
   util_unpaws_OffVoc = util_unpaws_vocptr;

   util_unpaws_NumLoc=daad_peek(util_unpaws_tOffs+2);
   if (quillversion==1) util_unpaws_OffObj = daad_peek_word(util_unpaws_tOffs+8);
   else util_unpaws_OffObj = daad_peek_word(util_unpaws_tOffs+9);

   util_unpaws_NumObj = daad_peek(util_unpaws_tOffs+1);

   if (quillversion==1) util_unpaws_OffPro = daad_peek_word(util_unpaws_tOffs+6);
   else util_unpaws_OffPro = daad_peek_word(util_unpaws_tOffs+7);

   if (quillversion==1) util_unpaws_OffResp = daad_peek_word(util_unpaws_tOffs+4);
   else util_unpaws_OffResp = daad_peek_word(util_unpaws_tOffs+5);

   if (quillversion==1) util_unpaws_OffAbreviations = util_unpaws_tOffs+24;
   else util_unpaws_OffAbreviations = util_unpaws_tOffs+29;

   util_unpaws_compressed=(quillversion==3) && (daad_peek(util_unpaws_OffAbreviations)<128) && (daad_peek(util_unpaws_OffAbreviations+1)==128);

   util_unpaws_Patched= (daad_peek_word(24791)==daad_peek_word(23606)) || (daad_peek_word(24802)==daad_peek_word(23606));
   
   if(!util_unpaws_Patched) {
                         if ((daad_peek_word(23606)<16384)) util_unpaws_NumFonts=0;
                         else util_unpaws_NumFonts=1;
   }

   else if ( (daad_peek_word(24791)<16384) && (daad_peek_word(24802)<16384) )  util_unpaws_NumFonts=0;
   else if ( (daad_peek_word(24791)<16384) || (daad_peek_word(24802)<16384) ) util_unpaws_NumFonts=1;
   else util_unpaws_NumFonts=2;


   if  ((daad_peek_word(64182)<=64182) && (daad_peek_word(64188) == 64181))
   {
      util_unpaws_OffGraph = daad_peek_word(64184);
      util_unpaws_OffGraphAttr=daad_peek_word(64186);
      util_unpaws_GraphCount=daad_peek(64190);
   }
   else 
    util_unpaws_OffGraph=0;
} 


}

void util_daad_get_version_daad(int *official_version,int *version_pointers)
{
    z80_int dir=util_daad_get_start_pointers();

    int version=daad_peek(dir);

    *official_version=version;

    //Nota: Puede que haya una versión oficial 2.0 pero no publicada
    //Nota2: mi "version_pointers" al final se muestra como PTR version, o sea:
    //Version= official_version PTR v.version_pointers
    //La 2.1 oficial corresponde con mi 2 PTR v1, luego salen otras variantes, que posiblemente no sean valores oficiales de versión:
    //Cozumel retorna 2 PTR v2
    //Aventura original retorna 1 PTR v3
    //Y el resto son 2 PTR v1

    switch (dir) {
        case 0x8380:
            *version_pointers=2;
        break;

        case 0x8480:
            *version_pointers=3;
        break;

        default:
            *version_pointers=1;
        break;
    }

}

void util_daad_get_language_parser(char *texto)
{
    z80_int dir=util_daad_get_start_pointers();

    z80_byte language=daad_peek(dir+1);

    /*
     2) En la siguiente direccion debe conterner un 0x10 o un 0x11 (marca que es juego de spectrum en ingles la primera, 
     juego de Spectrum en español la segunda)
    Para Amstrad, 0x31/0x30 en el 0x2882 en lugar de 0x11/0x10   
    */    

   z80_byte id_english=0x10;
   z80_byte id_spanish=0x11;

    if (MACHINE_IS_CPC) {
        id_english=0x30;
        id_spanish=0x31;
    }

    if (language==id_english) {
        strcpy(texto,"English");
    }
    else if (language==id_spanish) {
        strcpy(texto,"Spanish");
    }
    else strcpy(texto,"Language Unknown");

}

void util_unpaws_daad_get_version_string(char *texto)
{
    if (util_daad_detect() ) {
        int official_version;
        int version_pointers;

        util_daad_get_version_daad(&official_version,&version_pointers);
        sprintf(texto,"Daad %d (PTR v%d)",official_version,version_pointers);
        return;
    }
    else {
        int quillversion=util_unpaws_get_version();

        if (quillversion>=0) {
            strcpy (texto,quillversions_strings[quillversion]);
            return;
        }
    }

    strcpy(texto,"Unknown");
}

int util_unpaws_detect_version(z80_int *p_mainattr)
{

        int quillversion;
        z80_int MainTop;
        z80_int MainAttr;        

        util_unpaws_get_maintop_mainattr(&MainTop,&MainAttr,&quillversion);

    

  if (quillversion>=0) {
      //debug_printf (VERBOSE_DEBUG,"%s signature found",quillversions_strings[quillversion]);
  }

  *p_mainattr=MainAttr;
  return quillversion;

/*
QuillVersion:=0;

 MainTop:=DPeek(65533);
 MainAttr:=MainTop+311;
 IF   (MainTop<=(65535-321))
   and (MainTop>=(16384-311))
   and (Peek(MainAttr) = 16)
   and (Peek(MainAttr+2) = 17)
   and (Peek(MainAttr+4) = 18)
   and (Peek(MainAttr+6) = 19)
   and (Peek(MainAttr+8) = 20)
   and (Peek(MainAttr+10) = 21)
 THEN
     Writeln('PAW signature found.')
 ELSE BEGIN
      MainTop:=26931;
      MainAttr:=MainTop+977;
      IF    (Peek(MainAttr) = 16)
        and (Peek(MainAttr+2) = 17)
        and (Peek(MainAttr+4) = 18)
        and (Peek(MainAttr+6) = 19)
        and (Peek(MainAttr+8) = 20)
        and (Peek(MainAttr+10) = 21)
      THEN BEGIN
          Writeln('Quill.A signature found.');
          QuillVersion:=1;
      END
      ELSE BEGIN
           MainTop:=27356;
           MainAttr:=MainTop+169;
           IF    (Peek(MainAttr) = 16)
             and (Peek(MainAttr+2) = 17)
             and (Peek(MainAttr+4) = 18)
             and (Peek(MainAttr+6) = 19)
             and (Peek(MainAttr+8) = 20)
             and (Peek(MainAttr+10) = 21)
           THEN BEGIN
               Writeln('Quill.C signature found.');
               QuillVersion:=3;
           END
           ELSE
               Error(3);
      END
 END;

*/

}


/*
CONST tVocs:ARRAY [0..6] OF String=(
'Verb',
'Adverb',
'Noun',
'Adjective',
'Preposition',
'Conjunction',
'Pronoun');
*/
  char *unpaws_tvocs[]={
"Verb",
"Adverb",
"Noun",
"Adjective",
"Preposition",
"Conjunction",
"Pronoun"        
  };

int util_paws_dump_vocabulary(int *p_quillversion)
{


        /*

        Extracted from https://github.com/Utodev/unPAWs
UnPAWS takes snapshot files (.SNA, .SP, .Z80) of  Spectrum  games written  with PAWS or Quill system (version A or C), and  produces  text listings of the game database.

        if QuillVersion=0 then
  begin
     vocptr := DPeek(65509);
     OffVoc := DPeek(65509);

     WHILE (vocptr < 65509) AND  (Peek(vocptr)<>0) DO
     BEGIN
      WriteLn(FOut,char(PeekNeg(vocptr)),char(PeekNeg(vocptr+1)),
             char(PeekNeg(vocptr+2)),char(PeekNeg(vocptr+3)),char(PeekNeg(vocptr+4)),
             Peek(vocptr+5):5,' ',type_voc(Peek(vocptr+6)));
      VocPtr := VocPtr + 7 ;
     END;
  end
  else { Quill Vocabulary}
  begin
     if QuillVersion=1 then vocptr := DPeek(MainAttr+29)
     else vocptr := DPeek(MainAttr+32);
     OffVoc := vocptr;

     WHILE (vocptr < 65530) AND  (Peek(vocptr)<>0) DO
     BEGIN
      WriteLn(FOut,char(PeekNeg(vocptr)),char(PeekNeg(vocptr+1)),
             char(PeekNeg(vocptr+2)),char(PeekNeg(vocptr+3)),Peek(vocptr+4):5);
      VocPtr := VocPtr + 5 ;
     END;
  end;
        */

//Valores validos van de 0 hasta 6 inclusive
#define  MAXTVOC 6
/*
FUNCTION Type_Voc(C:Integer):String;
BEGIN
 IF (C>=0) AND (C<=MAXTVOC) then Type_Voc:=TVocs[c]
                            else Type_Voc:='RESERVED';
END;
*/





  int total_palabras=0;

  //Precargo palabras (solo 1 sinonimo de cada, la mas corta) antes en un array
  //tipo, indice
  char lista_palabras[MAXTVOC+1][256][PAWS_LONGITUD_PALABRAS+1];

  //Inicializarlas vacias
  int i,j;
  for (i=0;i<=MAXTVOC;i++) {
        for (j=0;j<256;j++) lista_palabras[i][j][0]=0;
  }

  int quillversion;

  z80_int mainattr;

  quillversion=util_unpaws_detect_version(&mainattr);

  if (quillversion<0) {
          debug_printf (VERBOSE_DEBUG,"It does not seem to be a Quill/PAW game");
          *p_quillversion=-1;
          return 0;
  }
  
  util_clear_text_adventure_kdb();

  z80_int vocptr;
  z80_int limite_voc;

  if (quillversion==0) {
          vocptr=peek_word_no_time(65509);
          limite_voc=65509;
  }

  else {
          if (quillversion==1) vocptr=peek_word_no_time(mainattr+29);
          else vocptr=peek_word_no_time(mainattr+32);
          limite_voc=65530;
  }



  while (vocptr < limite_voc && peek_byte_no_time(vocptr)!=0) {
          char palabra[PAWS_LONGITUD_PALABRAS+1];
          z80_byte indice_palabra;
          z80_byte tipo_palabra;

          if (quillversion==0) {

                for (j=0;j<PAWS_LONGITUD_PALABRAS;j++) palabra[j]=peek_byte_no_time(vocptr+j)^255;

                indice_palabra=peek_byte_no_time(vocptr+j);
                palabra[j]=0;
                tipo_palabra=peek_byte_no_time(vocptr+j+1);

          }

          else {
                for (j=0;j<QUILL_LONGITUD_PALABRAS;j++) palabra[j]=peek_byte_no_time(vocptr+j)^255;

                indice_palabra=peek_byte_no_time(vocptr+j);
                palabra[j]=0;
                tipo_palabra=0; //no hay tipos. lo dejamos forzado                  
          }

          char buf_tipo_palabra[30];

          int reservado=0;

          if (tipo_palabra>=0 && tipo_palabra<=MAXTVOC) strcpy (buf_tipo_palabra,unpaws_tvocs[tipo_palabra]);
          else {
                  strcpy(buf_tipo_palabra,"RESERVED");
                  reservado=1;
                  //printf ("tipo palabra: %d\n",tipo_palabra);
          }

          if (quillversion==0) debug_printf (VERBOSE_DEBUG,"unPAWs dump. Vocabulary word: %s Index: %d Type: %s",palabra,indice_palabra,buf_tipo_palabra);
          else debug_printf (VERBOSE_DEBUG,"unPAWs dump. Vocabulary word: %s Index: %d",palabra,indice_palabra);

          if (!reservado) {
                  //Meter en array. Quitar antes espacios del final
                  char palabra_sin_espacios[PAWS_LONGITUD_PALABRAS+1];
                  util_clear_final_spaces(palabra,palabra_sin_espacios);

                  int insertar=1;

                //Ver si ya habia un sinonimo, y en ese caso, ver si la de ahora es mas corta
                  if (lista_palabras[tipo_palabra][indice_palabra][0]!=0) {
                        int longitud_almacenada=strlen(lista_palabras[tipo_palabra][indice_palabra]);
                        int longitud_nueva=strlen(palabra_sin_espacios);

                        if (longitud_nueva>longitud_almacenada) insertar=0;
                  }

                  if (insertar) {
                        //if (strlen(palabra_sin_espacios)>5) printf ("ERROR");
                        //if (tipo_palabra>=MAXTVOC) printf ("tipo palabra: %d\n",tipo_palabra);

                        strcpy(lista_palabras[tipo_palabra][indice_palabra],palabra_sin_espacios);
                        debug_printf (VERBOSE_DEBUG,"Adding word %s to array list",palabra_sin_espacios);
                        total_palabras++;

                  }

                  else {
                      debug_printf (VERBOSE_DEBUG,"Not adding word %s to array list as it is a longer synonim of %s",
                      palabra_sin_espacios,lista_palabras[tipo_palabra][indice_palabra]);    
                  }

                  
          }

          if (quillversion==0) vocptr+=7;
          else vocptr+=5;
  }
  /*
            if (!reservado) {
                  //Meter a teclado de palabras. Quitar antes espacios del final
                  char palabra_sin_espacios[7];
                  util_clear_final_spaces(palabra,palabra_sin_espacios);
                  debug_printf (VERBOSE_DEBUG,"Adding word %s to OSD Adventure text keyboard",palabra_sin_espacios);
                  util_add_text_adventure_kdb(palabra_sin_espacios);

                  total_palabras++;
          }
        */

  //Y ahora agregamos la lista total del array
  if (quillversion==0) debug_printf (VERBOSE_DEBUG,"Adding words to OSD Adventure text keyboard");
  for (i=0;i<=MAXTVOC;i++) {
        debug_printf (VERBOSE_DEBUG,"Adding words type %s",unpaws_tvocs[i]);
        for (j=0;j<256;j++) {
                //printf ("i %d j %d\n",i,j);
                if (lista_palabras[i][j][0]!=0) {
                        debug_printf (VERBOSE_DEBUG,"Adding word %s to OSD Adventure text keyboard",lista_palabras[i][j]);
                        util_unpawsgac_add_word_kb(lista_palabras[i][j]);   
                }
        }
  }
  //printf ("despues cargar palabras\n");

  *p_quillversion=quillversion;

  //printf ("justo antes del return\n");
  return total_palabras;
/*
Debug: unPAWs dump. Vocabulary word: IT    Index: 2 Type: Pronoun
tipo palabra: 6
Debug: Adding word IT to array list
*/

}


//detectar si aventura es paws o quill
int util_textadv_detect_paws_quill(void) 
{

  if (!MACHINE_IS_SPECTRUM) return 0;

  z80_int mainattr;
  int quillversion=util_unpaws_detect_version(&mainattr);
  if (quillversion<0) return 0;
  else return 1;
}

z80_int readtokenised(z80_int puntero)
{
   z80_byte low, high;

   low=peek_byte_no_time(puntero++);
   high=peek_byte_no_time(puntero++);

   return ((high & 0x7f) << 8) + low;
}

#define MAX_DICT_GAC_ENTRIES 5000
#define MAX_DICT_GAC_STRING_LENGTH 30

int util_gac_palabras_agregadas;

int util_gac_get_offset_dictionary(int index)
{
        return index*(MAX_DICT_GAC_STRING_LENGTH+1);        
}

void util_gac_put_string_dictionary(int index,z80_byte *memoria,char *string)
{

        char string_shown[256];

        //menu_tape_settings_trunc_name(string,string_shown,MAX_DICT_GAC_STRING_LENGTH+1);       
        //int longitud_texto=strlen(string);
        strcpy(string_shown,string);
        string_shown[MAX_DICT_GAC_STRING_LENGTH]=0;
        int offset=util_gac_get_offset_dictionary(index);

        strcpy((char *)&memoria[offset],string_shown);
}

void util_gac_get_string_dictionary(int index,z80_byte *memoria,char *string)
{
        int offset=util_gac_get_offset_dictionary(index);
        if (index>MAX_DICT_GAC_ENTRIES) strcpy(string,"");

        else strcpy(string,(char *)&memoria[offset]);
}

void util_gac_readobjects(z80_int puntero,z80_int endptr,z80_byte *mem_diccionario)
{
        //z80_byte count,temp; 
        z80_int copia_puntero;

        z80_byte object,tamanyo,scrap;
        //z80_byte weight;

        int start;

      do {
      copia_puntero=puntero;
      object=peek_byte_no_time(puntero++);
      tamanyo=peek_byte_no_time(puntero++);
      //weight=peek_byte_no_time(puntero++);
      start=peek_byte_no_time(puntero++);
      scrap=peek_byte_no_time(puntero++);
      start+=scrap<<8;
      tamanyo-=3;
      //puntero+=5;
      if (object!=0 && tamanyo!=0)
      {

         //len=0;
         z80_int dictentry=readtokenised(puntero);
         if ((dictentry&0xC000)==0xC000) {
                 debug_printf (VERBOSE_DEBUG,"Ignore. Is a puntuation word");
         }
         else {
                char buffer_palabra[MAX_DICT_GAC_STRING_LENGTH+1];
                //printf("token %d\n",dictentry);
                util_gac_get_string_dictionary(dictentry,mem_diccionario,buffer_palabra);
         
                debug_printf (VERBOSE_DEBUG,"Dictionary entry %d word: %s",dictentry,buffer_palabra); 

                if (strlen(buffer_palabra)) {
                        debug_printf (VERBOSE_DEBUG,"Adding word %s to OSD Adventure text keyboard",buffer_palabra);
                        util_unpawsgac_add_word_kb(buffer_palabra);
                        util_gac_palabras_agregadas++;
                }
         
                //strcat(objects[current]->description,readstring(infile, size));
         }
         puntero+=tamanyo;
         puntero+=3;
         //current++;
      }
      // move up to next object
      puntero=copia_puntero+tamanyo+5;
      //fseek(infile,fileptr+size+5,SEEK_SET);          

  } while (puntero<endptr);
}

void util_gac_readwords(z80_int puntero,z80_int endptr,z80_byte *mem_diccionario)
{
        z80_byte count,temp;
       temp=1;
       z80_int dictentry;

       do {

      count=peek_byte_no_time(puntero++);
      if (count==0)
      {
         temp=peek_byte_no_time(puntero);
      }
      if (count!=0 && temp!=0)
      {
         dictentry=readtokenised(puntero);
         char buffer_palabra[256];
         util_gac_get_string_dictionary(dictentry,mem_diccionario,buffer_palabra);
         debug_printf (VERBOSE_DEBUG,"Dictionary entry %d word: %s",dictentry,buffer_palabra);
         puntero+=2;

                if (strlen(buffer_palabra)) {
                        debug_printf (VERBOSE_DEBUG,"Adding word %s to OSD Adventure text keyboard",buffer_palabra);
                        util_unpawsgac_add_word_kb(buffer_palabra);
                        util_gac_palabras_agregadas++;
                }

         //strncpy(words[current]->word,dictionary[dictentry],60);
         //words[current]->number=count;
         //current++;
      }


       } while (puntero<endptr && count!=0 && temp!=0);
}

char *gacversions_strings[]={
        "GAC version 0",
};

int util_gac_detect_version(void)
{

        int version=0;

        //en 67cf se encuentra esta cadena
        char *signature="You have run out of memory";

        //char read_signature[100];

        //Leemos la firma de la memoria
        int longitud_firma=strlen(signature);

        z80_int puntero=0x67cf;

        int i;
        for (i=0;i<longitud_firma;i++) {
            //read_signature[i]=peek_byte_no_time(puntero+i);
            //En el momento que cambie un solo byte, ya no es
            z80_byte byte_leido=peek_byte_no_time(puntero+i);
            if (byte_leido!=signature[i]) return -1;

        }

        //read_signature[i]=0;
        //if (strcmp(read_signature,signature)) version=-1;

        return version;

}

int util_gac_detect(void)
{
    if (util_gac_detect_version()>=0) {
        return 1;
    }
    else return 0;
}


/* based on grackle tool
http://ifarchive.jmac.org/indexes/if-archiveXsolutionsXtools.html
Grackle 0.2 Alpha, by David Lodge. A dumper and player for adventures written using the Graphic Adventure Creator. At the moment it supports games from the Spectrum (in .sna format) and the Amstrad CPC (as extracted files with the AMSDOS header from a disk image)
*/
int util_gac_dump_dictonary(int *p_gacversion)
{

        int gacversion; //Realmente no se si hay mas de una version

        gacversion=util_gac_detect_version();

  if (gacversion<0) {
          debug_printf (VERBOSE_DEBUG,"It does not seem to be a GAC game");
          *p_gacversion=-1;
          return 0;
  }        

        util_clear_text_adventure_kdb();
        util_gac_palabras_agregadas=0;

        //Asignar memoria para el diccionario. 
        z80_byte *diccionario_array;

        diccionario_array=malloc(MAX_DICT_GAC_ENTRIES*(MAX_DICT_GAC_STRING_LENGTH+1));

        if (diccionario_array==NULL) cpu_panic("Can not allocate memory");

        //Array para el diccionario. Palabras de mas de 30 caracteres los ignoramos

        //char diccionario_array[MAX_DICT_GAC_ENTRIES][MAX_DICT_GAC_STRING_LENGTH+1];

        //Inicializar a ""
        int i;
        for (i=0;i<MAX_DICT_GAC_ENTRIES;i++) util_gac_put_string_dictionary(i,diccionario_array,"");

        z80_int spec_start=0xA51F;
        z80_int room_data=0xA54D;

        //Vamos primero a hacer dump del dicccionario
        z80_int dictptr=peek_word_no_time(spec_start+9*2); //Saltar los 9 word de delante


        z80_int nounptr=peek_word_no_time(spec_start);
        z80_int adverbptr=peek_word_no_time(spec_start+1*2);
        z80_int objectptr=peek_word_no_time(spec_start+2*2);
        z80_int roomptr=peek_word_no_time(spec_start+3*2);

        z80_int endptr=peek_word_no_time(spec_start+10*2); 

        z80_int verbptr=room_data+2;

        debug_printf (VERBOSE_DEBUG,"Dictionary start: %04XH",dictptr);

        z80_byte longitud_palabra;

        z80_int puntero=dictptr;
        int indice=0;

        do {
                longitud_palabra=peek_byte_no_time(puntero++);
                if (longitud_palabra>0) {
                        char palabra[256];
                        int i;
                        for (i=0;i<longitud_palabra;i++) {
                                z80_byte caracter_leido=peek_byte_no_time(puntero++) & 127;
                                if (caracter_leido<32) caracter_leido=32;
                                palabra[i]=caracter_leido & 127;
                        }

                        palabra[i]=0;

                        debug_printf (VERBOSE_DEBUG,"Dictonary word index %d: %s (length: %d)",indice,palabra,longitud_palabra);
                        if (longitud_palabra<=MAX_DICT_GAC_STRING_LENGTH) {
                                //strcpy(diccionario_array[indice],palabra);
                                util_gac_put_string_dictionary(indice,diccionario_array,palabra);
                        }
                        indice++;
                }
        } while (longitud_palabra!=0 && puntero<endptr);

       debug_printf (VERBOSE_DEBUG,"Dumping verbs. Start at %04XH",verbptr);
       util_gac_readwords(verbptr,nounptr,diccionario_array);       

       debug_printf (VERBOSE_DEBUG,"Dumping nouns. Start at %04XH",nounptr);
       util_gac_readwords(nounptr,adverbptr,diccionario_array);

       debug_printf (VERBOSE_DEBUG,"Dumping adverbs. Start at %04XH",adverbptr);
       util_gac_readwords(adverbptr,objectptr,diccionario_array);

       debug_printf (VERBOSE_DEBUG,"Dumping objects. Start at %04XH",objectptr);
       util_gac_readobjects(objectptr,roomptr,diccionario_array);

   /*


   printf("Reading adverbs: %x\n",header->adverbptr);
   header->adverbs=readwords(infile,header,adverbs,header->adverbptr,header->objectptr);       */

       /*puntero=nounptr;
       z80_byte count,temp;
       temp=1;
       z80_int dictentry;

       do {

      count=peek_byte_no_time(puntero++);
      if (count==0)
      {
         temp=peek_byte_no_time(puntero);
      }
      if (count!=0 && temp!=0)
      {
         dictentry=readtokenised(puntero);
         printf ("nombre token %d palabra: %s\n",dictentry,diccionario_array[dictentry]);
         puntero+=2;
         //strncpy(words[current]->word,dictionary[dictentry],60);
         //words[current]->number=count;
         //current++;
      }


       } while (puntero<endptr && count!=0 && temp!=0);*/


       // free(mem_diccionario);

       free(diccionario_array);

        *p_gacversion=gacversion;
        return util_gac_palabras_agregadas;
}


//Retorna 0 si ok. -1 si error
int util_unpawsetc_dump_words(char *mensaje)
{

        util_init_unpawsgac_hotkeys();

        //Ver si es de daad
        if (util_daad_detect()) {
                int palabras=util_daad_dump_vocabulary(0,NULL,0);
                sprintf(mensaje,"OK. DAAD signature found. %d words added",palabras);
                return 0;
        }

        int version;

	int palabras=util_paws_dump_vocabulary(&version);      

        //printf ("Despues extraer palabras\n");  

	//Es Paws?
	if (version>=0) {
                sprintf(mensaje,"OK. %s signature found. %d words added",
			quillversions_strings[version],palabras);
	}

	else {
		//No es paws. Probar con GAC
		palabras=util_gac_dump_dictonary(&version);
		if (version>=0) {
			sprintf(mensaje,"OK. %s signature found. %d words added",
				gacversions_strings[version],palabras);
		}	

		else {
			//Ni paws ni gac
			sprintf(mensaje,"It does not seem to be a Quill/PAW/Daad/GAC game");
		}
	}

        return version;
}


int util_has_daad_signature(z80_int dir)
{

    z80_byte first_byte=daad_peek(dir);
    z80_byte second_byte=daad_peek(dir+1);
    z80_byte third_byte=daad_peek(dir+2);

    if (first_byte==1 || first_byte==2) {
            if (second_byte==0x10 || second_byte==0x11 || second_byte==0x30 || second_byte==0x31) {
                    if (third_byte==95) {
                            //printf("has signature on %X\n",dir);
                            return 1;
                    }
            }
    }

    return 0;    
}


z80_int util_daad_get_start_pointers(void)
{
    /*
    1) En la dirección 0x8400 ha de haber un 1 o un 2. Si son juegos DAAD hechos hoy en día habrá un 2, si son antiguos habrá un 1. 
    2) En la siguiente direccion debe conterner un 0x10 o un 0x11 (marca que es juego de spectrum en ingles la primera, juego de Spectrum en español la segunda)
    Para Amstrad, 0x31/0x30 en el 0x2882 en lugar de 0x11/0x10
    3) En la siguiente direccion lo normal es encontrar un 95 decimal. 

    */    
        if (MACHINE_IS_CPC) return 0x2880;

        else {
            //normalmente 0x8400, pero en algunos (la diosa de cozumel) es 0x8380
            if (util_has_daad_signature(0x8380)) return 0x8380;

            //aventura original, o Jabato 1 por ejemplo
            if (util_has_daad_signature(0x8480)) return 0x8480;

            //la mayoria
            return 0x8400;

        }
}



//Detecta si juego cargado en memoria está hecho con daad
//Condicion primera es que maquina actual sea spectrum
int util_daad_detect(void)
{


        if (MACHINE_IS_SPECTRUM || MACHINE_IS_CPC) {


                z80_int dir=util_daad_get_start_pointers();

                //Nota: esto es un poco redundante porque en util_daad_get_start_pointers ya se llama a util_has_daad_signature
                //esto venia porque antes el puntero por defecto en spectrum siempre lo tenia al mismo 0x8400,
                //pero luego agregue deteccion de varias posibles direcciones
                return util_has_daad_signature(dir);

        }

        return 0;
}

z80_int util_textadventure_get_start_connections(void)
{
    z80_int dir;


    if (util_daad_detect() ) {
        dir=value_8_to_16(daad_peek(util_daad_get_start_pointers()+0x15),daad_peek(util_daad_get_start_pointers()+0x14));
    }

    else if (util_textadv_detect_paws_quill() ){
        //Paws
        util_unpaws_init_parameters();
        dir=util_unpaws_OffCon;   
    }
    
    else {
        dir=0;
    }


    return dir;
}


z80_int util_daad_get_start_vocabulary(void)
{
        z80_int dir;


        dir=value_8_to_16(daad_peek(util_daad_get_start_pointers()+0x17),daad_peek(util_daad_get_start_pointers()+0x16));
       


        return dir;
}



z80_int util_paws_get_start_vocabulary(void)
{
        z80_int dir;

        
        util_unpaws_init_parameters();
        dir=util_unpaws_OffVoc;
        

        return dir;
}

//Volcar vocabulario para el extractor de palabras (teclado text adventure) o como un string con saltos de linea
//tipo=0: para text adventure. 1:para string 
int util_daad_dump_vocabulary(int tipo,char *texto,int max_string)
{

        debug_printf (VERBOSE_DEBUG,"Dumping Daad vocabulary");

        if (tipo==0) util_clear_text_adventure_kdb();

        z80_int puntero=util_daad_get_start_vocabulary();

        //Leer entradas de 7 bytes
        /*
        5 para 5 letras de la palabra (puede incluir espacios de padding al final si es más corta), con xor 255
        1 byte para el número de palabra 
        1 byte para el tipo de palabra 
        */

       //Rellenamos con espacio para que se vea centrado
       char *word_types[]={"verb       ", "adverb     ", "noun       ", "adjective  ", "preposition","conjugation", "pronoun    "};
       //char *word_types[]={"verb", "adverb", "noun", "adjective", "preposition","conjugation", "pronoun"};



       int palabras=0;

       char buffer_palabra[6];

       if (tipo) texto[0]=0;

       int salir=0;

       do {
               //Copiar palabra a buffer
               int i;
               z80_byte caracter;
               z80_byte tipo_palabra;
               z80_byte num_palabra;

                if (daad_peek(puntero)==0) salir=1;

                else {

               for (i=0;i<5;i++) {
                       caracter=daad_peek(puntero+i) ^255;
                       //Si hay espacio, fin
                       if (caracter==32) break;

                       caracter=chardetect_convert_daad_accents(caracter);

                       //Pasar a mayusculas por si acaso
                       caracter=letra_mayuscula(caracter);

                       if (caracter<32 || caracter>127) {
                               //printf ("%d\n",caracter);
                               //21=á
                               caracter='?';
                       }
                       buffer_palabra[i]=caracter;
               }
               buffer_palabra[i]=0;

               num_palabra=daad_peek(puntero+5);

               tipo_palabra=daad_peek(puntero+6);

               //if (buffer_palabra[0]<32 || buffer_palabra[0]>127) salir=1;
               //else  {
                       debug_printf (VERBOSE_DEBUG,"Adding word: %s",buffer_palabra);

                       if (tipo==0) {
                           util_unpawsgac_add_word_kb(buffer_palabra);
                       }
                       else {
		        char buffer_linea[32];
		        sprintf(buffer_linea,"%03d %s %s\n",num_palabra,(tipo_palabra<=6 ? word_types[tipo_palabra] : "unknown"),
                        buffer_palabra);

		        //Y concatenar a final
		        salir=util_concat_string(texto,buffer_linea,max_string);
                       }
                       palabras++;
               //}

               puntero+=7;
                }

       } while (!salir);

       return palabras;
}


//Volcar vocabulario para el extractor de palabras (teclado text adventure) o como un string con saltos de linea
//tipo=0: para text adventure. 1:para string 
//Nota: actualmente solo lo utilizo con tipo 1:para string
int util_paws_dump_vocabulary_tostring(int tipo,char *texto,int max_string)
{

        debug_printf (VERBOSE_DEBUG,"Dumping Daad vocabulary");

        if (tipo==0) util_clear_text_adventure_kdb();

        z80_int puntero=util_paws_get_start_vocabulary();

        //Leer entradas de 7 bytes
        /*
        5 para 5 letras de la palabra (puede incluir espacios de padding al final si es más corta), con xor 255
        1 byte para el número de palabra 
        1 byte para el tipo de palabra 

        Quill es de 5 bytes, 4 para caracteres de palabra y 1 numero
        */

       //Rellenamos con espacio para que se vea centrado
       char *word_types[]={"verb       ", "adverb     ", "noun       ", "adjective  ", "preposition","conjugation", "pronoun    "};
       //char *word_types[]={"verb", "adverb", "noun", "adjective", "preposition","conjugation", "pronoun"};



        z80_int maintop;
        z80_int mainattr;

        int quillversion;

        util_unpaws_get_maintop_mainattr(&maintop,&mainattr,&quillversion);

        int longitud_total_palabra;
        int longitud_palabra;

        int isquill;

        if (quillversion==0) {
                //paws
                longitud_total_palabra=7;
                longitud_palabra=5;
                isquill=0;
        }

        else {
                //quill
                longitud_total_palabra=5;
                longitud_palabra=4;
                isquill=1;
        }


       int palabras=0;

       char buffer_palabra[6];

       if (tipo) texto[0]=0;

       int salir=0;

       do {
               //Copiar palabra a buffer
               int i;
               z80_byte caracter;
               z80_byte tipo_palabra;
               z80_byte num_palabra;

                if (daad_peek(puntero)==0) salir=1;

                else {

               for (i=0;i<longitud_palabra;i++) {
                       caracter=daad_peek(puntero+i) ^255;
                       //Si hay espacio, fin
                       if (caracter==32) break;

                       caracter=chardetect_convert_daad_accents(caracter);

                       //Pasar a mayusculas por si acaso
                       caracter=letra_mayuscula(caracter);

                       if (caracter<32 || caracter>127) {
                               //printf ("%d\n",caracter);
                               //21=á
                               caracter='?';
                       }
                       buffer_palabra[i]=caracter;
               }
               buffer_palabra[i]=0;

               num_palabra=daad_peek(puntero+longitud_palabra);

               if (!isquill) tipo_palabra=daad_peek(puntero+6);
               else tipo_palabra=0;

               //if (buffer_palabra[0]<32 || buffer_palabra[0]>127) salir=1;
               //else  {
                       debug_printf (VERBOSE_DEBUG,"Adding word: %s",buffer_palabra);

                       if (tipo==0) {
                           util_unpawsgac_add_word_kb(buffer_palabra);
                       }
                       else {
		        char buffer_linea[32];
                        if (!isquill) sprintf(buffer_linea,"%03d %s %s\n",num_palabra,(tipo_palabra<=6 ? word_types[tipo_palabra] : "unknown"), buffer_palabra);
                        else sprintf(buffer_linea,"%03d %s\n",num_palabra, buffer_palabra);

		        //Y concatenar a final
		        salir=util_concat_string(texto,buffer_linea,max_string);
                       }
                       palabras++;
               //}

               puntero+=longitud_total_palabra;
                }

       } while (!salir);

       return palabras;
}


//Dice si aventura de daad es spanish. Si no, english
int util_daad_is_spanish(void)
{
        return (daad_peek(util_daad_get_start_pointers()+1) & 1);
}

//Dice si aventura de daad es spanish. Si no, english
//metodo poco eficiente pero...
//CUIDADO: Tener en cuenta no llamar aqui desde util_daad_get_sys_message, o se meteria en un bucle recursivo
int util_paws_is_spanish(void)
{

    //util_daad_get_sys_message
    //MS30 - la respuesta positiva que se espera por la acción END y QUIT.
    //Una "S" o "SI" en spanish. Detectamos simplemente si es s o S

    char buffer_palabra[256];
    util_daad_get_sys_message(30,buffer_palabra);

    if (buffer_palabra[0]=='s' || buffer_palabra[0]=='S') return 1;

    else return 0;



}

/*
Truco para encontrar los flags

Flag 33 tiene verbo
Flag 34 tiene nombre

Si escribimos algo tipo "verbo" "nombre" donde sabemos el id de ese verbo y el nombre,
se buscan esos dos bytes seguidos, y donde salga será la direccion+33. Restamos 33 y ahi
*/

z80_int util_daad_get_start_flags(void)
{

        if (util_daad_detect()) {

                if (MACHINE_IS_CPC) return 0x23c9;

                else  {
                    if (util_daad_is_spanish()) {
                        z80_int dir=util_daad_get_start_pointers();
                        //printf("dir: %x\n",dir);
                        
                        //excepciones
                        if (dir==0x8480) {

                            //Este caso es especial. Aventura original y Jabato tienen misma posicion de signature (0x8480),
                            //pero el inicio de flags es diferente. En este caso usaremos registro IX

                            //jabato1
                            if (reg_ix==0x8187) return 0x8187;
                            

                            //aventura original 1 y demas
                            return 0x8171;
                        }

                        if (dir==0x8380) return 0x80fa;

                        return 0x7f1c;
                    }
                    else return 0x7e55;
                }

        }

        else {
                //Paws
                z80_int dir=0x85c0;

                //Creo que todas las de english cambia esto:
                if (reg_ix==0x85b0) dir=0x85b0;

                //En superlopez y abracadabra por ejemplo
                if (reg_ix==0x8560) dir=0x8560;

                //TODO: esta deteccion mediante IX provoca que, al cambiar de habitacion,
                //temporalmente IX cambia de valor, y acabamos devolviendo la direccion por defecto,
                //que no es la correcta. Se observa en el Adventure Map, pues al cambiar de habitacion
                //se pierde temporalmente la localidad y los objetos

                //con quill
                if (util_paws_quill_is_quill() ) dir=0x5b00;
                //if (reg_ix==0x5b00) dir=0x5b00;

                return dir;
        }

        
}


z80_int util_daad_get_start_objects(void)
{

        //Quill empieza en una posicion caprichosa
        if (util_undaad_unpaws_is_quill () ) {
                return 0x5b00+37;
        }

        else return util_daad_get_start_flags()+256;
        
}

z80_byte util_daad_get_flag_value(z80_byte index)
{
        //7f1c

        return daad_peek(util_daad_get_start_flags()+index);
}

z80_byte util_daad_get_object_value(z80_byte index)
{

        return daad_peek(util_daad_get_start_objects()+index);
}


void util_daad_put_flag_value(z80_byte index,z80_byte value)
{
        //7f1c

        daad_poke(util_daad_get_start_flags()+index,value);
}

void util_daad_put_object_value(z80_byte index,z80_byte value)
{

        daad_poke(util_daad_get_start_objects()+index,value);
}

void util_daad_locate_word(z80_byte numero_palabra_buscar,z80_byte tipo_palabra_buscar,char *texto_destino)
{
        z80_int puntero=util_daad_get_start_vocabulary();

        //Leer entradas de 7 bytes
        /*
        5 para 5 letras de la palabra (puede incluir espacios de padding al final si es más corta), con xor 255
        1 byte para el número de palabra 
        1 byte para el tipo de palabra. Si 255, cualquiera. Si no, de 0 hasta: ("verb", "adverb", "noun", "adjective", "preposition","conjugation", "pronoun");
        */

       int palabras=0;

       char buffer_palabra[6];

       int salir=0;

        //Por defecto asumimos no encontrado
        strcpy(texto_destino,"?");

       do {
               //Copiar palabra a buffer
               int i;
               z80_byte caracter;

                if (daad_peek(puntero)==0) salir=1;
                else {

               for (i=0;i<5;i++) {
                       caracter=daad_peek(puntero+i) ^255;
                       //Si hay espacio, fin
                       //if (caracter==32) break;


                       caracter=chardetect_convert_daad_accents(caracter);

                       //Pasar a mayusculas por si acaso
                       caracter=letra_mayuscula(caracter);



                       if (caracter<32 || caracter>127) caracter='?';
                       buffer_palabra[i]=caracter;
               }
               buffer_palabra[i]=0;

               //if (buffer_palabra[0]<32 || buffer_palabra[0]>127) {
                       //No encontrado
                //       return;
               //}
               //else  {
                       z80_byte numero_palabra=daad_peek(puntero+5);
                       z80_byte tipo_palabra=daad_peek(puntero+6);
                       //debug_printf (VERBOSE_DEBUG,"Adding word: %s",buffer_palabra);
                       //util_unpawsgac_add_word_kb(buffer_palabra);
                       //palabras++;
                       if (numero_palabra==numero_palabra_buscar && (tipo_palabra==tipo_palabra_buscar || tipo_palabra==255)) {
                               strcpy(texto_destino,buffer_palabra);
                               return;
                       }
               //}

               puntero+=7;
               palabras++;

               //Agregar un limite por si acaso
               if (palabras==65535) salir=1;
                }

       } while (!salir);
     
} 

void util_paws_locate_word(z80_byte numero_palabra_buscar,z80_byte tipo_palabra_buscar,char *texto_destino)
{
        z80_int puntero=util_paws_get_start_vocabulary();

        //Leer entradas de 7 bytes
        /*
        5 para 5 letras de la palabra (puede incluir espacios de padding al final si es más corta), con xor 255
        1 byte para el número de palabra 
        1 byte para el tipo de palabra. Si 255, cualquiera. Si no, de 0 hasta: ("verb", "adverb", "noun", "adjective", "preposition","conjugation", "pronoun");

        En quill, entradas de 5 bytes
        4 letras para palabra
        1 byte para numero de palabra
        */

        z80_int maintop;
        z80_int mainattr;

        int quillversion;

        util_unpaws_get_maintop_mainattr(&maintop,&mainattr,&quillversion);

        int longitud_total_palabra;
        int longitud_palabra;

        if (quillversion==0) {
                longitud_total_palabra=7;
                longitud_palabra=5;
        }
        
        else {
                longitud_total_palabra=5;
                longitud_palabra=4;
        }

       int palabras=0;

       char buffer_palabra[6];

       int salir=0;

        //Por defecto asumimos no encontrado
        strcpy(texto_destino,"?");

       do {
               //Copiar palabra a buffer
               int i;
               z80_byte caracter;

                if (daad_peek(puntero)==0) salir=1;
                else {

               for (i=0;i<longitud_palabra;i++) {
                       caracter=daad_peek(puntero+i) ^255;
                       //Si hay espacio, fin
                       //if (caracter==32) break;


                       caracter=chardetect_convert_daad_accents(caracter);

                       //Pasar a mayusculas por si acaso
                       caracter=letra_mayuscula(caracter);



                       if (caracter<32 || caracter>126) caracter='?';
                       buffer_palabra[i]=caracter;
               }
               buffer_palabra[i]=0;

               //if (buffer_palabra[0]<32 || buffer_palabra[0]>127) {
                       //No encontrado
                //       return;
               //}
               //else  {
                       z80_byte numero_palabra=daad_peek(puntero+longitud_palabra);
                       z80_byte tipo_palabra;

                       if (quillversion==0) tipo_palabra=daad_peek(puntero+6);
                       else tipo_palabra=0;
                       //debug_printf (VERBOSE_DEBUG,"Adding word: %s",buffer_palabra);
                       //util_unpawsgac_add_word_kb(buffer_palabra);
                       //palabras++;
                       if (numero_palabra==numero_palabra_buscar && (tipo_palabra==tipo_palabra_buscar || tipo_palabra==255)) {
                               strcpy(texto_destino,buffer_palabra);
                               return;
                       }
               //}

               puntero+=longitud_total_palabra;
               palabras++;

               //Agregar un limite por si acaso
               if (palabras==65535) salir=1;
                }

       } while (!salir);
     
}

void util_daad_paws_locate_word(z80_byte numero_palabra_buscar,z80_byte tipo_palabra_buscar,char *texto_destino)
{
        if (util_daad_detect() ) util_daad_locate_word(numero_palabra_buscar,tipo_palabra_buscar,texto_destino);
        else util_paws_locate_word(numero_palabra_buscar,tipo_palabra_buscar,texto_destino);
}

//Listado de objetos daad
/*
En dirección 0x8400+26 está el puntero?
En esa direccion hay una tabla lookup, 2 bytes por objeto, que son a su vez un puntero a la direccion donde empieza el texto de cada objeto
el texto en sí, con XOR FF
texto acaba con 0x0A (o 0xF5 antes de hacerle el XOR FF para devolverlo)
caracteres con acentos etc códigos por debajo del 32
*/


//Comun para daad y paws
z80_int util_daad_get_start_objects_names(void)
{

        z80_int puntero;

        z80_int dir;

        if (util_daad_detect()) {
        puntero=util_daad_get_start_pointers()+12;
        dir=value_8_to_16(daad_peek(puntero+1),daad_peek(puntero));        
        }
        else {
                util_unpaws_init_parameters();
                dir=util_unpaws_OffObj;
                //printf ("Obj messages: %XH\n",dir);
        }

        return dir;
}

//Comun para daad y paws
z80_int util_daad_get_start_locat_messages(void)
{

        z80_int puntero;

        z80_int dir;

        if (util_daad_detect()) {
        puntero=util_daad_get_start_pointers()+14;
        dir=value_8_to_16(daad_peek(puntero+1),daad_peek(puntero));
        }
        else {
                //Paws
                util_unpaws_init_parameters();
                dir=util_unpaws_OffLoc;
        }


        return dir;
}

//Comun para daad y paws
z80_int util_daad_get_start_graphics(void)
{

        z80_int puntero;

        z80_int dir;

        if (util_daad_detect()) {
        puntero=65521;
        dir=value_8_to_16(daad_peek(puntero+1),daad_peek(puntero));

        //temp
        //dir=63308;
        }
        else {
                //Paws
                util_unpaws_init_parameters();
                dir=util_unpaws_OffGraph;
        }


        return dir;
}

z80_int util_daad_get_graphics_location(z80_byte location)
{
    z80_int table_dir=util_daad_get_start_graphics();
    if (table_dir==0) return 0;

    int offset=table_dir+location*2;
    //printf("offset location %d: %d\n",location,offset);

    if (offset>65533) {
        //Tabla se iria mas alla del limite. Decir fin
        return 0;
    }

    z80_int graphics=peek_word_no_time(offset);

    return graphics;
}

z80_int util_daad_get_total_graphics(void)
{

    //buscar hasta puntero 0
    int i=0;

    for (i=0;util_daad_get_graphics_location(i)!=0 && i<255;i++) {
        //printf("i %d\n",i);
    }

    //printf("total locations: %d\n",i);

    return i;
}

//Comun para daad y paws
z80_int util_daad_get_start_graphics_attr(void)
{

        z80_int puntero;

        z80_int dir;

        if (util_daad_detect()) {
        //puntero=util_daad_get_start_pointers()+14;
        puntero=65523;
        dir=value_8_to_16(daad_peek(puntero+1),daad_peek(puntero));
        }
        else {
                //Paws
                util_unpaws_init_parameters();
                dir=util_unpaws_OffGraphAttr;
        }


        return dir;
}

//Retorna table_attr. 0 si no se encuentra
z80_int util_daad_get_graphics_attr(z80_byte location,int *ink,int *paper,int *is_picture)
{

    int esdaad=util_daad_detect();

    z80_int table_attr=util_daad_get_start_graphics_attr();

    if (table_attr==0) {
        return 0;
    }    

    z80_byte gflag;    

    if (esdaad) {
        gflag = peek_byte_no_time(table_attr+location*5);    
    }
    else {    
        gflag = peek_byte_no_time(table_attr+location);
    }

    *is_picture=gflag & 0x80;
    *ink=gflag & 7;
    *paper=(gflag >> 3) & 7;
            

    return table_attr;
}

//Dice si la aventura tiene graficos o no
int util_daad_has_graphics(void)
{
   z80_int table_dir=util_daad_get_start_graphics();
   z80_int table_attr=util_daad_get_start_graphics_attr();

    if (table_dir==0 || table_attr==0) return 0;
    else return 1;    
}

z80_int util_gac_get_start_graphics(void)
{
    return peek_word_no_time(0xA52F);
}

z80_int util_gac_get_graphics_location(int location,int *location_id)
{
    z80_int table_dir=util_gac_get_start_graphics();
    if (table_dir==0) return 0;

    //Info:
    //word: location
    //word: longitud contando estos 4 bytes
    //byte: numero comandos
    //comandos...

    int i;

    //printf("inicio tabla: %d\n",table_dir);

    //hasta que se llegue a direccion o table_dir "de la vuelta" (salte a rom)
    for (i=0;i<location && table_dir>16383;i++) {
        z80_int longitud=peek_word_no_time(table_dir+2);
        //printf("tabla: %d longitud: %d\n",table_dir,longitud);
        table_dir +=longitud;
    }

    //printf("tabla final: %d\n",table_dir);

    if (table_dir<16384) return 0;

    //retornamos al byte de numero comandos
    else {
        *location_id=peek_word_no_time(table_dir);
        return table_dir+4;
    }

}

//Retorna el tamaño de un grafico, en comandos y en bytes
void util_gac_get_graphics_size(int location,int *location_commands,int *location_size)
{
    int location_id;

    z80_int table_dir=util_gac_get_graphics_location(location,&location_id);

    if (table_dir==0) {
        *location_commands=0;
        *location_size=0;
        return;
    }
    else {
        *location_commands=peek_byte_no_time(table_dir);
        *location_size=peek_word_no_time(table_dir-2);
        return;
    }


}

//Retorna en que posicion (0,1,...) esta el id de habitacion indicado
//-1 si no existe
int util_gac_get_index_location_by_id(int location_id)
{
    z80_int table_dir=util_gac_get_start_graphics();
    if (table_dir==0) return -1;

    //Info:
    //word: location
    //word: longitud contando estos 4 bytes
    //byte: numero comandos
    //comandos...

    int i;

    //printf("inicio tabla: %d\n",table_dir);

    //hasta que se llegue a direccion o table_dir "de la vuelta" (salte a rom)
    for (i=0;peek_word_no_time(table_dir)!=location_id && table_dir>16383;i++) {
        z80_int longitud=peek_word_no_time(table_dir+2);
        //printf("tabla: %d longitud: %d\n",table_dir,longitud);
        table_dir +=longitud;
    }

    //printf("tabla final: %d\n",table_dir);

    if (table_dir<16384) return -1;

    return i;

}



z80_int util_gac_get_total_graphics(void)
{
    z80_int table_dir=util_gac_get_start_graphics();
    if (table_dir==0) return 0;

    //Info:
    //word: id location
    //word: longitud contando estos 4 bytes
    //byte: numero comandos
    //comandos...

    int i;

    //printf("inicio tabla: %d\n",table_dir);

    //hasta que id_location sea 0 o contador "de la vuelta"
    for (i=0;peek_word_no_time(table_dir)!=0 && table_dir>16383;i++) {
        z80_int longitud=peek_word_no_time(table_dir+2);
        //printf("tabla: %d longitud: %d\n",table_dir,longitud);
        table_dir +=longitud;
    }

    //printf("tabla final: %d\n",table_dir);

    return i;

}

//comun para gac y daad/paws/quill
z80_int util_gac_daad_get_total_graphics(void)
{

    int max_localizaciones;

    if (util_gac_detect() ) {
        max_localizaciones=util_gac_get_total_graphics();
    }  
    else {
        max_localizaciones=util_daad_get_total_graphics();
    }    

    return max_localizaciones;
}



//Comun para daad y paws
z80_int util_daad_get_start_user_messages(void)
{

        z80_int puntero;

        z80_int dir;


        if (util_daad_detect()) {
        puntero=util_daad_get_start_pointers()+16;
        dir=value_8_to_16(daad_peek(puntero+1),daad_peek(puntero));
        }
        else {
                util_unpaws_init_parameters();
                dir=util_unpaws_OffMsg;
                //printf ("user messages: %XH\n",dir);
        }


        return dir;
}

//Comun para daad y paws
z80_int util_daad_get_start_sys_messages(void)
{

        z80_int puntero;

        z80_int dir;

        if (util_daad_detect()) {
        puntero=util_daad_get_start_pointers()+18;
        dir=value_8_to_16(daad_peek(puntero+1),daad_peek(puntero));        
        }
        else {
                //Paws
                util_unpaws_init_parameters();
                dir=util_unpaws_OffSys;
                //printf ("sys messages: %XH\n",dir);
        }

        return dir;
}


//Comun para daad y paws
z80_int util_daad_get_start_compressed_messages(void)
{

        z80_int puntero;

        z80_int dir;

        if (util_daad_detect()) {
        puntero=util_daad_get_start_pointers()+8;
        dir=value_8_to_16(daad_peek(puntero+1),daad_peek(puntero));        
        }
        else {
                //paws
                util_unpaws_init_parameters();
                dir=util_unpaws_OffAbreviations;
                //printf ("compressed: %XH\n",dir);
        }

        return dir;
}

//Comun para daad y paws
z80_int util_daad_get_num_objects_description(void)
{

        z80_int puntero;

        z80_int dir;

        if (util_daad_detect()) {
        puntero=util_daad_get_start_pointers()+3;
        dir=daad_peek(puntero);        
        }
        else {
                //paws
                util_unpaws_init_parameters();
                dir=util_unpaws_NumObj;
        }

        return dir;
}

//Comun para daad y paws
z80_int util_daad_get_num_locat_messages(void)
{

        z80_int puntero;

        z80_int dir;

        if (util_daad_detect()) {
        puntero=util_daad_get_start_pointers()+4;
        dir=daad_peek(puntero);        
        }
        else {
                //Paws
                util_unpaws_init_parameters();
                dir=util_unpaws_NumLoc;
        }

        return dir;
}

//Comun para daad y paws
z80_int util_daad_get_num_user_messages(void)
{

        z80_int puntero;

        z80_int dir;

        if (util_daad_detect()) {
                puntero=util_daad_get_start_pointers()+5;
                dir=daad_peek(puntero);
        }

        else {
                //paws
                util_unpaws_init_parameters();
                dir=util_unpaws_NumMsg;
        }


        return dir;
}

//Comun para daad y paws
z80_int util_daad_get_num_sys_messages(void)
{

        z80_int puntero;

        z80_int dir;

        if (util_daad_detect()) {
        puntero=util_daad_get_start_pointers()+6;
        dir=daad_peek(puntero);        
        }
        else {
                //paws
                util_unpaws_init_parameters();
                dir=util_unpaws_NumSys;
        }

        return dir;
}

z80_int util_daad_get_pc_parser(void)
{
        if (MACHINE_IS_CPC) return DAAD_PARSER_BREAKPOINT_PC_CPC;
        else {
            z80_int dir=util_daad_get_start_pointers();

            if (dir==0x8380) return 0x6360;

            if (dir==0x8480) return 0x647b;

            return DAAD_PARSER_BREAKPOINT_PC_SPECTRUM;
        }
}

int util_paws_is_opcodes_parser(z80_int dir)
{
                        /*
                LD A,(BC)
                CP FF

                0a fe ff
                 */

                 if (
                        daad_peek(dir)==0x0a &&
                        daad_peek(dir+1)==0xfe &&
                        daad_peek(dir+2)==0xff 
                ) {    
                        return 1;
                }
                else return 0;   
}

z80_int util_paws_get_pc_parser(void)
{

        z80_int maintop;
        z80_int mainattr;
        int quillversion;

        util_unpaws_get_maintop_mainattr(&maintop,&mainattr,&quillversion);

        z80_int dir=0x76a6; //por defecto

        

                z80_int dir2=0x7671;
                if (util_paws_is_opcodes_parser(dir2)) dir=dir2;


                z80_int dir3=0x76aa;
                if (util_paws_is_opcodes_parser(dir3)) dir=dir3;

                //quill
                z80_int dir4=0x63fc;
                if (util_paws_is_opcodes_parser(dir4)) dir=dir4;

                //quill (bugsy)
                z80_int dir5=0x6448;
                if (util_paws_is_opcodes_parser(dir5)) dir=dir5;          

                //quill (evil realm)
                z80_int dir6=0x644d;
                if (util_paws_is_opcodes_parser(dir6)) dir=dir6;                                         
    

        return dir;
}

int util_paws_is_in_parser(void)
{
        if (reg_pc==util_paws_get_pc_parser() ) return 1;
        
        else return 0;
}

int util_daad_is_in_parser(void)
{
        if (reg_pc==util_daad_get_pc_parser() ) return 1;
        
        else return 0;
}

//Retorna un mensaje de daad N, de la tabla indicada (tabla de punteros de 16 bits)
//Mensajes con xor 255 y finaliza mensaje con F5 (o 10 despues de hacerle el xor)
void util_daad_get_message_table_lookup(z80_byte index,z80_int table_dir,char *texto,int limite_mensajes)
{

        if (index>limite_mensajes-1) {
                strcpy(texto,"Message out of range");
                return;
        }

        int es_daad=util_daad_detect();

        z80_byte caracter_fin;
        z80_byte limite_caracter_comprimido;

        if (es_daad) {
                caracter_fin=10;
                limite_caracter_comprimido=127;
        }
        else {
                caracter_fin=31;
                limite_caracter_comprimido=164;
        }

        z80_int offset_pointer=table_dir+index*2;

        z80_int dir=value_8_to_16(daad_peek(offset_pointer+1),daad_peek(offset_pointer));

        //leer hasta byte valor 10, o maximo 255 longitud
        int destino=0;

        z80_byte caracter=0;

        while (destino<255 && caracter!=caracter_fin) {
                caracter=daad_peek(dir++) ^255;
                if (caracter!=caracter_fin) {

                        caracter=chardetect_convert_daad_accents(caracter);

                        if (caracter<32 || caracter>127) {
                                if (caracter>limite_caracter_comprimido) {
                                        //Meter token
                                        char buffer_temp[256];
                                        //printf ("token %d\n",caracter & 127);
                                        if (es_daad) util_daad_get_compressed_message(caracter & 127,buffer_temp);
                                        else util_daad_get_compressed_message(caracter-164,buffer_temp);
                                        unsigned int i;
                                        for (i=0;i<strlen(buffer_temp) && destino<255;i++) {
                                                texto[destino++]=buffer_temp[i];
                                        }
                                }
                                else {
                                        caracter='?';
                                        texto[destino++]=caracter;
                                }
                        }

                        else texto[destino++]=caracter;
                }

                //printf ("destino %d caracter %d\n",destino,caracter);
        }

        texto[destino]=0;
}

void util_textadventure_filter_message(char *texto)
{



    int inicio;
    int dest=0;
    int longitud_texto=strlen(texto);
    for (inicio=0;inicio<longitud_texto;inicio++) {
        z80_byte c=texto[inicio];

        //estos se utilizan en los char filters
        c=chardetect_convert_daad_accents(c);

        c=chardetect_convert_paws_accents(c);

        if (c>31 && c<127 && c!='?') {
            texto[dest++]=c;
        }

        
    }    

    texto[dest]=0;
}



void util_daad_get_object_description(z80_byte index,char *texto)
{

        z80_int table_dir=util_daad_get_start_objects_names();
        util_daad_get_message_table_lookup(index,table_dir,texto,util_daad_get_num_objects_description() );

    //filtrar caracteres
    util_textadventure_filter_message(texto);        

}


void util_daad_get_user_message(z80_byte index,char *texto)
{

        z80_int table_dir;

        table_dir=util_daad_get_start_user_messages();

        util_daad_get_message_table_lookup(index,table_dir,texto,util_daad_get_num_user_messages() );
}


void util_daad_get_sys_message(z80_byte index,char *texto)
{


       /*
        CUIDADO: No llamar a util_paws_is_spanish desde aqui o se meteria en un bucle recursivo
       */

        z80_int table_dir=util_daad_get_start_sys_messages();
        
        util_daad_get_message_table_lookup(index,table_dir,texto,util_daad_get_num_sys_messages());
}


void util_daad_get_locat_message(z80_byte index,char *texto)
{

    z80_int table_dir=util_daad_get_start_locat_messages();
    util_daad_get_message_table_lookup(index,table_dir,texto,util_daad_get_num_locat_messages() );

    //filtrar caracteres
    util_textadventure_filter_message(texto);
   
}


//Funcionalidad reemplazada con menu_debug_daad_view_graphics_render_recursive
void old_delete_util_daad_get_graphics_list_commands(z80_byte location,char *texto)
{

//See https://github.com/Utodev/unPAWs/blob/master/Unpaws.pas

    z80_int table_dir=util_daad_get_start_graphics();

    if (table_dir==0) {
        menu_error_message("Graphics not found");
        return;
    }

    z80_byte gflag;

    char buffer_temporal[200];       

    int esdaad=util_daad_detect();     


    z80_int table_attr=util_daad_get_start_graphics_attr();

    if (table_attr==0) {
        menu_error_message("Graphics attributes not found");
        return;
    }        

    //Write(FOut,'Location ',n:3, ' graphics flags: ');
    if (esdaad) {
        gflag = peek_byte_no_time(table_attr+location*5);    
    }
    else {
        gflag = peek_byte_no_time(table_attr+location);
    }


    /*
       if (gflag & 0x80)  
           Write(FOut,'Picture.    ')
       else
           Write(FOut,'Subroutine. ');

       WriteLn(FOut, 'Ink=',gflag mod 8 ,' Paper=',
               (gflag and $3f) div 8, ' Bit6=', (gflag and 64) div  64);*/

            
    sprintf(buffer_temporal,"Location %-3d graphics flags: %s Ink=%d Paper=%d Bit6=%d\n",location,
        (gflag & 0x80 ? "Picture.    " : "Subroutine. "),
        gflag & 7, (gflag >> 3) & 7, (gflag>>6) & 1 
    );

    util_concat_string(texto,buffer_temporal,MAX_TEXTO_GENERIC_MESSAGE);
        

        printf("OffGraph: %d\n",table_dir);

        //Inicio tabla graficos
        z80_int graphics=peek_word_no_time(table_dir+location*2);

        printf("Start graphics location %d: %d\n",location,graphics);
        //util_daad_get_message_table_lookup(index,table_dir,texto,util_daad_get_num_locat_messages() );

char *plot_moves[]= {
" 001  000",
" 001  001",
" 000  001",
"-001  001",
"-001  000",
"-001 -001",
" 000 -001",
" 001 -001" 
}; 

    int salir=0;

    z80_int neg[8];

    z80_int maintop;
    z80_int mainattr;

    int quillversion;

    util_unpaws_get_maintop_mainattr(&maintop,&mainattr,&quillversion);    

    //printf("quill version: %d\n",quillversion);


    

    while (!salir) {

        gflag=peek_byte_no_time(graphics);
        z80_byte nargs;

        z80_byte value;
        char inv, ovr;

        int estexto=0;

        int line_comprimido=0;

        //Formato del byte con el comando:
        //-----xxx 3 bits inferiores: comando
        //----x--- Bit 3 (0x08) : over / flags        -|
        //---x---- Bit 4 (0x10) : inverse / flags      |
        //--x----- Bit 5 (0x20): flags                 |  Parametro 0 ("value")
        //-x------ Bit 6 (0x40): signo parametro 1    -|
        //x------- Bit 7 (0x80): signo parametro 2 / flags   

        int i;
        for (i=0;i<8;i++) neg[i]=0;

        inv = ' '; ovr = ' ';
        if ((gflag & 8) != 0) ovr = 'o';
        if ((gflag & 16) !=0) inv = 'i';
        value = gflag /  8;
        nargs=0;        

        switch (gflag & 7) {
	         case 0:

                nargs = 2;
                if ((ovr=='o') && (inv=='i')) {
                    sprintf (buffer_temporal,"ABS MOVE   ");
                }
                else {
                    sprintf (buffer_temporal,"PLOT    %c%c ",ovr,inv);
                }
            break;

	         case 1: 
                nargs = 2;
                
                if ((gflag & 0x40) != 0) neg[0] = 1;
                if ((gflag & 0x80) != 0) neg[1] = 1;

		     if (ovr=='o' && inv=='i') 
                       sprintf (buffer_temporal,"REL MOVE   ");
		     else {
                       sprintf (buffer_temporal,"LINE    %c%c ",ovr,inv);
             }

                       if (esdaad) {
                           //Ver si tiene compresion
                           if (gflag & 0x20) {
                               line_comprimido=1;
                               nargs=1;
                           }
                       }
             
                       
		    break;


                case 2:
                
	         
                     if ((gflag & 0x10)!=0  && (gflag & 0x20)!=0) 
		      {
                            if ((gflag & 0x40) !=0) neg[0] = 1;
                            if ((gflag & 0x80) !=0) neg[1] = 1;
                        
			    nargs = 3;
                
                            if (quillversion==0) 
                             sprintf (buffer_temporal,"SHADE   %c%c ",ovr,inv);
                            else
                             sprintf (buffer_temporal,"BSHADE     ");
		      }
		     else
                     if ((gflag & 0x10) !=0)
                      {
		                nargs = 4;
                       sprintf (buffer_temporal,"BLOCK      ");
                      }
		     else
                     if ((gflag & 0x20) !=0)
		      {
                            if ((gflag & 0x40) !=0 ) neg[0] = 1;
                            if ((gflag & 0x80) !=0 ) neg[1] = 1;
			    nargs = 3;
                            sprintf (buffer_temporal,"SHADE   %c%c ",ovr,inv);
		      }
		     else
		      {
                            if ((gflag & 0x40) !=0 ) neg[0] = 1;
                            if ((gflag & 0x80) !=0 ) neg[1] = 1;
			    nargs = 2;
                            sprintf (buffer_temporal,"FILL       ");
		      }
		    
            
            break;


	         case 3: 
                     nargs = 1;
                     int mirror_x=gflag&64;
                     int mirror_y=gflag&128;

                    if (!esdaad) mirror_x=mirror_y=0;
 
                     //Chichen itza, localizacion 4 utiliza esto
                     sprintf (buffer_temporal,"GOSUB    sc=%d %s %s",value & 7,
                        (mirror_x ? "MX" : "  "),
                        (mirror_y ? "MY" : "  ")
                     );
                    
		    break;

            case 4:
            
                     if (quillversion==0)
                     {
                       nargs = 3;
                       sprintf (buffer_temporal,"TEXT    %c%c %d ",ovr,inv,value/4);
                       estexto=1;
                     }
                     else
                     {
                       nargs=0;
                       sprintf (buffer_temporal,"RPLOT   %c%c %s",ovr,inv,plot_moves[value/4]);
                     }
                     
            

		    break;

	        case 5:

                     nargs = 0;
                     
		     if ((gflag & 0x80) !=0) 
                      sprintf (buffer_temporal,"BRIGHT      %d",value & 15);
                     else
                      sprintf (buffer_temporal,"PAPER      %d",value & 15);
        
            
           break;

           case 6: 
                     nargs = 0;
                     
		     if ((gflag & 0x80) !=0) 
                      sprintf (buffer_temporal,"FLASH       %d",value & 15);
                     else
                      sprintf (buffer_temporal,"INK         %d",value & 15);
                      
		    
            break;
            case 7:
                sprintf (buffer_temporal,"END ");
                salir=1;
                nargs=0;
            break;
        }

        graphics++;
        
        util_concat_string(texto,buffer_temporal,MAX_TEXTO_GENERIC_MESSAGE);

        if (line_comprimido) {
            z80_byte byte_leido=peek_byte_no_time(graphics);

            z80_byte arg1=(byte_leido >> 4)&0xF;
            z80_byte arg2=byte_leido  &0xF ;


            sprintf(buffer_temporal,"%c%-3d ",(neg[0]!=0 ? '-' : ' ' ), arg1);
            util_concat_string(texto,buffer_temporal,MAX_TEXTO_GENERIC_MESSAGE);
            sprintf(buffer_temporal,"%c%-3d ",(neg[1]!=0 ? '-' : ' ' ), arg2);
            util_concat_string(texto,buffer_temporal,MAX_TEXTO_GENERIC_MESSAGE);            
            graphics++;
        }

        else {
        for (i=0;i<nargs;i++) {
            z80_byte byte_leido=peek_byte_no_time(graphics);
            if (estexto && i==0) {
                if (byte_leido>=32 && byte_leido<=126) sprintf(buffer_temporal,"%d('%c') ",byte_leido,byte_leido);
                else sprintf(buffer_temporal,"%3d ",byte_leido);
            }

            else {
                sprintf(buffer_temporal,"%c%-3d ",(neg[i]!=0 ? '-' : ' ' ), byte_leido);

	       //for m := 0 to nargs-1 do
           //     Write(FOut, Select(neg[m]<>0, '-',' '), IntToStr2(Peek(Offs+1+m),3,true),' ');
	       //WriteLn(Fout);

            }

            util_concat_string(texto,buffer_temporal,MAX_TEXTO_GENERIC_MESSAGE);

            graphics++;
        }
        }
        //printf("\n");
        util_concat_string(texto,"\n",MAX_TEXTO_GENERIC_MESSAGE);

    }

}


//Retorna un mensaje token de daad, que finaliza con bit 7 alzado
void util_daad_get_token_message(z80_byte index,z80_int table_dir,char *texto)
{

        //Ir contando tokens hasta llegar al que interesa
        int i;
        z80_byte caracter;
        for (i=0;i<index;) {
                caracter=daad_peek(table_dir++);
                if (caracter>127) i++;
        }

        int salir=0;
        int destino=0;

        do {
                caracter=daad_peek(table_dir++);

                       caracter=chardetect_convert_daad_accents(caracter);


                if (caracter>127) {
                        caracter -=128;
                        caracter=chardetect_convert_daad_accents(caracter);
                        salir=1;
                }
                texto[destino++]=caracter;

                //printf ("destino %d caracter %d\n",destino,caracter);
        } while (!salir && destino<255);

        texto[destino]=0;
}


void util_daad_get_compressed_message(z80_byte index,char *texto)
{

//fseek ($file, $pos_tokens + 1);  // It seems actual token table starts one byte after the one the header points to (daad)
        z80_int table_dir=util_daad_get_start_compressed_messages();
        if (util_daad_detect()) table_dir++;
        util_daad_get_token_message(index,table_dir,texto);
}


int util_daad_condact_uses_message(void)
{
        //Retorna 1 si BC apunta a un condacto que usa mensaje como parametro:
	//MES y MESSAGE a la tabla MTX (mensajes de usuario). SYSMES a STX (mensajes del sistema) y DESC a LTX (localidades)
	/*
  {1,"MES    "}, //  77 $4D

  {1,"MESSAGE"}, //  38 $26


  {1,"SYSMESS"}, //  54 $36


  {1,"DESC   "}, //  19 $13


  {1,"NOUN2  "}, //  69 $45


  {1,"ADJECT1"}, //  16 $10
    {1,"ADJECT2"}, //  70 $46
  {1,"ADVERB "}, //  17 $11
    {1,"PREP   "}, //  68 $44


	*/

	z80_int direccion_desensamblar=value_8_to_16(reg_b,reg_c);

	z80_byte opcode_daad=daad_peek(direccion_desensamblar) & 127;
	

	if (opcode_daad==77 || opcode_daad==38 || opcode_daad==54 || opcode_daad==19 || opcode_daad==69|| opcode_daad==16 || opcode_daad==70 || opcode_daad==17 || opcode_daad==68) {
                return 1;
	} 

	else return 0;
}

int util_daad_get_limit_flags(void)
{
        int limite_max=255;        
        		//quill tiene 33 flags y 210 objetos
		//Tabla Para quill de 33 flags y 210 objetos (33 oficiales, realmente 37)
		if (util_undaad_unpaws_is_quill() ) {
				limite_max=36;			
		}	

        return limite_max;
}

int util_daad_get_limit_objects(void)
{
        int limite_max=255;        
        		//quill tiene 33 flags y 210 objetos
		//Tabla Para quill de 33 flags y 210 objetos
		if (util_undaad_unpaws_is_quill() ) {
			//objetos
			limite_max=209;
		}	

        return limite_max;
}

//Retorna mensaje relacionado con condacto
void util_daad_get_condact_message(char *buffer)
{
	//MES y MESSAGE a la tabla MTX (mensajes de usuario). SYSMES a STX (mensajes del sistema) y DESC a LTX (localidades)
	/*
  {1,"MES    "}, //  77 $4D

  {1,"MESSAGE"}, //  38 $26


  {1,"SYSMESS"}, //  54 $36


  {1,"DESC   "}, //  19 $13

    {1,"NOUN2  "}, //  69 $45


  {1,"ADJECT1"}, //  16 $10
    {1,"ADJECT2"}, //  70 $46
  {1,"ADVERB "}, //  17 $11
    {1,"PREP   "}, //  68 $44

	*/

	z80_int direccion_desensamblar=value_8_to_16(reg_b,reg_c);

	z80_byte opcode_daad=daad_peek(direccion_desensamblar);
	z80_byte param_message=daad_peek(direccion_desensamblar+1);

	//int redireccion=0;
	if (opcode_daad>127) {
		//redireccion=1;
		opcode_daad -=128;
		param_message=util_daad_get_flag_value(param_message);
	}

	buffer[0]=0;

	if (opcode_daad==77 || opcode_daad==38) {
		util_daad_get_user_message(param_message,buffer);
	} 

	if (opcode_daad==54) {
		util_daad_get_sys_message(param_message,buffer);
	} 	

	if (opcode_daad==19) {
		util_daad_get_locat_message(param_message,buffer);
	} 		

	//{1,"NOUN2  "}, //  69 $45
	if (opcode_daad==69) {
		util_daad_paws_locate_word(param_message,2,buffer);
	} 		

  //{1,"ADJECT1"}, //  16 $10
  //{1,"ADJECT2"}, //  70 $46
  	if (opcode_daad==16 || opcode_daad==70) {
		util_daad_paws_locate_word(param_message,3,buffer);
	} 	



  	//{1,"ADVERB "}, //  17 $11
    if (opcode_daad==17) {
		util_daad_paws_locate_word(param_message,1,buffer);
	} 

    //{1,"PREP   "}, //  68 $44	
	if (opcode_daad==68) {
		util_daad_paws_locate_word(param_message,4,buffer);
	} 	


	menu_generic_message("Message",buffer);


}


void debug_get_daad_breakpoint_string(char *texto)
{
	/*
	Retorna cadena breakpoint tipo 	PC=617D si A=188
	Debe detener justo despues del tipico LD A,(BC)

	#define DAAD_PARSER_BREAKPOINT_PC 0x617c
#define DAAD_PARSER_CONDACT_BREAKPOINT 0xbc
	*/


	//de momento en decimal (dado que aun no mostamos hexadecimal en parser) para que al comparar salga igual
	sprintf (texto,"PC=%d AND A=%d",util_daad_get_pc_parser()+1,DAAD_PARSER_CONDACT_BREAKPOINT);

}


//Retorna cadena de breakpoint de step to step para pararse en el parser de condacts, y siempre que condact no sea FFH
void debug_get_daad_step_breakpoint_string(char *texto)
{
	z80_int breakpoint_dir;

	if (util_daad_detect() ) breakpoint_dir=util_daad_get_pc_parser();
	if (util_textadv_detect_paws_quill() ) breakpoint_dir=util_paws_get_pc_parser();	


	//de momento en decimal (dado que aun no mostamos hexadecimal en parser) para que al comparar salga igual
	sprintf (texto,"PC=%d AND PEEK(BC)<>255",breakpoint_dir);

}


//Retorna cadena de breakpoint cuando va a leer condact PARSE en daad
void debug_get_daad_runto_parse_string(char *texto)
{
	z80_int breakpoint_dir;

	if (util_daad_detect() ) breakpoint_dir=util_daad_get_pc_parser();
	if (util_textadv_detect_paws_quill() ) breakpoint_dir=util_paws_get_pc_parser();


	//de momento en decimal (dado que aun no mostamos hexadecimal en parser) para que al comparar salga igual
	sprintf (texto,"PC=%d AND PEEK(BC)=73",breakpoint_dir);

}


void util_clear_text_adventure_kdb(void)
{
	osd_adv_kbd_defined=0;
}


void util_add_text_adventure_kdb(char *texto)
{
	if (osd_adv_kbd_defined==MAX_OSD_ADV_KEYB_WORDS) {
		debug_printf (VERBOSE_ERR,"Maximum keyboard text entries reached");
		return;
	}

	if (strlen(texto)>MAX_OSD_ADV_KEYB_TEXT_LENGTH-1) {
		debug_printf (VERBOSE_ERR,"String %s too long to add to the keyboard text entries (max: %d)",texto,MAX_OSD_ADV_KEYB_TEXT_LENGTH-1);
		return;
	}



//OSD teclado aventura
/*
//numero maximo de entradas
#define MAX_OSD_ADV_KEYB_WORDS 40
//longitud maximo de cada entrada
#define MAX_OSD_ADV_KEYB_TEXT_LENGTH 20
*/


//3 entradas definidas de ejemplo
//int osd_adv_kbd_defined=100;
//char osd_adv_kbd_list[MAX_OSD_ADV_KEYB_WORDS][MAX_OSD_ADV_KEYB_TEXT_LENGTH]={
        //Truco para poder poner " en la configuracion, mientras no tenga un parser que me permita escapar,
        //es meter la barra invertida
        if (!strcmp(texto,"\\")) strcpy(osd_adv_kbd_list[osd_adv_kbd_defined++],"\"");

	else strcpy(osd_adv_kbd_list[osd_adv_kbd_defined++],texto);

}




z80_byte chardetect_convert_daad_accents(z80_byte c)
{
			if (c=='\x15') c='a';
			if (c=='\x16') c='e';
			if (c=='\x17') c='i';
			if (c=='\x18') c='o';
			if (c=='\x19') c='u';

			//eñe
			if (c=='\x1a') c='n';

			return c;	
}

z80_byte chardetect_convert_paws_accents(z80_byte c)
{
			//Acentuadas. De momento las retornamos tal cual sin acentos
			if (c=='@') c='a';
			if (c=='#') c='e';
			if (c=='$') c='i';
			if (c=='%') c='o';
			if (c=='&') c='u';
			
			
			//eñe
			if (c=='|') c='n';

            return c;    
}

//Tabla de desensamblado de condacts de daad



struct s_daad_paws_contacts paws_contacts_array[]={
{1,"AT     "}, //0
{1,"NOTAT  "},
{1,"ATGT   "},
{1,"ATLT   "},
{1,"PRESENT"},
{1,"ABSENT "}, //5
{1,"WORN   "},
{1,"NOTWORN"},
{1,"CARRIED"},
{1,"NOTCARR"},
{1,"CHANCE "}, //10
{1,"ZERO   "},
{1,"NOTZERO"},
{2,"EQ     "},
{2,"GT     "},
{2,"LT     "}, //15
{1,"ADJECT1"},
{1,"ADVERB "},
{0,"INVEN  "},
{0,"DESC   "},
{0,"QUIT   "}, //20
{0,"END    "},
{0,"DONE   "},
{0,"OK     "},
{0,"ANYKEY "},
{0,"SAVE   "}, //25
{0,"LOAD   "},
{0,"TURNS  "},
{0,"SCORE  "},
{0,"CLS    "},
{0,"DROPALL"}, //30
{0,"AUTOG  "},
{0,"AUTOD  "},
{0,"AUTOW  "},
{0,"AUTOR  "},
{1,"PAUSE  "}, //35
{0,"TIMEOUT"},
{1,"GOTO   "},
{1,"MESSAGE"},
{1,"REMOVE "},
{1,"GET    "}, //40
{1,"DROP   "},
{1,"WEAR   "},
{1,"DESTROY"},
{1,"CREATE "},
{2,"SWAP   "}, //45
{2,"PLACE  "},
{1,"SET    "},
{1,"CLEAR  "},
{2,"PLUS   "},
{2,"MINUS  "}, //50
{2,"LET    "},
{0,"NEWLINE"},
{1,"PRINT  "},
{1,"SYSMESS"},
{2,"ISAT   "}, //55
{2,"COPYOF "},
{2,"COPYOO "},
{2,"COPYFO "},
{2,"COPYFF "},
{0,"LISTOBJ"}, //60
{1,"EXTERN "},
{0,"RAMSAVE"},
{1,"RAMLOAD"},
{2,"BEEP   "},
{1,"PAPER  "}, //65
{1,"INK    "},
{1,"BORDER "},
{1,"PREP   "},
{1,"NOUN2  "},
{1,"ADJECT2"}, //70
{2,"ADD    "},
{2,"SUB    "},
{0,"PARSE  "},
{1,"LISTAT "},
{1,"PROCESS"}, //75
{2,"SAME   "},
{1,"MES    "},
{1,"CHARSET"},
{2,"NOTEQ  "},
{2,"NOTSAME"}, //80
{2,"MODE   "},
{1,"LINE   "},
{2,"TIME   "},
{1,"PICTURE"},
{1,"DOALL  "}, //85
{1,"PROMPT "},
{1,"GRAPHIC"},
{2,"ISNOTAT"},
{2,"WEIGH  "},
{2,"PUTIN  "}, //90
{2,"TAKEOUT"},
{0,"NEWTEXT"},
{2,"ABILITY"},
{1,"WEIGHT "},
{1,"RANDOM "}, //95
{1,"INPUT  "},
{0,"SAVEAT "},
{0,"BACKAT "},
{2,"PRINTAT"},
{0,"WHATO  "}, //100
{1,"RESET  "},
{1,"PUTO   "},
{0,"NOTDONE"},
{1,"AUTOP  "},
{1,"AUTOT  "}, //105
{1,"MOVE   "},
{0,"PROTECT"},  //107
{0,"UNKNOWN"},  //108
{0,"UNKNOWN"},  
{0,"UNKNOWN"},  //110
{0,"UNKNOWN"},  
{0,"UNKNOWN"},  
{0,"UNKNOWN"},  
{0,"UNKNOWN"},  
{0,"UNKNOWN"},  //115
{0,"UNKNOWN"},  
{0,"UNKNOWN"},  
{0,"UNKNOWN"},  
{0,"UNKNOWN"},  
{0,"UNKNOWN"},  //120
{0,"UNKNOWN"},  
{0,"UNKNOWN"},  
{0,"UNKNOWN"},  
{0,"UNKNOWN"},  
{0,"UNKNOWN"},  //125
{0,"UNKNOWN"},  
{0,"UNKNOWN"},  //127

};

struct s_daad_paws_contacts daad_contacts_array[]={
  {1,"AT     "}, //   0 $00
  {1,"NOTAT  "}, //   1 $01
  {1,"ATGT   "}, //   2 $$02
  {1,"ATLT   "}, //   3 $03
  {1,"PRESENT"}, //   4 $04
  {1,"ABSENT "}, //   5 $05
  {1,"WORN   "}, //   6 $06
  {1,"NOTWORN"}, //   7 $07
  {1,"CARRIED"}, //   8 $08
  {1,"NOTCARR"}, //   9 $09
  {1,"CHANCE "}, //  10 $0A
  {1,"ZERO   "}, //  11 $0B
  {1,"NOTZERO"}, //  12 $0C
  {2,"EQ     "}, //  13 $0D
  {2,"GT     "}, //  14 $0E
  {2,"LT     "}, //  15 $0F
  {1,"ADJECT1"}, //  16 $10
  {1,"ADVERB "}, //  17 $11
  {2,"SFX    "}, //  18 $12
  {1,"DESC   "}, //  19 $13
  {0,"QUIT   "}, //  20 $14
  {0,"END    "}, //  21 $15
  {0,"DONE   "}, //  22 $16
  {0,"OK     "}, //  23 $17
  {0,"ANYKEY "}, //  24 $18
  {1,"SAVE   "}, //  25 $19
  {1,"LOAD   "}, //  26 $1A
  {1,"DPRINT "}, //  27 * $1B
  {1,"DISPLAY"}, //  28 * $1C
  {0,"CLS    "}, //  29 $1D
  {0,"DROPALL"}, //  30 $1E
  {0,"AUTOG  "}, //  31 $1F
  {0,"AUTOD  "}, //  32 $20
  {0,"AUTOW  "}, //  33 $21
  {0,"AUTOR  "}, //  34 $22
  {1,"PAUSE  "}, //  35 $23
  {2,"SYNONYM"}, //  36 * $24
  {1,"GOTO   "}, //  37 $25
  {1,"MESSAGE"}, //  38 $26
  {1,"REMOVE "}, //  39 $27
  {1,"GET    "}, //  40 $28
  {1,"DROP   "}, //  41 $29
  {1,"WEAR   "}, //  42 $2A
  {1,"DESTROY"}, //  43 $2B
  {1,"CREATE "}, //  44 $2C
  {2,"SWAP   "}, //  45 $2D
  {2,"PLACE  "}, //  46 $2E
  {1,"SET    "}, //  47 $2F
  {1,"CLEAR  "}, //  48 $30
  {2,"PLUS   "}, //  49 $31
  {2,"MINUS  "}, //  50 $32
  {2,"LET    "}, //  51 $33
  {0,"NEWLINE"}, //  52 $34
  {1,"PRINT  "}, //  53 $35
  {1,"SYSMESS"}, //  54 $36
  {2,"ISAT   "}, //  55 $37
  {1,"SETCO  "}, //  56 $38 COPYOF in old games 
  {0,"SPACE  "}, //  57 $39 COPYOO in old games
  {1,"HASAT  "}, //  58 $3A COPYFO in old games
  {1,"HASNAT "}, //  59 $3B COPYFF in old games
  {0,"LISTOBJ"}, //  60 $3C
  {2,"EXTERN "}, //  61 $3D
  {0,"RAMSAVE"}, //  62 $3E
  {1,"RAMLOAD"}, //  63 $3F
  {2,"BEEP   "}, //  64 $40
  {1,"PAPER  "}, //  65 $41
  {1,"INK    "}, //  66 $42
  {1,"BORDER "}, //  67 $43
  {1,"PREP   "}, //  68 $44
  {1,"NOUN2  "}, //  69 $45
  {1,"ADJECT2"}, //  70 $46
  {2,"ADD    "}, //  71 $47
  {2,"SUB    "}, //  72 $48
  {1,"PARSE  "}, //  73 $49
  {1,"LISTAT "}, //  74 $4A
  {1,"PROCESS"}, //  75 $4B
  {2,"SAME   "}, //  76 $4C
  {1,"MES    "}, //  77 $4D
  {1,"WINDOW "}, //  78 $4E
  {2,"NOTEQ  "}, //  79 $4F
  {2,"NOTSAME"}, //  80 $50
  {1,"MODE   "}, //  81 $51
  {2,"WINAT  "}, //  82 $52
  {2,"TIME   "}, //  83 $53
  {1,"PICTURE"}, //  84 $54
  {1,"DOALL  "}, //  85 $55
  {1,"MOUSE  "}, //  86 $56
  {2,"GFX    "}, //  87 $57
  {2,"ISNOTAT"}, //  88 $58
  {2,"WEIGH  "}, //  89 $59
  {2,"PUTIN  "}, //  90 $5A
  {2,"TAKEOUT"}, //  91 $5B
  {0,"NEWTEXT"}, //  92 $5C
  {2,"ABILITY"}, //  93 $5D
  {1,"WEIGHT "}, //  94 $5E
  {1,"RANDOM "}, //  95 $5F
  {2,"INPUT  "}, //  96 $60
  {0,"SAVEAT "}, //  97 $61
  {0,"BACKAT "}, //  98 $62
  {2,"PRINTAT"}, //  99 $63
  {0,"WHATO  "}, // 100 $64
  {1,"CALL   "}, // 101 $65
  {1,"PUTO   "}, // 102 $66
  {0,"NOTDONE"}, // 103 $67
  {1,"AUTOP  "}, // 104 $68
  {1,"AUTOT  "}, // 105 $69
  {1,"MOVE   "}, // 106 $6A
  {2,"WINSIZE"}, // 107 $6B
  {0,"REDO   "}, // 108 $6C
  {0,"CENTRE "}, // 109 $6D
  {1,"EXIT   "}, // 110 $6E
  {0,"INKEY  "}, // 111 $6F
  {2,"BIGGER "}, // 112 $70
  {2,"SMALLER"}, // 113 $71
  {0,"ISDONE "}, // 114 $72
  {0,"ISNDONE"}, // 115 $73
  {1,"SKIP   "}, // 116 $74
  {0,"RESTART"}, // 117 $75
  {1,"TAB    "}, // 118 $76
  {2,"COPYOF "}, // 119 $77
  {0,"dumb   "}, // 120 $78 (according DAAD manual, internal)
  {2,"COPYOO "}, // 121 $79 
  {0,"dumb   "}, // 122 $7A (according DAAD manual, internal)
  {2,"COPYFO "}, // 123 $7B
  {0,"dumb   "}, // 124 $7C (according DAAD manual, internal)
  {2,"COPYFF "}, // 125 $7D
  {2,"COPYBF "}, // 126 $7E
  {0,"RESET  "}  // 127 $7F


};






struct s_debug_daad_flag_object debug_daad_flag_object[MENU_DEBUG_NUMBER_FLAGS_OBJECTS];

//inicializar la lista de flags/objetos a una por defecto, valida para daad y paws

void menu_debug_daad_init_flagobject(void)
{

	debug_daad_flag_object[0].indice=0;
	debug_daad_flag_object[1].indice=1;
	debug_daad_flag_object[2].indice=33;
	debug_daad_flag_object[3].indice=34;
	debug_daad_flag_object[4].indice=35;
	debug_daad_flag_object[5].indice=38;	
	debug_daad_flag_object[6].indice=51;	

	//todos tipo flag
	int i;
	for (i=0;i<MENU_DEBUG_NUMBER_FLAGS_OBJECTS;i++) 	debug_daad_flag_object[i].tipo=0;

			
}

//comprobamos si algun valor de la tabla se sale del rango admitido. Esto pasa en quill por ejemplo
void menu_debug_daad_check_init_flagobject(void)
{

	//todos tipo flag
	int i;
	for (i=0;i<MENU_DEBUG_NUMBER_FLAGS_OBJECTS;i++) {
		int tipo=debug_daad_flag_object[i].tipo;
		int indice=debug_daad_flag_object[i].indice;

		int limite_max;
		if (tipo==0) limite_max=util_daad_get_limit_flags();
		else limite_max=util_daad_get_limit_objects();

		if (indice>limite_max) debug_daad_flag_object[i].indice=0; //Poner un indice admitido

	}	
			
}


//Retornar el texto si es flag o objeto y valores:
//FXXX XXX o OXXX XXX
void menu_debug_daad_string_flagobject(z80_byte num_linea,char *destino)
{
	z80_byte valor;
	char letra_mostrar;

	z80_byte indice=debug_daad_flag_object[num_linea].indice;

	if (debug_daad_flag_object[num_linea].tipo==0) {
		letra_mostrar='F';
		valor=util_daad_get_flag_value(indice);
	}

	else {
		letra_mostrar='O';
		valor=util_daad_get_object_value(indice);		
	}

	sprintf (destino,"%d.%c%03d %d",num_linea+1,letra_mostrar,indice,valor);
}

z80_int util_textadventure_get_location_connections(z80_byte location)
{
    z80_int table=util_textadventure_get_start_connections();

    table +=location*2;

    z80_int dir=value_8_to_16(daad_peek(table+1),daad_peek(table));

    return dir;

}

int util_textadventure_get_current_location_flag(void)
{

    if (util_daad_detect() ) {
       return 38; 
    }

    else if (util_textadv_detect_paws_quill() ) {
        if (util_paws_quill_is_quill()) {
            return 35;
        }
        else return 38; 
    }

    //cualquier otra cosa
    return -1;

}

int util_textadventure_get_current_location_gac(void)
{
    return peek_byte_no_time(42221);
}


//Retorna -1 si no es aventura de texto
int util_textadventure_get_current_location(void)
{

    //Si es gac
    if (util_gac_detect()) {
        return util_textadventure_get_current_location_gac();
    }


    int flag=util_textadventure_get_current_location_flag();
    if (flag>=0) {
        return util_daad_get_flag_value(flag);
    }

    else {
        return -1;
    }



}






//Retorna 0 si ok. -1 si error
int util_textdaventure_dump_connections(char *texto,int max_string)
{
    char buffer_linea[MAX_ANCHO_LINEAS_GENERIC_MESSAGE];

    //Ver si es de daad
    if (util_daad_detect() || util_textadv_detect_paws_quill()) {
        util_textadventure_get_start_connections();

        //printf("current location (flag 35 in quill, 38 in paws or daad): %d\n",util_textadventure_get_current_location() );

        //printf("Start connections table: %d\n",start_connections_table);

        z80_byte total_locations=util_daad_get_num_locat_messages();

        int i;

        //int salir;

        for (i=0;i<total_locations;i++) {
            z80_int connection_table=util_textadventure_get_location_connections(i);


            char texto_localidad[MAX_ALLOWED_TEXT_ADVENTURE_LOCATION_LENGTH+1];
            util_daad_get_locat_message(i,texto_localidad); 


            //printf("location %3d connections table: %d\n",i,connection_table);

            sprintf(buffer_linea,"-Location %3d: ",i);

            //salir=util_concat_string(texto,buffer_linea,max_string);
            util_concat_string(texto,buffer_linea,max_string);

            //solo ver un trozo de la localidad
            texto_localidad[25]=0;
            //printf("%s\n",texto_localidad);
            sprintf(buffer_linea,"%s\n",texto_localidad);
            //salir=util_concat_string(texto,buffer_linea,max_string);
            util_concat_string(texto,buffer_linea,max_string);



            z80_byte palabra;
            z80_byte destino;
            while ((palabra=daad_peek(connection_table++))!=255) {
                destino=daad_peek(connection_table++);

                //printf("palabra %d ",palabra);


                char buffer_verbo[256]="";
                char buffer_nombre[256]="";
                char palabra_sin_espacios[PAWS_LONGITUD_PALABRAS+1];

				util_daad_paws_locate_word(palabra,0,buffer_verbo);

                if (buffer_verbo[0]!='?') {
                    //printf("%s",buffer_verbo);
                    util_clear_final_spaces(buffer_verbo,palabra_sin_espacios);
                    
                }

                else {
                    
                    //TODO: solo considerar en principio nombres < 16
                    util_daad_paws_locate_word(palabra,2,buffer_nombre);
                    if (buffer_nombre[0]!='?') {
                        //printf("%s",buffer_nombre);
                        util_clear_final_spaces(buffer_nombre,palabra_sin_espacios);
                    }

                    else {
                        //printf("unknown");
                        strcpy(palabra_sin_espacios,"unkn");
                    }
                }

                  
                
                //salir=util_concat_string(texto,palabra_sin_espacios,max_string);
                util_concat_string(texto,palabra_sin_espacios,max_string);
                

                //printf(" destino %d\n",destino);
                sprintf(buffer_linea," %d, ",destino);

                //salir=util_concat_string(texto,buffer_linea,max_string);
                util_concat_string(texto,buffer_linea,max_string);

            }


            //printf("\n");
            //salir=util_concat_string(texto,"\n\n",max_string);
            util_concat_string(texto,"\n\n",max_string);
        }

        //temporal
        //printf("temporal generamos tabla conexiones\n");
        //textadventure_follow_connections();


        return 0;
    }
    
    else {
        //unsuported parser
        return -1;
    }



}


//Para recorrer todas las localidades
/*
enum text_adventure_directions {
    TEXT_ADV_DIR_NORTH,
    TEXT_ADV_DIR_SOUTH,
    TEXT_ADV_DIR_EAST,
    TEXT_ADV_DIR_WEST,
    TEXT_ADV_DIR_NORTHWEST,
    TEXT_ADV_DIR_NORTHEAST,
    TEXT_ADV_DIR_SOUTHWEST,
    TEXT_ADV_DIR_SOUTHEAST,
    TEXT_ADV_DIR_UP,
    TEXT_ADV_DIR_DOWN
};
*/



struct text_adventure_conn text_adventure_connections_table[TEXT_ADVENTURE_MAX_LOCATIONS];


void init_textadventure_connections_table(void)
{
    int i;

    for (i=0;i<TEXT_ADVENTURE_MAX_LOCATIONS;i++) {
        text_adventure_connections_table[i].north=-1;
        text_adventure_connections_table[i].south=-1;
        text_adventure_connections_table[i].west=-1;
        text_adventure_connections_table[i].east=-1;
        text_adventure_connections_table[i].northwest=-1;
        text_adventure_connections_table[i].northeast=-1;
        text_adventure_connections_table[i].southwest=-1;
        text_adventure_connections_table[i].southeast=-1;
        text_adventure_connections_table[i].up=-1;
        text_adventure_connections_table[i].down=-1;

        text_adventure_connections_table[i].x=0;
        text_adventure_connections_table[i].y=0;
        text_adventure_connections_table[i].z=0;

        text_adventure_connections_table[i].recorrida=0;
        text_adventure_connections_table[i].mapa=0;

        text_adventure_connections_table[i].dudoso_north=0;
        text_adventure_connections_table[i].dudoso_south=0;
        text_adventure_connections_table[i].dudoso_west=0;
        text_adventure_connections_table[i].dudoso_east=0;
        text_adventure_connections_table[i].dudoso_northwest=0;
        text_adventure_connections_table[i].dudoso_northeast=0;
        text_adventure_connections_table[i].dudoso_southwest=0;
        text_adventure_connections_table[i].dudoso_southeast=0;
        text_adventure_connections_table[i].dudoso_up=0;
        text_adventure_connections_table[i].dudoso_down=0;        


        text_adventure_connections_table[i].habitacion_dudosa=0;        
    }
}

//Borra las entradas para decir que hemos pasado jugando por una habitacion
void init_textadventure_entrada_jugando(void)
{
    int i;

    for (i=0;i<TEXT_ADVENTURE_MAX_LOCATIONS;i++) {
        text_adventure_connections_table[i].entrado_jugando=0;
    }
}

int textadventure_walk_rooms_no_connections=0;

//Si una localidad tiene alguna salida y no es ella misma
int textadventure_room_has_exits(int i)
{
    if (textadventure_walk_rooms_no_connections) return 1;

    int n,s,w,e,nw,ne,sw,se,up,dn;

    n=text_adventure_connections_table[i].north;
    s=text_adventure_connections_table[i].south;
    w=text_adventure_connections_table[i].west;
    e=text_adventure_connections_table[i].east;

    nw=text_adventure_connections_table[i].northwest;
    ne=text_adventure_connections_table[i].northeast;
    sw=text_adventure_connections_table[i].southwest;
    se=text_adventure_connections_table[i].southeast;

    up=text_adventure_connections_table[i].up;
    dn=text_adventure_connections_table[i].down;

    if (
        (n!=-1 && n!=i) ||
        (s!=-1 && s!=i) ||
        (w!=-1 && w!=i) ||
        (e!=-1 && e!=i) ||

        (nw!=-1 && nw!=i) ||
        (ne!=-1 && ne!=i) ||
        (sw!=-1 && sw!=i) ||
        (se!=-1 && se!=i) ||

        (up!=-1 && up!=i) ||
        (dn!=-1 && dn!=i) 

    ) {
        return 1;
    }

    else return 0;
}

//generar la tabla de conexiones
void textadventure_generate_connections_table(void)
{
    init_textadventure_connections_table();


    //char buffer_linea[MAX_ANCHO_LINEAS_GENERIC_MESSAGE];

    //Ver si es de daad
    if (util_daad_detect() || util_textadv_detect_paws_quill()) {
        util_textadventure_get_start_connections();

        //printf("current location (flag 35 in quill, 38 in paws or daad): %d\n",util_textadventure_get_current_location() );

        //printf("Start connections table: %d\n",start_connections_table);

        z80_byte total_locations=util_daad_get_num_locat_messages();

        int i;

        //int salir;

        for (i=0;i<total_locations;i++) {
            z80_int connection_table=util_textadventure_get_location_connections(i);


            char texto_localidad[MAX_ALLOWED_TEXT_ADVENTURE_LOCATION_LENGTH+1];
            util_daad_get_locat_message(i,texto_localidad); 


            //printf("location %3d connections table: %d\n",i,connection_table);

            //sprintf(buffer_linea,"-Location %3d: ",i);

            //salir=util_concat_string(texto,buffer_linea,max_string);

            //solo ver un trozo de la localidad
            texto_localidad[25]=0;
            //printf("%s\n",texto_localidad);
            //sprintf(buffer_linea,"%s\n",texto_localidad);
            //salir=util_concat_string(texto,buffer_linea,max_string);



            z80_byte palabra;
            z80_byte destino;

            //Dado que solo las 16 primeras palabras (nombres y verbos) se consideran direcciones, para cada localidad habrá un maximo de 16+16=32 
            //posibles direcciones
            //Limitar esto a 32, esto tambien evita usar mucha cpu cuando una aventura se detecta erroneamente y esta corrupta
            //como por ejemplo cargar una aventura de 128k en modo 48k
            int direcciones_leidas=0;

            while ((palabra=daad_peek(connection_table++))!=255 && direcciones_leidas<32) {
                destino=daad_peek(connection_table++);

                //printf("palabra %d\n",palabra);

                //solo considerar en principio palabras < 16, que son las de direcciones
                if (palabra<16) {


                    char buffer_verbo[256]="";
                    char buffer_nombre[256]="";
                    char palabra_sin_espacios[PAWS_LONGITUD_PALABRAS+1];

                    util_daad_paws_locate_word(palabra,0,buffer_verbo);

                    if (buffer_verbo[0]!='?') {
                        //printf("%s",buffer_verbo);
                        util_clear_final_spaces(buffer_verbo,palabra_sin_espacios);
                        
                    }

                    else {
                        
                        util_daad_paws_locate_word(palabra,2,buffer_nombre);
                        if (buffer_nombre[0]!='?') {
                            //printf("%s",buffer_nombre);
                            util_clear_final_spaces(buffer_nombre,palabra_sin_espacios);
                        }

                        else {
                            //printf("unknown");
                            strcpy(palabra_sin_espacios,"unkn");
                        }
                    }

                    
                    
                    //salir=util_concat_string(texto,palabra_sin_espacios,max_string);
                    //salir=util_concat_string(texto,", ",max_string);

                    //printf(" destino %d\n",destino);
                    //sprintf(buffer_linea," %d, ",destino);
                    //salir=util_concat_string(texto,buffer_linea,max_string);

                    //asignar direcciones en tabla segun palabras conocidaas
                    if (!strcasecmp("n",palabra_sin_espacios) || !strcasecmp("adela",palabra_sin_espacios)
                        ) {
                        text_adventure_connections_table[i].north=destino;
                    }

                    if (!strcasecmp("s",palabra_sin_espacios)) text_adventure_connections_table[i].south=destino;

                    if (!strcasecmp("e",palabra_sin_espacios) || !strcasecmp("derec",palabra_sin_espacios)
                        ) {
                        text_adventure_connections_table[i].east=destino;
                    }

                    if (!strcasecmp("w",palabra_sin_espacios) || !strcasecmp("o",palabra_sin_espacios) ||
                        !strcasecmp("izqui",palabra_sin_espacios)
                        ) {
                        text_adventure_connections_table[i].west=destino;
                    }

                    if (!strcasecmp("nw",palabra_sin_espacios) || !strcasecmp("no",palabra_sin_espacios)) {
                        text_adventure_connections_table[i].northwest=destino;
                    }

                    if (!strcasecmp("ne",palabra_sin_espacios)) text_adventure_connections_table[i].northeast=destino;

                    if (!strcasecmp("sw",palabra_sin_espacios) || !strcasecmp("so",palabra_sin_espacios)) {
                        text_adventure_connections_table[i].southwest=destino;
                    }

                    if (!strcasecmp("se",palabra_sin_espacios)) text_adventure_connections_table[i].southeast=destino;

                    if (!strcasecmp("up",palabra_sin_espacios) || 
                        !strcasecmp("u",palabra_sin_espacios) || 
                        !strcasecmp("subo",palabra_sin_espacios) ||
                        !strcasecmp("sube",palabra_sin_espacios)) {
                        text_adventure_connections_table[i].up=destino;
                    }

                    if (!strcasecmp("down",palabra_sin_espacios) ||
                        !strcasecmp("d",palabra_sin_espacios) ||
                        !strcasecmp("bajo",palabra_sin_espacios) ||
                        !strcasecmp("baja",palabra_sin_espacios)) {
                        text_adventure_connections_table[i].down=destino;
                    }

                }

                direcciones_leidas++;

            }

            //dump direcciones
            /*
            printf("tabla direcciones: N%d S%d W%d E%d NW%d NE%d SW%d SE%d UP%d DN%d\n",
                    text_adventure_connections_table[i].north,
                    text_adventure_connections_table[i].south,
                    text_adventure_connections_table[i].west,
                    text_adventure_connections_table[i].east,
                    text_adventure_connections_table[i].northwest,
                    text_adventure_connections_table[i].northeast,
                    text_adventure_connections_table[i].southwest,
                    text_adventure_connections_table[i].southeast,
                    text_adventure_connections_table[i].up,
                    text_adventure_connections_table[i].down
            );            


            printf("\n");*/





            //salir=util_concat_string(texto,"\n",max_string);
        }

        return;
    }
    
    else {
        //unsuported parser
        return;
    }

 


}


int textdaventure_position_exists(int x,int y,int z,int current,int id_mapa)
{
    int i;


    //Evitamos esta pues la detecta al principio
    //TODO: esto no es perfecto pero...
    if (x==0 && y==0 && z==0) return -1;
    
    for (i=0;i<util_daad_get_num_locat_messages();i++) {
        if (i!=current) {
            if (text_adventure_connections_table[i].x==x &&
                text_adventure_connections_table[i].y==y &&
                text_adventure_connections_table[i].z==z &&
                text_adventure_connections_table[i].recorrida &&
                text_adventure_connections_table[i].mapa==id_mapa
                )
                 {
            
                return i;
            }
        }
    }

    return -1;
}

//0,0,0 abajo del todo. Norte incrementa coordenada Y
int textadventure_walk(int room, int x, int y, int z, int recurse_level,int id_mapa)
{
    //Si ya hemos pasado por aqui, volver sin mas
    if (text_adventure_connections_table[room].recorrida) return -1;

    debug_printf(VERBOSE_DEBUG,"Text Adventure Map. Entering room %d pos %d,%d,%d recurse_level %d",room,x,y,z,recurse_level);

    //si hemos llegado al limite de recursividad, salir
    if (recurse_level>20) {
        debug_printf(VERBOSE_DEBUG,"maximum recurse level on room %d pos %d,%d,%d recurse_level %d",room,x,y,z,recurse_level);
        return -1;
    }

    //decir que hemos pasado
    text_adventure_connections_table[room].recorrida=1;

    //indicar id de mapa
    text_adventure_connections_table[room].mapa=id_mapa;

    //indicar su posicion x,y,z
    //Ver si esa posicion ya esta ocupada por otra localidad

    int existe_posicion=textdaventure_position_exists(x,y,z,room,id_mapa);
    if (existe_posicion!=-1) {
        debug_printf(VERBOSE_DEBUG,"Position %d,%d,%d of this room %d already used by room %d",
            x,y,z,room,existe_posicion);

        //la marcamos como dudosa y salimos
        text_adventure_connections_table[room].habitacion_dudosa=1;
    }

    text_adventure_connections_table[room].x=x;
    text_adventure_connections_table[room].y=y;
    text_adventure_connections_table[room].z=z;

    //y ver a donde podemos ir
    if (text_adventure_connections_table[room].north!=-1) {
        int ret=textadventure_walk(text_adventure_connections_table[room].north,x,y+1,z,recurse_level+1,id_mapa);

        //Esa posicion ya estaba ocupada. Indicamos que esta direccion es dudosa
        if (ret!=-1) text_adventure_connections_table[room].dudoso_north=1;
            
    }

    if (text_adventure_connections_table[room].south!=-1) {
        int ret=textadventure_walk(text_adventure_connections_table[room].south,x,y-1,z,recurse_level+1,id_mapa);

        //Esa posicion ya estaba ocupada. Indicamos que esta direccion es dudosa
        if (ret!=-1) text_adventure_connections_table[room].dudoso_south=1;

    }

    if (text_adventure_connections_table[room].west!=-1) {
        int ret=textadventure_walk(text_adventure_connections_table[room].west,x-1,y,z,recurse_level+1,id_mapa);

        //Esa posicion ya estaba ocupada. Indicamos que esta direccion es dudosa
        if (ret!=-1) text_adventure_connections_table[room].dudoso_west=1;

    }

    if (text_adventure_connections_table[room].east!=-1) {
        int ret=textadventure_walk(text_adventure_connections_table[room].east,x+1,y,z,recurse_level+1,id_mapa);

        //Esa posicion ya estaba ocupada. Indicamos que esta direccion es dudosa
        if (ret!=-1) text_adventure_connections_table[room].dudoso_east=1;

    }        

    if (text_adventure_connections_table[room].northwest!=-1) {
        int ret=textadventure_walk(text_adventure_connections_table[room].northwest,x-1,y+1,z,recurse_level+1,id_mapa);

        //Esa posicion ya estaba ocupada. Indicamos que esta direccion es dudosa
        if (ret!=-1) text_adventure_connections_table[room].dudoso_northwest=1;

    }

    if (text_adventure_connections_table[room].northeast!=-1) {
        int ret=textadventure_walk(text_adventure_connections_table[room].northeast,x+1,y+1,z,recurse_level+1,id_mapa);

        //Esa posicion ya estaba ocupada. Indicamos que esta direccion es dudosa
        if (ret!=-1) text_adventure_connections_table[room].dudoso_northeast=1;

    }

    if (text_adventure_connections_table[room].southwest!=-1) {
        int ret=textadventure_walk(text_adventure_connections_table[room].southwest,x-1,y-1,z,recurse_level+1,id_mapa);

        //Esa posicion ya estaba ocupada. Indicamos que esta direccion es dudosa
        if (ret!=-1) text_adventure_connections_table[room].dudoso_southwest=1;

    }

    if (text_adventure_connections_table[room].southeast!=-1) {
        int ret=textadventure_walk(text_adventure_connections_table[room].southeast,x+1,y-1,z,recurse_level+1,id_mapa);

        //Esa posicion ya estaba ocupada. Indicamos que esta direccion es dudosa
        if (ret!=-1) text_adventure_connections_table[room].dudoso_southeast=1;

    }        

    if (text_adventure_connections_table[room].up!=-1) {
        int ret=textadventure_walk(text_adventure_connections_table[room].up,x,y,z+1,recurse_level+1,id_mapa);

        //Esa posicion ya estaba ocupada. Indicamos que esta direccion es dudosa
        if (ret!=-1) text_adventure_connections_table[room].dudoso_up=1;

    }

    if (text_adventure_connections_table[room].down!=-1) {
        int ret=textadventure_walk(text_adventure_connections_table[room].down,x,y,z-1,recurse_level+1,id_mapa);

        //Esa posicion ya estaba ocupada. Indicamos que esta direccion es dudosa
        if (ret!=-1) text_adventure_connections_table[room].dudoso_down=1;

    }         


    return existe_posicion;

}

//buscar habitacion que hay en coordenada x,y,z y ademas que tenga salidas
//-1 si no hay

int textadventure_find_room_by_coords(int x,int y,int z)
{
    int i;

    for (i=0;i<TEXT_ADVENTURE_MAX_LOCATIONS;i++) {
        if (text_adventure_connections_table[i].x==x && 
        text_adventure_connections_table[i].y==y && 
        text_adventure_connections_table[i].z==z &&
        textadventure_room_has_exits(i)
        ) {
            return i;
        }
    }    

    return -1;
}

//retornar maximo tamaño de ancho, alto, minimas y maximas coordenadas x,y
//de un id de mapa concreto (o de todos mapas y sin considerar z), para coordenada z concreta
int textadventure_get_size_map(int mapa,int z,int *ancho,int *alto,int *min_x,int *max_x,int *min_y,int *max_y,int si_todos_mapas)
{
    int i;

    //inicializar minimos y maximos a valores que sabemos que seran siempre modificables la primera vez
    *min_x=9999;
    *min_y=9999;
    *max_x=-9999;
    *max_y=-9999;
    
    int encontrado=0;

    for (i=0;i<TEXT_ADVENTURE_MAX_LOCATIONS;i++) {
        if ( 
            textadventure_room_has_exits(i)
        ) {

            int coincide_mapa=0;

            if (si_todos_mapas) coincide_mapa=1;
            else {
                //printf("z %d mapa %d - encontrado z %d mapa %d\n",z,mapa,text_adventure_connections_table[i].z,text_adventure_connections_table[i].mapa);
                if (text_adventure_connections_table[i].z==z && text_adventure_connections_table[i].mapa==mapa) coincide_mapa=1;
            }

            if (coincide_mapa) {
                encontrado=1;
                //habitacion es de ese mapa y coordenada z. Ver si x,y salen de los minimos y maximos que vamos calculando
                if (text_adventure_connections_table[i].x<(*min_x)) *min_x=text_adventure_connections_table[i].x;
                if (text_adventure_connections_table[i].y<(*min_y)) *min_y=text_adventure_connections_table[i].y;
                if (text_adventure_connections_table[i].x>(*max_x)) *max_x=text_adventure_connections_table[i].x;
                if (text_adventure_connections_table[i].y>(*max_y)) *max_y=text_adventure_connections_table[i].y;
            }

        }
    }        

    

    //Y calcular ancho y alto
    *ancho=(*max_x)-(*min_x)+1;
    *alto=(*max_y)-(*min_y)+1;

    return encontrado;
}


//retornar rango de coordenadas Z para todos los mapas
void textadventure_get_range_z_map(int *min_z,int *max_z)
{
    int i;

    //inicializar minimos y maximos a valores que sabemos que seran siempre modificables la primera vez
    *min_z=9999;
    *max_z=-9999;

    for (i=0;i<TEXT_ADVENTURE_MAX_LOCATIONS;i++) {
        if ( 
            textadventure_room_has_exits(i) && text_adventure_connections_table[i].recorrida
        ) {


                if (text_adventure_connections_table[i].z<(*min_z)) *min_z=text_adventure_connections_table[i].z;
                if (text_adventure_connections_table[i].z>(*max_z)) *max_z=text_adventure_connections_table[i].z;
            

        }
    }        

}



//Desplazar coordenadas x,y de cada habitacion de un mapa concreto
void textadventure_apply_offset_map(int mapa,int z,int offset_x,int offset_y)
{
    int i;


    for (i=0;i<TEXT_ADVENTURE_MAX_LOCATIONS;i++) {
        if (text_adventure_connections_table[i].z==z && 
            text_adventure_connections_table[i].mapa==mapa &&
            textadventure_room_has_exits(i)
        ) {

            //printf("Aplicando offset %d %d a habitacion %d\n",offset_x,offset_y,i);
            text_adventure_connections_table[i].x +=offset_x;
            text_adventure_connections_table[i].y +=offset_y;
            
        }
    }        

}

//Maximo de posiciones en horizontal que muestra nuestro mapa
//Esto se usa para poner, a la derecha, cada mapa aislado con el resto. Cuando se llega a esas posiciones, el mapa se pone arriba del otro
//A mayor valor, tendremos un mapa total mas ancho que alto. A menor valor, tendremos un mapa total mas alto que ancho
#define TEXTADVENTURE_MAX_X_MAPA 30

//ajustar todos los mapas de una coordenada z para que quepan en la tabla del mapa
//mismo algoritmo practicamente que en rearrange de ventanas de zx vision
void textadventure_rearrange_maps(int z,int total_mapas)
{
    //Maximo alto en la fila actual
	int alto_maximo_en_fila=0;

    //maximo columnas temporal
    int xfinal=TEXTADVENTURE_MAX_X_MAPA;

    int i;
    int x=0;
    int y=0;

    for (i=1;i<=total_mapas;i++) {
        //printf ("Setting mapa %d z %d to %d,%d\n",i,z,x,y);

        //Sacar dimensiones de cada mapa
        int ancho_mapa,alto_mapa,min_x_mapa,max_x_mapa,min_y_mapa,max_y_mapa;

        int encontrado;
    
        encontrado=textadventure_get_size_map(i,z,&ancho_mapa,&alto_mapa,&min_x_mapa,&max_x_mapa,&min_y_mapa,&max_y_mapa,0);

        if (encontrado) {

            //printf("1 Mapa %d Ancho %d Alto %d minx: %d miny: %d maxx: %d maxy: %d\n",
            //    i,ancho_mapa,alto_mapa,min_x_mapa,min_y_mapa,max_x_mapa,max_y_mapa);        

            //hacer el calculo del offset_x y offset_y
            //restarle el minimo (con lo que se ubicaria en 0,0) y sumar el offset que tenemos de x
            int offset_x=-min_x_mapa;
            offset_x +=x;

            int offset_y=-min_y_mapa;
            offset_y +=y;

            //printf("Setting offset %d,%d to map %d\n",offset_x,offset_y,i);
            textadventure_apply_offset_map(i,z,offset_x,offset_y);


            if (alto_mapa>alto_maximo_en_fila) alto_maximo_en_fila=alto_mapa;

            int ancho_antes=ancho_mapa;

            //Siguiente donde ira
            if (i<total_mapas) {
                //En principio a la derecha
                x +=ancho_antes;

                //algo mas de margen
                x+=2;

                //printf ("%d %d %d\n",x,ventana->visible_width,ancho);

                encontrado=textadventure_get_size_map(i+1,z,&ancho_mapa,&alto_mapa,&min_x_mapa,&max_x_mapa,&min_y_mapa,&max_y_mapa,0);

                //printf("2 Mapa %d Ancho %d Alto %d minx: %d miny: %d maxx: %d maxy: %d\n",
                //    i+1,ancho_mapa,alto_mapa,min_x_mapa,min_y_mapa,max_x_mapa,max_y_mapa);            

                //No cabe a la derecha, pues va para arriba
                if (encontrado && x+ancho_mapa>=xfinal) {

                    //Siguiente fila
                    x=0;

                    //Para arriba (coordenada y aumenta)
                    y+=alto_maximo_en_fila;

                    //algo mas de margen
                    y +=2;

                    alto_maximo_en_fila=0;


                }


            }
        }
	}


}

void textadventure_follow_connections(int follow_rooms_no_connections)
{

    textadventure_walk_rooms_no_connections=follow_rooms_no_connections;

    //generar tabla de conexiones
    textadventure_generate_connections_table();

    if (!util_textadventure_is_daad_quill_paws()) return;

    //Y recorrer todas conexiones, generando posiciones x,y,z para cada una

    int i;
    int id_mapa=1;

    int x,y,z;

    //coords iniciales
    x=y=z=0;

    //int offset_x;
    //int offset_y;

    //offset_x=0;
    //offset_y=0;

    //text_adventure_connections_table[i].mapa

    //empezar por cada habitacion no recorrida
    for (i=0;i<util_daad_get_num_locat_messages();i++) {
        //Si no hemos entrado ya ahi
        if (!text_adventure_connections_table[i].recorrida) {
            textadventure_walk(i,x,y,z,0,id_mapa);


            //si tiene conexiones, entonces incrementamos el id_mapa para el siguiente
            //pues la siguiente posicion libre no recorrida tendra que tener otro id_mapa
            if (textadventure_room_has_exits(i)) {
                //juego con 7 mapas: journey to the centre of eddie... (de quill)

                id_mapa++;
            }
        }
    }

    int total_mapas=id_mapa-1; //este es el total aun considerando todas las Z

    //printf("Total mapas: %d\n",total_mapas);

    
    int min_z,max_z;
    textadventure_get_range_z_map(&min_z,&max_z);
    //printf("Rango Z: min %d max %d\n",min_z,max_z);

    //Sacar dimensiones de cada mapa en cada z. Esto es solo para debug y en principio no tiene utilidad practica
    /*
    int ancho_mapa,alto_mapa,min_x_mapa,max_x_mapa,min_y_mapa,max_y_mapa;
    for (z=min_z;z<=max_z;z++) {
        for (i=1;i<=total_mapas;i++) {
            int encontrado=textadventure_get_size_map(i,z,&ancho_mapa,&alto_mapa,&min_x_mapa,&max_x_mapa,&min_y_mapa,&max_y_mapa,0);
            if (encontrado) {
                printf("Mapa %d Z %d Ancho %d Alto %d minx: %d miny: %d maxx: %d maxy: %d\n",
                    i,z,ancho_mapa,alto_mapa,min_x_mapa,min_y_mapa,max_x_mapa,max_y_mapa);
            }


        }
    }
    */




    //Ubicar todos los mapas en cada capa Z
    for (z=min_z;z<=max_z;z++) {
        textadventure_rearrange_maps(z,total_mapas);
    }



    //textadventure_rearrange_maps(z,total_mapas);



}

//Mostrar mapa en consola de texto. Codigo no usado, solo para debug 
void textadventure_debug_show_map(void)
{
    int x,y,z;

    //int i;

    int ancho_mapa,alto_mapa,min_x_mapa,max_x_mapa,min_y_mapa,max_y_mapa;

    z=0;

    //z sera la de la habitacion actual
    int current_room=util_textadventure_get_current_location();

    if (current_room<TEXT_ADVENTURE_MAX_LOCATIONS) {

        if (text_adventure_connections_table[current_room].recorrida) {
            z=text_adventure_connections_table[current_room].z;
        }
    }        

    //mostrarlas en listado para debug

    /*
    printf("Begin show all coords\n");
    //decir todas posiciones
    for (i=0;i<util_daad_get_num_locat_messages();i++) {
        printf("room %3d: %d,%d,%d\n",
            i,text_adventure_connections_table[i].x,text_adventure_connections_table[i].y,text_adventure_connections_table[i].z);
    }
    */

    //mostrarlas cutrecillas

    //de arriba a abajo. deberia ser minimo 0,0. pongo inicio en -5,-5 ahora para probar



    //obtener tamaño de todos los mapas 
            //Sacar dimensiones de cada mapa
    //int ancho_mapa,alto_mapa,min_x_mapa,max_x_mapa,min_y_mapa,max_y_mapa;
    
    textadventure_get_size_map(0,0,&ancho_mapa,&alto_mapa,&min_x_mapa,&max_x_mapa,&min_y_mapa,&max_y_mapa,1);

    //printf("Tamaño para todos los mapas: Ancho %d Alto %d minx: %d miny: %d maxx: %d maxy: %d\n",
    //        ancho_mapa,alto_mapa,min_x_mapa,min_y_mapa,max_x_mapa,max_y_mapa);

    //vigilar que no salga de rango x o y : TEXTADVENTURE_MAX_X_MAPA...

    printf("Map\n");

    for (y=max_y_mapa;y>=min_y_mapa;y--) {
    //for (y=10;y>-40;y--) {
        //for (x=-10;x<40;x++) {
        for (x=min_x_mapa;x<=max_x_mapa;x++) {
            int habitacion=textadventure_find_room_by_coords(x,y,z);
            if (habitacion>=0) {
                if (current_room==habitacion) printf("|>%3d",habitacion);
                else printf("| %3d",habitacion);
            }
            else printf("|    ");
        }

        printf("\n");
    }
   

}





char textimage_filter_program[PATH_MAX]="";

void textadv_location_desc_run_convert(void);

//para controlar el tiempo desde el borrado de pantalla hasta fin de descripcion localidad
int textadv_location_desc_counter=0;
//Maximo valor para ese contador a partir del cual se considera fin de localidad
int max_textadv_location_desc_counter=1000;

//Estado de la deteccion de localidad:
//0: esperando borrado
//1: se ha producido borrado, esperando a que se pida leer una tecla
#define TEXTADV_LOC_STATE_IDLE 0
#define TEXTADV_LOC_STATE_CLS  1
int textadv_location_desc_state=TEXTADV_LOC_STATE_IDLE;

//Para la detección de descripción de localidades
int textadv_location_desc_nested_id_poke_byte;
int textadv_location_desc_nested_id_poke_byte_no_time;
int textadv_location_desc_nested_id_peek_byte;
int textadv_location_desc_nested_id_peek_byte_no_time;
z80_bit textadv_location_desc_enabled={0};

//Total de conversiones realizadas, para llevar un conteo de los creditos consumidos de la api externa
int textadv_location_total_conversions=0;

//Contador de tiempo desde ultimo caracter recibido
int textadv_location_desc_no_char_counter=0;
//Maximo valor para ese contador a partir del cual se considera fin de localidad
//En juegos de paws por ejemplo (Juanito y su baloncito, o super lopez), es importante este parametro,
//porque meten texto en el dibujo y lo considera erroneamente como parte de localidad y podria acabar
//antes de leer todo el texto de localidad
int max_textadv_location_desc_no_char_counter=1000;

//agregar caracter que le llega desde chardetect
#define TEXTADV_LOCATION_MAX_DESCRIPTION 500
char textadv_location_text[TEXTADV_LOCATION_MAX_DESCRIPTION+1];
int textadv_location_text_index=0;

//Cuando se ha generado la ultima imagen. Para poner limites y no generar imagenes muy seguidas 
//(que pueden generar coste economico en la API externa de OpenAI por ejemplo)
int textadv_location_desc_last_image_generated_counter=0;

//minimo de tiempo entre cada generacion, en ms
int textadv_location_desc_last_image_generated_min=5000;


int textadv_location_additional_room_change_method=TEXTADV_LOCATION_ADD_ROOM_CHANGE_METHOD_CLS_AND_ROOM_NUMBER;

char *textadv_location_additional_room_change_method_strings[]={
    "CLS",
    "Room_number",
    "CLS_and_Room_number"
};

void textadv_location_print_method_strings(void)
{
    int i;

    for (i=0;i<=TEXTADV_LOCATION_ADD_ROOM_CHANGE_METHOD_CLS_AND_ROOM_NUMBER;i++) {
        printf("%s ",textadv_location_additional_room_change_method_strings[i]);
    }
}

//Establecer el metodo de deteccion por string
//Retorna no 0 si error
int textadv_location_set_method_by_string(char *s)
{
    int i;

    for (i=0;i<=TEXTADV_LOCATION_ADD_ROOM_CHANGE_METHOD_CLS_AND_ROOM_NUMBER;i++) {
        if (!strcasecmp(s,textadv_location_additional_room_change_method_strings[i])) {
            textadv_location_additional_room_change_method=i;
            return 0;
        }
    }    

    return 1;
}

//Ultima habitacion leida, para usar con los metodos de deteccion de numero de habitacion
z80_byte textadv_location_last_location=0;


void textadv_location_add_char(z80_byte c)
{
    if (textadv_location_desc_enabled.v==0) return;

    //Solo si estamos en estado de recepcion texto de localidad
    if (textadv_location_desc_state==TEXTADV_LOC_STATE_IDLE) return;

    textadv_location_desc_no_char_counter=0;

    //si llega al limite
    if (textadv_location_text_index==TEXTADV_LOCATION_MAX_DESCRIPTION) {
        debug_printf(VERBOSE_PARANOID,"Reached maximum description length");
        return;
    }   

    //Fitros de otros caracteres especiales
    switch (c) {
        //case '\'':
        case '"':
        case '#':
        case '!':
        case '%':
        case '/':
        case '[':
        case ']':
        case '$':
        case '+':
        case '-':
        case '>':
        case '<':
        case '`':
        case '^':
            c=32;
        break;
    }
    
    //filtros de caracteres, se supone que aqui solo llegan caracteres imprimibles, pero por si acaso
    if (c>31 && c<127) {

        textadv_location_text[textadv_location_text_index++]=c;

    }

}

void textadv_location_reset_last_room_number(void)
{
    if (textadv_location_additional_room_change_method==TEXTADV_LOCATION_ADD_ROOM_CHANGE_METHOD_ROOM_NUMBER ||
    textadv_location_additional_room_change_method==TEXTADV_LOCATION_ADD_ROOM_CHANGE_METHOD_CLS_AND_ROOM_NUMBER
    ) {
        
        int current_location=util_textadventure_get_current_location();

        //Y solo si realmente son aventuras de texto

        if (current_location>=0) {
                textadv_location_last_location=current_location;
        }

        
    }  

    textadv_location_desc_state=TEXTADV_LOC_STATE_IDLE;    
}


void textadv_location_desc_ended_description(void)
{

    //timer_sleep(100);
    //solo aceptarlo si ha pasado 1 segundo al menos
    if (textadv_location_desc_counter<max_textadv_location_desc_counter) {
        debug_printf(VERBOSE_PARANOID,"Do not accept finish location description until some time passes");
        return;
    }
    

    //Aceptar un minimo de caracteres
    if (textadv_location_text_index<10) {
        debug_printf(VERBOSE_PARANOID,"Do not accept finish location description until minimum text (received: %d)",textadv_location_text_index);
        return;
    }

    debug_printf(VERBOSE_PARANOID,"No char counter: %d",textadv_location_desc_no_char_counter);

    //Aceptarlo solo cuando haya pasado un tiempo desde el ultimo caracter recibido
    //En milisegundos
    if (textadv_location_desc_no_char_counter<max_textadv_location_desc_no_char_counter) {
        debug_printf(VERBOSE_PARANOID,"Do not accept finish location before certain time with no chars received");
        return;
    }

    //Si habia empezado una localidad, decir que se ha finalizado la localidad pues se pide tecla
    debug_printf(VERBOSE_DEBUG,"Finish reading location description");

    textadv_location_text[textadv_location_text_index]=0;
    debug_printf(VERBOSE_DEBUG,"Location description: [%s]",textadv_location_text);



    //Ver si se generan mas imagenes de las permitidas, ver ultima vez
    if (textadv_location_desc_last_image_generated_counter<textadv_location_desc_last_image_generated_min) {
        debug_printf(VERBOSE_DEBUG,"Last image was generated %d ms ago, do not allow generate another one until %d ms passes",
            textadv_location_desc_last_image_generated_counter,textadv_location_desc_last_image_generated_min);
    }

    else {    
        //Avisar a la ventana de text adventure image que estamos recreando imagen
        menu_textadv_loc_image_tell_show_creating_image(); 
        textadv_location_desc_run_convert();

    }

    textadv_location_desc_state=TEXTADV_LOC_STATE_IDLE;

    debug_printf(VERBOSE_PARANOID,"Location detection: changed to state idle");


    //Y actualizar posicion actual, por si el usuario ha cambiado de posicion antes del timeout, esto no se detectara bien
    //Y la posicion actual no corresponderia con la variable
    //Esto tambien implica que si el usuario se mueve muy rapido cambiando de posicion, y si solo se usa el metodo de deteccion
    //de cambio de numero de habitacion (y no el de cls) no detectara cambio de habitacion. El usuario debe espearse al tiempo
    //de timeout de no-char

    textadv_location_reset_last_room_number();     

}

int check_cls_display(void)
{
    

    //Ver la pantalla esta borrada
    //TODO: quiza solo comprobar primer tercio?
    int linea,x;

    //Ver si filas 1-7 (exceptuando fila 0 donde se situan el texto de localidad de daad) estan a 0

    for (linea=8;linea<64;linea++) {
        for (x=0;x<32;x++) {
            z80_int dir=16384+ (screen_addr_table[linea*32+x] & 8191);
            if (peek_byte_no_time(dir)!=0) return 0;
        }
    }

    return 1;
}

void handle_textadv_location_changed(void)
{
    textadv_location_desc_state=TEXTADV_LOC_STATE_CLS;
    textadv_location_desc_counter=0;
    textadv_location_text_index=0;    
}


//Usado desde menu para cambiar al siguiente metodo
void textadv_location_change_method(void)
{
    textadv_location_additional_room_change_method++;
    if (textadv_location_additional_room_change_method>TEXTADV_LOCATION_ADD_ROOM_CHANGE_METHOD_CLS_AND_ROOM_NUMBER) {
        textadv_location_additional_room_change_method=TEXTADV_LOCATION_ADD_ROOM_CHANGE_METHOD_CLS;
    }

    //Para que no detecte cambio de localidad al cambiar el metodo
    textadv_location_reset_last_room_number();

}

//OJO! Funciones de deteccion de paws, gac, etc utilizan llamadas a peek_byte_no_time
//Por  tanto no llamar aqui desde funciones textadv_location_desc_peek_byte_no_time y textadv_location_desc_peek_byte
//Porque si no se quedaria en un bucle llamandose a si mismo
void handle_textadv_location_states(void)
{


    if (textadv_location_desc_state==TEXTADV_LOC_STATE_IDLE) {
        if (textadv_location_additional_room_change_method==TEXTADV_LOCATION_ADD_ROOM_CHANGE_METHOD_CLS ||
            textadv_location_additional_room_change_method==TEXTADV_LOCATION_ADD_ROOM_CHANGE_METHOD_CLS_AND_ROOM_NUMBER
        ) {
            int borrado=check_cls_display();
            if (borrado) {
                debug_printf(VERBOSE_DEBUG,"Display has been cleared");

                handle_textadv_location_changed();
            }

        }
    }
        
    if (textadv_location_desc_state==TEXTADV_LOC_STATE_IDLE) {

        

        if (textadv_location_additional_room_change_method==TEXTADV_LOCATION_ADD_ROOM_CHANGE_METHOD_ROOM_NUMBER ||
        textadv_location_additional_room_change_method==TEXTADV_LOCATION_ADD_ROOM_CHANGE_METHOD_CLS_AND_ROOM_NUMBER
        ) {
           
            int current_location=util_textadventure_get_current_location();

            //Y solo si realmente son aventuras de texto

            if (current_location>=0) {

                //printf("\nlocation: %d\n",current_location);
            
                if (current_location!=textadv_location_last_location) {
                    
                    
                    textadv_location_last_location=current_location;
                    
                    debug_printf(VERBOSE_DEBUG,"Room number detection: room has changed to %d",textadv_location_last_location);

                    handle_textadv_location_changed();
                    
                }

            }
            

        }        
    }

 


}

void textadv_location_timer_event(void)
{
    //Se llama aqui cada 20ms
    textadv_location_desc_counter+=20;

    //Contador de tiempo desde ultimo caracter recibido
    textadv_location_desc_no_char_counter +=20;

    //Contador de tiempo desde la ultima imagen generada
    textadv_location_desc_last_image_generated_counter +=20;
}



//Si se lee direccion de sistema donde se guarda la tecla
void handle_textadv_read_keyboard_memory(z80_int dir)
{
    //printf("read key mem\n");
    if (textadv_location_desc_state==TEXTADV_LOC_STATE_IDLE) return;

    //TODO: juegos de paws finalizan antes: Ended description reading from 23560 PC=8DE9H
    //quiza hacer que se haya leido un minimo de texto (al menos 10 caracteres?)

    if (reg_pc<16384) return;
    
    if (dir==0x5c00) {
        debug_printf(VERBOSE_PARANOID,"Ended description reading from 0x5c00 PC=%XH",reg_pc);
        textadv_location_desc_ended_description();
    }

    //5c3b. bit 6 Set when a new key has been pressed
    if (dir==0x5c3b) {
        debug_printf(VERBOSE_PARANOID,"Ended description reading from 0x5c3b PC=%XH",reg_pc);
        textadv_location_desc_ended_description();
    }    

    /*if (dir==0x5c08) {
        printf("\nEnded description reading from 0x5c08 PC=%XH\n",reg_pc);
        textadv_location_desc_ended_description();
    } */
}


//Se llama aqui cuando se lee puerto de teclado
void textadv_location_desc_read_keyboard_port(void)
{
    if (textadv_location_desc_state==TEXTADV_LOC_STATE_IDLE) return;

    //Si estan interrupciones habilitadas e im0/1, entendemos que se lee desde la rom y por tanto esto no indica que estemos
    //leyendo el prompt
    //if (iff1.v && (im_mode==0 || im_mode==1)) return;

    if (reg_pc<16384) return;

    debug_printf(VERBOSE_PARANOID,"Ended description reading keyboard port PC=%XH",reg_pc);

    textadv_location_desc_ended_description();
}

z80_byte textadv_location_desc_poke_byte_no_time(z80_int dir,z80_byte valor)
{
	debug_nested_poke_byte_no_time_call_previous(textadv_location_desc_nested_id_poke_byte_no_time,dir,valor);

    handle_textadv_location_states();


	//Para que no se queje el compilador
	return 0;
}

z80_byte textadv_location_desc_poke_byte(z80_int dir,z80_byte valor)
{
	debug_nested_poke_byte_call_previous(textadv_location_desc_nested_id_poke_byte,dir,valor);

    handle_textadv_location_states();

	//Para que no se queje el compilador
	return 0;
}

z80_byte textadv_location_desc_peek_byte_no_time(z80_int dir,z80_byte value GCC_UNUSED)
{
	z80_byte valor_leido=debug_nested_peek_byte_no_time_call_previous(textadv_location_desc_nested_id_peek_byte_no_time,dir);

    handle_textadv_read_keyboard_memory(dir);
    
    return valor_leido;

	
}

z80_byte textadv_location_desc_peek_byte(z80_int dir,z80_byte value GCC_UNUSED)
{
	z80_byte valor_leido=debug_nested_peek_byte_call_previous(textadv_location_desc_nested_id_peek_byte,dir);

    handle_textadv_read_keyboard_memory(dir);

  return valor_leido;
  

}

//Establecer rutinas propias
void textadv_location_desc_set_peek_poke_functions(void)
{

    textadv_location_desc_nested_id_poke_byte=debug_nested_poke_byte_add(textadv_location_desc_poke_byte,"textadv_location_desc poke_byte");
    textadv_location_desc_nested_id_poke_byte_no_time=debug_nested_poke_byte_no_time_add(textadv_location_desc_poke_byte_no_time,"textadv_location_desc poke_byte_no_time");
            textadv_location_desc_nested_id_peek_byte=debug_nested_peek_byte_add(textadv_location_desc_peek_byte,"textadv_location_desc peek_byte");
            textadv_location_desc_nested_id_peek_byte_no_time=debug_nested_peek_byte_no_time_add(textadv_location_desc_peek_byte_no_time,"textadv_location_desc peek_byte_no_time");    

}

//Restaurar rutinas de textadv_location_desc
void textadv_location_desc_restore_peek_poke_functions(void)
{
		debug_printf (VERBOSE_DEBUG,"Restoring original poke / peek functions before textadv_location_desc");
                //poke_byte=textadv_location_desc_original_poke_byte;
                //poke_byte_no_time=textadv_location_desc_original_poke_byte_no_time;
                //peek_byte=textadv_location_desc_original_peek_byte;
                //peek_byte_no_time=textadv_location_desc_original_peek_byte_no_time;

    debug_nested_poke_byte_del(textadv_location_desc_nested_id_poke_byte);
    debug_nested_poke_byte_no_time_del(textadv_location_desc_nested_id_poke_byte_no_time);
        debug_nested_peek_byte_del(textadv_location_desc_nested_id_peek_byte);
        debug_nested_peek_byte_no_time_del(textadv_location_desc_nested_id_peek_byte_no_time);    


}

void textadv_location_desc_enable(void)
{
    if (textadv_location_desc_enabled.v) return;

    textadv_location_desc_set_peek_poke_functions();
    textadv_location_reset_last_room_number();
    textadv_location_desc_enabled.v=1;
}

void textadv_location_desc_disable(void)
{
    if (textadv_location_desc_enabled.v==0) return;

    textadv_location_desc_restore_peek_poke_functions();
    textadv_location_desc_enabled.v=0;
}


void textadv_location_desc_mete_comillas(char *origen,char *destino)
{
	sprintf (destino,"\"%s\"",origen);
}

int proceso_hijo_text_convert;

void textadv_location_desc_run_convert(void)
{

  
    if (textimage_filter_program[0]==0) return;


    textadv_location_desc_last_image_generated_counter=0;

    //Incrementar contador de conversiones realizadas
    textadv_location_total_conversions++;

#ifndef MINGW



   
    //printf("Launching child process\n");
    proceso_hijo_text_convert = fork();


    switch (proceso_hijo_text_convert) {

        case -1:
            debug_printf (VERBOSE_ERR,"Can not run fork to text to image");
        break;

        case 0:
      
            execlp(textimage_filter_program,textimage_filter_program,textadv_location_text,NULL);

            //Si se llega aqui es que ha habido un error al executar programa filtro
            debug_printf (VERBOSE_DEBUG,"Error running %s",textimage_filter_program);
            exit(0);
        break;

        default:

           //TODO: de momento no esperar hijo, para no detener juego

           /*
                    printf("Wait for text filter child\n");
                    waitpid (proceso_hijo_text_convert, NULL, 0);
         

            printf("despues de waitpid\n");
            */


                
        break;

    }

#else


	all_interlace_scr_refresca_pantalla();

	

    //En caso de Windows esto simplemente dice que hay hijo pero no el pid concreto
    proceso_hijo_text_convert=1;




    //Por defecto no esperar
    int modo=P_NOWAIT;

    //if (esperarhijo) modo=P_WAIT;



    //Parametro 1 es la descripcion de la localidad


    //importante las comillas cuando hay rutas con espacios
    //Al script de windows le llegan las comillas tal cual,
    //por tanto los parametros en un .bat de windows se deben usar tal cual %1 y no "%1", sino le meteria doble comillas ""%1""
    //+2 para meter las comillas
    char parametro_programa[PATH_MAX];
    char parametro_uno[TEXTADV_LOCATION_MAX_DESCRIPTION+2];


    //parametro programa sin comillas, porque sino, no inicia ni tan siquiera programa sin espacios
    sprintf (parametro_programa,"%s",textimage_filter_program);



    //Esto si que es necesario para poder enviar la descripcion de la localidad
    textadv_location_desc_mete_comillas(textadv_location_text,parametro_uno);



    //con spawnl
    int resultado=spawnl(modo, parametro_programa, parametro_programa, parametro_uno, NULL);
    debug_printf (VERBOSE_DEBUG,"Running program %s with parameters %s and %s",parametro_programa,parametro_uno);
    //printf ("Running program %s with parameters %s and %s\n",parametro_programa,parametro_uno);

    //printf ("Resultado spawn: %d\n",resultado);
    if (resultado<0) {
        debug_printf (VERBOSE_DEBUG,"Error running text to image program");

    }





	
#endif

}


//Mira si la ruta al script tiene espacios y en ese caso da error y desactiva la ruta
//En sistemas no Windows no hace nada
int textimage_filter_program_check_spaces(void)
{
#ifdef MINGW
	int i;
	int tiene_espacios=0;
	for (i=0;textimage_filter_program[i];i++) {
		if (textimage_filter_program[i]==' ') {
			tiene_espacios=1;
			break;
		}
	}

	if (tiene_espacios) {
		debug_printf (VERBOSE_ERR,"Full path to Text to Image program %s has spaces. It won't work on Windows.",
			textimage_filter_program);
		return 1;
	}
#endif

return 0;
}
