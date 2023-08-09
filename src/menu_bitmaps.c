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

#include "menu_bitmaps.h"
#include "settings.h"


/*
0 ZEsarUX
1 Smartload
2 Snapshot
3 Machine
4 Audio
5 Display
6 Storage
7 Debug
8 Network
9 Windows

10 Settings
11 Help
12 Close all menus
13 Exit

*/
char **zxdesktop_buttons_bitmaps[EXT_DESKTOP_TOTAL_BUTTONS]={
    zesarux_ascii_logo,
    bitmap_button_ext_desktop_smartload,
    bitmap_button_ext_desktop_snapshot,
    bitmap_button_ext_desktop_machine,
    bitmap_button_ext_desktop_audio,
    bitmap_button_ext_desktop_display, //5
    bitmap_button_ext_desktop_storage,
    bitmap_button_ext_desktop_debug,
    bitmap_button_ext_desktop_network,
    bitmap_button_ext_desktop_windows,
    bitmap_button_ext_desktop_settings, //10
    bitmap_button_ext_desktop_help,
    bitmap_button_ext_desktop_close_all_menus,
    bitmap_button_ext_desktop_exit //13

};

//Usado en watermark y en botones
char *zesarux_ascii_logo[ZESARUX_ASCII_LOGO_ALTO]={
    //01234567890123456789012345
    "wwwwwwwwwwwwwwwwwwwwwwwwww",     //0
  	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wwwwwwwwwwwwwwwwwxxxxwwwww",
	"                wxxxxw   w",
	"                wxxxxw  rw",
	"             wwwwxxxxw rrw",
	"            wxxxxwwww rrrw",
	"            wxxxxw   rrrrw",	//10
	"            wxxxxw  rrrryw",
	"         wwwwxxxxw rrrryyw",
	"        wxxxxwwww rrrryyyw",
	"        wxxxxw   rrrryyyyw",
	"        wxxxxw  rrrryyyygw",
	"     wwwwxxxxw rrrryyyyggw",
	"    wxxxxwwww rrrryyyygggw",
	"    wxxxxw   rrrryyyyggggw",
	"    wxxxxw  rrrryyyyggggcw",
	"wwwwwxxxxw rrrryyyyggggccw",    //20
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wwwwwwwwwwwwwwwwwwwwwwwwww" 		//25
};

//Usado en watermark y en botones
//logo X anniversary
char *zesarux_ascii_logo_xanniversary[ZESARUX_ASCII_LOGO_ALTO]={
    //01234567890123456789012345
    "wwwwwwwwwwwwwwwwwwwwwwwwww",     //0
  	"wxxrrrrrrrrrrrrrrrrrrrrxxw",
	"wxxrrrrrrrrrrrrrrrrrrrrxxw",
	"wccxxrrrrrrrrrrrrrrrrxxyyw",
	"wccxxrrrrrrrrrrrrrrrrxxyyw",
	"wccccxxrrrrrrrrrrrrxxyyyyw",
	"wccccxxrrrrrrrrrrrrxxyyyyw",
	"wccccccxxrrrrrrrrxxyyyyyyw",
	"wccccccxxrrrrrrrrxxyyyyyyw",
	"wccccccccxxrrrrxxyyyyyyyyw",
	"wccccccccxxrrrrxxyyyyyyyyw",	//10
	"wccccccccccxxxxyyyyyyyyyyw",
	"wccccccccccxxxxyyyyyyyyyyw",
	"wccccccccccxxxxyyyyyyyyyyw",
	"wccccccccccxxxxyyyyyyyyyyw",
	"wccccccccxxggggxxyyyyyyyyw",
	"wccccccccxxggggxxyyyyyyyyw",
	"wccccccxxggggggggxxyyyyyyw",
	"wccccccxxggggggggxxyyyyyyw",
	"wccccxxggggggggggggxxyyyyw",
	"wccccxxggggggggggggxxyyyyw",    //20
	"wccxxggggggggggggggggxxyyw",
	"wccxxggggggggggggggggxxyyw",
	"wxxggggggggggggggggggggxxw",
	"wxxggggggggggggggggggggxxw",
	"wwwwwwwwwwwwwwwwwwwwwwwwww" 		//25
};

char **get_zesarux_ascii_logo(void)
{
    //Retorna uno de los dos logos dependiendo si modo xanniversary o no
    if (xanniversary_logo.v) return zesarux_ascii_logo_xanniversary;
    else return zesarux_ascii_logo;
}

//Altera el bitmap cuando se va a referenciar el zesarux_ascii_logo, o dejarlo tal cual o poner el xanniversary
char **alter_zesarux_ascii_logo(char **p)
{
    if (p!=zesarux_ascii_logo) return p;

    else return get_zesarux_ascii_logo();
}

//Usado en el footer. marco de color blanco con brillo para
//luego modificarlo segun el color del footer
//Y sin el marco por la derecha en la zona del arco iris
char *zesarux_ascii_logo_whitebright[ZESARUX_ASCII_LOGO_ALTO]={
    //01234567890123456789012345
    "WWWWWWWWWWWWWWWWWWWWWWWWWW",     //0
  	"WxxxxxxxxxxxxxxxxxxxxxxxxW",
	"WxxxxxxxxxxxxxxxxxxxxxxxxW",
	"WxxxxxxxxxxxxxxxxxxxxxxxxW",
	"WxxxxxxxxxxxxxxxxxxxxxxxxW",
	"WWWWWWWWWWWWWWWWWxxxxWWWWW",
	"                WxxxxW    ",
	"                WxxxxW  r ",
	"             WWWWxxxxW rr ",
	"            WxxxxWWWW rrr ",
	"            WxxxxW   rrrr ",	//10
	"            WxxxxW  rrrry ",
	"         WWWWxxxxW rrrryy ",
	"        WxxxxWWWW rrrryyy ",
	"        WxxxxW   rrrryyyy ",
	"        WxxxxW  rrrryyyyg ",
	"     WWWWxxxxW rrrryyyygg ",
	"    WxxxxWWWW rrrryyyyggg ",
	"    WxxxxW   rrrryyyygggg ",
	"    WxxxxW  rrrryyyyggggc ",
	"WWWWWxxxxW rrrryyyyggggcc ",    //20
	"WxxxxxxxxxxxxxxxxxxxxxxxxW",
	"WxxxxxxxxxxxxxxxxxxxxxxxxW",
	"WxxxxxxxxxxxxxxxxxxxxxxxxW",
	"WxxxxxxxxxxxxxxxxxxxxxxxxW",
	"WWWWWWWWWWWWWWWWWWWWWWWWWW" 		//25
};




//Usado en el footer, de decimo aniversario.
//marco de color blanco con brillo para
//luego modificarlo segun el color del footer
//Y sin el marco por la derecha en la zona del arco iris
char *zesarux_ascii_logo_whitebright_xanniversary[ZESARUX_ASCII_LOGO_ALTO]={
    //01234567890123456789012345
    "WWWWWWWWWWWWWWWWWWWWWWWWWW",     //0
  	"WxxrrrrrrrrrrrrrrrrrrrrxxW",
	"WxxrrrrrrrrrrrrrrrrrrrrxxW",
	"WccxxrrrrrrrrrrrrrrrrxxyyW",
	"WccxxrrrrrrrrrrrrrrrrxxyyW",
	"WccccxxrrrrrrrrrrrrxxyyyyW",
	"WccccxxrrrrrrrrrrrrxxyyyyW",
	"WccccccxxrrrrrrrrxxyyyyyyW",
	"WccccccxxrrrrrrrrxxyyyyyyW",
	"WccccccccxxrrrrxxyyyyyyyyW",
	"WccccccccxxrrrrxxyyyyyyyyW",	//10
	"WccccccccccxxxxyyyyyyyyyyW",
	"WccccccccccxxxxyyyyyyyyyyW",
	"WccccccccccxxxxyyyyyyyyyyW",
	"WccccccccccxxxxyyyyyyyyyyW",
	"WccccccccxxggggxxyyyyyyyyW",
	"WccccccccxxggggxxyyyyyyyyW",
	"WccccccxxggggggggxxyyyyyyW",
	"WccccccxxggggggggxxyyyyyyW",
	"WccccxxggggggggggggxxyyyyW",
	"WccccxxggggggggggggxxyyyyW",    //20
	"WccxxggggggggggggggggxxyyW",
	"WccxxggggggggggggggggxxyyW",
	"WxxggggggggggggggggggggxxW",
	"WxxggggggggggggggggggggxxW",
	"WWWWWWWWWWWWWWWWWWWWWWWWWW" 		//25
};

/*char *get_zesarux_ascii_logo_whitebright(int linea)
{
    //Retorna uno de los dos logos dependiendo si modo xanniversary o no
    if (xanniversary_logo.v) return zesarux_ascii_logo_whitebright_xanniversary[linea];
    else return zesarux_ascii_logo_whitebright[linea];
}*/

char **get_zesarux_ascii_logo_whitebright(void)
{
    //Retorna uno de los dos logos dependiendo si modo xanniversary o no
    if (xanniversary_logo.v) return zesarux_ascii_logo_whitebright_xanniversary;
    else return zesarux_ascii_logo_whitebright;
}

//Iconos con contenido 26x26.
	//Hay que dejar margen de 6 por cada lado (3 izquierdo, 3 derecho, 3 alto, 3 alto)
	//Cada 3 pixeles de margen son: fondo-negro(rectangulo)-gris(de dentro boton)
	//total maximo 32x32
	//Ejemplo:
	/*

char *zesarux_ascii_logo[ZESARUX_ASCII_LOGO_ALTO]={
  ................................
  ################################
  --------------------------------
    "WWWWWWWWWWWWWWWWWWWWWWWWWW",     //0
  	"WXXXXXXXXXXXXXXXXXXXXXXXXW",
	"WXXXXXXXXXXXXXXXXXXXXXXXXW",
	"WXXXXXXXXXXXXXXXXXXXXXXXXW",
	"WXXXXXXXXXXXXXXXXXXXXXXXXW",
	"WWWWWWWWWWWWWWWWWXXXXWWWWW",
	"                WXXXXW   W",
	"                WXXXXW  RW",
	"             WWWWXXXXW RRW",
	"            WXXXXWWWW RRRW",
	"            WXXXXW   RRRRW",	//10
	"            WXXXXW  RRRRYW",
	"         WWWWXXXXW RRRRYYW",
	"        WXXXXWWWW RRRRYYYW",
	"        WXXXXW   RRRRYYYYW",
	"        WXXXXW  RRRRYYYYGW",
	"     WWWWXXXXW RRRRYYYYGGW",
	"    WXXXXWWWW RRRRYYYYGGGW",
	"    WXXXXW   RRRRYYYYGGGGW",
	"    WXXXXW  RRRRYYYYGGGGCW",
	"WWWWWXXXXW RRRRYYYYGGGGCCW",    //20
	"WXXXXXXXXXXXXXXXXXXXXXXXXW",
	"WXXXXXXXXXXXXXXXXXXXXXXXXW",
	"WXXXXXXXXXXXXXXXXXXXXXXXXW",
	"WXXXXXXXXXXXXXXXXXXXXXXXXW",
	"WWWWWWWWWWWWWWWWWWWWWWWWWW" 		//25
};
	*/


//Boton ayuda
char *bitmap_button_ext_desktop_help[EXT_DESKTOP_BUTTONS_ANCHO]={
   //01234567890123456789012345
    "        bbbbbbbb          ",     //0
  	"       bbbbbbbbbb         ",
	"      bbb      bbb        ",
	"     bbb        bbb       ",
	"    bbb          bbb      ",
	"    bbb          bbb      ",
	"    bbb          bbb      ",
	"    bbb          bbb      ",
	"                 bbb      ",
	"                 bbb      ",
	"                bbbb      ",	//10
	"               bbbb       ",
	"              bbbb        ",
	"             bbbb         ",
	"            bbbb          ",
	"            bbb           ",
	"            bbb           ",
	"            bbb           ",
	"            bbb           ",
	"            bbb           ",
	"                          ",    //20
	"             b            ",
	"            bbb           ",
	"           bbbbb          ",
	"            bbb           ",
	"             b            " 	 //25
};


char *bitmap_button_ext_desktop_debug[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                      RR  ",     //0
  	"                     RR   ",
	"                    RR    ",
	"                   RR     ",
	"                  RRR     ",
	"           RRRRRRRRR      ",
	"         RRxxxxxxxxR      ",
	"        RRxxxxxxxxxxR     ",
	"       RRxxxxxxxRRRxxR    ",
	"      RRxxxxxxxxRRRxxR    ",
	"      RxxxxxxxxxxxxxxR    ",	//10
	"      RRxxxxxRxxxxxxxR    ",
	"   RRRRRxxxxxRxxxxxxRR    ",
	"  RxxxxRRxxxxxRRRRxxRRR   ",
	"  RxxxxR RRxxxxxxxxR RRR  ",
	" RxxxxxRR  RRRRRRRR    RR ",
	" RxxxxxxR              RR ",
	" RxxxxxxR      RR       RR",
	" RxxxxxRR   RRRRR       RR",
	"  RRxxRRRRRR   RR         ",
	"  RRRRR                   ",    //20
	"  RR  RR                  ",
	" RR   RR                  ",
	" RR    RR                 ",
	"RR     RRR                ",
	"RR      RR                " 	 //25
};


char *bitmap_button_ext_desktop_display[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"RRRRRRRRRRRRRRRRRRRRRRRRRR",
	"RrrrrrrrrrrrrrrrrrrrrrrrrR",
	"Rr                      rR",
	"Rr         mmmm         rR",
	"Rr         mmmmm        rR",
	"Rr         mmmm         rR",
	"Rr         xx           rR",
	"Rr         xx           rR",
	"Rr  xxxxxxxxxxxxxxxxx   rR",	//10
	"Rr  xxxxxxxxxxxxxxxxx   rR",
	"Rr   xxxxxxxxxxxxxxx    rR",
	"Rr    xxxxxxxxxxxxx     rR",
	"RrbbbbbbbbbbbbbbbbbbbbbbrR",
	"RrBBccccccbbbcccccccccccrR",
	"RrBBBBbbbbbbccBBBBBbbbbbrR",
	"RrBBBBbbbBBBBcBBBBBBBBBBrR",
	"RrBBBBBBBBBBBBBBBBBBBBBBrR",
	"RrBBBBBBBBBBBBBBBBBBBBBBrR",
	"RrrrrrrrrrrrrrrrrrrrrrrrrR",    //20
	"RRRRRRRRRRRRRRRRRRRRRRRRRR",
	"        RRRRRRRRRR        ",
	"       RRRRRRRRRRRR       ",
	"      RRRRRRRRRRRRRR      ",
	"     rrrrrrrrrrrrrrrr     " 	 //25
};



char *bitmap_button_ext_desktop_network[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "    x                x    ",     //0
  	"   xx   x        x   xx   ",
	"  xx   xx        xx   xx  ",
	"  xx   x          x   xx  ",
	" xx   xx    xx    xx   xx ",
	" xx   xx   xxxx   xx   xx ",
	" xx   xx   xxxx   xx   xx ",
	" xx    x    xx    x    xx ",
  	"  xx   xx        xx   xx  ",
	"  xx    x        x    xx  ",
	"   xx       xx       xx   ",	//10
	"    x       xx       x    ",
	"           x  x           ",
	"           x  x           ",
	"           x  x           ",
	"          x    x          ",
	"          x    x          ",
	"          x    x          ",
	"         x x  x x         ",
	"         x  xx  x         ",
	"        x   xx   x        ",    //20
	"        x  x  x  x        ",
	"       x  x    x  x       ",
	"       x x      x x       ",
	"      x x        x x      ",
	"      xxxxxxxxxxxxxx      " 	 //25
};



char *bitmap_button_ext_desktop_audio[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "        xxx               ",     //0
  	"        xxxxx             ",
	"        xxxxxxx           ",
	"        xx  xxxx          ",
	"        xx    xxxx        ",
	"        xxxx    xxxx      ",
	"        xxxxxx    xxx     ",
	"        xx  xxxx   xxxx   ",
	"        xx    xxxx  xxx   ",
	"        xx      xxx  xx   ",
	"        xx        xx xx   ",	//10
	"        xx         xxxx   ",
	"        xx          xxx   ",
	"        xx           xx   ",
	"        xx           xx   ",
	"        xx           xx   ",
	"        xx           xx   ",
	"      xxxx           xx   ",
	"     xxxxx           xx   ",
	"     xxxxx           xx   ",
	"     xxxxx           xx   ",    //20
	"      xxx          xxxx   ",
	"                  xxxxx   ",
	"                  xxxxx   ",
	"                  xxxxx   ",
	"                   xxx    " 	 //25
};




char *bitmap_button_ext_desktop_snapshot[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"                          ",
	"         bbbbbbbb         ",
	"        bbbbbbbbbb        ",
	"       bbbbbbbbbbbb       ",
	"      bbbbbbbbbbbbbb      ",
	"     bbbbbbbbbbbbbbbb     ",
	" bbbbbbbbbbbbbbbbbbbbbbbb ",
	"bbbbbbbbbbb    bbbbbbbbbbb",	//10
	"bbbbbbbbbb      bbbbbbbbbb",
	"bbbWWbbbb        bbbbbbbbb",
	"bbbWWbbb   xx     bbbbbbbb",
	"bbbbbbbb  x       bbbbbbbb",
	"bbbbbbb   x        bbbbbbb",
	"bbbbbbb            bbbbbbb",
	"bbbbbbb            bbbbbbb",
	"bbbbbbbb          bbbbbbbb",
	"bbbbbbbb          bbbbbbbb",
	"bbbbbbbbb        bbbbbbbbb",	//20
	"bbbbbbbbbb      bbbbbbbbbb",
	"bbbbbbbbbbb    bbbbbbbbbbb",
	"bbbbbbbbbbbbbbbbbbbbbbbbbb",
	"bbbbbbbbbbbbbbbbbbbbbbbbbb",
	"bbbbbbbbbbbbbbbbbbbbbbbbbb"	//25


};



char *bitmap_button_ext_desktop_windows[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "          xxxxxx          ",     //0
  	"        xxxxxxxxxx        ",
	"       xxcccxxcccxx       ",
	"       xxcccxxcccxx       ",
	"      xxccccxxccccxx      ",
	"      xxccccxxccccxx      ",
	"      xxxxxxxxxxxxxx      ",
	"      xxxxxxxxxxxxxx      ",
	"      xxccccxxccccxx      ",
	"      xxccccxxccccxx      ",
	"      xxccccxxccccxx      ",	//10
	"      xxccccxxccccxx      ",
	"      xxccccxxccccxx      ",
	"      xxxxxxxxxxxxxx      ",
	"      xxxxxxxxxxxxxx      ",
	"      xxccccxxccccxx      ",
	"      xxccccxxccccxx      ",
	"      xxccccxxccccxx      ",
	"      xxccccxxccccxx      ",
	"      xxccccxxccccxx      ",
	"      xxxxxxxxxxxxxx      ",    //20
	"      xxxxxxxxxxxxxx      ",
	"                          ",
	"                          ",
	"     xxxxxxxxxxxxxxxx     ",
	"     xxxxxxxxxxxxxxxx     " 	 //25
};

char *bitmap_button_ext_desktop_exit[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "RRRRRRRRRRRRRRRRRRRRRRRRRR",     //0
  	"RRRRRRRRRRRRRRRRRRRRRRRRRR",
	"RRRRRRRRRRRRRRRRRRRRRRRRRR",
	"RRRRRRRRRRRRRRRRRRRRRRRRRR",
	"RRRRRRRRRRRRRRRRRRRRRRRRRR",
	"RRRRRRRRRRRRRRRRRRRRRRRRRR",
	"RRRRRRRRRRRRRRRRRRRRRRRRRR",
	"RRRRRRRRRRRRRRRRRRRRRRRRRR",
	"RRRRRRRRRRRRRRRRRRRRRRRRRR",
	"xxxxxRRxxRRxxRRxxRRxxxxxxR",
	"xxxxxRRxxRRxxRRxxRRxxxxxxR",	//10
	"xxRRRRRRxxxxRRRxxRRRRxxRRR",
	"xxxxxRRRRxxRRRRxxRRRRxxRRR",
	"xxxxxRRRRxxRRRRxxRRRRxxRRR",
	"xxRRRRRRxxxxRRRxxRRRRxxRRR",
	"xxxxxRRxxRRxxRRxxRRRRxxRRR",
	"xxxxxRRxxRRxxRRxxRRRRxxRRR",
	"RRRRRRRRRRRRRRRRRRRRRRRRRR",
	"RRRRRRRRRRRRRRRRRRRRRRRRRR",
	"RRRRRRRRRRRRRRRRRRRRRRRRRR",
	"RRRRRRRRRRRRRRRRRRRRRRRRRR",    //20
	"RRRRRRRRRRRRRRRRRRRRRRRRRR",
	"RRRRRRRRRRRRRRRRRRRRRRRRRR",
	"RRRRRRRRRRRRRRRRRRRRRRRRRR",
	"RRRRRRRRRRRRRRRRRRRRRRRRRR",
	"RRRRRRRRRRRRRRRRRRRRRRRRRR" 	 //25
};


char *bitmap_button_ext_desktop_close_all_menus[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
	"                rryyggcc  ",     //0
	" xxx xx x  x x  rryyggcc  ",
	" x x x   x xxx  rryyggcc  ",
	"                rryyggcc  ",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"                          ",
	"      R                 R ",
	" xxx xRRxxxxxxxxx      RR ",
	" xxx   RR             RR  ",
	" xxx xxxRRxxxxxxx    RR   ",
	"         RR         RR    ",	//10
	"          RR       RR     ",
	" xxx xxxxxxRRxxxx RR      ",
	" xxx        RR   RR       ",
	" xxx xxxxxxxxRRxRR        ",
	"              RRR         ",
	"              RRR         ",
	" xxx xxxxxxxxRRxRR        ",
	" xxx        RR   RR       ",
	" xxx xxxxxxRRxxxx RR      ",
	"          RR       RR     ",    //20
	"         RR         RR    ",
	" xxx xxxRRxxxxxxx    RR   ",
	" xxx   RR             RR  ",
	" xxx xRRxxxxxxxxx      RR ",
	"                          " 	 //25
};


char *bitmap_button_ext_desktop_storage[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"                          ",
	" xxxxxxxxxxx              ",
	"xxgggggggggxx             ",
	"xgggxxxgggggx             ",
	"xggxx xxggggx             ",
	"xggx   xggggx             ",
	"xggxx xxggxxx     bbbbbbbb",
	"xgggxxxggxggx    bbbbbbbbb",	//10
	"xggggggggxggx   bbybybybbb",
	"xggg   ggxggx  bbbybybybbb",
	"xggg   ggxggx bbybybybybbb",
	"xggg   ggxggx bbybybybybbb",
	"xggg   ggxggx bbybybybybbb",
	"xggg   ggxggx bbybybybybbb",
	"xggg   ggxggx bbybbbbbbbbb",
	"xggggggggxggx bbbbbbbbbbbb",
	"xgggxxxggxggx bbwwwbwwbbbb",
	"xggxx xxggxxx bbwbbbwbwbbb",	//20
	"xggx   xggggx bbwwwbwbwbbb",
	"xggxx xxggggx bbbbwbwbwbbb",
	"xgggxxxgggggx bbwwwbwwbbbb",
	"xxgggggggggxx bbbbbbbbbbbb",
	" xxxxxxxxxxx  bbbbbbbbbbbb"  //25

};




char *bitmap_button_ext_desktop_smartload[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                x         ",     //0
  	"           x    x     x   ",
	"            x   x    x    ",
	"             x      x     ",
	"               xxxx       ",
	"             xxyyyyxx     ",
	"            xyyyyyyyyx    ",
	"            xyyyyxyyyx    ",
	"           xyyyyxyyyyyx   ",
	"        xx xyyyxyyyyyyx xx",
	"           xyyxxxxxyyyx   ",	//10
	"           xyyyyyxyyyyx   ",
	"            xyyyxyyyyx    ",
	"            xyyxyyyyyx    ",
	"             xyyyyyyx     ",
	" xxxxx       xyyyyyyx     ",
	"x     x      xxyyyyxx     ",
	"x xxx x       xyyyyx      ",
	"x     x  xxxx xyyyyx      ",
	"x     x    xx xxxxxx      ",
	"x xxx x   x x             ",    //20
	"x     x  x  x xxxxxx      ",
	"x     x x     xxxxxx      ",
	"x xxx x                   ",
	"x     x        xxxx       ",
	" xxxxx         xxxx       " 	 //25
};


char *bitmap_button_ext_desktop_settings[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "   xxx      xxx      xxx  ",     //0
  	"   xxx      xxx      xxx  ",
	"   xxx      xxx      xxx  ",
	"   xxx      xxx      xxx  ",
	"   xxx      xxx      xxx  ",
	"   xxx      xxx      xxx  ",
	"   xxx      xxx      xxx  ",
	"            xxx      xxx  ",
	" xxxxxxx    xxx      xxx  ",
	" xxxxxxx    xxx      xxx  ",
	" xxxxxxx    xxx      xxx  ",	//10
	"            xxx      xxx  ",
	"   xxx      xxx      xxx  ",
	"   xxx      xxx      xxx  ",
	"   xxx      xxx           ",
	"   xxx      xxx    xxxxxxx",
	"   xxx      xxx    xxxxxxx",
	"   xxx      xxx    xxxxxxx",
	"   xxx                    ",
	"   xxx    xxxxxxx    xxx  ",
	"   xxx    xxxxxxx    xxx  ",    //20
	"   xxx    xxxxxxx    xxx  ",
	"   xxx               xxx  ",
	"   xxx      xxx      xxx  ",
	"   xxx      xxx      xxx  ",
	"   xxx      xxx      xxx  " 	 //25
};



char *bitmap_button_ext_desktop_machine[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "          xxx             ",     //0
  	"         xxxxx            ",
	"        xx   xx           ",
	"             xx           ",
	"            xx            ",
	"           xx             ",
	"           xx             ",
	"                          ",
	"           xx             ",
	"           xx             ",
	"                          ",	//10
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxwwxwxwwxwwxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxwwxwwxwwxwwxwwxwwxwwwxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxwwxwwxwwxwwxwwxwwxwwxxx",
	"xxxxxxxxxxxxxxxxxxxxxxwxxr",    //20
	"xxxxwwxwwxwwxwwxwwxwwxwxry",
	"xxxxxxxxxxxxxxxxxxxxxxxryg",
	"xxxxxxwwxwwwwwwwwxwwxxrygb",
	"xxxxxxxxxxxxxxxxxxxxxrygbx",
	" xxxxxxxxxxxxxxxxxxxrygbx " 	 //25
};



char *bitmap_button_ext_desktop_set_machine[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"              xx          ",
	"              xgx         ",
	"       xxxxxxxxggx        ",
	"       xggggggggggx       ",
	"       xgggggggggggx      ",
	"       xggggggggggx       ",
	"       xxxxxxxxggx        ",
	"              xgx         ",
    "              xx          ",
	"                          ",	//10
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxwwxwxwwxwwxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxwwxwwxwwxwwxwwxwwxwwwxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxwwxwwxwwxwwxwwxwwxwwxxx",
	"xxxxxxxxxxxxxxxxxxxxxxwxxr",    //20
	"xxxxwwxwwxwwxwwxwwxwwxwxry",
	"xxxxxxxxxxxxxxxxxxxxxxxryg",
	"xxxxxxwwxwwwwwwwwxwwxxrygb",
	"xxxxxxxxxxxxxxxxxxxxxrygbx",
	" xxxxxxxxxxxxxxxxxxxrygbx " 	 //25
};


