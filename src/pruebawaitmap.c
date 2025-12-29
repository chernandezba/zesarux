#include <stdio.h>

int min(int a,int b)
{
if (a<b) return a;
else return b;
}

int waitmap_size=207;

void init_waitmap(int cc_hsync) {


        int i = 0;
        for (; i < min(waitmap_size - cc_hsync, 16); i++) {
            printf("1 Escribir %d con %d\n",cc_hsync + i,16 - i);
            //waitmap[cc_hsync + i] = 16 - i;
        }
        for (; i < 16; i++) {
	    printf("2 Escribir %d con %d\n",cc_hsync - waitmap_size + i,16 - i);
            //waitmap[cc_hsync - waitmap_size + i] = 16 - i;
        }

}

main() {
  init_waitmap(200);
}
