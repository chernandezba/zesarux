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
    "                  rryyggcc",     //0
  	" xxx xx x  x x   rryyggcc ",      
	" x x x   x xxx  rryyggcc  ",		
	"               rryyggcc   ",		
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


char *bitmap_lowericon_ext_desktop_z88_active[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
	"wwwwwwwwwwwwwwwwwwwwwwwwww", //0
    "wxxxxxxxxxxxxxxxxxxxxxxxxw",     
  	"w xwwwwwwwwwwwwwwwwwwwwx w",      
	"wxxwwxxxwxwwwxxxwxxxwwwxxw",		
	"w xwwxwwwxwwwxwxwwxwwwwx w",		
	"wxxwwxxxwxwwwxwxwwxwwwwxxw",	
	"w xwwwwxwxwwwxwxwwxwwwwx w",			
	"wxxwwxxxwxxxwxxxwwxwwwwxxw",			
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


char *bitmap_lowericon_ext_desktop_z88_inactive[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
	"wwwwwwwwwwwwwwwwwwwwwwwwww", //0
    "wxxxxxxxxxxxxxxxxxxxxxxxxw",     
  	"w x                    x w",      
	"wxx  xxx x   xxx xxx   xxw",		
	"w x  x   x   x x  x    x w",		
	"wxx  xxx x   x x  x    xxw",	
	"w x    x x   x x  x    x w",			
	"wxx  xxx xxx xxx  x    xxw",			
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
	"  wxxxxxxxxxwwxxxxxxxxxw  ", 		
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
	"wxggggxggggggggggggxggggxw",			
	"wxggggxggg      gggxggggxw",		
	"wxggxxxxxg      gxxxxxggxw",		
	"wxggggxggg      gggxggggxw",		
	"wxggggxggggggggggggxggggxw",		
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
	"          xxxxxx          ",		
	"          x    x          ",	
	"          x    x          ",			
	" xxxxxxxxxxxxxxxxxxxxxxxx ",			
	" xxxxxxxxxxxxxxxxxxxxxxxx ", 		
	"   x                  x   ",		
	"   x                  x   ",			
	"   x                  x   ",	//10		
	"   x                  x   ", 		
	"   x                  x   ",		
	"   x   x   x   x   x  x   ",	
    "   x   x   x   x   x  x   ",    		
	"   x   x   x   x   x  x   ",		
	"   x   x   x   x   x  x   ",		
	"   x   x   x   x   x  x   ",		
	"   x   x   x   x   x  x   ",		
	"   x   x   x   x   x  x   ",		
	"   x   x   x   x   x  x   ",    //20
	"   x   x   x   x   x  x   ",		
	"   x                  x   ",		
	"   x                  x   ",		
	"   x                  x   ",
	"   xxxxxxxxxxxxxxxxxxxx   " 	 //25
};


char *bitmap_button_ext_desktop_trash_not_empty[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "                          ",     //0
  	"                          ",      
	"                          ",		
	"         x                ",		
	"        x x   xxxx        ",	
	"       x   x x   x  x     ",			
	"      x     x    x xx     ",			
	"   xxxxxxxxxxxxxxxxxxxx   ", 		
	"   x                  x   ",		
	"   x                  x   ",			
	"   x                  x   ",	//10		
	"   x                  x   ", 		
	"   x                  x   ",		
	"   x   x   x   x   x  x   ",	
    "   x   x   x   x   x  x   ",    		
	"   x   x   x   x   x  x   ",		
	"   x   x   x   x   x  x   ",		
	"   x   x   x   x   x  x   ",		
	"   x   x   x   x   x  x   ",		
	"   x   x   x   x   x  x   ",		
	"   x   x   x   x   x  x   ",    //20
	"   x   x   x   x   x  x   ",		
	"   x                  x   ",		
	"   x                  x   ",		
	"   x                  x   ",
	"   xxxxxxxxxxxxxxxxxxxx   " 	 //25
};


char *bitmap_button_ext_desktop_file_snapshot[EXT_DESKTOP_BUTTONS_ANCHO]={
    //01234567890123456789012345
    "xxxxxxxxxxxxxxxxxxxxx     ",     //0
  	"x                   xx    ",  
	"x                   x x   ",	
	"x                   x  x  ",      
	"x                   x   x ",		
	"x        bbbbbbbb   xxxxxx",		
	"x       bbbbbbbbbb       x",	
	"x      bbbbbbbbbbbb      x",			
	"x     bbbbbbbbbbbbbb     x",			
	"x  bbbbbbbbbbbbbbbbbbbb  x", 		
	"x bbbbbbbbbbb bbbbbbbbbb x",	//10	
	"x bbbbbbbbb    bbbbbbbbb x",		
	"x bWWbbbbb      bbbbbbbb x",		
	"x bWWbbbb  xx    bbbbbbb x",		
	"x bbbbbbb x      bbbbbbb x",		
	"x bbbbbb  x       bbbbbb x",		
	"x bbbbbb          bbbbbb x",		
	"x bbbbbb          bbbbbb x",		
	"x bbbbbbb        bbbbbbb x",		
	"x bbbbbbb        bbbbbbb x",		
	"x bbbbbbbb      bbbbbbbb x",	//20	
	"x bbbbbbbbb    bbbbbbbbb x",		
	"x bbbbbbbbbb  bbbbbbbbbb x",    
	"x bbbbbbbbbbbbbbbbbbbbbb x",		
	"x                        x",		
	"xxxxxxxxxxxxxxxxxxxxxxxxxx"	//25	
 
	 
};