char *bitmap_button_ext_desktop_set_machine_only_arrow[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "             www          ",     //0
  	"             wxxw         ",
	"      wwwwwwwwxgxw        ",
	"      wxxxxxxxxggxw       ",
	"      wxggggggggggxw      ",
	"      wxgggggggggggxw     ",
	"      wxggggggggggxw      ",
	"      wxxxxxxxxggxw       ",
	"      wwwwwwwwxgxw        ",
    "             wxxw         ",
	"             www          ",	//10
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",    //20
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ", 	 //25
};



//Para lower icons

char *bitmap_lowericon_ext_desktop_cassette_std_active[EXT_DESKTOP_BUTTONS_ANCHO]={
   //01234567890123456789012345
    "                          ",//0
  	"                          ",
	"                          ",
	"                          ",
	" wwwwwwwwwwwwwwwwwwwwwwww ",
	"wwxxxxxxxxxxxxxxxxxxxxxxww",
	"wxxggggggggggggggggggggxxw",
	"wxggggggggggggggggggggggxw",
	"wxgggxxxggggggggggxxxgggxw",
	"wxggxx xxg      gxx xxggxw",
	"wxggx   xg      gx   xggxw",	//10
	"wxggxx xxg      gxx xxggxw",
	"wxgggxxxggggggggggxxxgggxw",
	"wxggggggggggggggggggggggxw",
	"wxggggggggggggggggggggggxw",
	"wxggggggggggggggggggggggxw",
	"wxgggggxxxxxxxxxxxxgggggxw",
	"wxggggxggggggggggggxggggxw",
	"wxxgggxggggggggggggxgggxxw",
	"wwxxxxxxxxxxxxxxxxxxxxxxww",
	" wwwwwwwwwwwwwwwwwwwwwwww ",	//20
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          "//25

};

char *bitmap_lowericon_ext_desktop_cassette_std_inactive[EXT_DESKTOP_BUTTONS_ANCHO]={
   //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"                          ",
	" wwwwwwwwwwwwwwwwwwwwwwww ",
	"wwxxxxxxxxxxxxxxxxxxxxxxww",
	"wxx                    xxw",
	"wx                      xw",
	"wx   xxx          xxx   xw",
	"wx  xx xx        xx xx  xw",
	"wx  x   x        x   x  xw",	//10
	"wx  xx xx        xx xx  xw",
	"wx   xxx          xxx   xw",
	"wx                      xw",
	"wx                      xw",
	"wx                      xw",
	"wx     xxxxxxxxxxxx     xw",
	"wx    x            x    xw",
	"wxx   x            x   xxw",
	"wwxxxxxxxxxxxxxxxxxxxxxxww",
	" wwwwwwwwwwwwwwwwwwwwwwww ",	//20
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          "  //25

};

char *bitmap_lowericon_ext_desktop_cassette_active[EXT_DESKTOP_BUTTONS_ANCHO]={
   //01234567890123456789012345
    "                          ",//0
  	"                          ",
	"                          ",
	"                          ",
	" wwwwwwwwwwwwwwwwwwwwwwww ",
	"wwxxxxxxxxxxxxxxxxxxxxxxww",

	"wxxggggggggggggggggggggxxw",
	"wxggggggggggggggggggggggxw",
	"wxggggBggggggggggggBggggxw",
	"wxggggBggg      gggBggggxw",
	"wxggBBBBBg      gBBBBBggxw",	//10
	"wxggggBggg      gggBggggxw",
	"wxggggBggggggggggggBggggxw",
	"wxggggggggggggggggggggggxw",
	"wxggggggggggggggggggggggxw",

	"wxggggggggggggggggggggggxw",
	"wxgggggxxxxxxxxxxxxgggggxw",
	"wxggggxggggggggggggxggggxw",
	"wxxgggxggggggggggggxgggxxw",
	"wwxxxxxxxxxxxxxxxxxxxxxxww",
	" wwwwwwwwwwwwwwwwwwwwwwww ",	//20
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          "//25

};


char *bitmap_lowericon_ext_desktop_cassette_active_frametwo[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",//0
  	"                          ",
	"                          ",
	"                          ",
	" wwwwwwwwwwwwwwwwwwwwwwww ",
	"wwxxxxxxxxxxxxxxxxxxxxxxww",

	"wxxggggggggggggggggggggxxw",
	"wxggggggggggggggggggggggxw",
	"wxgggBggggggggggggBgggggxw",
	"wxgggBgBBg      ggBgBBggxw",
	"wxggggBggg      gggBggggxw",	//10
	"wxggBBgBgg      gBBgBgggxw",
	"wxgggggBggggggggggggBgggxw",
	"wxggggggggggggggggggggggxw",
	"wxggggggggggggggggggggggxw",

	"wxggggggggggggggggggggggxw",
	"wxgggggxxxxxxxxxxxxgggggxw",
	"wxggggxggggggggggggxggggxw",
	"wxxgggxggggggggggggxgggxxw",
	"wwxxxxxxxxxxxxxxxxxxxxxxww",
	" wwwwwwwwwwwwwwwwwwwwwwww ",	//20
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          "//25

};

char *bitmap_lowericon_ext_desktop_cassette_active_framethree[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",//0
  	"                          ",
	"                          ",
	"                          ",
	" wwwwwwwwwwwwwwwwwwwwwwww ",
	"wwxxxxxxxxxxxxxxxxxxxxxxww",

	"wxxggggggggggggggggggggxxw",
	"wxggggggggggggggggggggggxw",
	"wxggBgggBggggggggBgggBggxw",
	"wxgggBgBgg      ggBgBgggxw",
	"wxggggBggg      gggBggggxw",	//10
	"wxgggBgBgg      ggBgBgggxw",
	"wxggBgggBggggggggBgggBggxw",
	"wxggggggggggggggggggggggxw",
	"wxggggggggggggggggggggggxw",

	"wxggggggggggggggggggggggxw",
	"wxgggggxxxxxxxxxxxxgggggxw",
	"wxggggxggggggggggggxggggxw",
	"wxxgggxggggggggggggxgggxxw",
	"wwxxxxxxxxxxxxxxxxxxxxxxww",
	" wwwwwwwwwwwwwwwwwwwwwwww ",	//20
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          "//25

};

char *bitmap_lowericon_ext_desktop_cassette_active_framefour[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",//0
  	"                          ",
	"                          ",
	"                          ",
	" wwwwwwwwwwwwwwwwwwwwwwww ",
	"wwxxxxxxxxxxxxxxxxxxxxxxww",

	"wxxggggggggggggggggggggxxw",
	"wxggggggggggggggggggggggxw",
	"wxgggggBggggggggggggBgggxw",
	"wxggBBgBgg      gBBgBgggxw",
	"wxggggBggg      gggBggggxw",	//10
	"wxgggBgBBg      ggBgBBggxw",
	"wxgggBggggggggggggBgggggxw",
	"wxggggggggggggggggggggggxw",
	"wxggggggggggggggggggggggxw",

	"wxggggggggggggggggggggggxw",
	"wxgggggxxxxxxxxxxxxgggggxw",
	"wxggggxggggggggggggxggggxw",
	"wxxgggxggggggggggggxgggxxw",
	"wwxxxxxxxxxxxxxxxxxxxxxxww",
	" wwwwwwwwwwwwwwwwwwwwwwww ",	//20
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          "//25

};
/*
	"wx   xxx          xxx   xw",
	"wx  xx xx        xx xx  xw",
	"wx  x   x        x   x  xw",	//10
	"wx  xx xx        xx xx  xw",
	"wx   xxx          xxx   xw",



	"wxggggggggggggggggggggggxw",
	"wxgggxxxggggggggggxxxgggxw",
	"wxggx x xg      gx x xggxw",
	"wxggxx xxg      gxx xxggxw",	//10
	"wxggx x xg      gx x xggxw",
	"wxgggxxxggggggggggxxxgggxw",
	"wxggggggggggggggggggggggxw",


	"wxx                    xxw",
	"wx                      xw",
	"wx   xxx          xxx   xw",
	"wx  xx xx        xx xx  xw",
	"wx  x   x        x   x  xw",	//10
	"wx  xx xx        xx xx  xw",
	"wx   xxx          xxx   xw",
	"wx                      xw",
	"wx                      xw",


	"wxx                    xxw",
	"wx                      xw",
	"wx   xBx          xBx   xw",
	"wx  xxBxx        xxBxx  xw",
	"wx  BBBBB        BBBBB  xw",	//10
	"wx  xxBxx        xxBxx  xw",
	"wx   xBx          xBx   xw",
	"wx                      xw",
	"wx                      xw",

	"wxx                    xxw",
	"wx                      xw",
	"wx   xx B         Bxx   xw",
	"wx  BB Bx        xB BB  xw",
	"wx  x B x        x B x  xw",	//10
	"wx  xB BB        BB Bx  xw",
	"wx  B xx          xxB   xw",
	"wx                      xw",
	"wx                      xw",


	"wxx                    xxw",
	"wx                      xw",
	"wx   Bxx          xx    xw",
	"wx  xB BB        BB  x  xw",
	"wx  x B x        x B x  xw",	//10
	"wx  BB Bx        xB BB  xw",
	"wx   xxB         B xx   xw",
	"wx                      xw",
	"wx                      xw",


*/

char *bitmap_lowericon_ext_desktop_cassette_inactive[EXT_DESKTOP_BUTTONS_ANCHO]={
   //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"                          ",
	" wwwwwwwwwwwwwwwwwwwwwwww ",
	"wwxxxxxxxxxxxxxxxxxxxxxxww",
	"wxx                    xxw",
	"wx                      xw",
	"wx    x            x    xw",
	"wx    x            x    xw",
	"wx  xxxxx        xxxxx  xw",	//10
	"wx    x            x    xw",
	"wx    x            x    xw",
	"wx                      xw",
	"wx                      xw",
	"wx                      xw",
	"wx     xxxxxxxxxxxx     xw",
	"wx    x            x    xw",
	"wxx   x            x   xxw",
	"wwxxxxxxxxxxxxxxxxxxxxxxww",
	" wwwwwwwwwwwwwwwwwwwwwwww ",	//20
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          "  //25

};



char *bitmap_lowericon_ext_desktop_mmc_active[EXT_DESKTOP_BUTTONS_ANCHO]={
   //01234567890123456789012345
    "           wwwwwwwwwwww   ",     //0
  	"          wbbbbbbbbbbbw   ",
	"         wbbbbbbbbbbbbw   ",
	"        wbbbbbbbbbbbbbw   ",
	"       wbbyybbyybbyybbw   ",
	"      wbbbyybbyybbyybbw   ",
	"     wbbbbyybbyybbyybbw   ",
	"    wbbbbbyybbyybbyybbw   ",
	"   wbbbbbbyybbyybbyybbw   ",
	"   wbbyybbyybbyybbyybbw   ",
	"   wbbyybbyybbyybbyybbw   ",	//10
	"   wbbyybbyybbyybbyybbw   ",
	"   wbbyybbyybbyybbyybbw   ",
	"   wbbyybbbbbbbbbbbbbbw   ",
	"   wbbyybbbbbbbbbbbbbbw   ",
	"   wbbbbbbbbbbbbbbbbbbw   ",
	"   wbbbbwwwwbbwwwbbbbbw   ",
	"   wbbbwbbbbbbwbbwbbbbw   ",
	"   wbbbwbbbbbbwbbbwbbbw   ",
	"   wbbbbwwwbbbwbbbwbbbw   ",
	"   wbbbbbbbwbbwbbbwbbbw   ",	//20
	"   wbbbbbbbwbbwbbwbbbbw   ",
	"   wbbbwwwwbbbwwwbbbbbw   ",
	"   wbbbbbbbbbbbbbbbbbbw   ",
	"   wbbbbbbbbbbbbbbbbbbw   ",
	"   wwwwwwwwwwwwwwwwwwww   "  //25

};


char *bitmap_lowericon_ext_desktop_mmc_inactive[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "           wwwwwwwwwwww   ",     //0
  	"          wxxxxxxxxxxxw   ",
	"         wx          xw   ",
	"        wx           xw   ",
	"       wx xx  xx  xx xw   ",
	"      wx  xx  xx  xx xw   ",
	"     wx   xx  xx  xx xw   ",
	"    wx    xx  xx  xx xw   ",
	"   wx     xx  xx  xx xw   ",
	"   wx xx  xx  xx  xx xw   ",
	"   wx xx  xx  xx  xx xw   ",	//10
	"   wx xx  xx  xx  xx xw   ",
	"   wx xx  xx  xx  xx xw   ",
	"   wx xx             xw   ",
	"   wx xx             xw   ",
	"   wx                xw   ",
	"   wx   xxxx  xxx    xw   ",
	"   wx  x      x  x   xw   ",
	"   wx  x      x   x  xw   ",
	"   wx   xxx   x   x  xw   ",
	"   wx      x  x   x  xw   ",	//20
	"   wx      x  x  x   xw   ",
	"   wx  xxxx   xxx    xw   ",
	"   wx                xw   ",
	"   wxxxxxxxxxxxxxxxxxxw   ",
	"   wwwwwwwwwwwwwwwwwwww   "  //25

};

char *bitmap_lowericon_ext_desktop_z88_slot_nonumber_active[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
	"wwwwwwwwwwwwwwwwwwwwwwwwww", //0
    "wxxxxxxxxxxxxxxxxxxxxxxxxw",
  	"w xwwwwwwwwwwwwwwwwwwwwx w",
	"wxxwxxxwxwwwxxxwxxxwwwwxxw",
	"w xwxwwwxwwwxwxwwxwwwwwx w",
	"wxxwxxxwxwwwxwxwwxwwwwwxxw",
	"w xwwwxwxwwwxwxwwxwwwwwx w",
	"wxxwxxxwxxxwxxxwwxwwwwwxxw",
	"w xwwwwwwwwwwwwwwwwwwwwx w",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",  //10
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",   //20
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxwwwwwwwwwwwwwwwwwwwwxxw",
	"wwwwwwwwwwwwwwwwwwwwwwwwww" 	 //25
};


char *bitmap_lowericon_ext_desktop_z88_slot_nonumber_inactive[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
	"wwwwwwwwwwwwwwwwwwwwwwwwww", //0
    "wxxxxxxxxxxxxxxxxxxxxxxxxw",
  	"w x                    x w",
	"wxx xxx x   xxx xxx    xxw",
	"w x x   x   x x  x     x w",
	"wxx xxx x   x x  x     xxw",
	"w x   x x   x x  x     x w",
	"wxx xxx xxx xxx  x     xxw",
	"w x                    x w",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wx                      xw",  //10
	"wx                      xw",
	"wx                      xw",
	"wx                      xw",
	"wx                      xw",
	"wx                      xw",
	"wx                      xw",
	"wx                      xw",
	"wx                      xw",
	"wx                      xw",
	"wx                      xw",   //20
	"wx                      xw",
	"wx                      xw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxwwwwwwwwwwwwwwwwwwwwxxw",
	"wwwwwwwwwwwwwwwwwwwwwwwwww" 	 //25
};

char *bitmap_lowericon_ext_desktop_z88_slot_one_active[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
	"wwwwwwwwwwwwwwwwwwwwwwwwww", //0
    "wxxxxxxxxxxxxxxxxxxxxxxxxw",
  	"w xwwwwwwwwwwwwwwwwwwwwx w",
	"wxxwxxxwxwwwxxxwxxxwwxwxxw",
	"w xwxwwwxwwwxwxwwxwwxxwx w",
	"wxxwxxxwxwwwxwxwwxwxwxwxxw",
	"w xwwwxwxwwwxwxwwxwwwxwx w",
	"wxxwxxxwxxxwxxxwwxwwwxwxxw",
	"w xwwwwwwwwwwwwwwwwwwwwx w",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",  //10
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",   //20
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxwwwwwwwwwwwwwwwwwwwwxxw",
	"wwwwwwwwwwwwwwwwwwwwwwwwww" 	 //25
};


char *bitmap_lowericon_ext_desktop_z88_slot_one_inactive[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
	"wwwwwwwwwwwwwwwwwwwwwwwwww", //0
    "wxxxxxxxxxxxxxxxxxxxxxxxxw",
  	"w x                    x w",
	"wxx xxx x   xxx xxx  x xxw",
	"w x x   x   x x  x  xx x w",
	"wxx xxx x   x x  x x x xxw",
	"w x   x x   x x  x   x x w",
	"wxx xxx xxx xxx  x   x xxw",
	"w x                    x w",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wx                      xw",  //10
	"wx                      xw",
	"wx                      xw",
	"wx                      xw",
	"wx                      xw",
	"wx                      xw",
	"wx                      xw",
	"wx                      xw",
	"wx                      xw",
	"wx                      xw",
	"wx                      xw",   //20
	"wx                      xw",
	"wx                      xw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxwwwwwwwwwwwwwwwwwwwwxxw",
	"wwwwwwwwwwwwwwwwwwwwwwwwww" 	 //25
};

char *bitmap_lowericon_ext_desktop_z88_slot_two_active[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
	"wwwwwwwwwwwwwwwwwwwwwwwwww", //0
    "wxxxxxxxxxxxxxxxxxxxxxxxxw",
  	"w xwwwwwwwwwwwwwwwwwwwwx w",
	"wxxwxxxwxwwwxxxwxxxwxxwxxw",
	"w xwxwwwxwwwxwxwwxwwwxwx w",
	"wxxwxxxwxwwwxwxwwxwwxxwxxw",
	"w xwwwxwxwwwxwxwwxwwxwwx w",
	"wxxwxxxwxxxwxxxwwxwwxxwxxw",
	"w xwwwwwwwwwwwwwwwwwwwwx w",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",  //10
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",   //20
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxwwwwwwwwwwwwwwwwwwwwxxw",
	"wwwwwwwwwwwwwwwwwwwwwwwwww" 	 //25
};


char *bitmap_lowericon_ext_desktop_z88_slot_two_inactive[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
	"wwwwwwwwwwwwwwwwwwwwwwwwww", //0
    "wxxxxxxxxxxxxxxxxxxxxxxxxw",
  	"w x                    x w",
	"wxx xxx x   xxx xxx xx xxw",
	"w x x   x   x x  x   x x w",
	"wxx xxx x   x x  x  xx xxw",
	"w x   x x   x x  x  x  x w",
	"wxx xxx xxx xxx  x  xx xxw",
	"w x                    x w",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wx                      xw",  //10
	"wx                      xw",
	"wx                      xw",
	"wx                      xw",
	"wx                      xw",
	"wx                      xw",
	"wx                      xw",
	"wx                      xw",
	"wx                      xw",
	"wx                      xw",
	"wx                      xw",   //20
	"wx                      xw",
	"wx                      xw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxwwwwwwwwwwwwwwwwwwwwxxw",
	"wwwwwwwwwwwwwwwwwwwwwwwwww" 	 //25
};


char *bitmap_lowericon_ext_desktop_z88_slot_three_active[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
	"wwwwwwwwwwwwwwwwwwwwwwwwww", //0
    "wxxxxxxxxxxxxxxxxxxxxxxxxw",
  	"w xwwwwwwwwwwwwwwwwwwwwx w",
	"wxxwxxxwxwwwxxxwxxxwxxwxxw",
	"w xwxwwwxwwwxwxwwxwwwxwx w",
	"wxxwxxxwxwwwxwxwwxwwxxwxxw",
	"w xwwwxwxwwwxwxwwxwwwxwx w",
	"wxxwxxxwxxxwxxxwwxwwxxwxxw",
	"w xwwwwwwwwwwwwwwwwwwwwx w",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",  //10
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",   //20
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxwwwwwwwwwwwwwwwwwwwwxxw",
	"wwwwwwwwwwwwwwwwwwwwwwwwww" 	 //25
};


char *bitmap_lowericon_ext_desktop_z88_slot_three_inactive[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
	"wwwwwwwwwwwwwwwwwwwwwwwwww", //0
    "wxxxxxxxxxxxxxxxxxxxxxxxxw",
  	"w x                    x w",
	"wxx xxx x   xxx xxx xx xxw",
	"w x x   x   x x  x   x x w",
	"wxx xxx x   x x  x  xx xxw",
	"w x   x x   x x  x   x x w",
	"wxx xxx xxx xxx  x  xx xxw",
	"w x                    x w",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wx                      xw",  //10
	"wx                      xw",
	"wx                      xw",
	"wx                      xw",
	"wx                      xw",
	"wx                      xw",
	"wx                      xw",
	"wx                      xw",
	"wx                      xw",
	"wx                      xw",
	"wx                      xw",   //20
	"wx                      xw",
	"wx                      xw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxwwwwwwwwwwwwwwwwwwwwxxw",
	"wwwwwwwwwwwwwwwwwwwwwwwwww" 	 //25
};



char *bitmap_lowericon_ext_desktop_mdv_active[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    " wwwwwww            wwwww ",     //0
  	" wxxxxxww          wxxxxw ",
	" wxxxxxxxwwwwwwwwwwxxxxxw ",
	" wxxxxxxxxxxxxxxxxxxxxxxw ",
	" wxxxxxxxxxxxxxxxxxxxxxxw ",
	"  wxxxxxxxxxxxxxxxxxxxxxw ",
	"   wxxxxxxxxxxxxxxxxxxxxw ",
	"   wxxxxxxxxxxxxxxxxxxxxw ",
	"   wxxxxxxxxxxxxxxxxxxxxw ",
	"  wxxxxxxxxxxxxxxxxxxxxxw ",
	" wxxxxxxxxxxxxxxxxxxxxxxw ",	//10
	" wxxwwwwwwwwwwwwwwwwwwxxw ",
	" wxxwwwwwwwwwwwwwwwwwwxxw ",
	" wxxwwxwwwxwxxxwwxwxwwxxw ",
	" wxxwwxxwxxwxwwxwxwxwwxxw ",
	" wxxwwxwxwxwxwwxwxwxwwxxw ",
	" wxxwwxwwwxwxxxwwwxwwwxxw ",
	" wxxwwwwwwwwwwwwwwwwwwxxw ",
	" wxxwwwwwwwwwwwwwwwwwwxxw ",
	" wxxxxxxxxxxxxxxxxxxxxxxw ",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",    //20
	" wxxxxxxxxxxxxxxxxxxxxxxw ",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	" wxxxxxxxxxxxxxxxxxxxxxxw ",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wwwwwwwwwwwwwwwwwwwwwwwwww" 	 //25
};


char *bitmap_lowericon_ext_desktop_mdv_inactive[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    " wwwwww             wwwww ",     //0
  	" wxxxxxw           wxxxxw ",
	" wx    xxwwwwwwwwwwx   xw ",
	" wx      xxxxxxxxxx    xw ",
	" wx                    xw ",
	"  wx                   xw ",
	"   wx                  xw ",
	"   wx                  xw ",
	"   wx                  xw ",
	"  wx                   xw ",
	" wx                    xw ",	//10
	" wx                    xw ",
	" wx                    xw ",
	" wx   x   x xxx  x x   xw ",
	" wx   xx xx x  x x x   xw ",
	" wx   x x x x  x x x   xw ",
	" wx   x   x xxx   x    xw ",
	" wx                    xw ",
	" wx                    xw ",
	" wx                    xw ",
	"wxx                    xxw",    //20
	" wx                    xw ",
	"wxx                    xxw",
	" wx                    xw ",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wwwwwwwwwwwwwwwwwwwwwwwwww" 	 //25
};



char *bitmap_lowericon_ext_desktop_msx_cart_active[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"                          ",
	"wwwwwwwwwwwwwwwwwwwwwwwwww",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxccccccccccccccccccccxxw",
	"wxxccccccccccccccccccccxxw",
	"wxxcxcxxxcxcxccxccxxcxcxxw",
	"wxxcxxcxxcxcxcxcxcxxcccxxw",
	"wxxcxccxxcxxxcxxxcxxcxcxxw",   //10
	"wxxcxccxxcxcxcxcxcxxcxcxxw",
	"wxxcxxcxxcxcxcxcxcxxcxcxxw",
	"wxxcxcxxxcxcxcxcxcxxcxcxxw",
	"wxxccccccccccccccccccccxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxwwwwwwwxxxxxxxxw",
	"wxx xxxxxxxxwwwxxxxxxxxxxw",
	"wxxxxxxxxxxxxwxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wwwwwwwwwwwwwwwwwwwwwwwwww",  //20
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          " 	 //25
};

char *bitmap_lowericon_ext_desktop_msx_cart_inactive[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"                          ",
	"wwwwwwwwwwwwwwwwwwwwwwwwww",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wx                      xw",
	"wx                      xw",
	"wx  x xxx x x  x  xx x  xw",
	"wx  xx xx x x x x xx    xw",
	"wx  x  xx xxx xxx xx x  xw",	//10
	"wx  x  xx x x x x xx x  xw",
	"wx  xx xx x x x x xx x  xw",
	"wx  x xxx x x x x xx x  xw",
	"wx                      xw",
	"wx                      xw",
	"wx        xxxxxxx       xw",
	"wx x        xxx         xw",
	"wx           x          xw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wwwwwwwwwwwwwwwwwwwwwwwwww",	//20
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          " 	 //25
};


char *bitmap_lowericon_ext_desktop_svi_active[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "     wwwwwwwwwwwwwwwww    ",     //0
  	"    wxxxxxxxxxxxxxxxxxw   ",
	"   wxxxxxwwwwwwwwxxxxxxw  ",
	"  wxxxxxxwxxxxxxwxxxxxxw  ",
	"  wxxxxxxwwwwwwwwxxxxxxw  ",
	"  wxxxRRRwxxxxxxwRRRxxxw  ",
	"  wxxRyyywwwwwwwwyyyRxxw  ",
	"  wxxRyxxxxxxxxxxxxyRxxw  ",
	"  wxxRyxxRRRRRRRRxxyRxxw  ",
	"  wxxRyxxxxxxxxxxxxyRxxw  ",
	"  wxxRyxRRxRRxRRxRxyRxxw  ",	//10
	"  wxxRyxRxxRRxRxxRxyRxxw  ",
	"  wxxRyxRRxRRxxRxRxyRxxw  ",
	"  wxxRyxxxxxxxxxxxxyRxxw  ",
	"  wxxRywxxwxxwxxwxwyRxxw  ",
	"  wxxRyxwxxwxwxwxwxyRxxw  ",
	"  wxxRyxxwxwxwxwxwxyRxxw  ",
	"  wxxRyyyyyyyyyyyyyyRxxw  ",
	"  wxxxRRRRRRRRRRRRRRxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",    //20
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxWxWxWxWxWxWxxxxw  ",
	"  wxwxxxWxWxWxWxWxWxxwxw  ",
	"  wxwxxxxxxxxxxxxxxxxwxw  ",
	"  wwwwwwwwwwwwwwwwwwwwww  " 	 //25
};

char *bitmap_lowericon_ext_desktop_svi_inactive[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "     wwwwwwwwwwwwwwww     ",     //0
  	"    wxxxxxxxxxxxxxxxxw    ",
	"   wx                xw   ",
	"  wx                  xw  ",
	"  wx                  xw  ",
	"  wx                  xw  ",
	"  wx                  xw  ",
	"  wx                  xw  ",
	"  wx                  xw  ",
	"  wx                  xw  ",
	"  wx                  xw  ",	//10
	"  wx                  xw  ",
	"  wx                  xw  ",
	"  wx                  xw  ",
	"  wx                  xw  ",
	"  wx                  xw  ",
	"  wx                  xw  ",
	"  wx                  xw  ",
	"  wx                  xw  ",
	"  wx                  xw  ",
	"  wx                  xw  ",    //20
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxx x x x x x xxxxw  ",
	"  wx xxx x x x x x xx xw  ",
	"  wx xxxxxxxxxxxxxxxx xw  ",
	"  wwwwwwwwwwwwwwwwwwwwww  " 	 //25
};


char *bitmap_lowericon_ext_desktop_coleco_active[EXT_DESKTOP_BUTTONS_ANCHO]={
   //01234567890123456789012345
    "   wwwwwwwwwwwwwwwwwwww   ",     //0
  	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	" wxxxxxxxxxxxxxxxxxxxxxxw ",
	"wxxxxxWWWWWWWWWWWWWWxxxxxw",
	" wxxxWWxxxxxxxxxxxxWWxxxw ",
	"wxxxxWxxxxxxxxxxxxxxWxxxxw",
	" wxxxWxxxxxxxxxxxxxxWxxxw ",
	"wxxxxWxWWxWWxWxxWWxxWxxxxw",
	" wxxxWxWxxWWxWxxWxxxWxxxw ",
	"  wxxWxWWxWWxWWxWWxxWxxw  ",
	"  wxxWxxxxxxxxxxxxxxWxxw  ",	//10
	"  wxxWxxxxxxxxxxxxxxWxxw  ",
	"  wxxWxRxxRxRRxRxRRxWxxw  ",
	"  wxxWxRRxRxxRxRxRRxWxxw  ",
	"  wxxWxRRxRxRxxRxRRxWxxw  ",
	"  wxxWxxxxxxxxxxxxxxWxxw  ",
	"  wxxWxxxxxxxxxxxxxxWxxw  ",
	"  wxxWxxxxxxxxxxxxxxWxxw  ",
	"  wxxWWxxxxxxxxxxxxWWxxw  ",
	"  wxxxWWWWWWWWWWWWWWxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",    //20
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wwwwwwwwwwwwwwwwwwwwww  " 	 //25
};

char *bitmap_lowericon_ext_desktop_coleco_inactive[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "   wwwwwwwwwwwwwwwwwwww   ",     //0
  	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	" wxx                  xxw ",
	"wxxx  xxxxxxxxxxxxxx  xxxw",
	" wxx xx            xx xxw ",
	"wxxx x              x xxxw",
	" wxx x              x xxw ",
	"wxxx x              x xxxw",
	" wxx x              x xxw ",
	"  wx x              x xw  ",
	"  wx x              x xw  ",	//10
	"  wx x              x xw  ",
	"  wx x              x xw  ",
	"  wx x              x xw  ",
	"  wx x              x xw  ",
	"  wx x              x xw  ",
	"  wx x              x xw  ",
	"  wx x              x xw  ",
	"  wx xx            xx xw  ",
	"  wx  xxxxxxxxxxxxxx  xw  ",
	"  wx                  xw  ",    //20
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wwwwwwwwwwwwwwwwwwwwww  " 	 //25
};



char *bitmap_lowericon_ext_desktop_sg1000_active[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "   wwwwwwwwwwwwwwwwwwww   ",     //0
  	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	" wxxxyxxyxyxxxyxxyxyxxxxw ",
	"  wxxyyxyxxyxxyyxyxyxxxw  ",
	" wxxxyyxyxyyxxyyxyxxyxxxw ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	" wxxxwwwwwwwwwwwwwwwwxxxw ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	" wxxxxxxxxxxxxxxxxxxxxxxw ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	" wxxxxxxxxxxxxxxxxxxxxxxw ",	//10
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxwwwwwwwwwwwwwwwwxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxBxxBxxBxxxw  ",
	"  wxxxxxxxxxxxBxBxxBBxxw  ",
	"  wxxxxxxxxxxBBxBBxBBxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",    //20
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wwwwwwwwwwwwwwwwwwwwww  " 	 //25
};

char *bitmap_lowericon_ext_desktop_sg1000_inactive[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "   wwwwwwwwwwwwwwwwwwww   ",     //0
  	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	" wxx                  xxw ",
	"  wx                  xw  ",
	" wxx                  xxw ",
	"  wx                  xw  ",
	" wxx xxxxxxxxxxxxxxxx xxw ",
	"  wx                  xw  ",
	" wxx                  xxw ",
	"  wx                  xw  ",
	" wxx                  xxw ",	//10
	"  wx                  xw  ",
	"  wx                  xw  ",
	"  wx xxxxxxxxxxxxxxxx xw  ",
	"  wx                  xw  ",
	"  wx                  xw  ",
	"  wx                  xw  ",
	"  wx                  xw  ",
	"  wx                  xw  ",
	"  wx                  xw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",    //20
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wwwwwwwwwwwwwwwwwwwwww  " 	 //25
};

char *bitmap_lowericon_ext_desktop_sms_active[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"wwwwwwwwwwwwwwwwwwwwwwwwww",
	"wxrrrrrrrrrrrrrrrrrrrrrrxw",
	"wxrrrrrrrrrrrrrrrrrrrrrrxw",
	"wxrwwrwwrwwrrrrrrwwrrwrrxw",
	"wxrwrrwwrrwrrrrrrwrrwwwrxw",
	"wxrrrrrrrrrrrrrrrrrrrrrrxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wwxxxxxxxxxxxxxxxxxxxxxxww",	//10
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wwxxxxxxxxxxxxxxxxxxxxxxww",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",    //20
	"wwwwwwwwwwwwwwwwwwwwwwwwww",
	"                          ",
	"                          ",
	"                          ",
	"                          ", //25
};

char *bitmap_lowericon_ext_desktop_sms_inactive[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"wwwwwwwwwwwwwwwwwwwwwwwwww",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wx                      xw",
	"wx                      xw",
	"wx                      xw",
	"wx                      xw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wwx                    xww",	//10
	"wx                      xw",
	"wwx                    xww",
	"wx                      xw",
	"wx                      xw",
	"wx                      xw",
	"wx                      xw",
	"wx                      xw",
	"wx                      xw",
	"wx                      xw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",    //20
	"wwwwwwwwwwwwwwwwwwwwwwwwww",
	"                          ",
	"                          ",
	"                          ",
	"                          ", //25
};


char *bitmap_lowericon_ext_desktop_plus3_flp_active[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "  wwwwwwwwwwwwwwwwwwwwww  ",     //0
  	"  wxxxx xxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxx xxxxxxwwxxxxxx xxw  ",
	"  wxxxxxxxxxwwxxxxxxxxxw  ",
	"  wxxxxxxxxxwwxxxxxxxxxw  ",
	"  wxxxxxxxxxwwxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxWWxxxxxxxxxw  ",	//10
	"  wxxxxxxxxWWWWxxxxxxxxw  ",
	"  wxxxxxxxxWWWWxxxxxxxxw  ",
	"  wxxxxxxxxxWWxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxRRRRRRRRRRRRRRxxxw  ",
	"  wxxRRWWRWRWWRRWRWRRxxw  ",
	"  wxxRRRRRRRRRRRRRRRRxxw  ",	//20
	"  wxxWWWWWWWWWWWWWWWWxxw  ",
	"  wxxWWWWWWWWWWWWWWWWxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxwwwwwwwwwwwwwwwwxxw  ",
	"  wxxwwwwwwwwwwwwwwwwxxw  " 	 //25
};


char *bitmap_lowericon_ext_desktop_plus3_flp_active_framezero[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "  wwwwwwwwwwwwwwwwwwwwww  ",     //0
  	"  wxxxx xxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxx xxxxxxwwxxxxxx xxw  ",
	"  wxxxxxxxxxwwxxxxxxxxxw  ",
	"  wxxxxxxxxxwwxxxxxxxxxw  ",
	"  wxxxxxxxxxwwxxxxxxxxxw  ",
	"  wxxxxxxxxxggxxxxxxxxxw  ",
	"  wxxxxxxxxxggxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxWWxxxxxxxxxw  ",	//10
	"  wxxxxxxxxWWWWxxxxxxxxw  ",
	"  wxxxxxxxxWWWWxxxxxxxxw  ",
	"  wxxxxxxxxxWWxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxRRRRRRRRRRRRRRxxxw  ",
	"  wxxRRWWRWRWWRRWRWRRxxw  ",
	"  wxxRRRRRRRRRRRRRRRRxxw  ",	//20
	"  wxxWWWWWWWWWWWWWWWWxxw  ",
	"  wxxWWWWWWWWWWWWWWWWxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxwwwwwwwwwwwwwwwwxxw  ",
	"  wxxwwwwwwwwwwwwwwwwxxw  " 	 //25
};

char *bitmap_lowericon_ext_desktop_plus3_flp_active_frameone[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "  wwwwwwwwwwwwwwwwwwwwww  ",     //0
  	"  wxxxx xxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxx xxxxxxwwxxxxxx xxw  ",
	"  wxxxxxxxxxwwxxxxxxxxxw  ",
	"  wxxxxxxxxxwwxxxxxxxxxw  ",
	"  wxxxxxxxxxwwxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxWWxxxxxxxxxw  ",	//10
	"  wxxxxxggxWWWWxxxxxxxxw  ",
	"  wxxxxxggxWWWWxxxxxxxxw  ",
	"  wxxxxxxxxxWWxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxRRRRRRRRRRRRRRxxxw  ",
	"  wxxRRWWRWRWWRRWRWRRxxw  ",
	"  wxxRRRRRRRRRRRRRRRRxxw  ",	//20
	"  wxxWWWWWWWWWWWWWWWWxxw  ",
	"  wxxWWWWWWWWWWWWWWWWxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxwwwwwwwwwwwwwwwwxxw  ",
	"  wxxwwwwwwwwwwwwwwwwxxw  " 	 //25
};

char *bitmap_lowericon_ext_desktop_plus3_flp_active_frametwo[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "  wwwwwwwwwwwwwwwwwwwwww  ",     //0
  	"  wxxxx xxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxx xxxxxxwwxxxxxx xxw  ",
	"  wxxxxxxxxxwwxxxxxxxxxw  ",
	"  wxxxxxxxxxwwxxxxxxxxxw  ",
	"  wxxxxxxxxxwwxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxWWxxxxxxxxxw  ",	//10
	"  wxxxxxxxxWWWWxxxxxxxxw  ",
	"  wxxxxxxxxWWWWxxxxxxxxw  ",
	"  wxxxxxxxxxWWxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxggxxxxxxxxxw  ",
	"  wxxxxxxxxxggxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxRRRRRRRRRRRRRRxxxw  ",
	"  wxxRRWWRWRWWRRWRWRRxxw  ",
	"  wxxRRRRRRRRRRRRRRRRxxw  ",	//20
	"  wxxWWWWWWWWWWWWWWWWxxw  ",
	"  wxxWWWWWWWWWWWWWWWWxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxwwwwwwwwwwwwwwwwxxw  ",
	"  wxxwwwwwwwwwwwwwwwwxxw  " 	 //25
};

char *bitmap_lowericon_ext_desktop_plus3_flp_active_framethree[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "  wwwwwwwwwwwwwwwwwwwwww  ",     //0
  	"  wxxxx xxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxx xxxxxxwwxxxxxx xxw  ",
	"  wxxxxxxxxxwwxxxxxxxxxw  ",
	"  wxxxxxxxxxwwxxxxxxxxxw  ",
	"  wxxxxxxxxxwwxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxWWxxxxxxxxxw  ",	//10
	"  wxxxxxxxxWWWWxggxxxxxw  ",
	"  wxxxxxxxxWWWWxggxxxxxw  ",
	"  wxxxxxxxxxWWxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxRRRRRRRRRRRRRRxxxw  ",
	"  wxxRRWWRWRWWRRWRWRRxxw  ",
	"  wxxRRRRRRRRRRRRRRRRxxw  ",	//20
	"  wxxWWWWWWWWWWWWWWWWxxw  ",
	"  wxxWWWWWWWWWWWWWWWWxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxwwwwwwwwwwwwwwwwxxw  ",
	"  wxxwwwwwwwwwwwwwwwwxxw  " 	 //25
};

char *bitmap_lowericon_ext_desktop_plus3_flp_active_save_framezero[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "  wwwwwwwwwwwwwwwwwwwwww  ",     //0
  	"  wxxxx xxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxx xxxxxxwwxxxxxx xxw  ",
	"  wxxxxxxxxxwwxxxxxxxxxw  ",
	"  wxxxxxxxxxwwxxxxxxxxxw  ",
	"  wxxxxxxxxxwwxxxxxxxxxw  ",
	"  wxxxxxxxxxrrxxxxxxxxxw  ",
	"  wxxxxxxxxxrrxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxWWxxxxxxxxxw  ",	//10
	"  wxxxxxxxxWWWWxxxxxxxxw  ",
	"  wxxxxxxxxWWWWxxxxxxxxw  ",
	"  wxxxxxxxxxWWxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxRRRRRRRRRRRRRRxxxw  ",
	"  wxxRRWWRWRWWRRWRWRRxxw  ",
	"  wxxRRRRRRRRRRRRRRRRxxw  ",	//20
	"  wxxWWWWWWWWWWWWWWWWxxw  ",
	"  wxxWWWWWWWWWWWWWWWWxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxwwwwwwwwwwwwwwwwxxw  ",
	"  wxxwwwwwwwwwwwwwwwwxxw  " 	 //25
};

char *bitmap_lowericon_ext_desktop_plus3_flp_active_save_frameone[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "  wwwwwwwwwwwwwwwwwwwwww  ",     //0
  	"  wxxxx xxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxx xxxxxxwwxxxxxx xxw  ",
	"  wxxxxxxxxxwwxxxxxxxxxw  ",
	"  wxxxxxxxxxwwxxxxxxxxxw  ",
	"  wxxxxxxxxxwwxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxWWxxxxxxxxxw  ",	//10
	"  wxxxxxrrxWWWWxxxxxxxxw  ",
	"  wxxxxxrrxWWWWxxxxxxxxw  ",
	"  wxxxxxxxxxWWxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxRRRRRRRRRRRRRRxxxw  ",
	"  wxxRRWWRWRWWRRWRWRRxxw  ",
	"  wxxRRRRRRRRRRRRRRRRxxw  ",	//20
	"  wxxWWWWWWWWWWWWWWWWxxw  ",
	"  wxxWWWWWWWWWWWWWWWWxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxwwwwwwwwwwwwwwwwxxw  ",
	"  wxxwwwwwwwwwwwwwwwwxxw  " 	 //25
};

char *bitmap_lowericon_ext_desktop_plus3_flp_active_save_frametwo[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "  wwwwwwwwwwwwwwwwwwwwww  ",     //0
  	"  wxxxx xxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxx xxxxxxwwxxxxxx xxw  ",
	"  wxxxxxxxxxwwxxxxxxxxxw  ",
	"  wxxxxxxxxxwwxxxxxxxxxw  ",
	"  wxxxxxxxxxwwxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxWWxxxxxxxxxw  ",	//10
	"  wxxxxxxxxWWWWxxxxxxxxw  ",
	"  wxxxxxxxxWWWWxxxxxxxxw  ",
	"  wxxxxxxxxxWWxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxrrxxxxxxxxxw  ",
	"  wxxxxxxxxxrrxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxRRRRRRRRRRRRRRxxxw  ",
	"  wxxRRWWRWRWWRRWRWRRxxw  ",
	"  wxxRRRRRRRRRRRRRRRRxxw  ",	//20
	"  wxxWWWWWWWWWWWWWWWWxxw  ",
	"  wxxWWWWWWWWWWWWWWWWxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxwwwwwwwwwwwwwwwwxxw  ",
	"  wxxwwwwwwwwwwwwwwwwxxw  " 	 //25
};

char *bitmap_lowericon_ext_desktop_plus3_flp_active_save_framethree[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "  wwwwwwwwwwwwwwwwwwwwww  ",     //0
  	"  wxxxx xxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxx xxxxxxwwxxxxxx xxw  ",
	"  wxxxxxxxxxwwxxxxxxxxxw  ",
	"  wxxxxxxxxxwwxxxxxxxxxw  ",
	"  wxxxxxxxxxwwxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxWWxxxxxxxxxw  ",	//10
	"  wxxxxxxxxWWWWxrrxxxxxw  ",
	"  wxxxxxxxxWWWWxrrxxxxxw  ",
	"  wxxxxxxxxxWWxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxRRRRRRRRRRRRRRxxxw  ",
	"  wxxRRWWRWRWWRRWRWRRxxw  ",
	"  wxxRRRRRRRRRRRRRRRRxxw  ",	//20
	"  wxxWWWWWWWWWWWWWWWWxxw  ",
	"  wxxWWWWWWWWWWWWWWWWxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxwwwwwwwwwwwwwwwwxxw  ",
	"  wxxwwwwwwwwwwwwwwwwxxw  " 	 //25
};


char *bitmap_lowericon_ext_desktop_plus3_flp_inactive[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "  wwwwwwwwwwwwwwwwwwwwww  ",    //0
  	"  wxxxxwxxxxxxxxxxxxxxxw  ",
	"  wx                  xw  ",
	"  wx x      xx      x xw  ",
	"  wx        xx        xw  ",
	"  wx        xx        xw  ",
	"  wx        xx        xw  ",
	"  wx        xx        xw  ",
	"  wx                  xw  ",
	"  wx                  xw  ",
	"  wx        xx        xw  ",	//10
	"  wx       x  x       xw  ",
	"  wx       x  x       xw  ",
	"  wx        xx        xw  ",
	"  wx                  xw  ",
	"  wx                  xw  ",
	"  wx                  xw  ",
	"  wx                  xw  ",
	"  wx  xxxxxxxxxxxxxx  xw  ",
	"  wx x              x xw  ",
	"  wx x              x xw  ",    //20
	"  wx x              x xw  ",
	"  wx x              x xw  ",
	"  wx x              x xw  ",
	"  wx x              x xw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  " 	 //25
};


char *bitmap_lowericon_ext_desktop_betadisk_active[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"  wwwwwwwwwwwwwwwwwww     ",
	"  wBBBBBBBBBBBBBBBBBBw    ",
	"  wBBBBBwwwwwwwwwwBBBBw   ",
	"  wBBBBBwwwwwwBBBwBBBBBw  ",
	"  wBBBBBwwwwwwBBBwBBBBBw  ",
	"  wBBBBBwwwwwwBBBwBBBBBw  ",
	"  wBBBBBwwwwwwBBBwBBBBBw  ",
	"  wBBBBBwwwwwwBBBwBBBBBw  ",
	"  wBBBBBwwwwwwwwwwBBBBBw  ",
	"  wBBBBBBBBBBBBBBBBBBBBw  ",	//10
	"  wBBBBBBBBBBBBBBBBBBBBw  ",
	"  wBBBWWWWWWWWWWWWWWBBBw  ",
	"  wBBBWWWxxWWxxWWWWWBBBw  ",
	"  wBBBWWWxWxWxWxWWWWBBBw  ",
	"  wBBBWWWxxWWxWxWWWWBBBw  ",
	"  wBBBWWWxWxWxWxWWWWBBBw  ",
	"  wBBBWWWxxWWxxWWWWWBBBw  ",
	"  wBBBWWWWWWWWWWWWWWBBBw  ",
	"  wBBBWWWWWWWWWWWWWWBBBw  ",
	"  wBBBWWxxxxxxxxxxWWBBBw  ",    //20
	"  wBBBWWWWWWWWWWWWWWBBBw  ",
	"  wB BWWWWWWWWWWWWWWB Bw  ",
	"  wBBBWWWWWWWWWWWWWWBBBw  ",
	"  wBBBBBBBBBBBBBBBBBBBBw  ",
	"  wwwwwwwwwwwwwwwwwwwwww  " 	 //25
};

char *bitmap_lowericon_ext_desktop_betadisk_inactive[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"  wwwwwwwwwwwwwwwwwww     ",
	"  wxxxxxxxxxxxxxxxxxxw    ",
	"  wx    xxxxxxxxxx   xw   ",
	"  wx    xxxxxx   x    xw  ",
	"  wx    xxxxxx   x    xw  ",
	"  wx    xxxxxx   x    xw  ",
	"  wx    xxxxxx   x    xw  ",
	"  wx    xxxxxx   x    xw  ",
	"  wx    xxxxxxxxxx    xw  ",
	"  wx                  xw  ",	//10
	"  wx                  xw  ",
	"  wx  xxxxxxxxxxxxxx  xw  ",
	"  wx  x            x  xw  ",
	"  wx  x            x  xw  ",
	"  wx  x            x  xw  ",
	"  wx  x xxxxxxxxxx x  xw  ",
	"  wx  x            x  xw  ",
	"  wx  x            x  xw  ",
	"  wx  x            x  xw  ",
	"  wx  x xxxxxxxxxx x  xw  ",    //20
	"  wx  x            x  xw  ",
	"  wx  x            x  xw  ",
	"  wx  x            x  xw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wwwwwwwwwwwwwwwwwwwwww  " 	 //25
};

char *bitmap_lowericon_ext_desktop_cart_timex_active[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"     wwwwwwwwwwwwwwww     ",
	"    wxxxxxxxxxxxxxxxxw    ",
	"   wxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxx            xxxxw  ",
	"  wxxx              xxxw  ",
	"  wxxx              xxxw  ",
	"  wxxxx            xxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wxwwwwwwwwwwwwwwwwwwxw  ",	//10
	"  wxwxxxwxwxwxwxxwxwxwxw  ",
	"  wxwwxwwxwxxxwxwwwxwwxw  ",
	"  wxwwxwwxwxwxwxxwxwxwxw  ",
	"  wxwwwwwwwwwwwwwwwwwwxw  ",
	"  wxwxxwxwxxwxxwxwwxxwxw  ",
	"  wxwxwwxwxxwxwwxwwxwwxw  ",
	"  wxwwxwxwxxwxxwxxwxwwxw  ",
	"  wxwwwwwwwwwwwwwwwwwwxw  ",
	"  wxwrrrrwwggggwwccccwxw  ",
	"  wxwrrrrwwggggwwccccwxw  ",	//20
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wwwxxxxxxxxxxxxxxxxwww  ",
	"    wxxxxxxxxxxxxxxxxw    ",
	"    wxxxxxxxxxxxxxxxxw    ",
	"    wwwwwwwwwwwwwwwwww    " 	 //25
};

char *bitmap_lowericon_ext_desktop_cart_timex_inactive[EXT_DESKTOP_BUTTONS_ANCHO]={
     //01234567890123456789012345
    "                          ",     //0
  	"     wwwwwwwwwwwwwwww     ",
	"    wxxxxxxxxxxxxxxxxw    ",
	"   wx                xw   ",
	"  wx   xxxxxxxxxxxx   xw  ",
	"  wx  xxxxxxxxxxxxxx  xw  ",
	"  wx  xxxxxxxxxxxxxx  xw  ",
	"  wx   xxxxxxxxxxxx   xw  ",
	"  wx                  xw  ",
	"  wx                  xw  ",
	"  wx                  xw  ",	//10
	"  wx                  xw  ",
	"  wx                  xw  ",
	"  wx                  xw  ",
	"  wx                  xw  ",
	"  wx                  xw  ",
	"  wx                  xw  ",
	"  wx                  xw  ",
	"  wx                  xw  ",
	"  wx                  xw  ",
	"  wx                  xw  ",    //20
	"  wxxxxxxxxxxxxxxxxxxxxw  ",
	"  wwwxxxxxxxxxxxxxxxxwww  ",
	"    wxxxxxxxxxxxxxxxxw    ",
	"    wxxxxxxxxxxxxxxxxw    ",
	"    wwwwwwwwwwwwwwwwww    " 	 //25
};



char *bitmap_lowericon_ext_desktop_ide_active[EXT_DESKTOP_BUTTONS_ANCHO]={
   //01234567890123456789012345
	" wwwwwwwwwwwwwwwwwwwwwwww ",   //0
	" wxxxxxxxxxxxxxxxxxxxxxxw ",
	" wxggggggggggggggggggggxw ",
	" wxgxxgggggxxxxgggggxxgxw ",
	" wxgxxgggxxccccxxgggxxgxw ",
	" wxgggggxccccccccxgggggxw ",
	" wxggggxccccccccccxggggxw ",
	" wxggggxccccccccccxggggxw ",
	" wxgggxccccccccccccxgggxw ",
	" wxgggxcccccxxcccccxgggxw ",
	" wxgggxcccccxxcccccxgggxw ",  //10
	" wxgggxccccccccccccxgggxw ",
	" wxggggxcccxxcccccxggggxw ",
	" wxggggxcxxxccccccxggggxw ",
	" wxggggxxxxxcccccxgggggxw ",
	" wxggxxxxxxccccxxggggggxw ",
	" wxgxxxxxxxxxxxggggggggxw ",
	" wxggxxxxxxggggggggggggxw ",
	" wxgggxxxxggxgxxggxxxggxw ",
	" wxggggxxgggxgxgxgxggggxw ",
	" wxgggggxgggxgxgxgxxgggxw ",  //20
	" wxgxxggggggxgxgxgxggggxw ",
	" wxgxxggggggxgxxggxxxggxw ",
	" wxggggggggggggggggggggxw ",
	" wxxxxxxxxxxxxxxxxxxxxxxw ",
	" wwwwwwwwwwwwwwwwwwwwwwww "   //25
};

char *bitmap_lowericon_ext_desktop_ide_inactive[EXT_DESKTOP_BUTTONS_ANCHO]={
   //01234567890123456789012345
    " wwwwwwwwwwwwwwwwwwwwwwww ",     //0
  	" wxxxxxxxxxxxxxxxxxxxxxxw ",
	" wx                    xw ",
	" wx xx     xxxx     xx xw ",
	" wx xx   xx    xx   xx xw ",
	" wx     x        x     xw ",
	" wx    x          x    xw ",
	" wx    x          x    xw ",
	" wx   x            x   xw ",
	" wx   x     xx     x   xw ",
	" wx   x     xx     x   xw ",	//10
	" wx   x            x   xw ",
	" wx    x   xx     x    xw ",
	" wx    x xxx      x    xw ",
	" wx    xx  x     x     xw ",
	" wx  xx   x    xx      xw ",
	" wx x     xxxxx        xw ",
	" wx  x    x            xw ",
	" wx   x  x  x xx  xxx  xw ",
	" wx    xx   x x x x    xw ",
	" wx     x   x x x xx   xw ",    //20
	" wx xx      x x x x    xw ",
	" wx xx      x xx  xxx  xw ",
	" wx                    xw ",
	" wxxxxxxxxxxxxxxxxxxxxxxw ",
	" wwwwwwwwwwwwwwwwwwwwwwww " 	 //25
};




char *bitmap_lowericon_ext_desktop_dandanator_active[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",   //0
    "                          ",
	"                          ",
	"wwwwwwwwwwwwwwwwwwwwwwwwww",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxggggggggggggggggggggggxw",
	"wxggggggggggggggggggxxggxw",
	"wxggggggggggggggggggxxggxw",
	"wxggggggggggggggggggggggxw",
	"wxggxxxxggggxxxggggxxxggxw",
	"wxggxxxxxggggggxggxgggxgxw",
	"wxggxxggxxggxxxxggxgggxgxw",
	"wxggxxggxxgxgggxggxgggxgxw",
	"wxggxxggxxggxxxgxgxgggxgxw",
	"wxggxxggxxggggggggggggggxw",
	"wxggxxxxxgggggggggggggggxw",
	"wxggxxxxggggggggggggggggxw",
	"wxggggggggggggggggggggggxw",
	"wxggggggggggggggggggggggxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wwwwwwwxxxxxxxxxxxxxxwwwww",	//20
	"      wxxxxxxxxxxxxxxw    ",
	"      wxxxxxxxxxxxxxxw    ",
	"      wxxxxxxxxxxxxxxw    ",
	"      wwwwwwwwwwwwwwww    ",
	"                          "    //25
};

char *bitmap_lowericon_ext_desktop_dandanator_inactive[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"wwwwwwwwwwwwwwwwwwwwwwwwww",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wx                      xw",
	"wx                      xw",
	"wx                      xw",
	"wx                      xw",
	"wx  xxxx    xxx    xxx  xw",
	"wx  xxxxx      x  x   x xw",
	"wx  xx  xx  xxxx  x   x xw",
	"wx  xx  xx x   x  x   x xw",
	"wx  xx  xx  xxx x x   x xw",
	"wx  xx  xx              xw",
	"wx  xxxxx               xw",
	"wx  xxxx                xw",
	"wx                      xw",
	"wx                      xw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wwwwwwwxxxxxxxxxxxxxxwwwww",	//20
	"      wxxxxxxxxxxxxxxw    ",
	"      wxxxxxxxxxxxxxxw    ",
	"      wxxxxxxxxxxxxxxw    ",
	"      wwwwwwwwwwwwwwww    ",
	"                          " 	 //25
};


char *bitmap_lowericon_ext_desktop_zxunoflash[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "   wxxw  wxxw  wxxw  wxxw ",     //0
  	"   wxxw  wxxw  wxxw  wxxw ",
	"   wxxw  wxxw  wxxw  wxxw ",
	"  wxxw  wxxw  wxxw  wxxw  ",
	"wwwxxwwwwxxwwwwxxwwwwxxwww",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxCCCxCxCxCxCxCxxCxCCCxxw",
	"wxxxxCxCxCxCxCxCCxCxCxCxxw",
	"wxxxCxxxCxxCxCxCxCCxCxCxxw",
	"wxxCxxxCxCxCxCxCxxCxCxCxxw",
	"wxxCCCxCxCxCCCxCxxCxCCCxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxCCCxCxxxxCxxCCCxCxCxxxw",
	"wxxCxxxCxxxCxCxCxxxCxCxxxw",
	"wxxCCxxCxxxCCCxCCCxCCCxxxw",
	"wxxCxxxCxxxCxCxxxCxCxCxxxw",
	"wxxCxxxCCCxCxCxCCCxCxCxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",  //20
	"wwwxxwwwwxxwwwwxxwwwwxxwww",
	"  wxxw  wxxw  wxxw  wxxw  ",
    "   wxxw  wxxw  wxxw  wxxw ",
  	"   wxxw  wxxw  wxxw  wxxw ",
	"   wxxw  wxxw  wxxw  wxxw "	//25
};

char *bitmap_lowericon_ext_desktop_hilow_active[EXT_DESKTOP_BUTTONS_ANCHO]={
   //01234567890123456789012345
    "                          ",//0
  	"                          ",
	"                          ",
	"                          ",
	" wwwwwwwwwwwwwwwwwwwwwwww ",
	"wwxxxxxxxxxxxxxxxxxxxxxxww",
	"wxxggggggggggggggggggggxxw",
	"wxgggxxxggggggggggxxxgggxw",
	"wxggxx xxg      gxx xxggxw",
	"wxggx   xg      gx   xggxw",
	"wxggxx xxg      gxx xxggxw",
	"wxgggxxxggggggggggxxxgggxw",
	"wxggggggggggggggggggggggxw",
	"wxgxgxgxgxgggxxxgxgggxggxw",
	"wxgxxxgxgxgggxgxgxgxgxggxw",
	"wxgxgxgxgxxxgxxxggxgxgggxw",
	"wxggggggggggggggggggggggxw",
	"wxgggggxxxxxxxxxxxxgggggxw",
	"wxggggxggggggggggggxggggxw",
	"wxxgggxggggggggggggxgggxxw",
	"wwxxxxxxxxxxxxxxxxxxxxxxww",
	" wwwwwwwwwwwwwwwwwwwwwwww ",
	"                          ",
	"                          ",
	"                          ",
	"                          "//25

};

char *bitmap_lowericon_ext_desktop_hilow_inactive[EXT_DESKTOP_BUTTONS_ANCHO]={
   //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"                          ",
	" wwwwwwwwwwwwwwwwwwwwwwww ",
	"wwxxxxxxxxxxxxxxxxxxxxxxww",
	"wxx                    xxw",
	"wx   xxx          xxx   xw",
	"wx  xx xx        xx xx  xw",
	"wx  x   x        x   x  xw",
	"wx  xx xx        xx xx  xw",
	"wx   xxx          xxx   xw",
    "wx                      xw",
	"wx x x x x   xxx x   x  xw",
	"wx xxx x x   x x x x x  xw",
	"wx x x x xxx xxx  x x   xw",
    "wx                      xw",
	"wx     xxxxxxxxxxxx     xw",
	"wx    x            x    xw",
	"wxx   x            x   xxw",
	"wwxxxxxxxxxxxxxxxxxxxxxxww",
	" wwwwwwwwwwwwwwwwwwwwwwww ",
	"                          ",
	"                          ",
	"                          ",
	"                          "  //25

};


char *bitmap_lowericon_ext_desktop_hilow_convert[EXT_DESKTOP_BUTTONS_ANCHO]={
   //01234567890123456789012345
	"                          ", //0
	"  xxxxxxxxxxxxxxxxxxxxxx  ",
	" xxggggggggggggggggggggxx ",
	" xgggxxxggggggggggxxxgggx ",
	" xggxx xxg      gxx xxggx ",
	" xggx   xg      gx   xggx ",
	" xggxx xxg      gxx xxggx ",
	" xgggxxxggggggggggxxxgggx ",
	" xggggggggggggggggggggggx ",
	" xgxgxgxgxgggxxxgxgggxggx ",
	" xgxxxgxgxgggxgxgxgxgxggx ",	//10
	" xgxgxgxgxxxgxxxggxgxgggx ",
	" xggggggggggggggggggggggx ",
	" xgggggxxxxxxxxxxxxgggggx ",
	" xggggxggggggggggggxggggx ",
	" xxgggxggggggggggggxgggxx ",
	"  xxxxxxxxxxxxxxxxxxxxxx  ",
	"             x            ",
    "             x            ",
    "           xxxxx          ",
    "            xxx           ",    //20
    "             x            ",
	" xx xxx xxx xxx xxx xx xx ",
	"  x x x x x x x x x  x  x ",
	"  x xxx xxx xxx xxx  x  x ",
    "                          "  //25

};


char *bitmap_button_ext_desktop_asciitable[EXT_DESKTOP_BUTTONS_ANCHO]={
	//01234567890123456789012345
	"  xxxxxxxxxxxxxxxxxxxxxx  ",	//0
	"  xbbbbbbbbbbbbbbbbbbbbx  ",
	"  xbbbbbbbbbbbbbbbbbbbbx  ",
	"  xbbWWWbWWWbbbbbbWWbbbx  ",
	"  xbbWbbbWbbbbWbbWbbWbbx  ",
	"  xbbWWWbWWWbbbbbWWWWbbx  ",
	"  xbbWbWbbbWbbWbbWbbWbbx  ",
	"  xbbWWWbWWWbbbbbWbbWbbx  ",
	"  xbbbbbbbbbbbbbbbbbbbbx  ",
	"  xbbbbbbbbbbbbbbbbbbbbx  ",
	"  xbbWWWbWWWbbbbbWWWbbbx  ",	//10
	"  xbbWbbbWbbbbWbbWbbWbbx  ",
	"  xbbWWWbWWWbbbbbWWWbbbx  ",
	"  xbbWbWbWbWbbWbbWbbWbbx  ",
	"  xbbWWWbWWWbbbbbWWWbbbx  ",
	"  xbbbbbbbbbbbbbbbbbbbbx  ",
	"  xbbbbbbbbbbbbbbbbbbbbx  ",
	"  xbbWWWbWWWbbbbbbWWWbbx  ",
	"  xbbWbbbbbWbbWbbWbbbbbx  ",
	"  xbbWWWbbbWbbbbbWbbbbbx  ",
	"  xbbWbWbbWbbbWbbWbbbbbx  ",	//20
	"  xbbWWWbbWbbbbbbbWWWbbx  ",
	"  xbbbbbbbbbbbbbbbbbbbbx  ",
	"  xbbbbbbbbbbbbbbbbbbbbx  ",
	"  xxxxxxxxxxxxxxxxxxxxxx  ",
	"                          "	//25
};

/*
Template
char *bitmap_button_ext_desktop_xxxxx[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",	//10
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",    //20
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          " 	 //25
};

*/


/*
char *bitmap_button_ext_desktop_userdefined[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
  	"  x  x  xxxx xxxx xxx     ",
	"  x  x  x    x    x  x    ",
	"  x  x  xxxx xxxx xxx     ",
	"  x  x     x x    x  x    ",
	"  xxxx  xxxx xxxx x   x   ",	//10
	"                          ",
	"                          ",
  	"                          ",
	"     xx   xxxx xxxx       ",
	"     x x  x    x          ",
	"     x  x xxxx xxx        ",
	"     x x  x    x          ",
	"     xx   xxxx x          ",
	"                          ",
	"                          ",    //20
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          " 	 //25
};
*/
char *bitmap_button_ext_desktop_userdefined[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",  //0
    "                          ",
    "      x    x  xxxx        ",
	"      xx   x x    x       ",
	"      x x  x x    x       ",
	"      x  x x x    x       ",
	"      x   xx x    x       ",
	"      x    x  xxxx        ",
  	"                          ",
    "                          ",
	"   x  xxxx  xxxx  x    x  ",	//10
	"   x x     x    x xx   x  ",
	"   x x     x    x x x  x  ",
	"   x x     x    x x  x x  ",
	"   x x     x    x x   xx  ",
	"   x  xxxx  xxxx  x    x  ",
  	"                          ",
    "                          ",
	"    x   x xxxxxx xxxxx    ",
	"     x x  x        x      ",
	"      x   xxxxxx   x      ",	//20
	"      x   x        x      ",
	"      x   x        x      ",
	"      x   xxxxxx   x      ",
	"                          ",
	"                          " 	 //25
};

char *bitmap_button_ext_desktop_fullscreen[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
	" BBB                  BBB ",
	" BB                    BB ",
	" B B                  B B ",
	"    B                B    ",
	"     B              B     ",
  	"                          ",
	"     xxxxxxxxxxxxxxxx     ",
	"     x              x     ",
	"     x              x     ",
	"     x              x     ",	//10
	"     x              x     ",
    "     x              x     ",
    "     x              x     ",
    "     x              x     ",
	"     x              x     ",
  	"     x              x     ",
	"     xxxxxxxxxxxxxxxx     ",
	"                          ",
	"     B              B     ",
	"    B                B    ",	 //20
	" B B                  B B ",
	" BB                    BB ",
	" BBB                  BBB ",
	"                          ",
	"                          " 	 //25
};


char *bitmap_button_ext_desktop_reset[EXT_DESKTOP_BUTTONS_ANCHO]={
   //01234567890123456789012345
    "          RRRRRR          ",     //0
  	"        RRRRRRRRRR        ",
	"       RRR      RRR       ",
	"      RR          RR      ",
	"     RR            RR     ",
	"    RR              RR    ",
	"   RR                RR   ",
	"   RR                 RR  ",
	"RRRRRRRRR             RR  ",
	" RRRRRRR              RR  ",
	"  RRRRR                RR ",	//10
	"  RRRRR                RR ",
	"   RRR                 RR ",
	"    R                  RR ",
	"                       RR ",
	"                       RR ",
	"   RR                  RR ",
	"   RR                 RR  ",
	"   RR                 RR  ",
	"   RR                RR   ",
	"    RR              RR    ",    //20
	"     RR            RR     ",
	"      RR          RR      ",
	"       RRR      RRR       ",
	"        RRRRRRRRRR        ",
	"          RRRRRR          " 	 //25
};

char *bitmap_button_ext_desktop_hardreset[EXT_DESKTOP_BUTTONS_ANCHO]={
   //01234567890123456789012345
    "          RRRRRR          ",     //0
  	"        RRRRRRRRRR        ",
	"       RRR      RRR       ",
	"      RR          RR      ",
	"     RR            RR     ",
	"    RR              RR    ",
	"   RR                RR   ",
	"   RR                 RR  ",
	"RRRRRRRRR             RR  ",
	" RRRRRRR              RR  ",
	"  RRRRR   bb    bb     RR ",	//10
	"  RRRRR   bb    bb     RR ",
	"   RRR    bb    bb     RR ",
	"    R     bbbbbbbb     RR ",
	"          bbbbbbbb     RR ",
	"          bb    bb     RR ",
	"   RR     bb    bb     RR ",
	"   RR     bb    bb    RR  ",
	"   RR                 RR  ",
	"   RR                RR   ",
	"    RR              RR    ",    //20
	"     RR            RR     ",
	"      RR          RR      ",
	"       RRR      RRR       ",
	"        RRRRRRRRRR        ",
	"          RRRRRR          " 	 //25
};

char *bitmap_button_ext_desktop_waveform[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
	"                          ",
	"            bb            ",
	"           b  b           ",
	"           b  b           ",
	"           b  b           ",
	"           b  b           ",
	"           b  b           ",
	"          b   b    b      ",
	"   b      b   b   b b     ",
	"  b b     b   b   b  b    ",	//10
	" b   b    b   b   b  b    ",
	" b    b   b   b   b  b  b ",
	"      b   b    b  b  b b  ",
	"      b   b    b  b   b   ",
	"      b   b    b  b       ",
	"       b b     b  b       ",
	"        b      b b        ",
	"               b b        ",
	"               b b        ",
	"                b         ",	 //20
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          " 	 //25
};


char *bitmap_button_ext_desktop_audioregisters[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "  xx                      ",
    " x  x  xxxxxxxxxxx        ",
    " xxxx                     ",
    " x  x  xxxxxxxxxxx        ",
    "                          ",
	" xxx                      ",
	" x  x  xxxxxxxxxxxrrrrrrr ",
	" xxx                      ",
	" x  x  xxxxxxxxxxxrrrrrrr ",
	" xxx                      ",
    "                          ",
    "  xxx  xxxxx              ",
    " x                        ",
	" x     xxxxx              ",
    "  xxx                     ",
	"      x   x   x   x   x   ",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxwwwwxxxwwwxxxxxxxx",
	"xwxxxxxwxxxxxxxxwxxxxxxxxx",
	"xwwxxxxwxxwwxxxxwxxxxxxxxx",
	"xwxxxxxwxxxwxxxxwxxxxxxxxx",
	"xxxxxxxxwwwxxxxwwwxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"  x   x   x   x   x   x   ",

};



char *bitmap_button_ext_desktop_geneneralsoundregisters[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "  x                       ",
    " x x   xxxxxxxxxxx        ",
    " x x                      ",
    "  xxx  xxxxxxxxxxx        ",
    "                          ",
	" x                        ",
	" xxx   xxxxxxxxxxxrrrrrrr ",
	" x x                      ",
	" xxx   xxxxxxxxxxxrrrrrrr ",
	"                          ",
    " xxx   xxxxx              ",
	" x                        ",
    " xxx   xxxxx              ",
    "   x                      ",
    " xxx   xxxxxxxxxxxrr      ",
    " x x                      ",
    " xxx   xxxxxxxxxxxrr      ",
    "                          ",
	"      x   x   x   x   x   ",
    "xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxwwwwxxxwwwxxxxxxxx",
	"xwxxxxxwxxxxxxxwxxxxxxxxxx",
	"xwwxxxxwxxwwxxxwwwxxxxxxxx",
	"xwxxxxxwxxxwxxxxxwxxxxxxxx",
	"xxxxxxxxwwwxxxxwwwxxxxxxxx",

	"  x   x   x   x   x   x   "

};

char *bitmap_button_ext_desktop_audiosheet[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345

    "WWWWWWxWWWWWWWWWWWWWWWWWWW",//0
    "WWWWWxxxWWWWWWWWWWWWWWWWWW",
    "WWWWWxWWxWWWWWWWWWWWWxWWWW",
    "WWWWxxWWxWWWxxxxxxxxxxxxxx",
	"WWWWxWWWxWWWWWWWWWWWWxWWWW",
	"WWWWxWWxxWWWWWWWWWWWWxWWWW",
	"WWWWWxWxWWWWWWWWWWWWWxWWWW",
	"WWWWWxxxWWWWxxxxxxxxxxxxxx",
	"WWWWxxxWWWWWWWWWWWWWWxWWWW",
	"WWWxxxWWWWWWWWWWWWWWWxWWWW",
	"WWxxxxWWWWWWWWWWWWWWWxWWWW",//10
	"WxxxWWxWWWWWxxxxxxxxxxxxxx",
    "WxxWWWxWWWWWWWWWWWWWWxWWWW",
	"xxWWxxxxxWWWWWWWWWWxxxWWWW",
	"xxWxWWxWxxWWWWWWWWxxxxWWWW",
	"xxWxWWxWWxxWxxxxxxxxxxxxxx",
	"WxWxWWWxWWxWWWWWWWxxxxWWWW",
	"WxWWxWWxWWxWWWWWWWWxxWWWWW",
	"WWxWWWWxWxWWWWWWWWWWWWWWWW",
	"WWWxxxxxxWWWxxxxxxxxxxxxxx",
	"WWWWWWWxWWWWWWWWWWWWWWWWWW",//20
	"WWWWWWWWxWWWWWWWWWWWWWWWWW",
	"WWWxxWWWxWWWWWWWWWWWWWWWWW",
	"WWxxxxWWxWWWWWWWWWWWWWWWWW",
	"WWWxxWWWxWWWWWWWWWWWWWWWWW",
	"WWWWxxxxWWWWWWWWWWWWWWWWWW"//25
};

char *bitmap_button_ext_desktop_audiopiano[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
	"                          ",
	"                          ",
	" xxxxxxxxxxxxxxxxxxxxxxxxx",
	" xWxxxWxxxWxWxxxWxxxWxxxWx",
	" xWxxxWxxxWxWxxxWxxxWxxxWx",
	" xWxxxWxxxWxWxxxWxxxWxxxWx",
	" xWxxxWxxxWxWxxxWxxxWxxxWx",
	" xWWxWWWxWWxWWxWWWxWWWxWWx",
	" xWWxWWWxWWxWWxWWWxWWWxWWx",
    " xWWxWWWxWWxWWxWWWxWWWxWWx",
	" xxxxxxxxxxxxxxxxxxxxxxxxx",
	"                          ",
    "                          ",
	"  x   x   x   x   x   x   ",
	"  x   x   x   x   x   x   ",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxwwwwxxxwwwxxxxxxxx",
	"xwxxxxxwxxxxxxxxwxxxxxxxxx",
	"xwwxxxxwxxwwxxxxwxxxxxxxxx",
	"xwxxxxxwxxxwxxxxwxxxxxxxxx",
	"xxxxxxxxwwwxxxxwwwxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"  x   x   x   x   x   x   ",
	"  x   x   x   x   x   x   "


};


char *bitmap_button_ext_desktop_wavepiano[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
	"                          ",
	"                          ",
	" xxxxxxxxxxxxxxxxxxxxxxxxx",
	" xWxxxWxxxWxWxxxWxxxWxxxWx",
	" xWxxxWxxxWxWxxxWxxxWxxxWx",
	" xWxxxWxxxWxWxxxWxxxWxxxWx",
	" xWxxxWxxxWxWxxxWxxxWxxxWx",
	" xWWxWWWxWWxWWxWWWxWWWxWWx",
	" xWWxWWWxWWxWWxWWWxWWWxWWx",
    " xWWxWWWxWWxWWxWWWxWWWxWWx",
	" xxxxxxxxxxxxxxxxxxxxxxxxx",
	"                          ",
	"            bb            ",
	"           b  b           ",
	"           b  b           ",
	"          b   b    b      ",
	"   b      b   b   b b     ",
	"  b b     b   b   b  b    ",
	" b   b    b   b   b  b    ",
	" b    b   b   b   b  b  b ",
	"      b   b    b  b  b b  ",
	"      b   b    b  b   b   ",
	"       b b     b  b       ",
	"        b      b b        ",
	"               b b        ",
	"                b         "
};


char *bitmap_button_ext_desktop_visualrealtape[EXT_DESKTOP_BUTTONS_ANCHO]={
   //01234567890123456789012345
	"  xxxxxxxxxxxxxxxxxxxxxx  ",
	" xxggggggggggggggggggggxx ",
	" xggggggggggggggggggggggx ",
	" xggggBggggggggggggBggggx ",
	" xggggBggg      gggBggggx ",
	" xggBBBBBg      gBBBBBggx ",
	" xggggBggg      gggBggggx ",
	" xggggBggggggggggggBggggx ",
	" xggggggggggggggggggggggx ",
	" xggggggggggggggggggggggx ",
	" xggggggggggggggggggggggx ",	//10
	" xgggggxxxxxxxxxxxxgggggx ",
	" xggggxggggggggggggxggggx ",
	" xxgggxggggggggggggxgggxx ",
	"  xxxxxxxxxxxxxxxxxxxxxx  ",
	"                          ",
	"           bbb            ",
	"          b   b    bb     ",
	"   b      b   b   b  b    ",
	"  b b     b   b   b  b    ",
	" b   b    b    b  b  b    ", //20
	" b    b   b    b  b  b  b ",
	"      b   b    b  b  b b  ",
	"       b b     b b    b   ",
	"        b      b b        ",
	"                b         "  //25
};



char *bitmap_button_ext_desktop_aymixer[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "   xxx      xxx      xxx  ",     //0
	"   xxx      xxx      xxx  ",
	"            xxx      xxx  ",
	" xxxxxxx    xxx      xxx  ",
	" xxxxxxx    xxx           ",
	"            xxx    xxxxxxx",
	"   xxx             xxxxxxx",
	"   xxx    xxxxxxx         ",
	"   xxx    xxxxxxx    xxx  ",
	"   xxx               xxx  ",
	"   xxx      xxx      xxx  ",
	"   xxx      xxx      xxx  ",
    "                          ",
    "  x   x   x   x   x   x   ",
	"  x   x   x   x   x   x   ",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxwwwwxxxwwwxxxxxxxx",
	"xwxxxxxwxxxxxxxxwxxxxxxxxx",
	"xwwxxxxwxxwwxxxxwxxxxxxxxx",
	"xwxxxxxwxxxwxxxxwxxxxxxxxx",
	"xxxxxxxxwwwxxxxwwwxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"  x   x   x   x   x   x   ",
    "  x   x   x   x   x   x   ",
};



char *bitmap_button_ext_desktop_ayplayer[EXT_DESKTOP_BUTTONS_ANCHO]={
   //01234567890123456789012345
    "      xxxxxxxxxxxxxx      ",
	"     xWWWWWWWWWWWWWWx     ",
	"     xWxxxxxxxxxxxxWx     ",
	"     xWxWWWWWWWWWWxWx     ",
	"     xWxWWxxWWxWxWxWx     ",
	"     xWxWxWWxWxWxWxWx     ",
	"     xWxWxWWxWxWxWxWx     ",
	"     xWxWxxxxWWxWWxWx     ",
	"     xWxWxWWxWWxWWxWx     ",
	"     xWxWxWWxWWxWWxWx     ",
	"     xWxWWWWWWWWWWxWx     ",
	"     xWxxxxxxxxxxxxWx     ",
    "     xWWWWWWWWWWWWWWx     ",
    "     xWWWWWWWWWWWWWWx     ",
	"     xWWWWxxxxxxWWWWx     ",
	"     xWWWxWWWWWWxWWWx     ",
	"     xWWxWWWxxWWWxWWx     ",
	"     xWWxWWxWWxWWxWWx     ",
	"     xWWxWxWWWWxWxWWx     ",
	"     xWWxWxWWWWxWxWWx     ",
	"     xWWxWWxWWxWWxWWx     ",
	"     xWWxWWWxxWWWxWWx     ",
	"     xWWWxWWWWWWxWWWx     ",
	"     xWWWWxxxxxxWWWWx     ",
	"     xWWWWWWWWWWWWWWx     ",
    "      xxxxxxxxxxxxxx      ",
};

char *bitmap_button_ext_desktop_colour_palettes[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
  	"        xxxxxxxx          ",
	"       xwwwwwwwwx         ",
	"     xxwwwbbwwwwwxx       ",
	"    xwwwwbbbbwwwwwwx      ",
	"   xwwwwwbbbbwwwrrwwx     ",
	"  xwwwwwwwbbwwwrrrrwwx    ",
	"  xwwxxwwwwwwwwrrrrwwx    ",
	" xwwxxxxwwwwwwwwrrwwwwx   ",
	" xwwxxxxwwwwwwwwwwwwwwx   ",
	"xwwwwxxwwwwwwwwwwwmmwwwx  ",
	"xwwwwwwwwwwwwwwwwmmmmwwx  ",
	"xwwwxxxxwwwwwwwwwmmmmwwx  ",
	" xxx    xwwwwwwwwwmmwwwx  ",
	"        xwwwwwwwwwwwwwwx  ",
	"       xwwwwwwwwwwggwwwx  ",
	"       xwwwwwwwwwggggwwx  ",
	"    xxxwwwwwwwwwwggggwwx  ",
	"  xxwwwwwwwwwwwwwwggwwx   ",
	" xwwwWWwwwwwwwwwwwwwwwx   ",
	" xwwWWWWwwwwwwwccwwwwx    ",
	"  xwWWWWwwyywwccccwwwx    ",
	"   xwWWwwyyyywccccwwx     ",
	"    xwwwwyyyywwccwwx      ",
	"     xxwwwyywwwwwxx       ",
	"       xwwwwwwwwx         ",
    "        xxxxxxxx          ",    //25
};


char *bitmap_button_ext_desktop_zxeyes[EXT_DESKTOP_BUTTONS_ANCHO]={
   //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"     xxx          xxx     ",
	"    xx xx        xx xx    ",
	"   x     x      x     x   ",
	"  x       x    x       x  ",
	" x         x  x         x ",
	" x         x  x         x ",
	" x         x  x         x ",
	"x           xx           x",	//10
	"x           xx           x",
	"x       xx  xx       xx  x",
	"x      xxxx xx      xxxx x",
	"x       xx  xx       xx  x",
	"x           xx           x",
	"x           xx           x",
	" x         x  x         x ",
	" x         x  x         x ",
	"  x       x    x       x  ",
	"   x     x      x     x   ",    //20
	"    xx xx        xx xx    ",
	"     xxx          xxx     ",
	"                          ",
	"                          ",
	"                          " 	 //25
};


char *bitmap_button_ext_desktop_reloadmmc[EXT_DESKTOP_BUTTONS_ANCHO]={
   //01234567890123456789012345
    "             RRRRR        ",     //0
  	"           RR     RR      ",
	"          R         R     ",
	"         R           R    ",
	"        R            R    ",
	"        R             R   ",
	"       R              R   ",
	"       R              R   ",
	"     RRRRR           R    ",
	"      RRR   bbbbbbbb R    ",
	"       R   bbbbbbbbb      ",	//10
	"          bbybybybbb      ",
	"         bbbybybybbb      ",
	"        bbybybybybbb      ",
	"        bbybybybybbb      ",
	"        bbybybybybbb      ",
	"        bbybybybybbb      ",
	"        bbybbbbbbbbb      ",
	"        bbbbbbbbbbbb      ",
	"        bbwwwbwwbbbb      ",
	"        bbwbbbwbwbbb      ",	//20
	"        bbwwwbwbwbbb      ",
	"        bbbbwbwbwbbb      ",
	"        bbwwwbwwbbbb      ",
	"        bbbbbbbbbbbb      ",
	"        bbbbbbbbbbbb      "  //25

};



char *bitmap_button_ext_desktop_loadbinary[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"           bb             ",
	"          bbbb            ",
	"         bbbbbb           ",
	"           bb             ",
	"           bb             ",
	"           bb             ",
	"           bb             ",
	"                          ",	//10
	" xxxxxxxxxxxxxxxxxxxxxxx  ",
	" xcccccccccccccccccccccx  ",
	" xccxxcxxxcxxxccxxcxxxcx  ",
	" xcxcxcxcxcxcxcxcxcxcxcx  ",
	" xcccxcxcxcxcxcccxcxcxcx  ",
	" xcccxcxxxcxxxcccxcxxxcx  ",
	" xcccccccccccccccccccccx  ",
	" xcxxxcxxxcxxxccxxccxxcx  ",
	" xcxcxcxcxcxcxcxcxcxcxcx  ",
	" xcxcxcxcxcxcxcccxcccxcx  ",   //20
	" xcxxxcxxxcxxxcccxcccxcx  ",
	" xcccccccccccccccccccccx  ",
	" xxxxxxxxxxxxxxxxxxxxxxx  ",
	"                          ",
	"                          " 	 //25
};

char *bitmap_button_ext_desktop_savebinary[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"           bb             ",
	"           bb             ",
	"           bb             ",
	"           bb             ",
	"         bbbbbb           ",
	"          bbbb            ",
	"           bb             ",
	"                          ",	//10
	" xxxxxxxxxxxxxxxxxxxxxxx  ",
	" xcccccccccccccccccccccx  ",
	" xccxxcxxxcxxxccxxcxxxcx  ",
	" xcxcxcxcxcxcxcxcxcxcxcx  ",
	" xcccxcxcxcxcxcccxcxcxcx  ",
	" xcccxcxxxcxxxcccxcxxxcx  ",
	" xcccccccccccccccccccccx  ",
	" xcxxxcxxxcxxxccxxccxxcx  ",
	" xcxcxcxcxcxcxcxcxcxcxcx  ",
	" xcxcxcxcxcxcxcccxcccxcx  ",   //20
	" xcxxxcxxxcxxxcccxcccxcx  ",
	" xcccccccccccccccccccccx  ",
	" xxxxxxxxxxxxxxxxxxxxxxx  ",
	"                          ",
	"                          " 	 //25
};

char *bitmap_button_ext_desktop_snapinramrewind[EXT_DESKTOP_BUTTONS_ANCHO]={
   //01234567890123456789012345
    "  xxxxxxxxxxxxxxxxxxxxxx  ", //0
  	" xxxxxxxxxxxxxxxxxxxxxxxx ",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxccccccccccccccccccccxxx",
	"xxxccccccccccccccccccccxxx",
	"xxxccccrrrccccccrrrccccxxx",
	"xxxcccrccrrccccrrccrcccxxx",
	"xxxccrccccrccccrccccrccxxx",
	"xxxccrccccrccccrccccrccxxx",
	"xxxcccrccrrYccYrrccrcccxxx",
	"xxxccccrrrYYYYYYrrrccccxxx",	//10
	"xxxcccccccYYYYYYcccccccxxx",
	"xxxccccccccYYYYccccccccxxx",
	"xxxcccccccccYYcccccccccxxx",
	"xxxcccccccccYYcccccccccxxx",
	"xxxccccccccrrrrccccccccxxx",
	"xxxccccBccrrccrrcccccccxxx",
	"xxxcccBBccrccccrcccccccxxx",
	"xxxccBBBccrccccrcccccccxxx",
	"xxxcBBBBcccrccrccccccccxxx",
	"xxxccBBBccccrrcccccccccxxx", //20
	"xxxcccBBcccccccccccccccxxx",
	"xxxccccBcccccccccccccccxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	" xxxxxxxxxxxxxxxxxxxxxxxx ",
	"  xxxxxxxxxxxxxxxxxxxxxx  "  //25
};


char *bitmap_button_ext_desktop_snapinramffw[EXT_DESKTOP_BUTTONS_ANCHO]={
   //01234567890123456789012345
    "  xxxxxxxxxxxxxxxxxxxxxx  ", //0
  	" xxxxxxxxxxxxxxxxxxxxxxxx ",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxccccccccccccccccccccxxx",
	"xxxccccccccccccccccccccxxx",
	"xxxccccrrrccccccrrrccccxxx",
	"xxxcccrccrrccccrrccrcccxxx",
	"xxxccrccccrccccrccccrccxxx",
	"xxxccrccccrccccrccccrccxxx",
	"xxxcccrccrrYccYrrccrcccxxx",
	"xxxccccrrrYYYYYYrrrccccxxx",	//10
	"xxxcccccccYYYYYYcccccccxxx",
	"xxxccccccccYYYYccccccccxxx",
	"xxxcccccccccYYcccccccccxxx",
	"xxxcccccccccYYcccccccccxxx",
	"xxxccccccccrrrrccccccccxxx",
	"xxxcBcccccrrccrrcccccccxxx",
	"xxxcBBccccrccccrcccccccxxx",
	"xxxcBBBcccrccccrcccccccxxx",
	"xxxcBBBBcccrccrccccccccxxx",
	"xxxcBBBcccccrrcccccccccxxx", //20
	"xxxcBBcccccccccccccccccxxx",
	"xxxcBccccccccccccccccccxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	" xxxxxxxxxxxxxxxxxxxxxxxx ",
	"  xxxxxxxxxxxxxxxxxxxxxx  "  //25
};


char *bitmap_button_ext_desktop_pause[EXT_DESKTOP_BUTTONS_ANCHO]={
   //01234567890123456789012345
    "                          ",     //0
  	"         bbb   bbb        ",
	"         bbb   bbb        ",
	"         bbb   bbb        ",
	"         bbb   bbb        ",
	"         bbb   bbb        ",
	"         bbb   bbb        ",
	"         bbb   bbb        ",
	"         bbb   bbb        ",
	"                          ",
	"                          ",	//10
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxwwxwxwwxwwxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxwwxwwxwwxwwxwwxwwxwwwxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxwwxwwxwwxwwxwwxwwxwwxxx",
	"xxxxxxxxxxxxxxxxxxxxxxwxxr",    //20
	"xxxxwwxwwxwwxwwxwwxwwxwxry",
	"xxxxxxxxxxxxxxxxxxxxxxxryg",
	"xxxxxxwwxwwwwwwwwxwwxxrygb",
	"xxxxxxxxxxxxxxxxxxxxxrygbx",
	" xxxxxxxxxxxxxxxxxxxrygbx " 	 //25
};


char *bitmap_button_ext_desktop_debugcpu[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
	"  x   x   x   x   x   x   ",	//0
	"  x   x   x   x   x   x   ",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxwwwwxxwwxxxwwxxx",
	"xwxxxxxxxxxxxwxwxxwxwxxwxx",
	"xwwxxxxxxxxxwxxxwwxxwxxwxx",
	"xwxxxxxxxxxwxxxwxxwxwxxwxx",
	"xxxxxxxxxxwwwwxxwwxxxwwxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",  //10
	"  x   x   x   x   x   x   ",
	"  x   x   x   x   x   x   ",
    "                          ",
    "                 R        ",
    "                R         ",
  	"          RRRRRR          ",
	"         RRRRRRRR         ",
	"         RRRRRRRR         ",
	"          RRRRRRR         ",
	"         RR      R        ",	//20
	"        RRRR  R   R       ",
	"        RRRRRRR   R       ",
	"         RR               ",
	"        R  R              ",
    "       R    R             "   //25
};


char *bitmap_button_ext_desktop_cpustatistics[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
	"  x   x   x   x   x   x   ",	//0
	"  x   x   x   x   x   x   ",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxwwwwxxwwxxxwwxxx",
	"xwxxxxxxxxxxxwxwxxwxwxxwxx",
	"xwwxxxxxxxxxwxxxwwxxwxxwxx",
	"xwxxxxxxxxxwxxxwxxwxwxxwxx",
	"xxxxxxxxxxwwwwxxwwxxxwwxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",  //10
	"  x   x   x   x   x   x   ",
	"  x   x   x   x   x   x   ",
    "                          ",
    " x                  xxx   ",
    " x                 x      ",
  	" x   xxxx  xxxx    x mm   ",
	" x  x    xx    xx  x mm   ",
	" x x  rr    gg   x x mm   ",
	" x    rr    gg rr x  mm   ",
	" x    rr mm gg rr    mm   ",	//20
	" x BB rr mm gg rr    mm   ",
	" x BB rr mm gg rr    mm   ",
	" x BB rr mm gg rr BB mm   ",
	" x BB rr mm gg rr BB mm   ",
    " xxxxxxxxxxxxxxxxxxxxxxxxx"   //25
};



char *bitmap_button_ext_desktop_corestatistics[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
	"                          ",	//0
	"       xxxxxxxxxxxx       ",
	"       xxxxxxxxxxxx       ",
	"               xx         ",
	"               xx r       ",
	"             xx  rr       ",
	"             xx rry       ",
	"           xx  rryy       ",
	"           xx rryyg       ",
	"         xx  rryygg       ",
	"         xx rryyggc       ",  //10
	"       xxxxxxxxxxxx       ",
	"       xxxxxxxxxxxx       ",
    "                          ",
    " x                  xxx   ",
    " x                 x      ",
  	" x   xxxx  xxxx    x mm   ",
	" x  x    xx    xx  x mm   ",
	" x x  rr    gg   x x mm   ",
	" x    rr    gg rr x  mm   ",
	" x    rr mm gg rr    mm   ",	//20
	" x BB rr mm gg rr    mm   ",
	" x BB rr mm gg rr    mm   ",
	" x BB rr mm gg rr BB mm   ",
	" x BB rr mm gg rr BB mm   ",
    " xxxxxxxxxxxxxxxxxxxxxxxxx"   //25
};

char *bitmap_button_ext_desktop_ioports[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
	"  x   x   x   x   x   x   ",	//0
	"  x   x   x   x   x   x   ",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxwwwwxxwwxxxwwxxx",
	"xwxxxxxxxxxxxwxwxxwxwxxwxx",
	"xwwxxxxxxxxxwxxxwwxxwxxwxx",
	"xwxxxxxxxxxwxxxwxxwxwxxwxx",
	"xxxxxxxxxxwwwwxxwwxxxwwxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",  //10
	"  x   x   x   x   x   x   ",
	"  x   x   x   x   x   x   ",
    "  x   x   x               ",
    "  x   x   x       xx      ",
    "  x   x   xxxxxx   x xxxxx",
  	"  x   x            x      ",
	"  x   x                   ",
	"  x   x           xx      ",
	"  x   xxxxxxxxxx   x xxxxx",
	"  x                x      ",	//20
	"  x                       ",
	"  x              xxx      ",
	"  x              x x      ",
	"  xxxxxxxxxxxxxx x x xxxxx",
    "                 xxx      "   //25
};

char *bitmap_button_ext_desktop_watches[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
	"  x   x   x   x   x   x   ",	//0
	"  x   x   x   x   x   x   ",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxwwwwxxwwxxxwwxxx",
	"xwxxxxxxxxxxxwxwxxwxwxxwxx",
	"xwwxxxxxxxxxwxxxwwxxwxxwxx",
	"xwxxxxxxxxxwxxxwxxwxwxxwxx",
	"xxxxxxxxxxwwwwxxwwxxxwwxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",  //10
	"  x   x   x   x   x   x   ",
	"  x   x   x   x   x   x   ",
    "                          ",
    "         xxxx             ",
    "        x    x            ",
    "       x      x           ",
  	"       x      x           ",
	"       x      x           ",
	"       x      x           ",
	"        x    xxx          ",
	"         xxxx  xx         ",
	"                xx        ",
	"                 xx       ",
	"                  xx      ",
    "                   x      "   //25
};

char *bitmap_button_ext_desktop_visualmem[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	" WW WW WW WW WW WW WW WW  ",
	" WW WW WW WW WW WW WW WW  ",
	"                          ",
	" WW WW XX XX XX WW WW WW  ",
	" WW WW XX XX XX WW WW WW  ",
	"                          ",
	" WW WW WW WW WW WW WW WW  ",
	" WW WW WW WW WW WW WW WW  ",
	"                          ",
	" WW WW WW WW WW WW WW WW  ",	//10
	" WW WW WW WW WW WW WW WW  ",
	"                          ",
	" WW WW WW WW WW WW WW WW  ",
    " WW WW WW WW WW WW WW WW  ",
	"                          ",
	" WW WW WW WW WW XX WW WW  ",
	" WW WW WW WW WW XX WW WW  ",
	"                          ",
	" WW WW WW WW WW WW WW WW  ",
	" WW WW WW WW WW WW WW WW  ",    //20
	"                          ",
	" WW WW WW WW WW WW WW WW  ",
	" WW WW WW WW WW WW WW WW  ",
	"                          ",
	"                          " 	 //25
};

char *bitmap_button_ext_desktop_zxlife[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	" WW WW RR RR WW WW WW WW  ",
	" WW WW RR RR WW WW WW WW  ",
	"                          ",
	" WW RR WW WW RR WW WW WW  ",
	" WW RR WW WW RR WW WW WW  ",
	"                          ",
	" WW WW RR RR WW WW WW WW  ",
	" WW WW RR RR WW WW WW WW  ",
	"                          ",
	" WW WW WW WW WW WW WW WW  ",	//10
	" WW WW WW WW WW WW WW WW  ",
	"                          ",
	" WW WW WW RR RR RR RR WW  ",
    " WW WW WW RR RR RR RR WW  ",
	"                          ",
	" WW WW RR WW WW WW RR WW  ",
	" WW WW RR WW WW WW RR WW  ",
	"                          ",
	" WW WW WW WW WW WW RR WW  ",
	" WW WW WW WW WW WW RR WW  ",    //20
	"                          ",
	" WW WW RR WW WW RR WW WW  ",
	" WW WW RR WW WW RR WW WW  ",
	"                          ",
	"                          " 	 //25
};

char *bitmap_button_ext_desktop_view_sensors[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"                          ",
	"         gggggggg         ",
	"      gggggggggggggg      ",
	"    ggg            ggg    ",
    "   ggg              ggg   ",
	"  gg                  gg  ",
	"  gg x                gg  ",
	" gg   x                gg ",	//10
	" gg    x               gg ",
	" gg     x              gg ",
	"gg       x              gg",
	"gg        x             gg",
	"gg         x            gg",
	"gg          x           gg",
	"                          ",
	"                          ",
	"                          ",
	"                          ",	//20
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          " 	 //25
};


char *bitmap_button_ext_desktop_visualfloppy[EXT_DESKTOP_BUTTONS_ANCHO]={
   //01234567890123456789012345
    "          xxxxxx          ",//0
  	"        xxxxxxxxxx        ",
	"       xxxxxxxxxxxx       ",
	"      xxxxxxrxxxxxxx      ",
	"     xxxxxxxrxxxxxxxx     ",
	"    xxxxxxxxrxxxxxxxxx    ",
	"   xxxxxxxxxrxxxxxxxxxx   ",
	"  xxxxcxxxxxrxxxxxxxxxxx  ",
	"  xxxxxcxxxxrxxxxxxgxxxx  ",
	"  xxxxxxcxxxrxxxxxgxxxxx  ",
	" xxxxxxxxcxxrxxxxgxxxxxxx ",	//10
	" xxxxxxxxxcxxxxxgxxxxxxxx ",
	" xxxxxxxxxxxwwxxxxxxxxxxx ",
	" xxxggggggxwxxwxbbbbbbxxx ",
	" xxxxxxxxxxwxxwxxxxxxxxxx ",
	" xxxxxxxxxxxwwxxxxxxxxxxx ",
	" xxxxxxxxxwxxxxyxxxxxxxxx ",
	"  xxxxxxxwxxmxxxyxxxxxxx  ",
	"  xxxxxxwxxxmxxxxyxxxxxx  ",
	"   xxxxwxxxxmxxxxxyxxxx   ",
	"    xxxxxxxxmxxxxxxxxx    ",//20
	"     xxxxxxxmxxxxxxxx     ",
	"      xxxxxxmxxxxxxx      ",
	"       xxxxxxxxxxxx       ",
	"        xxxxxxxxxx        ",
	"          xxxxxx          " 	//25
};


char *bitmap_button_ext_desktop_debugcpu_view_adventure[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
	"                          ",	//0
	"    bbb  bbb  b b  bbb    ",
	"     b   b    b b   b     ",
	"     b   bb    b    b     ",
	"     b   b    b b   b     ",
	"     b   bbb  b b   b     ",
	"                          ",
	"  b   bb   b b  bbb  b  b ",
	" b b  b b  b b  b    bb b ",
	" bbb  b b  b b  bb   b bb ",
	" b b  b b  b b  b    b  b ",  //10
	" b b  bb    b   bbb  b  b ",
	"                          ",
    "                          ",
    "                 R        ",
    "                R         ",
  	"          RRRRRR          ",
	"         RRRRRRRR         ",
	"         RRRRRRRR         ",
	"          RRRRRRR         ",
	"         RR      R        ",	//20
	"        RRRR  R   R       ",
	"        RRRRRRR   R       ",
	"         RR               ",
	"        R  R              ",
    "       R    R             "   //25
};


char *old_bitmap_button_ext_desktop_text_adventure_map[EXT_DESKTOP_BUTTONS_ANCHO]={
   //01234567890123456789012345
    "  xxxxxxxxxxxxxxxxxxxxxx  ",     //0
  	" x                  x   x ",
	"x                  x     x",
	"x                  x     x",
    "x                  x     x",
	" x                  x    x",
	"  xxxxxxxxxxxxxxxxxxxx   x",
	"  x                      x",
	"  x               x      x",
	"  x   x   x   x  xxx     x",
	"  x  xxx        xxxxx    x",	//10
	"  x xxxxx                x",
	"  x                      x",
	"  x     x    x   x       x",
	"  x           x x        x",
	"  x       x    x         x",
	"  x           x x        x",
	"  x          x   x       x",
	"  x                      x",
    "  xxxxxxxxxxxxxxxxxxxx   x",
  	" x                  x    x",     //20
	"x                  x     x",
	"x                  x     x",
    "x                  x     x",
	" x                  x   x ",
	"  xxxxxxxxxxxxxxxxxxxxxx  "     //25
};

char *bitmap_button_ext_desktop_text_adventure_map[EXT_DESKTOP_BUTTONS_ANCHO]={
   //01234567890123456789012345
    "  xxxxxxxxxxxxxxxxxxxxxx  ",     //0
  	" xyyyyyyyyyyyyyyyyyyxyyyx ",
	"xyyyyyyyyyyyyyyyyyyxyyyyyx",
	"xyyyyyyyyyyyyyyyyyyxyyyyyx",
    "xyyyyyyyyyyyyyyyyyyxyyyyyx",
	" xyyyyyyyyyyyyyyyyyyxyyyyx",
	"  xxxxxxxxxxxxxxxxxxxxyyyx",
	"  xyyyyyyyyyyyyyyyyyyyyyyx",
	"  xyyyyyyyyyyyyyyyyyyyyyyx",
	"  xyyyyyyyyyyyyyyyyyyyxyyx",
	"  xyyyyyyyyyyyyyyyyyyyxyyx",	//10
	"  xyyyyyyyyyyyyyyyyyyyyyyx",
	"  xyyyyyyyyryyyryyyyxyyyyx",
	"  xyyyyyyyyyryryyyyxyyyyyx",
	"  xyyyxxyxxyyryyxxyyyyyyyx",
	"  xyyyyyyyyyryryyyyyyyyyyx",
	"  xyyxyyyyyryyyryyyyyyyyyx",
	"  xyyxyyyyyyyyyyyyyyyyyyyx",
	"  xyyyyyyyyyyyyyyyyyyyyyyx",
    "  xxxxxxxxxxxxxxxxxxxxyyyx",
  	" xyyyyyyyyyyyyyyyyyyxyyyyx",  //20
	"xyyyyyyyyyyyyyyyyyyxyyyyyx",
	"xyyyyyyyyyyyyyyyyyyxyyyyyx",
    "xyyyyyyyyyyyyyyyyyyxyyyyyx",
	" xyyyyyyyyyyyyyyyyyyxyyyx ",
	"  xxxxxxxxxxxxxxxxxxxxxx  "  //25
};

char *bitmap_button_ext_desktop_hexeditor[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "             r            ",     //0
  	"            rrr           ",
	"           YYrrr          ",
	"          YYxYrrr         ",
	"         YYxYYYr          ",
	"        YYxYYYY           ",
	"       YYxYYYY            ",
	"      YYxYYYY             ",
	"     xYxYYYY              ",
	"     xxYYYY               ",
	"     xxxYY                ",	//10
	"     xxxx                 ",
    "                          ",
	" xxxxxxxxxxxxxxxxxxxxxxx  ",
	" xcccccccccccccccccccccx  ",
	" xccxxcxxxcxxxccxxcxxxcx  ",
	" xcxcxcxcxcxcxcxcxcxcxcx  ",
	" xcccxcxcxcxcxcccxcxcxcx  ",
	" xcccxcxxxcxxxcccxcxxxcx  ",
	" xcccccccccccccccccccccx  ",
	" xcxxxcxxxcxxxccxxccxxcx  ",	//20
	" xcxcxcxcxcxcxcxcxcxcxcx  ",
	" xcxcxcxcxcxcxcccxcccxcx  ",
	" xcxxxcxxxcxxxcccxcccxcx  ",
	" xcccccccccccccccccccccx  ",
	" xxxxxxxxxxxxxxxxxxxxxxx  "	//25
};


char *bitmap_button_ext_desktop_poke[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "             r            ",     //0
    "            rrr           ",
	"           YYrrr          ",
	"          YYxYrrr         ",
	"         YYxYYYr          ",
	"        YYxYYYY           ",
	"       YYxYYYY            ",
	"      YYxYYYY             ",
	"     xYxYYYY              ",
	"     xxYYYY               ",
	"     xxxYY                ",	//10
	"     xxxx                 ",
    "                          ",
	"xxxxxxxxxxxxxxxxxxxxxxxxx ",
	"xcRRRcRcccRcRRRcRRcccRccx ",
	"xcRccccRcRcccRccRcRcRcRcx ",
	"xcRRRcccRccccRccRRccRRRcx ",
	"xcRccccRcRcccRccRRccRcRcx ",
	"xcRRRcRcccRccRccRcRcRcRcx ",
	"xcccccccccccccccccccccccx ",
	"xcccRcccRcRcRcRRRcRRRcccx ",	//20
	"xcccRcccRcRcRcRcccRcccccx ",
	"xcccRcccRcRcRcRRRcRRRcccx ",
	"xcccRcccRcRcRcRcccccRcccx ",
	"xcccRRRcRccRccRRRcRRRcccx ",
	"xxxxxxxxxxxxxxxxxxxxxxxxx "    //25
};


char *bitmap_button_ext_desktop_viewsprites[EXT_DESKTOP_BUTTONS_ANCHO]={
   //01234567890123456789012345
    "                          ",	//0
    "      xxxxxxxxxxxxxxx     ",
  	"      xxxxxWWWWWxxxxx     ",
	"      xxxxWWxxWWWxxxx     ",
	"      xxxWxWWWxWxWWWx     ",
	"      xxxWxWWWxxWWxxx     ",
	"      xxxWxWxxWWxxxxx     ",
	"      xxxWxxWWxxWxxxx     ",
	"      xxxWWWxxWWxWWxx     ",
	"      xxWWxxWWWWWWWxx     ",
	"      xxxxxWWWWWWxxxx     ",	//10
	"      xxWxxxxxxxxxxxx     ",
	"      xWWWxWxxxWxxxxx     ",
	"      xxWxxWxxWWWxxxx     ",
	"      xxWxWWxxxWWxxxx     ",
	"      xxWxWWWxxxxxxxx     ",
	"      xxWxxxxxWxxxxxx     ",
	"      xxWxWWWWWWWxxxx     ",
	"      xxWxxWWWxWxxxxx     ",
	"      xxxxWWWxWWWxxxx     ",
	"      xxxWxxxxxxxxWxx     ",	//20
	"      xxxWWWxxxWWWWxx     ",
	"      xxxxWWWxxWWWxxx     ",
	"      xxxxxxxxxxxxxxx     ",
	"                          ",
	"                          " 	 //25
};


char *bitmap_button_ext_desktop_fileutils[EXT_DESKTOP_BUTTONS_ANCHO]={
   //01234567890123456789012345
    " xxxxxxxxxxxxxxxxxxx      ",   //0
	" xwwwwwwwwwwwwwwwwwxx     ",
	" xwwwwwwwwwwwwwwwwwxwx    ",
	" xwwwwwwwwwwwwwwwwwxwwx   ",
	" xwwwwwwbbwwwwwwwwwxwwwx  ",
	" xwwwwwbbbbbwwwwwwwxxxxxx ",
	" xwwwwwbbbbbbwwwwwwwwwwwx ",
	" xwwwwwwbbbbbbwwwwwwwwwwx ",
	" xwwbbwwwbbbbbbwwwwwwwwwx ",
	" xwbbbbwwwbbbbbwwwwwwwwwx ",
	" xwbbbbbwwwbbbwwwwwwwwwwx ",
	" xwbbbbbbwbbbbwwwwwwwwwwx ",
	" xwwbbbbbbbbbbbwwwwwwwwwx ",
	" xwwwbbbbbbbbbbbwwwwwwwwx ",
	" xwwwwwbbbbbbbbbbwwwwwwwx ",
	" xwwwwwwwwwbbbbbbbwwwwwwx ",
	" xwwwwwwwwwwbbbbbbbwwwwwx ",
	" xwwwwwwwwwwwbbbbbbbwwwwx ",
	" xwwwwwwwwwwwwbbbbbbbwwwx ",
	" xwwwwwwwwwwwwwbbb  bbwwx ",
	" xwwwwwwwwwwwwwwbb  bbwwx ",
	" xwwwwwwwwwwwwwwwbbbbbwwx ",
    " xwwwwwwwwwwwwwwwwbbbwwwx ",
    " xwwwwwwwwwwwwwwwwwwwwwwx ",
	" xwwwwwwwwwwwwwwwwwwwwwwx ",
	" xxxxxxxxxxxxxxxxxxxxxxxx "
};


char *bitmap_button_ext_desktop_pauseunpausetape[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
  	"         bbb   bbb        ",   //0
	"         bbb   bbb        ",
	"         bbb   bbb        ",
	"         bbb   bbb        ",
	"         bbb   bbb        ",
	"         bbb   bbb        ",
	"         bbb   bbb        ",
	"         bbb   bbb        ",
	"                          ",
	" wwwwwwwwwwwwwwwwwwwwwwww ",
	"wwxxxxxxxxxxxxxxxxxxxxxxww",	//10
	"wxxggggggggggggggggggggxxw",
	"wxggggggggggggggggggggggxw",
	"wxggggBggggggggggggBggggxw",
	"wxggggBggg      gggBggggxw",
	"wxggBBBBBg      gBBBBBggxw",
	"wxggggBggg      gggBggggxw",
	"wxggggBggggggggggggBggggxw",
	"wxggggggggggggggggggggggxw",
	"wxggggggggggggggggggggggxw",
	"wxggggggggggggggggggggggxw",	//20
	"wxgggggxxxxxxxxxxxxgggggxw",
	"wxggggxggggggggggggxggggxw",
	"wxxgggxggggggggggggxgggxxw",
	"wwxxxxxxxxxxxxxxxxxxxxxxww",
	" wwwwwwwwwwwwwwwwwwwwwwww "	//25


};


char *bitmap_button_ext_desktop_reinserttape[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
  	"           RRRRRRR        ",    //0
	"          R       R       ",
	"         R         R      ",
	"        R          R      ",
	"        R           R     ",
	"       R            R     ",
	"     RRRRR         R      ",
	"      RRR          R      ",
	"       R                  ",
	" wwwwwwwwwwwwwwwwwwwwwwww ",
	"wwxxxxxxxxxxxxxxxxxxxxxxww",	//10
	"wxxggggggggggggggggggggxxw",
	"wxggggggggggggggggggggggxw",
	"wxgggxxxggggggggggxxxgggxw",
	"wxggxx xxg      gxx xxggxw",
	"wxggx   xg      gx   xggxw",
	"wxggxx xxg      gxx xxggxw",
	"wxgggxxxggggggggggxxxgggxw",
	"wxggggggggggggggggggggggxw",
	"wxggggggggggggggggggggggxw",
	"wxggggggggggggggggggggggxw",	//20
	"wxgggggxxxxxxxxxxxxgggggxw",
	"wxggggxggggggggggggxggggxw",
	"wxxgggxggggggggggggxgggxxw",
	"wwxxxxxxxxxxxxxxxxxxxxxxww",
	" wwwwwwwwwwwwwwwwwwwwwwww "	//25


};

char *bitmap_button_ext_desktop_reinsertrealtape[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
  	"           RRRRRRR        ",    //0
	"          R       R       ",
	"         R         R      ",
	"        R          R      ",
	"        R           R     ",
	"       R            R     ",
	"     RRRRR         R      ",
	"      RRR          R      ",
	"       R                  ",
	" wwwwwwwwwwwwwwwwwwwwwwww ",
	"wwxxxxxxxxxxxxxxxxxxxxxxww",	//10
	"wxxggggggggggggggggggggxxw",
	"wxggggggggggggggggggggggxw",
	"wxggggBggggggggggggBggggxw",
	"wxggggBggg      gggBggggxw",
	"wxggBBBBBg      gBBBBBggxw",
	"wxggggBggg      gggBggggxw",
	"wxggggBggggggggggggBggggxw",
	"wxggggggggggggggggggggggxw",
	"wxggggggggggggggggggggggxw",
	"wxggggggggggggggggggggggxw",	//20
	"wxgggggxxxxxxxxxxxxgggggxw",
	"wxggggxggggggggggggxggggxw",
	"wxxgggxggggggggggggxgggxxw",
	"wwxxxxxxxxxxxxxxxxxxxxxxww",
	" wwwwwwwwwwwwwwwwwwwwwwww "	//25


};


char *bitmap_button_ext_desktop_ocr[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"xxxx                  xxxx",
	"x                        x",
	"x                        x",
	"x                        x",
	"                          ",
	"                          ",
	"                          ",
	"    bbbb   bbbb bbbbb     ",
	"   b    b b     b    b    ",	//10
	"   b    b b     b    b    ",
	"   b    b b     bbbbb     ",
	"   b    b b     b    b    ",
	"    bbbb   bbbb b    b    ",
    "                          ",
	"                          ",
	"                          ",
	"x                        x",
	"x                        x",
	"x                        x",    //20
	"xxxx                  xxxx",
	"                          ",
	"                          ",
	"                          ",
	"                          " 	 //25
};




char *bitmap_button_ext_desktop_switchborder[EXT_DESKTOP_BUTTONS_ANCHO]={
   //01234567890123456789012345
    "           RRRRR          ",     //0
  	"         RRRRRRRRR        ",
	"        RR      RRR       ",
	"       RR      RR RR      ",
	"       RR     RR  RR      ",
	"      RR     RR    RR     ",
	"      RR    RR     RR     ",
	"      RR   RR      RR     ",
	"       RR RR      RR      ",
	"       RRRR       RR      ",
	"        RR       RR       ",	//10
	"         RRRRRRRRR        ",
	"           RRRRR          ",
	"                          ",
	"  bbbbbbbbbbbbbbbbbbbbbb  ",
	"  bbbbbbbbbbbbbbbbbbbbbb  ",
	"  bbbbbbbbbbbbbbbbbbbbbb  ",
	"  bbb                bbb  ",
	"  bbb                bbb  ",
	"  bbb                bbb  ",
	"  bbb                bbb  ",    //20
	"  bbb                bbb  ",
	"  bbb                bbb  ",
	"  bbbbbbbbbbbbbbbbbbbbbb  ",
	"  bbbbbbbbbbbbbbbbbbbbbb  ",
	"  bbbbbbbbbbbbbbbbbbbbbb  " 	 //25
};


char *bitmap_button_ext_desktop_topspeed[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"                   xxx    ",
	"                   xxx    ",
	"                   xxx    ",
	"             xxxxx      xx",
	"xxxxxxx     xx  xxxx   xx ",
	"           xx   xxxxxxxx  ",
	"                xxx       ",
	"xxxxxxx        xxxx       ",	//10
	"               xxx        ",
	"              xxxx        ",
	"xxxxxxx      xxxxx        ",
	"        xx  xxx xxx       ",
	"        xxxxxx    xx      ",
	"xxxxxxx            xx     ",
	"                    xx    ",
	"                    xx    ",
	"                     x    ",
	"                          ",    //20
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          " 	 //25
};






char *bitmap_button_ext_desktop_osdkeyboard[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "xxxxxxxxxxxxxxxxxxxxxxxxxx", //0
    "x  xx  xxx x  x    rryyggx",
    "x x  x x   x x    rryyggcx",
  	"x x  x xxx xx    rryyggccx",
	"x x  x   x x x  rryyggcc x",
	"x  xx  xxx x  xrryyggcc  x",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"x                        x",
	"x                        x",
	"x                        x",
	"x                        x",	//10
	"x                        x",
	"x xx xx xx xx xx xx xxx  x",
    "x xx xx xx xx xx xx xxx  x",
	"x                        x",
    "x  xx xx xx xx xx xx xx  x",
	"x  xx xx xx xx xx xx xx  x",
	"x                     x  x",
    "x   xx xx xx xx xx xx x  x",
	"x   xx xx xx xx xx xx x  x",
	"x                        x",  //20
    "x     xx xxxxxxxx xx     x",
	"x     xx xxxxxxxx xx     x",
	"x                        x",
    "x                        x",
    "xxxxxxxxxxxxxxxxxxxxxxxxxx"   //25
};


char *bitmap_button_ext_desktop_osdadvkeyboard[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "xxxxxxxxxxxxxxxxxxxxxxxxxx", //0
    "x  xx  xxx  xx     rryyggx",
    "x x  x x   x  x   rryyggcx",
  	"x x  x xxx xxxx  rryyggccx",
	"x x  x   x x  x rryyggcc x",
	"x  xx  xxx x  xrryyggcc  x",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"x                        x",
	"x                        x",
	"x                        x",
	"x                        x",	//10
	"x                        x",
	"x  xxx xxx xxx xxx xxx   x",
    "x  xxx xxx xxx xxx xxx   x",
	"x                        x",
	"x  xxx xxx xxx xxx xxx   x",
    "x  xxx xxx xxx xxx xxx   x",
	"x                        x",
	"x  xxx xxx xxx xxx xxx   x",
    "x  xxx xxx xxx xxx xxx   x",
	"x                        x", //20
	"x  xxx xxx xxx xxx xxx   x",
    "x  xxx xxx xxx xxx xxx   x",
	"x                        x",
    "x                        x",
    "xxxxxxxxxxxxxxxxxxxxxxxxxx"   //25
};


char *bitmap_button_ext_desktop_quickload[EXT_DESKTOP_BUTTONS_ANCHO]={
     //01234567890123456789012345
    "                     xx   ",     //0
  	"                    xxxx  ",
	"                   xxxxxx ",
	"                     xx   ",
	"         bbbbbbbb    xx   ",
	"        bbbbbbbbbb   xx   ",
	"       bbbbbbbbbbbb  xx   ",
	"      bbbbbbbbbbbbbb      ",
	"     bbbbbbbbbbbbbbbb     ",
	" bbbbbbbbbbbbbbbbbbbbbbbb ",
	"bbbbbbbbbbb    bbbbbbbbbbb",	//10
	"bbbbbbbbbb      bbbbbbbbbb",
	"bbbWWbbbb        bbbbbbbbb",
	"bbbWWbbb   xx     bbbbbbbb",
	"bbbbbbbb  x       bbbbbbbb",
	"bbbbbbb   x        bbbbbbb",
	"bbbbbbb            bbbbbbb",
	"bbbbbbb            bbbbbbb",
	"bbbbbbbb          bbbbbbbb",
	"bbbbbbbb          bbbbbbbb",
	"bbbbbbbbb        bbbbbbbbb",	//20
	"bbbbbbbbbb      bbbbbbbbbb",
	"bbbbbbbbbbb    bbbbbbbbbbb",
	"bbbbbbbbbbbbbbbbbbbbbbbbbb",
	"bbbbbbbbbbbbbbbbbbbbbbbbbb",
	"bbbbbbbbbbbbbbbbbbbbbbbbbb"	//25


};


char *bitmap_button_ext_desktop_quicksave[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                     xx   ",     //0
  	"                     xx   ",
	"                     xx   ",
	"                     xx   ",
	"         bbbbbbbb  xxxxxx ",
	"        bbbbbbbbbb  xxxx  ",
	"       bbbbbbbbbbbb  xx   ",
	"      bbbbbbbbbbbbbb      ",
	"     bbbbbbbbbbbbbbbb     ",
	" bbbbbbbbbbbbbbbbbbbbbbbb ",
	"bbbbbbbbbbb    bbbbbbbbbbb",	//10
	"bbbbbbbbbb      bbbbbbbbbb",
	"bbbWWbbbb        bbbbbbbbb",
	"bbbWWbbb   xx     bbbbbbbb",
	"bbbbbbbb  x       bbbbbbbb",
	"bbbbbbb   x        bbbbbbb",
	"bbbbbbb            bbbbbbb",
	"bbbbbbb            bbbbbbb",
	"bbbbbbbb          bbbbbbbb",
	"bbbbbbbb          bbbbbbbb",
	"bbbbbbbbb        bbbbbbbbb",	//20
	"bbbbbbbbbb      bbbbbbbbbb",
	"bbbbbbbbbbb    bbbbbbbbbbb",
	"bbbbbbbbbbbbbbbbbbbbbbbbbb",
	"bbbbbbbbbbbbbbbbbbbbbbbbbb",
	"bbbbbbbbbbbbbbbbbbbbbbbbbb"	//25


};


char *bitmap_button_ext_desktop_nmi[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
	"  x   x   x   x   x   x   ",	//0
	"  x   x   x   x   x   x   ",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxwwwwxxwwxxxwwxxx",
	"xwxxxxxxxxxxxwxwxxwxwxxwxx",
	"xwwxxxxxxxxxwxxxwwxxwxxwxx",
	"xwxxxxxxxxxwxxxwxxwxwxxwxx",
	"xxxxxxxxxxwwwwxxwwxxxwwxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",  //10
	"  x   x   x   x   x   x   ",
	"  x   x   x   x   x   x   ",
    "                  Y       ",
    "                  Y       ",
    "                 YY       ",
  	"                 YY       ",
	"                YYY       ",
	"                YYYYYYY   ",
	"               YYYYYYY    ",
	"x    x x   x x     YYY    ",	//20
	"xx   x xx xx x     YY     ",
	"x x  x x x x x     YY     ",
	"x  x x x   x x     Y      ",
	"x   xx x   x x     Y      ",
    "x    x x   x x            ",   //25
};



char *bitmap_button_ext_desktop_zxunoprismswitch[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "xxxxxxxxxxxxxxxxxxxxxxxxxx",   //0
    "ggggxgxxxgxgxxgxgxxxgxgggg",
	"xxgxxxgxgxxgxxgxggxxgxgxxg",
	"xgxxxxxgxxxgxxgxgxgxgxgxxg",
	"gxxxxxgxgxxgxxgxgxxggxgxxg",
	"ggggxgxxxgxggggxgxxxgxgggg",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxWxxxxxxxxxxxxx",
	"xxxxxxxxxxxWxWxxxxxxxxxxxx",
	"xxxxxxxxxxWxxxWxxxxxxxxxxx",	//10
	"xxxxxxxxxxWxxxWxxxxxxxxxxx",
	"xxxxxxxxxWxxxxxWxxxxxxxxxx",
	"xxxxxxxxWWWWWWWWWrrrrrrrrr",
	"xxxxxxxWWWWWWWWWWrrrrrrrrr",
	"xxxxxWWWWWWWWWWWWWyyyyyyyy",
	"xxxWWxWxxxWWWWWWWWWyyyyyyy",
	"xWWxxxWxxxxxWWWWWWWggggggg",
	"WxxxxWxxxxxxxxWWWWWWgggggg",
	"xxxxWxxxxxxxxxxxWWWWWccccc",
	"xxxxWxxxxxxxxxxxxxWWWccccc",   //20
	"xxxWxxxxxxxxxxxxxxxxxWxxxx",
	"xxWxxxxxxxxxxxxxxxxxxxWxxx",
	"xxWxxxxxxxxxxxxxxxxxxxWxxx",
	"xWxxxxxxxxxxxxxxxxxxxxxWxx",
	"WWWWWWWWWWWWWWWWWWWWWWWWWx"    //25
};



char *bitmap_button_ext_desktop_nothing[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"      x    x x x          ",
	"      xx   x x x          ",	//10
	"      x x  x x x          ",
	"      x  x x x x          ",
	"      x   xx x x          ",
    "      x    x x xxxxx      ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",    //20
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          " 	 //25
};






char *bitmap_button_ext_desktop_zengmessage[EXT_DESKTOP_BUTTONS_ANCHO]={
   //01234567890123456789012345
    "                          ",     //0
  	"  xxxxxxxxxxxxxxxxxxxxxx  ",
	" x                      x ",
	" x                      x ",
	" x  bbbbbbbbbbbbbbbbbb  x ",
	" x                      x ",
	" x                      x ",
	" x  bbbbbbbbbbbbb       x ",
	" x                      x ",
	" x                      x ",
	" x                      x ",	//10
	" x                      x ",
	"  xxxxxxxxxxxxx     xxxx  ",
	"              x    x      ",
	"              x   x       ",
	"              x  x        ",
	"              x x         ",
	"              xx          ",
	"              x           ",
	"                          ",
	"xxxxx xxxxx x    x  xxxxx ",    //20
	"    x x     xx   x x      ",
	"   x  xxxxx x x  x x      ",
	"  x   x     x  x x x  xxx ",
	" x    x     x   xx x    x ",
	"xxxxx xxxxx x    x  xxxx   " 	 //25
};



char *bitmap_button_ext_desktop_rewindtape[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
  	"                          ",   //0
	"           b     b        ",
	"          bb    bb        ",
	"         bbb   bbb        ",
	"        bbbb  bbbb        ",
	"         bbb   bbb        ",
	"          bb    bb        ",
	"           b     b        ",
	"                          ",
	" wwwwwwwwwwwwwwwwwwwwwwww ",
	"wwxxxxxxxxxxxxxxxxxxxxxxww",	//10
	"wxxggggggggggggggggggggxxw",
	"wxggggggggggggggggggggggxw",
	"wxggggBggggggggggggBggggxw",
	"wxggggBggg      gggBggggxw",
	"wxggBBBBBg      gBBBBBggxw",
	"wxggggBggg      gggBggggxw",
	"wxggggBggggggggggggBggggxw",
	"wxggggggggggggggggggggggxw",
	"wxggggggggggggggggggggggxw",
	"wxggggggggggggggggggggggxw",	//20
	"wxgggggxxxxxxxxxxxxgggggxw",
	"wxggggxggggggggggggxggggxw",
	"wxxgggxggggggggggggxgggxxw",
	"wwxxxxxxxxxxxxxxxxxxxxxxww",
	" wwwwwwwwwwwwwwwwwwwwwwww "	//25


};

char *bitmap_button_ext_desktop_ffwdtape[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
  	"                          ",   //0
	"           b     b        ",
	"           bb    bb       ",
	"           bbb   bbb      ",
	"           bbbb  bbbb     ",
	"           bbb   bbb      ",
	"           bb    bb       ",
	"           b     b        ",
	"                          ",
	" wwwwwwwwwwwwwwwwwwwwwwww ",
	"wwxxxxxxxxxxxxxxxxxxxxxxww",	//10
	"wxxggggggggggggggggggggxxw",
	"wxggggggggggggggggggggggxw",
	"wxggggBggggggggggggBggggxw",
	"wxggggBggg      gggBggggxw",
	"wxggBBBBBg      gBBBBBggxw",
	"wxggggBggg      gggBggggxw",
	"wxggggBggggggggggggBggggxw",
	"wxggggggggggggggggggggggxw",
	"wxggggggggggggggggggggggxw",
	"wxggggggggggggggggggggggxw",	//20
	"wxgggggxxxxxxxxxxxxgggggxw",
	"wxggggxggggggggggggxggggxw",
	"wxxgggxggggggggggggxgggxxw",
	"wwxxxxxxxxxxxxxxxxxxxxxxww",
	" wwwwwwwwwwwwwwwwwwwwwwww "	//25


};


char *bitmap_button_ext_desktop_joyleftright[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
    "                          ",
    "                          ",
  	"           rrrr           ",
	"          rrrrrr          ",
	"         rrrrrrrr         ",
	"         rrrrrrrr         ",
	"         rrrrrrrr         ",
	"         rrrrrrrr         ",
	"          rrrrrr          ",
	"           rrrr           ",  //10
	"    x       xx       x    ",
	"   xxxxx    xx    xxxxx   ",
	"  xxxxxx    xx    xxxxxx  ",
	"   xxxxx    xx    xxxxx   ",
	"    x       xx       x    ",
	"            xx            ",
	"      bbbbbbbbbbbbbb      ",
	"     bbbbbbbbbbbbbbbx     ",
	"    bbbbbbbbbbbbbbbbbb    ",
	"   bbbrrbbbbbbbbbbbbbbb   ",	//20
	"  bbbrrrrbbbbbbbbbbbbbbb  ",
	" bbbbrrrrbbbbbbbbbbbbbbbb ",
	" bbbbbrrbbbbbbbbbbbbbbbbb ",
	" bbbbbbbbbbbbbbbbbbbbbbbb ",
	" bbbbbbbbbbbbbbbbbbbbbbbb ",	//25
};


char *bitmap_button_ext_desktop_trash[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"          xxxxxxx         ",
	"          xwwwwwx         ",
	"          xwwwwwx         ",
	"  xxxxxxxxxxxxxxxxxxxxxxx ",
	"  xxxxxxxxxxxxxxxxxxxxxxx ",
	"    xwwwwwwwwwwwwwwwwwx   ",
	"    xwwwwwwwwwwwwwwwwwx   ",
	"    xwwwwwwwwwwwwwwwwwx   ",	//10
	"    xwwwwwwwwwwwwwwwwwx   ",
	"    xwwwwwwwwwwwwwwwwwx   ",
	"    xwwwwxwwwxwwwxwwwwx   ",
    "    xwwwwxwwwxwwwxwwwwx   ",
	"    xwwwwxwwwxwwwxwwwwx   ",
	"    xwwwwxwwwxwwwxwwwwx   ",
	"    xwwwwxwwwxwwwxwwwwx   ",
	"    xwwwwxwwwxwwwxwwwwx   ",
	"    xwwwwxwwwxwwwxwwwwx   ",
	"    xwwwwxwwwxwwwxwwwwx   ",    //20
	"    xwwwwxwwwxwwwxwwwwx   ",
	"    xwwwwwwwwwwwwwwwwwx   ",
	"    xwwwwwwwwwwwwwwwwwx   ",
	"    xwwwwwwwwwwwwwwwwwx   ",
	"    xxxxxxxxxxxxxxxxxxx   " 	 //25
};
/*
char *bitmap_button_ext_desktop_trash_open[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"          xxxxxxx         ",
	"          xwwwwwx         ",
	"          xwwwwwx         ",
	"  xxxxxxxxxxxxxxxxxxxxxxx ",
	"  xxxxxxxxxxxxxxxxxxxxxxx ",
*/

char *bitmap_button_ext_desktop_trash_open[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"   xx        xxxxx        ",
	"   xxxxxx   xwwwwwx       ",
	"     xxxxxxxwwwwwx        ",
	"         xxxxxxxx         ",
	"             xxxxxxx      ",
	"                 xxxxxxxx ",
	"                     xxxx ",
	"    xwwwwwwwwwwwwwwwwwx   ",
	"    xwwwwwwwwwwwwwwwwwx   ",
	"    xwwwwwwwwwwwwwwwwwx   ",	//10
	"    xwwwwwwwwwwwwwwwwwx   ",
	"    xwwwwwwwwwwwwwwwwwx   ",
	"    xwwwwxwwwxwwwxwwwwx   ",
    "    xwwwwxwwwxwwwxwwwwx   ",
	"    xwwwwxwwwxwwwxwwwwx   ",
	"    xwwwwxwwwxwwwxwwwwx   ",
	"    xwwwwxwwwxwwwxwwwwx   ",
	"    xwwwwxwwwxwwwxwwwwx   ",
	"    xwwwwxwwwxwwwxwwwwx   ",
	"    xwwwwxwwwxwwwxwwwwx   ",    //20
	"    xwwwwxwwwxwwwxwwwwx   ",
	"    xwwwwwwwwwwwwwwwwwx   ",
	"    xwwwwwwwwwwwwwwwwwx   ",
	"    xwwwwwwwwwwwwwwwwwx   ",
	"    xxxxxxxxxxxxxxxxxxx   " 	 //25
};

char *bitmap_button_ext_desktop_trash_not_empty[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"         x                ",
	"        xxx   xxxx        ",
	"       xxxxx xxxxx  x     ",
	"      xxxxxxxxxxxx xx     ",
	"  xxxxxxxxxxxxxxxxxxxxxxx ",
	"    xwwwwwwwwwwwwwwwwwx   ",
	"    xwwwwwwwwwwwwwwwwwx   ",
	"    xwwwwwwwwwwwwwwwwwx   ",	//10
	"    xwwwwwwwwwwwwwwwwwx   ",
	"    xwwwwwwwwwwwwwwwwwx   ",
	"    xwwwwxwwwxwwwxwwwwx   ",
    "    xwwwwxwwwxwwwxwwwwx   ",
	"    xwwwwxwwwxwwwxwwwwx   ",
	"    xwwwwxwwwxwwwxwwwwx   ",
	"    xwwwwxwwwxwwwxwwwwx   ",
	"    xwwwwxwwwxwwwxwwwwx   ",
	"    xwwwwxwwwxwwwxwwwwx   ",
	"    xwwwwxwwwxwwwxwwwwx   ",    //20
	"    xwwwwxwwwxwwwxwwwwx   ",
	"    xwwwwwwwwwwwwwwwwwx   ",
	"    xwwwwwwwwwwwwwwwwwx   ",
	"    xwwwwwwwwwwwwwwwwwx   ",
	"    xxxxxxxxxxxxxxxxxxx   " 	 //25
};

char *bitmap_button_ext_desktop_trash_open_not_empty[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "         x       xx       ",     //0
  	"        xx     xxxxx      ",
	"       xxxx   xxxxxx  x   ",
	"    x xxxxxx  xxxxxx xx   ",
	"    xxxxxxxxx xxxxx xxx   ",
	"    xxxxxxxx xxxxxxxxxxx  ",
	"   xxxxxxxxxxxxxxxxxxxxx  ",
	"  xxxxxxxxxxxxxxxxxxxxxxx ",
	"    xwwwwwwwwwwwwwwwwwx   ",
	"    xwwwwwwwwwwwwwwwwwx   ",
	"    xwwwwwwwwwwwwwwwwwx   ",	//10
	"    xwwwwwwwwwwwwwwwwwx   ",
	"    xwwwwwwwwwwwwwwwwwx   ",
	"    xwwwwxwwwxwwwxwwwwx   ",
    "    xwwwwxwwwxwwwxwwwwx   ",
	"    xwwwwxwwwxwwwxwwwwx   ",
	"    xwwwwxwwwxwwwxwwwwx   ",
	"    xwwwwxwwwxwwwxwwwwx   ",
	"    xwwwwxwwwxwwwxwwwwx   ",
	"    xwwwwxwwwxwwwxwwwwx   ",
	"    xwwwwxwwwxwwwxwwwwx   ",    //20
	"    xwwwwxwwwxwwwxwwwwx   ",
	"    xwwwwwwwwwwwwwwwwwx   ",
	"    xwwwwwwwwwwwwwwwwwx   ",
	"    xwwwwwwwwwwwwwwwwwx   ",
	"    xxxxxxxxxxxxxxxxxxx   " 	 //25
};

char *bitmap_button_ext_desktop_file_snapshot[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    " xxxxxxxxxxxxxxxxxxx      ",   //0
	" xwwwwwwwwwwwwwwwwwxx     ",
	" xwwwwwwwwwwwwwwwwwxwx    ",
	" xwwwwwwwwwwwwwwwwwxwwx   ",
	" xwwwwwwwwwwwwwwwwwxwwwx  ",
	" xwwwwwwwwwwwwwwwwwxxxxxx ",
	" xwwwwwwwbbbbbbbbwwwwwwwx ",
	" xwwwwwwbbbbbbbbbbwwwwwwx ",
	" xwwwwwbbbbbbbbbbbbwwwwwx ",
	" xwwbbbbbbbbbbbbbbbbbbwwx ",
	" xwbbbbbbbbbbbbbbbbbbbbwx ",	//10
	" xwbbWWbbbb    bbbbbbbbwx ",
	" xwbbWWbbb      bbbbbbbwx ",
	" xwbbbbbb  xx    bbbbbbwx ",
	" xwbbbbbb x      bbbbbbwx ",
	" xwbbbbb  x       bbbbbwx ",
	" xwbbbbb          bbbbbwx ",
	" xwbbbbb          bbbbbwx ",
	" xwbbbbbb        bbbbbbwx ",
	" xwbbbbbb        bbbbbbwx ",
	" xwwwwwwbb      bbbbbbbwx ",	//20
	" xwwxxxwbbb    bbbbbbbbwx ",
	" xwwwxxwbbbbbbbbbbbbbbbwx ",
	" xwwxwxwbbbbbbbbbbbbbbbwx ",
	" xwxwwwwwwwwwwwwwwwwwwwwx ",
	" xxxxxxxxxxxxxxxxxxxxxxxx "	//25

};


char *bitmap_button_ext_desktop_file_generic_smartload[EXT_DESKTOP_BUTTONS_ANCHO]={
   //01234567890123456789012345
    " xxxxxxxxxxxxxxxxxxx      ",  //0
  	" xwwwwwwwwwwwwwwwwwxx     ",
	" xwwwwwwwwxwwwwxwwwxwx    ",
	" xwwwwwwwwwxwwwxwwwxwwx   ",
	" xwwwwwwwwwwxwwxwwwxwwwx  ",
	" xwwwwwwwwwwwwwwwwwxxxxxx ",
	" xwwwwwwwwwwwwxxxxxwwwwwx ",
	" xwwwwwwwwwwwxyyyyyxwwwwx ",
	" xwwwwwwwwwwxyyyyxyyxwwwx ",
	" xwwwwwwwwwxyyyyxyyyyxwwx ",
	" xwwwwwwxxwxyyyxyyyyyxwwx ",   //10
	" xwwwwwwwwwxyyxxxxxyyxwwx ",
	" xwwwwwwwwwxyyyyyxyyyxwwx ",
	" xwwwwwwwwwwxyyyxyyyxwwwx ",
	" xwwwwwwwwwwxyyxyyyyxwwwx ",
	" xwwwwwwwwwwwxyyyyyxwwwwx ",
	" xwwxxxwwwwwwxxyyyxxwwwwx ",
	" xwxwwwxwwwwwwxxxxxwwwwwx ",
	" xwxwxwxwxxxxwwwwwwwwwwwx ",
	" xwxwwwxwwwxxwxxxxxwwwwwx ",
	" xwxwxwxwwxwxwxxxxxwwwwwx ",   //20
	" xwxwwwxwxwwxwwwwwwwwwwwx ",
	" xwxwwwxwwwwwwwxxxwwwwwwx ",
	" xwwxxxwwwwwwwwxxxwwwwwwx ",
	" xwwwwwwwwwwwwwwwwwwwwwwx ",
	" xxxxxxxxxxxxxxxxxxxxxxxx "    //25
};


char *bitmap_button_ext_desktop_file_tape[EXT_DESKTOP_BUTTONS_ANCHO]={
   //01234567890123456789012345
    " xxxxxxxxxxxxxxxxxxx      ",//0
	" xwwwwwwwwwwwwwwwwwxx     ",
	" xwwwwwwwwwwwwwwwwwxwx    ",
	" xwwwwwwwwwwwwwwwwwxwwx   ",
	" xwwwwwwwwwwwwwwwwwxwwwx  ",
    " xwwwwwwwwwwwwwwwwwxxxxxx ",
    " xwwwwwwwwwwwwwwwwwwwwwwx ",
    " xwwwwwwwwwwwwwwwwwwwwwwx ",
	" xwwxxxxxxxxxxxxxxxxxxwwx ",
	" xwxxggggggggggggggggxxwx ",
	" xwxggggggggggggggggggxwx ",
	" xwxggxxxggggggggxxxggxwx ",
	" xwxgxx xxg    gxx xxgxwx ",
	" xwxgx   xg    gx   xgxwx ",
	" xwxgxx xxg    gxx xxgxwx ",
	" xwxggxxxggggggggxxxggxwx ",
	" xwxggggggggggggggggggxwx ",
	" xwxggggggggggggggggggxwx ",
	" xwxggggxxxxxxxxxxggggxwx ",
	" xwxgggxggggggggggxgggxwx ",
	" xwwwwwwggggggggggxggxxwx ",
	" xwwxxxwxxxxxxxxxxxxxxwwx ",
	" xwwwxxwwwwwwwwwwwwwwwwwx ",	//20
	" xwwxwxwwwwwwwwwwwwwwwwwx ",
	" xwxwwwwwwwwwwwwwwwwwwwwx ",
	" xxxxxxxxxxxxxxxxxxxxxxxx "//25
};


//Icono "My machine" generico
char *bitmap_button_ext_desktop_my_machine_generic[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
    "                          ",
	"                          ",	//10
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxwwxwxwwxwwxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxwwxwwxwwxwwxwwxwwxwwwxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxwwxwwxwwxwwxwwxwwxwwxxx",
	"xxxxxxxxxxxxxxxxxxxxxxwxxr",    //20
	"xxxxwwxwwxwwxwwxwwxwwxwxry",
	"xxxxxxxxxxxxxxxxxxxxxxxryg",
	"xxxxxxwwxwwwwwwwwxwwxxrygb",
	"xxxxxxxxxxxxxxxxxxxxxrygbx",
	" xxxxxxxxxxxxxxxxxxxrygbx " 	 //25
};


char *bitmap_button_ext_desktop_helpkeyboard[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"  x  x  xxxx  x     xxxx  ",
	"  x  x  x     x     x   x ",
	"  xxxx  xx    x     xxxx  ",
	"  x  x  x     x     x     ",
    "  x  x  xxxx  xxxx  x     ",
    "                          ",
    "                          ",
	"                          ",	//10
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxwwxwxwwxwwxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxwwxwwxwwxwwxwwxwwxwwwxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxwwxwwxwwxwwxwwxwwxwwxxx",
	"xxxxxxxxxxxxxxxxxxxxxxwxxr",    //20
	"xxxxwwxwwxwwxwwxwwxwwxwxry",
	"xxxxxxxxxxxxxxxxxxxxxxxryg",
	"xxxxxxwwxwwwwwwwwxwwxxrygb",
	"xxxxxxxxxxxxxxxxxxxxxrygbx",
	" xxxxxxxxxxxxxxxxxxxrygbx " 	 //25
};

//Icono "My machine" para un gomas
char *bitmap_button_ext_desktop_my_machine_gomas[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
    "                          ",
	"                          ",	//10
	" xxxxxxxxxxxxxxxxxxxxxxxx ",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxwwxwxwwxwwxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxwwxwwxwwxwwxwwxwwxwwxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxwwxwwxwwxwwxwwxwwxwwxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxr",    //20
	"xxxxwwxwwxwwxwwxwwxwwxwwry",
	"xxxxxxxxxxxxxxxxxxxxxxxryg",
	"xxxwwxwwxwwxwwxwwxwwxwrygb",
	"xxxxxxxxxxxxxxxxxxxxxrygbx",
	" xxxxxxxxxxxxxxxxxxxrygbx " 	 //25
};

//Icono "My machine" para ZX81
char *bitmap_button_ext_desktop_my_machine_zx81[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
    "xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",	//10
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxrrxrxrxrrxrxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxWWxWWxWWxWWxWWxWWxWWxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxWWxWWxWWxWWxWWxWWxWWxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",    //20
	"xxxxWWxWWxWWxWWxWWxWWxWWxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxWWxWWxWWxWWxWWxWWxWWxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx" 	 //25
};

//Icono "My machine" para TS1000
char *bitmap_button_ext_desktop_my_machine_ts1000[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxccccxwwwwwwxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
    "xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",	//10
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxWWxWWxWWxWWxWWxWWxWWxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxWWxWWxWWxWWxWWxWWxWWxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",    //20
	"xxxxWWxWWxWWxWWxWWxWWxWWxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxWWxWWxWWxWWxWWxWWxWWxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx" 	 //25
};


//Icono "My machine" para un ts1500
char *bitmap_button_ext_desktop_my_machine_ts1500[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
    "                          ",
	"                          ",	//10
	" wwwwwwwwwwwwwwwwwwwwwwww ",
	"wwwwwwwwwwwwwwwwwwwwwwwwww",
	"wwwwwwwwwwwwwwwwwwwwwwwwww",
	"wwccwWWWWwwwwwwwwwwwwwwwww",
	"wwwwwwwwwwwwwwwwwwwwwwwwww",
	"wwwwwwwwwwwwwwwwwwwwwwwwww",
	"wwWWwWWwWWwWWwWWwWWwWWwwww",
	"wwwwwwwwwwwwwwwwwwwwwwwwww",
	"wwwWWwWWwWWwWWwWWwWWwWWwww",
	"wwwwwwwwwwwwwwwwwwwwwwwwww",    //20
	"wwwwWWwWWwWWwWWwWWwWWwWWww",
	"wwwwwwwwwwwwwwwwwwwwwwwwww",
	"wwwWWwWWwWWwWWwWWwWWwwwwww",
	"wwwwwwwwwwwwwwwwwwwwwwwwww",
	" wwwwwwwwwwwwwwwwwwwwwwww " 	 //25
};


//Icono "My machine" para TK82c
char *bitmap_button_ext_desktop_my_machine_tk82c[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xwwwwwwwwwwwwwwwwwwwwwwwwx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xwwwwwwwwwwxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
    "xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",	//10
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxWWxWWxWWxWWxWWxWWxWWxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxWWxWWxWWxWWxWWxWWxWWxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",    //20
	"xxxxWWxWWxWWxWWxWWxWWxWWxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxWWxWWxWWxWWxWWxWWxWWxxx",
	"xxxxxxxxxxxxxxxxxxxxrrrrxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx" 	 //25
};

//Icono "My machine" para TK83
char *bitmap_button_ext_desktop_my_machine_tk83[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"wwwwwwwwwwwwwwwwwwwwwwwwww",
	"wwwwwwwwwwwwwwwwwwwwwwwwww",
	"wwwxxxxxxxwwwwwwwwwwwwwwww",
	"wwwwwwwwwwwwwwwwwwwwwwwwww",
	"wwwwwwwwwwwwwwwwwwwwwwwwww",
	"wwwwwwwwwwwwwwwwwwwwwwwwww",
	"wwwwwwwwwwwwwwwwwwwwwwwwww",
    "wwwwwwwwwwwwwwwwwwwwwwwwww",
	"wwwwwwwwwwwwwwwwwwwwwwwwww",	//10
	"wwwwwwwwwwwwwwwwwwwwwwwwww",
	"wwwwwwwwwwwwwwwwwwwxxwxxww",
	"wwwwwwwwwwwwwwwwwwwwwwwwww",
	"wwwwwwwwwwwwwwwwwwwxxwxxww",
	"wwwwwwwwwwwwwwwwwwwwwwwwww",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxWWxWWxWWxWWxWWxWWxWWxxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxWWxWWxWWxWWxWWxWWxWWxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",    //20
	"wxxxWWxWWxWWxWWxWWxWWxWWxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wxxWWxWWxWWxWWxWWxWWxWWxxw",
	"wxxxxxxxxxxxxxxxxxxxxxxxxw",
	"wwwwwwwwwwwwwwwwwwwwwwwwww" 	 //25
};

//Icono "My machine" para un tk85
char *bitmap_button_ext_desktop_my_machine_tk85[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
    "                          ",
	"                          ",	//10
	" xxxxxxxxxxxxxxxxxxxxxxxx ",
    "xxwwxwxwwxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxyyyyyyyxxxxxxxxxxxxxxxxx",
    "xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxwwxwwxwwxwwxwwxwwxwwxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxwwxwwxwwxwwxwwxwwxwwxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",    //20
	"xxxxwwxwwxwwxwwxwwxwwxwwxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxyyxwwxwwxwwxwwxwwxwxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxwwx",
	" xxxxxxxxxxxxxxxxxxxxxxxx " 	 //25
};

//Icono "My machine" para ZX80
char *bitmap_button_ext_desktop_my_machine_zx80[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"WWWWWWWWWWWWWWWWWWWWWWWWWW",
	"WWWWWWWWWWxxxxxxxWWWWWWWWW",
	"WWWWWWWWWWWWWWWWWWWWWWWWWW",
	"WWWWWWWWWWxxxxxxxWWWWWWWWW",
	"WWWWWWWWWWWWWWWWWWWWWWWWWW",
	"WWWWWWWWWWxxxxxxxWWWWWWWWW",
	"WWWWWWWWWWWWWWWWWWWWWWWWWW",
    "WWWWWWWWWWWWWWWWWWWWWWWWWW",
	"WWWWWWWWWWWWWWWWWWWWWWWWWW",	//10
	"WWxxWxxWxxWxxWWWWWWWWWWWWW",
	"WWWWWWWWWWWWWWWWWWWWWWWWWW",
	"WWyyWyWyWyyWyyWWWWWWWWWWWW",
	"WWWWWWWWWWWWWWWWWWWWWWWWWW",
	"WWWWWWWWWWWWWWWWWWWWWWWWWW",
	"WxxxxxxxxxxxxxxxxxxxxxxxxW",
	"WxbbxbbxbbxbbxbbxbbxbbxxxW",
	"WxxxxxxxxxxxxxxxxxxxxxxxxW",
	"WxxbbxbbxbbxbbxbbxbbxbbxxW",
	"WxxxxxxxxxxxxxxxxxxxxxxxxW",    //20
	"WxxxbbxbbxbbxbbxbbxbbxbbxW",
	"WxxxxxxxxxxxxxxxxxxxxxxxxW",
	"WxxbbxbbxbbxbbxbbxbbxbbxxW",
	"WxxxxxxxxxxxxxxxxxxxxxxxxW",
	"WWWWWWWWWWWWWWWWWWWWWWWWWW" 	 //25
};

//Icono "My machine" para TK80
char *bitmap_button_ext_desktop_my_machine_tk80[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxwxxxwxxx",
	"xxxxxxxxxxxxxxxxxxwxxxwxxx",
	"xxxxxxxxxxxxxxxxxxwxxxwxxx",
	"xxxxxxxxxxxxxxxxxxwxxxwxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
    "xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",	//10
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxWWxWWxWWxWWxWWxWWxWWxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxWWxWWxWWxWWxWWxWWxWWxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",    //20
	"xxxxWWxWWxWWxWWxWWxWWxWWxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxWWxWWxWWxWWxWWxWWxWWxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx" 	 //25
};


//Icono "My machine" para TK82
char *bitmap_button_ext_desktop_my_machine_tk82[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
    "xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",	//10
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxWWxWWxWWxWWxWWxWWxWWxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxWWxWWxWWxWWxWWxWWxWWxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",    //20
	"xxxxWWxWWxWWxWWxWWxWWxWWxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxWWxWWxWWxWWxWWxWWxWWxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx" 	 //25
};

//Icono "My machine" para Jupiter ACE
char *bitmap_button_ext_desktop_my_machine_ace[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"WWWWWWWWWWWWWWWWrrrWWWWWWW",
	"WWxWxxWxxWxWxWWWWWWWWWWWWW",
	"WWxWxxWxxWxWxWWrrrrrrrrrrW",
	"WxxWxxWxxWxWxWWWWWWWWWWWWW",
	"WWWWWWWWWWWWWWrrrWWWWWWWWW",
	"WWWWWWWxxxxWWWWWWWWWWWWWWW",
    "WWWWWWWxxxxWWrrrWWWWWWWWWW",
	"WWWWWWWWWWWWWWWWWWWWWWWWWW",	//10
	"WWWWWWWWWWWWWWWWWWWWWWWWWW",
	"WWWWWWWWWWWWWWWWWWWWWWWWWW",
	"WWWWWWWWWWWrrrWWWWWWWWWWWW",
	"WWWWWWWWWWWWWWWWWWWWWWWWWW",
	"WWWWWWWWWWrrrWWWWWWWWWWWWW",
	"WWWWWWWWWWWWWWWWWWWWWWWWWW",
	"WWxxWxxWxxWxxWxxWxxWxxWWWW",
	"WWWWWWWWWWWWWWWWWWWWWWWWWW",
	"WWWxxWxxWxxWxxWxxWxxWxxWWW",
	"WWWWWWWWWWWWWWWWWWWWWWWWWW",    //20
	"WWWWxxWxxWxxWxxWxxWxxWxxWW",
	"WWWWWWWWWWWWWWWWWWWWWWWWWW",
	"WWWxxWxxWxxWxxWxxWxxWxxxWW",
	"WWWWWWWWWWWWWWWWWWWWWWWWWW",
	"WWWWWWWWWWWWWWWWWWWWWWWWWW" 	 //25
};


//Icono "My machine" para un QL
char *bitmap_button_ext_desktop_my_machine_ql[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
    "                          ",
	"                          ",	//10
	"                          ",
	"                          ",
	"                          ",
	"xxxxxxxxxxxxxxxxxxxxxxxxx ",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xwwxxwxwwxwwxwwxwwxwxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xwwxxwwwxwwxwwxwwxwxxxxxxx",
	"xxxxxxxxxxxxxxxxxxwxxxxxxx",    //20
	"xwwxxwwxwwxwwxwwxwwxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xwwxxwwxwwwwwxwwxwwxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxWWxWWx",
	"xxxxxxxxxxxxxxxxxxxxxxxxx " 	 //25
};


//Icono "My machine" para un Spectrum 128k+ Spanish
char *bitmap_button_ext_desktop_my_machine_spectrum_128_spa[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
    "                          ",
	"                          ",	//10
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxx  ",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxrxrrxxxxxxxxxxxxxxxxxx  ",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxx  ",
	"xxwwxwwxwwxwwxwwxwwwxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxx  ",
	"xxwxwwxwwxwwxwwxwwxwxxxxxx",
	"xxxxxxxxxxxxxxxxxxxwxxxx  ",    //20
	"xxwwxwwxwwxwwxwwxwwwxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxx  ",
	"xxwxwwxwwwwwwwxwwxwwrygbxx",
	"xxxxxxxxxxxxxxxxxxxrygbx  ",
	"xxxxxxxxxxxxxxxxxxrygbxxxx" 	 //25
};

//Icono "My machine" para un Spectrum 48k+ Spanish
char *bitmap_button_ext_desktop_my_machine_spectrum_48_spa[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
    "                          ",
	"                          ",	//10
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxrxrrxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxwwxwwxwwxwwxwwxwwxwwwxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxwxwwxwwxwwxwwxwwxwwxwxxx",
	"xxxxxxxxxxxxxxxxxxxxxxwxxx",    //20
	"xxwwwxwwxwwxwwxwwxwwxwwxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxwxwwxwwxwwwwwwxwwxwwrygb",
	"xxxxxxxxxxxxxxxxxxxxxrygbx",
	"xxxxxxxxxxxxxxxxxxxxrygbxx" 	 //25
};


//Icono "My machine" para un Inves
char *bitmap_button_ext_desktop_my_machine_inves[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
    "                          ",
	"                          ",	//10
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxrxrrxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxwwxwwxwwxwwxwwxwwxwwwxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxwxwwxwwxwwxwwxwwxwwxwxxx",
	"xxxxxxxxxxxxxxxxxxxxxxwxxx",    //20
	"xxwwwxwwxwwxwwxwwxwwxwwxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxwxwwxwwxwwwwwwwxwwxwwxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx" 	 //25
};

//Icono "My machine" para un Spectrum 128k+ English
char *bitmap_button_ext_desktop_my_machine_spectrum_128_eng[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
    "                          ",
	"                          ",	//10
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxrrxrxrrxxxxxxxxxxxxxxx  ",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxrxrrxxxxxxxxxxxxxxxxxx  ",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxx  ",
	"xxwwxwwxwwxwwxwwxwwwxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxx  ",
	"xxwxwwxwwxwwxwwxwwxwxxxxxx",
	"xxxxxxxxxxxxxxxxxxxwxxxx  ",    //20
	"xxwwxwwxwwxwwxwwxwwwxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxx  ",
	"xxwxwwxwwwwwwwxwwxwwrygbxx",
	"xxxxxxxxxxxxxxxxxxxrygbx  ",
	"xxxxxxxxxxxxxxxxxxrygbxxxx" 	 //25
};



//Icono "My machine" para un Spectrum +2
char *bitmap_button_ext_desktop_my_machine_spectrum_p2[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
    "                          ",
	"                          ",	//10
	"wwwwwwwwwwwwwwwwwwwwwwwwww",
	"wwrrwrwrrwwwwwwrrwwWWwWWww",
	"wwwwwwwwwwwwwwwwwwwwwwwwww",
	"wwwwwwwwwwwwwwwwwwwwwwwwww",
	"wwwwwwwwwwwwwwwwwwwwwwwwww",
	"wwwwwwwwwwwwwwwwwwwwwwwwww",
	"wwWWwWWwWWwWWwWWWwwwWwWwww",
	"wwwwwwwwwwwwwwwwwwwwwwwwww",
	"wwWwWWwWWwWWwWWwWwwrrrrrrw",
	"wwwwwwwwwwwwwwwwWwwyyyyyyw",    //20
	"wwWWwWWwWWwWWwWWWwwggggggw",
	"wwwwwwwwwwwwwwwwwwwbbbbbbw",
	"wwWwWWwWWWWwWWwWWwwwwwwwww",
	"wwwwwwwwwwwwwwwwwwwrwWwWww",
	"wwwwwwwwwwwwwwwwwwwwwwwwww" 	 //25
};

//Icono "My machine" para un Spectrum +2A
char *bitmap_button_ext_desktop_my_machine_spectrum_p2a[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
    "                          ",
	"                          ",	//10
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxrrxrxrrxxxxxxrrxxWWxWWxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxwwxwwxwwxwwxwwwxxxwxwxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxwxwwxwwxwwxwwxwxxrrrrrrx",
	"xxxxxxxxxxxxxxxxwxxyyyyyyx",    //20
	"xxwwxwwxwwxwwxwwwxxggggggx",
	"xxxxxxxxxxxxxxxxxxxbbbbbbx",
	"xxwxwwxwwwwxwwxwwxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxrxwxwxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx" 	 //25
};

//Icono "My machine" para un Spectrum +3
char *bitmap_button_ext_desktop_my_machine_spectrum_p3[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
    "                          ",
	"                          ",	//10
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxrrxrxrrxxxxxxrrxxWWxWWxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxwwxwwxwwxwwxwwwxxxrxrxrx",
	"xxxxxxxxxxxxxxxxxxxxrxrxrx",
	"xxwxwwxwwxwwxwwxwxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxwxxxxxxxxx",    //20
	"xxwwxwwxwwxwwxwwwxxrrrrrrx",
	"xxxxxxxxxxxxxxxxxxxyyyyyyx",
	"xxwxwwxwwwwxwwxwwxxggggggx",
	"xxxxxxxxxxxxxxxxxxxbbbbbbx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx" 	 //25
};



//Icono "My machine" para un CPC 464
char *bitmap_button_ext_desktop_my_machine_cpc_464[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
    "                          ",
	"                          ",	//10
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxwwxwxwxxxxxxrgbxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxrxwwxwwxwwxwwxgxxxwxwxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxggxwwxwwxwwxwxbxxxxxxxxx",
	"xxxxxxxxxxxxxxxxbxxxxxxxxx",    //20
	"xxgggxwwxwwxwwxbbxxrrrrrrx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxgxwwxwwwwxwwxggxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxrxwxwxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx" 	 //25
};


//Icono "My machine" para un CPC 6128
char *bitmap_button_ext_desktop_my_machine_cpc_6128[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
    "                          ",
	"                          ",	//10
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxwwxwxwxxxxxxrgbxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxwxwwxwwxwwxwwxwxxrrrrrrx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxwwxwwxwwxwwxwxwxxxxxxxxx",
	"xxxxxxxxxxxxxxwxxxxggggggx",    //20
	"xxwwxwwxwwxwwxwxwxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxwxwwxwwwwxwwxwwxxbbbbbbx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx" 	 //25
};

//Icono "My machine" para un CPC 664
char *bitmap_button_ext_desktop_my_machine_cpc_664[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
    "                          ",
	"                          ",	//10
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxwwxwxwxxxxxxrgbxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxcxwwxwwxwwxwwxgxxxwxwxwx",
	"xxxxxxxxxxxxxxxxxxxxwxwxwx",
	"xxccxwwxwwxwwxwxcxxxxxxxxx",
	"xxxxxxxxxxxxxxxxcxxxwxwxwx",    //20
	"xxcccxwwxwwxwwxccxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxwxwxx",
	"xxcxwwxwwwwxwwxccxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxccxxcx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx" 	 //25
};


//Icono "My machine" PCW 8256, 8512
char *bitmap_button_ext_desktop_my_machine_pcw_8256[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
    "                          ",
	"                          ",	//10
	"wwwwwwwwwwwwwwwwwwwwwwwwww",
	"wwwwwwwwwwwwwwwwwwwwwwwwww",
	"wwWWrrWWWWWwwwwwwwwwwwwwww",
	"wwwwwwwwwwwwwwwwwwwwwwwwww",
	"wwwwwwwwwwwwwwwwwwwwwwwwww",
	"wwwwwwwwwwwwwwwwwwwwwwwwww",
	"wwWWwWWwWWwWWwWWWwWWwWWwWw",
	"wwwwwwwwwwwwwwwwwwwwwwwwww",
	"wwWWWwWWwWWwWWwWWwWWwWWwWw",
	"wwwwwwwwwwwwwwwwWwwwwwwwww",    //20
	"wwWwWWwWWwWWwWWwWwWWwWWwWw",
	"wwwwwwwwwwwwwwwwwwwwwwwwww",
	"wwWWwWWWWWWWWWWwWwWWwWWwWw",
	"wwwwwwwwwwwwwwwwwwwwwwwwww",
	"wwwwwwwwwwwwwwwwwwwwwwwwww" 	 //25
};

//Icono "My machine" para SMS
char *bitmap_button_ext_desktop_my_machine_sms[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
    "                          ",
	"                          ",	//10
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xx xx xx xx xx xx xx xxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxx         xxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxWWxWWxWWxxxxrrrrrrrrrxxx",
	"xxxxxxxxxxxxxxrrwwrrrrrxxx",
	"xxxxxxxxxxxxxxrxrrrrrrrxxx",
	"xxxxxxxxxxxxxxrxxrrrxxrxxx",
	"xxxxxxxxxxxxxxrxrrrrrrrxxx",    //20
	"xxxxxxxxxxxxxxrrrwwrrrrxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxrrrxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxx   xxxx" 	 //25
};


//Icono "My machine" para SG1000
char *bitmap_button_ext_desktop_my_machine_sg1000[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
    "                          ",
	"                          ",	//10
	"  WWWWWWWWWWWWWWWWWWWWWW  ",
	" WWWWWWWWwwwwwwwwWWWWWWWW ",
	" WWwwwwWWw      wWWwwwwWW ",
	" WWWWWWWWw      wWWWWWWWW ",
	"WWWwwwwWWwwwwwwwwWWwwwwWWW",
    "WWWWWWWWWWWWWWWWWWWWWWWWWW",
	"WWWWbbbbbbbbbbbbbbbbbbWWWW",
	"WWWWbWWWWWWWWWWWWWWWWbWWWW",
	"WWWbWWWWWWWbbbbWWWWWWWbWWW",
	"WWWbWWWWWWWWWWWWWWWWWWbWWW",    //20
	"WWbRRRRRRRRRRRRRRRRRRRRbWW",
	"WWbbbbbbbbbbbbbbbbbbbbbbWW",
	"WWbbbbbbbbbbbbbbbbbbbbbbWW",
	"WWWWWWWWWWWWWWWWWWWWWWWWWW",
	" WWWWWWWWWWWWWWWWWWRRWWWW ", 	 //25
};

//Icono "My machine" para Colecoo
char *bitmap_button_ext_desktop_my_machine_coleco[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
    "xxxxxxxxxxxxxx x x x x x x",
	"xxwwxxwxwxwxwx x x x x x x",	//10
	"xwwwwxxxxxxxxx x x x x x x",
	"xwwwwxwxwxwxwxxxxxxxxxxxxx",
	"xxwwxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxwwxxwxwxwxwxxxxxxxxxxxxx",
	"xwwwwxxxxxxxxxxxxxxxxxxxxx",
	"xwwwwxwxwxwxwxxxxxxxxxxxxx",
	"xxwwxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxwwwwwwxxxx",
	"xxxxxxxxxxxxxxxxwwwwwwxxxx",
	"x x x x x xxxxwwwwwwwwwxxx",
	"x x x x x xxxxwwwwwwwwwxxx",
    "x x x x x xxxxrrxxxxxrrxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx" 	 //25
};


//Icono "My machine" para un TK90X
char *bitmap_button_ext_desktop_my_machine_tk90x[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
    "                          ",
	"                          ",	//10
	" xxxxxxxxxxxxxxxxxxxxxxxx ",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxWWxWWxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxwwxwwxwwxwwxwwxwwxwwxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxwwxwwxwwxwwxwwxwwxwwxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",    //20
	"xxxxwwxwwxwwxwwxwwxwwxwwxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxwwxwwxwwxwwxwwxwwxwrygb",
	"xxxxxxxxxxxxxxxxxxxxxrygbx",
	" xxxxxxxxxxxxxxxxxxxrygbx " 	 //25
};

//Icono "My machine" para un TK95
char *bitmap_button_ext_desktop_my_machine_tk95[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
    "                          ",
	"                          ",	//10
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxWWWxxrygbxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxcxwwxwwxwwxwwxwwxwwxcxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxccxwwxwwxwwxwwxwwxcccxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",    //20
	"xxcccxwwxwwxwwxwwxwwxccxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxccxwwxwwxwwwwwwwxccccxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx" 	 //25
};


//Icono "My machine" para un MSX
char *bitmap_button_ext_desktop_my_machine_msx[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
    "xWWWWxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",		//10
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxrxxrxxxx       xxxx",
	"xxggxxxgxxgxxxx       xxRx",
	"xxxxxxxxxxxxxxxRRxxxxxxxxx",
    "xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxwwxwwxwwxwwxwwwxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxwwwxxx",
	"xxwxwwxwwxwwxwwxwxxwxwxwxx",
	"xxxxxxxxxxxxxxxxwxxwwxwwxx",    //20
	"xxwwxwwxwwxwwxwwwxxwxwxwxx",
	"xxxxxxxxxxxxxxxxxxxxwwwxxx",
	"xxwxwwxwwwwxwwxwwxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx" 	 //25
};


//Icono "My machine" para un SVI318
char *bitmap_button_ext_desktop_my_machine_svi318[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"WWWWWWWWWWWWWWWWWWWWWWWWWW",
    "WWWWWWWWWWWWWWWWwwwwwwWWWW",
	"WWWWWWWWWWWWWWWWWWWWWWWWWW",		//10
	"WWWWWWWWWWWWWWWWwwwwwwWWWW",
	"WWWWWWWWWWWWWWWWWWWWWWWWWW",
	"WWWWWWWWWWWWWWWW      WWWW",
	"WWxxxxWWWWWWWWWW      WWWW",
	"WWWWWWWWWWWWWWWWWWWWWWWWWW",
    "WWWWWWWWWWWWWWWWWWWWWWWWWW",
	"WWxxWxxWxxWxxWxxxWxxWWWWWW",
	"WWWWWWWWWWWWWWWWWWWWWWWWWW",
	"WWxWxxWxxWxxWxxWxWWWWrrWWW",
	"WWWWWWWWWWWWWWWWWWWWrrrrWW",    //20
	"WWxxWxxWxxWxxWxxxWWWrrrrWW",
	"WWWWWWWWWWWWWWWWWWWWWrrWWW",
	"WWWWxxWxxxxxxxWWWWWWWWWWWW",
	"WWWWWWWWWWWWWWWWWWWWWWWWWW",
	"WWWWWWWWWWWWWWWWWWWWWWWWWW" 	 //25
};


//Icono "My machine" para un SVI328
char *bitmap_button_ext_desktop_my_machine_svi328[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"WWWWWWWWWWWWWWWWWWWWWWWWWW",
    "WWWWWWWWWWWWWWWWwwwwwwWWWW",
	"WWWWWWWWWWWWWWWWWWWWWWWWWW",		//10
	"WWWWWWWWWWWWWWWWwwwwwwWWWW",
	"WWWWWWWWWWWWWWWWWWWWWWWWWW",
	"WWWWWWWWWWWWWWWW      WWWW",
	"WWxxxxWWWWWWWWWW      WWWW",
	"WWWWWWWWWWWWWWWWWWWWWWWWWW",
    "WWWWWWWWWWWWWWWWWWWWWWWWWW",
	"WWxxWxxWxxWxxWxxxWxWxWxWxW",
	"WWWWWWWWWWWWWWWWWWWWWWWWWW",
	"WWxWxxWxxWxxWxxWxWxWxWxWxW",
	"WWWWWWWWWWWWWWWWWWWWWWWWWW",    //20
	"WWxxWxxWxxWxxWxxxWxWxWxWxW",
	"WWWWWWWWWWWWWWWWWWWWWWWWxW",
	"WWxxWxWxxxxxxxWxxWxWxWxWxW",
	"WWWWWWWWWWWWWWWWWWWWWWWWWW",
	"WWWWWWWWWWWWWWWWWWWWWWWWWW" 	 //25
};


//Icono "My machine" para Timex TS 2068
char *bitmap_button_ext_desktop_my_machine_timex_ts2068[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
    "                          ",
	"                          ",	//10
	"wwwwwwwwwwwwwwwwwwwwwwwwww",
	"wwwwwwwwwwwwwwwwwwwwwwwwww",
	"wwBBBxxxwwwwwwwwwwwwwwwwww",
	"wwwwwwwwwwwwwwwwwwwwwwwwww",
	"wwwwwwwwwwwwwwwwwwwwwwwwww",
	"wwwwwwwwwwwwwwwwwwwwwwwwww",
	"wwWWwWWwWWwWWwWWwwwxxxxxxx",
	"wwwwwwwwwwwwwwwwwwwxwwwwww",
	"wwwWWwWWwWWwWWwWWwwxwwwwww",
	"wwwwwwwwwwwwwwwwwwwxwwwwww",    //20
	"wwwwWWwWWwWWwWWwWWwxwwwwww",
	"wwwwwwwwwwwwwwwwwwwxwwwwww",
	"wrgbwwWWWWWWWWWwwwwxwwwwww",
	"wwwwwwwwwwwwwwwwwwwxwwwrrw",
	"wwwwwwwwwwwwwwwwwwwxwwwwww" 	 //25
};

//Icono "My machine" para Timex TC 2048
char *bitmap_button_ext_desktop_my_machine_timex_tc2048[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
    "                          ",
	"                          ",	//10
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxBBBWWWxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxWWxWWxWWxWWxWWxWWxWWxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxWWxWWxWWxWWxWWxWWxWWxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",    //20
	"xxxxWWxWWxWWxWWxWWxWWxWWxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xrgbxxWWWWWWWWWWWWWWWxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx" 	 //25
};


//Icono "My machine" para Z88
char *bitmap_button_ext_desktop_my_machine_z88[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
    " xxxxxxxxxxxxxxxxxxxxxxxx ",
	"xxxxWWWxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxgggggggggggggggggggxxxx",
	"xxxgggggggggggggggggggxxxx",
	"xxxgggggggggggggggggggxxxx",
    "xxxwwwwwwwwwwwwwwwwwwwxxxx",
    "xxxwwwwwwwwwwwwwwwwwwwxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxwwxwwxwwxwwxwwxwwxwwwxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxwwwxwwxwwxwwxwwxwwxwwxxx",
	"xxxxxxxxxxxxxxxxxxxxxxwxxx",    //20
	"xxwwwwxwwxwwxwwxwwxwwxwxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxwxwxwxwwwwwwwwwxwxwxwxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx" 	 //25
};





//Icono "My machine" para un ZX-Uno
char *bitmap_button_ext_desktop_my_machine_zxuno[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"            www           ",
	"            www           ",
    "xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",	//10
	"xxxxxxxxxxxxxxxxxxxxx   xx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xWWWWxWxWxxxxWxxWWxxWWWWxx",
	"xxxWxxxWxxWWxWxxWxWxWxxxWx",
	"xxWxxxWxWxxxxWxxWxxWWxWxWx",
	"xWWWWWxxxWxxxxWWxxxxWxxWxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",    //20
	"xrrxyyxggxBBxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	" xxxxxxxxxxxxxxxxxxxxxxxxx",
	" xxxxxxxxxxxxxxxxxxxxxxxxx",
	" xxxxxxxxxxxxxxxxxxxxxxxxx" 	 //25
};



//Icono "My machine" para un TSConf
char *bitmap_button_ext_desktop_my_machine_tsconf[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "RRRRRRxxxxxxxxxxxxxxxRRRR ",     //0
  	"R RRRRxxxxxxxxxxxxxxxRR R ",
	"RRRRRRRRRRRRRRRRRRRRRRRRR ",
	"RRxxRRxxxxxxxxxxxxxxxRRwww",
	"RRxxRRxxxxxxxxxxxxxxxRRRR ",
	"RRRRRRRRRRRRRRRRRRRRRRRww ",
	"RRRRRRRRRRRRRRRRRRRRRRRww ",
	"RRRRRRRRRRxxxxxxxxxRRRRRR ",
	"RRRRRRRRRRxxxxxxxxxRRRxxxw",
    "RRRRRRRRRRxxxxxxxxxRRRxxxw",
	"RRRRRRRRRRRRRRRRRRRRRRxxxw",	//10
	"xxRxxxRRRRRRRRRRRRRRRRxxxw",
	"xxRxxxRRRRRRRRWWWRWWWRRRR ",
	"xxRxxxRRxxxxxRRWRRWRRRxxxw",
	"xxRxxxRRxxxxxRRWRRWWWRxxxw",
	"xxRxxxRRxxxxxRRWRRRRWRxxxw",
	"xxRxxxRRxxxxxRRWRRWWWRxxxw",
	"xxRxxxRRxxxxxRRRRRRRRRRRR ",
	"xxRxxxRRRRRRRRRRRRRRRRRRR ",
	"RRRRRRRRRRRRRRRRRRRRwwwww ",
	"RRRRRRRRRRRRRRRxxxRRwwwwR ",    //20
	"RwwRRRxxxxxxxRRxxxRRwwwwR ",
	"RwwRRRxxxxxxxRRxxxRRwwwww ",
	"RRRRRRRRRRRRRRRRRRRRRRRRR ",
	"R RRRWWWWWWWRRRRRRR RRRwww",
	"RRRRRWWWWWWWRRRRRRRRRRRwww" 	 //25
};

//Icono "My machine" para un BaseConf
char *bitmap_button_ext_desktop_my_machine_baseconf[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "RRRRRRxxxxxxxxxxxxxxxRRRR ",     //0
  	"R RRRRxxxxxxxxxxxxxxxRR R ",
	"RRRRRRRRRRRRRRRRRRRRRRRRR ",
	"RRxxRRxxxxxxxxxxxxxxxRRwww",
	"RRxxRRxxxxxxxxxxxxxxxRRRR ",
	"RRRRRRRRRRRRRRRRRRRRRRRww ",
	"RRRRRRRRRRRRRRRRRRRRRRRww ",
	"RRRRRRRRRRxxxxxxxxxRRRRRR ",
	"RRRRRRRRRRxxxxxxxxxRRRxxxw",
    "RRRRRRRRRRxxxxxxxxxRRRxxxw",
	"RRRRRRRRRRRRRRRRRRRRRRxxxw",	//10
	"xxRxxxRRRRRRRRRRRRRRRRxxxw",
	"xxRxxxRRRRRRRRWWRRRWWRRRR ",
	"xxRxxxRRxxxxxRWRWRWRRRxxxw",
	"xxRxxxRRxxxxxRWWRRWRRRxxxw",
	"xxRxxxRRxxxxxRWRWRWRRRxxxw",
	"xxRxxxRRxxxxxRWWRRRWWRxxxw",
	"xxRxxxRRxxxxxRRRRRRRRRRRR ",
	"xxRxxxRRRRRRRRRRRRRRRRRRR ",
	"RRRRRRRRRRRRRRRRRRRRwwwww ",
	"RRRRRRRRRRRRRRRxxxRRwwwwR ",    //20
	"RwwRRRxxxxxxxRRxxxRRwwwwR ",
	"RwwRRRxxxxxxxRRxxxRRwwwww ",
	"RRRRRRRRRRRRRRRRRRRRRRRRR ",
	"R RRRWWWWWWWRRRRRRR RRRwww",
	"RRRRRWWWWWWWRRRRRRRRRRRwww" 	 //25
};

//Icono "My machine" para un MK14
char *bitmap_button_ext_desktop_my_machine_mk14[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "   gxgxgxgxgxgxgxgxgxgg   ",     //0
  	"   gxgxgxgxgxgxgxgxgxgg   ",
	"   ggggggBBBBBgggxxxggg   ",
	"   gxxxggBBBBBgggxxxggg   ",
	"   gxxxggBBBBBggggggggg   ",
	"   gggggggggggggxxxgggg   ",
	"   ggggxxxxxxxggxxxgggg   ",
	"   ggggxxxxxxxggggggggg   ",
	"   ggggxxxxxxxgggxxxggg   ",
    "   ggggggggggggggxxxggg   ",
	"   ggxxxggggggggggggggg   ",	//10
	"   ggxxxggggggggggxxxgg   ",
	"   ggggggwwwwwwwggxxxgg   ",
	"   ggggggwwwwwwwggggggg   ",
	"   gggggggggggggggggggg   ",
	"   gggxxxxwwwwwxxxxgggg   ",
	"   gggxxxxxxxxxxxxxgggg   ",
	"   gggxwwxwwxwwxwwxgrrg   ",
	"   gggxxxxxxxxxxxxxgrrg   ",
	"   gggxwwxwwxwwxwwxgggg   ",
	"   gggxxxxxxxxxxxxxggxx   ",    //20
	"   gggxwwxwwxwwxwwxgggg   ",
	"   gggxxxxxxxxxxxxxggxx   ",
	"   gggxwwxwwxwwxwwxgggg   ",
	"   gggxxxxxxxxxxxxxggxx   ",
	"   gggxxxxwwwwwxxxxgggg   " 	 //25
};


//Icono "My machine" para un Pentagon
char *bitmap_button_ext_desktop_my_machine_pentagon[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
    "                          ",
	"                          ",	//10
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xx x x x x x x x x x x xxx",
	"xx x x x x x x x x x x xxx",
	"xx x x x x x x x x x x xxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxwwxwwxwwxwwxwwxwwxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxwwxwwxwwxwwxwwxwwxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",    //20
	"xxxxxwwxwwxwwxwwxwwxwwxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxwwxwwxwwxwwxwwxwwxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx" 	 //25
};


//Icono "My machine" para un Spectrum Next
char *bitmap_button_ext_desktop_my_machine_spectrum_next[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
    "                          ",
	"                          ",	//10
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxrxrrxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxwwxwwxwwxwwxwwxwwwxxxrrr",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxwxwwxwwxwwxwwxwwxwxxxyyy",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",    //20
	"xxwwwxwwxwwxwwxwwxwwxxxggg",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxwxwwxwwwwwwwxwwxwwxxxBBB",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	" xxxxxxxxxxxxxxxxxxxxxxxxx" 	 //25
};





//Icono "My machine" para un Sam Coupe
char *bitmap_button_ext_desktop_my_machine_sam[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"                          ",
	"W W W W W W W W W W W W WW",
	"W W W W W W W W W W W W WW",
	"WWWWWWWWWWWWWWWWWWWWWWbbbW",
	"WWwwWwwWwwWwwWwwWwwwWWwWwW",
	"WWWWWWWWWWWWWWWWWWWWWWWWWW",
	"WWwWwwWwwWwwWwwWwwWwWWwWwW",
	"WWWWWWWWWWWWWWWWWWWwWWWWWW",
	"WWwwwWwwWwwWwwWwwWwwWWwWwW",
	"WWWWWWWWWWWWWWWWWWWWWWWWWW",
	"WWwWwwWwwwwwwwWwwWwwWWwWwW",
	"WWWWWWWWWWWWWWWWWWWWWWWWWW",
    "WWWWWWWWWWWWWWWWWWWWWWWWWW",
    "WWWWWWWWWWWWWWWWWWWWWWWWWW",
    "WWWWWWWWWWWWWWWWWWWWWWWWWW",
    "WWWWWWWWWWWWWWWWWWWWWWWWWW",
    "WWWWWWWWWWWWWWWWWWWWWWWWWW",
    "WWbbbrrWWWWWWWWWWWWWWWWWWW",
    "WWWWWWWWWWWWWWWWWWWWWWWWWW",
	"WWWWWWWWWWWWWWWWWWWWWWWWWW" 	 //25
};



char *bitmap_button_ext_desktop_speccy_online[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "            xxx           ",     //0
  	"            xxx           ",
	"            xxx           ",
	"            xxx           ",
	"          xxxxxxx         ",
	"           xxxxx          ",
	"            xxx           ",
	"    x        x        x   ",
	"    x                 x   ",
	"    xxxxxxxxxxxxxxxxxxx   ",
	"                          ",	//10
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxwwxwxwwxwwxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxwwxwwxwwxwwxwwxwwxwwwxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxwwxwwxwwxwwxwwxwwxwwxxx",
	"xxxxxxxxxxxxxxxxxxxxxxwxxr",    //20
	"xxxxwwxwwxwwxwwxwwxwwxwxry",
	"xxxxxxxxxxxxxxxxxxxxxxxryg",
	"xxxxxxwwxwwwwwwwwxwwxxrygb",
	"xxxxxxxxxxxxxxxxxxxxxrygbx",
	" xxxxxxxxxxxxxxxxxxxrygbx " 	 //25
};


char *bitmap_button_ext_desktop_zx81_online[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "            xxx           ",     //0
  	"            xxx           ",
	"            xxx           ",
	"            xxx           ",
	"          xxxxxxx         ",
	"           xxxxx          ",
	"            xxx           ",
	"    x        x        x   ",
	"    x                 x   ",
	"    xxxxxxxxxxxxxxxxxxx   ",
	"                          ",	//10
	"xxxxxxxxxxxxxxxxxxxxxxxxxx", //11
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxrrxrxrxrrxrxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxWWxWWxWWxWWxWWxWWxWWxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxWWxWWxWWxWWxWWxWWxWWxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",    //20
	"xxxxWWxWWxWWxWWxWWxWWxWWxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxWWxWWxWWxWWxWWxWWxWWxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx",
	"xxxxxxxxxxxxxxxxxxxxxxxxxx" 	 //25
};

char *bitmap_button_ext_desktop_openwindow[EXT_DESKTOP_BUTTONS_ANCHO]={
   //01234567890123456789012345
	" xxxxxxxxxxxxxxxxxxx      ",   //0
	" xwwwwwwwwwwwwwwwwwxx     ",
	" xwwwwwwwwwwwwwwwwwxwx    ",
	" xwwwwwwwwwwwwwwwwwxwwx   ",
	" xwwwwwwwwwwwwwwwwwxwwwx  ",
	" xwwwwwwwwxxxxxxwwwxxxxxx ",
	" xwwwwwwxxxxxxxxxxwwwwwwx ",
	" xwwwwwxxcccxxcccxxwwwwwx ",
	" xwwwwxxccccxxccccxxwwwwx ",
	" xwwwwxxxxxxxxxxxxxxwwwwx ",
	" xwwwwxxxxxxxxxxxxxxwwwwx ",  //10
	" xwwwwxxccccxxccccxxwwwwx ",
	" xwwwwxxccccxxccccxxwwwwx ",
	" xwwwwxxccccxxccccxxwwwwx ",
	" xwwwwxxxxxxxxxxxxxxwwwwx ",
	" xwwwwxxxxxxxxxxxxxxwwwwx ",
	" xwwwwxxccccxxccccxxwwwwx ",
	" xwwwwxxccccxxccccxxwwwwx ",
	" xwwwwxxccccxxccccxxwwwwx ",
	" xwwwwxxxxxxxxxxxxxxwwwwx ",
	" xwwwwwwxxxxxxxxxxxxwwwwx ",  //20
	" xwwxxxwwwwwwwwwwwwwwwwwx ",
	" xwwwxxwxxxxxxxxxxxxxwwwx ",
	" xwwxwxwxxxxxxxxxxxxxwwwx ",
	" xwxwwwwwwwwwwwwwwwwwwwwx ",
	" xxxxxxxxxxxxxxxxxxxxxxxx "	//25
};

char *bitmap_button_ext_desktop_processmanagement[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "          xxxxxx          ",     //0
  	"        xxxxxxxxxx        ",
	"       xxcccxxcccxx       ",
	"       xxcccxxcccxx       ",
	"      xxccccxxccccxx      ",
	"      xxccccxxccccxx      ",
	"      xxxxxxxxxxxxxx      ",
	"      xxxxxxxxxxxxxx      ",
	"      xxccccxxccccxx      ",
	"      xxccccxxccccxx      ",
	"      xxccccxxccccxx      ",	//10
	"      xxccccxxccccxx      ",
	"      xxccccxxccccxx      ",
	"      xxxxxxxxxxxxxx      ",
	"      xxxxxxxxxxxxxx      ",
	"      xxccccxxccccxx      ",
	"      xxccccxxccccxx      ",
	"  wwwwwwwwwcxxccccxx      ",
	"  wwwwrwwwwcxxccccxx      ",
	"  wrwrrrwrwcxxccccxx      ",
	"  wwrrwrrwwxxxxxxxxx      ",    //20
	"  wrrwwwrrwxxxxxxxxx      ",
	"  wwrrwrrww               ",
	"  wrwrrrwrw               ",
	"  wwwwrwwwwxxxxxxxxxx     ",
	"  wwwwwwwwwxxxxxxxxxx     " 	 //25
};

/*
	" xwwwwrwwwwcxxccccxxwwwwx ",
	" xwrwrrrwrwxxxxxxxxxwwwwx ",
	" xwwrrwrrwwxxxxxxxxxwwwwx ",  //20
	" xwrrwwwrrwwwwwwwwwwwwwwx ",
	" xwwrrwrrwwxxxxxxxxxxwwwx ",
	" xwrwrrrwrwxxxxxxxxxxwwwx ",
	" xwwwwrwwwwwwwwwwwwwwwwwx ",
*/

/*

char *bitmap_button_ext_desktop_processmanagement[EXT_DESKTOP_BUTTONS_ANCHO]={
   //01234567890123456789012345
	" xxxxxxxxxxxxxxxxxxx      ",   //0
	" xwwwwwwwwwwwwwwwwwxx     ",
	" xwwwwwwwwwwwwwwwwwxwx    ",
	" xwwwwwwwwwwwwwwwwwxwwx   ",
	" xwwwwwwwwwwwwwwwwwxwwwx  ",
	" xwwwwwwwwxxxxxxwwwxxxxxx ",
	" xwwwwwwxxxxxxxxxxwwwwwwx ",
	" xwwwwwxxcccxxcccxxwwwwwx ",
	" xwwwwxxccccxxccccxxwwwwx ",
	" xwwwwxxxxxxxxxxxxxxwwwwx ",
	" xwwwwxxxxxxxxxxxxxxwwwwx ",  //10
	" xwwwwxxccccxxccccxxwwwwx ",
	" xwwwwxxccccxxccccxxwwwwx ",
	" xwwwwxxccccxxccccxxwwwwx ",
	" xwwwwxxxxxxxxxxxxxxwwwwx ",
	" xwwwwxxxxxxxxxxxxxxwwwwx ",
	" xwwwwxxccccxxccccxxwwwwx ",
	" xwwwwwwwwwcxxccccxxwwwwx ",
	" xwwwwrwwwwcxxccccxxwwwwx ",
	" xwrwrrrwrwxxxxxxxxxwwwwx ",
	" xwwrrwrrwwxxxxxxxxxwwwwx ",  //20
	" xwrrwwwrrwwwwwwwwwwwwwwx ",
	" xwwrrwrrwwxxxxxxxxxxwwwx ",
	" xwrwrrrwrwxxxxxxxxxxwwwx ",
	" xwwwwrwwwwwwwwwwwwwwwwwx ",
	" xxxxxxxxxxxxxxxxxxxxxxxx "	//25
};
*/


/*
char *bitmap_button_ext_desktop_processswitcher[EXT_DESKTOP_BUTTONS_ANCHO]={
   //01234567890123456789012345
	" xxxxxxxxxxxxxxxxxxx      ",   //0
	" xwwwwwwwwwwwwwwwwwxx     ",
	" xwwwwwwwwwwwwwwwwwxwx    ",
	" xwwwwwwwwwwwwwwwwwxwwx   ",
	" xwwwwwwwwwwwwwwwwwxwwwx  ",
	" xwwwwwwwwxxxxxxwwwxxxxxx ",
	" xwwwwwwxxxxxxxxxxwwwwwwx ",
	" xwwwwwxxcccxxcccxxwwwwwx ",
	" xwwwwxxccccxxccccxxwwwwx ",
	" xwwwwxxxxxxxxxxxxxxwwwwx ",
	" xwwwwxxxxxxxxxxxxxxwwwwx ",  //10
	" xwwwwxxccccxxccccxxwwwwx ",
	" xwwwwxxccccxxccccxxwwwwx ",
	" xwwwwxxccccxxccccxxwwwwx ",
	" xwwwwxxxxxxxxxxxxxxwwwwx ",
	" xwwwwxxxxxxxxxxxxxxwwwwx ",
	" xwwwwxxccccxxccccxxwwwwx ",
	" xwwwwwwwwwwxxccccxxwwwwx ",
	" xwwwbwwbwwwxxccccxxwwwwx ",
	" xwwbbwwbbwwxxxxxxxxwwwwx ",
	" xwbbbbbbbbwxxxxxxxxwwwwx ",  //20
	" xwbbbbbbbbwwwwwwwwwwwwwx ",
	" xwwbbwwbbwwxxxxxxxxxwwwx ",
	" xwwwbwwbwwwxxxxxxxxxwwwx ",
	" xwwwwwwwwwwwwwwwwwwwwwwx ",
	" xxxxxxxxxxxxxxxxxxxxxxxx "	//25
};
*/

char *bitmap_button_ext_desktop_processswitcher[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "          xxxxxx          ",     //0
  	"        xxxxxxxxxx        ",
	"       xxcccxxcccxx       ",
	"       xxcccxxcccxx       ",
	"      xxccccxxccccxx      ",
	"      xxccccxxccccxx      ",
	"      xxxxxxxxxxxxxx      ",
	"      xxxxxxxxxxxxxx      ",
	"      xxccccxxccccxx      ",
	"      xxccccxxccccxx      ",
	"      xxccccxxccccxx      ",	//10
	"      xxccccxxccccxx      ",
	"      xxccccxxccccxx      ",
	"      xxxxxxxxxxxxxx      ",
	"      xxxxxxxxxxxxxx      ",
	"      xxccccxxccccxx      ",
	"      xxccccxxccccxx      ",
	"      xxccccxxccccxx      ",
	"  wwwwwwwwwwxxccccxx      ",
	"  wwwbwwbwwwxxccccxx      ",
	"  wwbbwwbbwwxxxxxxxx      ",    //20
	"  wbbbbbbbbwxxxxxxxx      ",
	"  wbbbbbbbbw              ",
	"  wwbbwwbbww              ",
	"  wwwbwwbwwwxxxxxxxxx     ",
	"  wwwwwwwwwwxxxxxxxxx     " 	 //25
};


char *bitmap_button_ext_desktop_videoinfo[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",
	"RRRRRRRRRRRRRRRRRRRRRRRRRR",
	"RrrrrrrrrrrrrrrrrrrrrrrrrR",
	"Rr                      rR",
	"Rr         mmmm         rR",
	"Rr         mmmmm        rR",
	"Rr         mmmm         rR",
	"Rr         xx           rR",
	"Rr         xx           rR",
	"Rr  xxxxxxxxxxxxxxxxx   rR",	//10
	"Rr  xxxxxxxxxxxxxxxxx   rR",
	"Rr   xxxxxxxxxxxxxxx    rR",
	"Rr    xxxxxxxxxxxxx     rR",
	"RrbbbbbbbbbbbbbbbbbbbbbbrR",
	"RrBBccccccbbbcccccccccccrR",
	"RrBBBBbbbbbbccBBBBBbbbbbrR",
	"wwwwwwbbbBBBBcBBBBBBBBBBrR",
	"wwbbwwBBBBBBBBBBBBBBBBBBrR",
	"wwwwwwBBBBBBBBBBBBBBBBBBrR",
	"wbbbwwrrrrrrrrrrrrrrrrrrrR",    //20
	"wwbbwwRRRRRRRRRRRRRRRRRRRR",
	"wwbbww  RRRRRRRRRR        ",
	"wwbbww RRRRRRRRRRRR       ",
	"wbbbbw RRRRRRRRRRRRRR     ",
	"wwwwwwrrrrrrrrrrrrrrr     " 	 //25
};

char *bitmap_button_ext_desktop_textadvlocimage[EXT_DESKTOP_BUTTONS_ANCHO]={
   //01234567890123456789012345
	"wwwwwwwwwwwwwwwwwwwwwwwwww", //0
	"xwxwxwxwxxxwxwxwxxxwxxwxwx",
	"xwxwxwxwxwxwxwxwxwwwxwwxwx",
	"wwwwxxxwxwxwxwxwxxxwxxwwww",
	"wwwwxwxwxwxwxwxwwwxwxwwwww",
	"wwwwxwxwxxxwxxxwxxxwxxwwww",
	"wwwwwwwwwwwwwwwwwwwwwwwwww",
	"           wxxxw          ",
	"           wxxxw          ",
	"          wxxxxxw         ",
	"           wxxxw          ",	//10
	"            wxw           ",
	"             w            ",
	"            rrr   rr      ",
	"          rrrrrrr rr      ",
	"        rrrrrWrrrrrr      ",
	"      rrrrrWWWWWrrrrr     ",
	"    rrrrrWWWWWWWWWrrrrr   ",
	"     wWWWWWWWWWWWWWWWw    ",
	"     wWWbbbWWWWWWWWWWw    ",
	"     wWWbbbWWWWWWWWWWw    ",    //20
	"     wWWbbbWWxxxWWWWWw    ",
	"     wWWWWWWxxxxxWWWWw    ",
	"     wWWWWWWxxxxxWWWWw    ",
	"     wWWWWWWxxxxxWWWWw    ",
	"     wwwwwwwwwwwwwwwww    ", 	 //25
};