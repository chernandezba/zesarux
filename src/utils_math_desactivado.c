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


//Funciones para interpolar coordenadas 2d a 3d (o sea, sabiendo que un pixel en 2d en pantalla, viene de un triangulo concreto en 3d, obtenemos la coordenada 3d de ese pixel)
//Estas funciones estaban usadas para poder renderizar objetos 3d ocultando los que no se ven, y esa parte NO funciona
//Diferentes alternativas consideran que un punto está detrás asi:
//                    if (-P3D.x/2.0f + P3D.y/2.0f + P3D.z > -buffz[offset_zbuffer].x_3d/2.0f + buffz[offset_zbuffer].y_3d/2.0f + buffz[offset_zbuffer].z_3d) {

                        //Está detrás

//Pero eso NO va. por tanto, lo dejo aquí como código histórico, comentado, por si en un futuro lo reuso

//Parte que va en el .h:


typedef struct {
    float x, y, z;
} Vec3;

typedef struct {
    float x, y;
} Vec2;

extern int pointOnTriangle3D(Vec2 p2D, Vec3 A3, Vec3 B3, Vec3 C3);
extern Vec3 barycentricTo3D(Vec2 p2D,Vec3 A3, Vec3 B3, Vec3 C3);


typedef struct {
    //Con nuestra proyeccion 3D, la coordenada y es la que da la profundidad
  //  float
    int usado;


    float x_3d,y_3d,z_3d;
} zbuffer;



//Parte que va en el .c


#define EPS 1e-6f




Vec2 project3Dto2D(Vec3 p) {
    Vec2 out;



    out.x=p.x+ p.y;
    out.y=p.z+ p.y/2-p.x/2;

    return out;
}

// ===== Producto escalar 2D
float dot2(Vec2 a, Vec2 b) {
    return a.x * b.x + a.y * b.y;
}

// ===== Punto dentro de triángulo 2D (baricéntricas) =====
int pointInTriangle2D(Vec2 p, Vec2 a, Vec2 b, Vec2 c) {
    Vec2 v0 = { c.x - a.x, c.y - a.y };
    Vec2 v1 = { b.x - a.x, b.y - a.y };
    Vec2 v2 = { p.x - a.x, p.y - a.y };

    float d00 = dot2(v0, v0);
    float d01 = dot2(v0, v1);
    float d11 = dot2(v1, v1);
    float d20 = dot2(v2, v0);
    float d21 = dot2(v2, v1);

    float denom = d00 * d11 - d01 * d01;

    // Triángulo degenerado
    if (denom > -EPS && denom < EPS)
        return 0;

    float beta  = (d11 * d20 - d01 * d21) / denom;
    float gamma = (d00 * d21 - d01 * d20) / denom;
    float alpha = 1.0f - beta - gamma;

    return (alpha >= -EPS && beta >= -EPS && gamma >= -EPS);
}

// Dado un punto 2D y un triángulo 3D
int pointOnTriangle3D(Vec2 p2D, Vec3 A3, Vec3 B3, Vec3 C3) {
    // Proyectamos el triángulo
    Vec2 A2 = project3Dto2D(A3);
    Vec2 B2 = project3Dto2D(B3);
    Vec2 C2 = project3Dto2D(C3);

    // Test en 2D
    return pointInTriangle2D(p2D, A2, B2, C2);
}

//Interpola una coordenada en 2D a 3D, suponiendo que está en la superficie de un triangulo
//Entrada: p2D: coordenada 2D a interpolar
//         A3,B3,C3: vertices del triangulo
Vec3 barycentricTo3D(Vec2 p2D,Vec3 A3, Vec3 B3, Vec3 C3)
{
    Vec2 A2 = project3Dto2D(A3);
    Vec2 B2 = project3Dto2D(B3);
    Vec2 C2 = project3Dto2D(C3);

    Vec2 v0 = { C2.x - A2.x, C2.y - A2.y };
    Vec2 v1 = { B2.x - A2.x, B2.y - A2.y };
    Vec2 v2 = { p2D.x - A2.x, p2D.y - A2.y };

    float d00 = dot2(v0, v0);
    float d01 = dot2(v0, v1);
    float d11 = dot2(v1, v1);
    float d20 = dot2(v2, v0);
    float d21 = dot2(v2, v1);

    float denom = d00 * d11 - d01 * d01;

    Vec3 out = {0,0,0};

    if (denom > -EPS && denom < EPS)
        return out;

    float beta  = (d11 * d20 - d01 * d21) / denom;
    float gamma = (d00 * d21 - d01 * d20) / denom;
    float alpha = 1.0f - beta - gamma;

    // Reconstrucción 3D
    out.x = alpha * A3.x + beta * B3.x + gamma * C3.x;
    out.y = alpha * A3.y + beta * B3.y + gamma * C3.y;
    out.z = alpha * A3.z + beta * B3.z + gamma * C3.z;

    return out;
}

// ===== Ejemplo de uso
int ejemplo_main() {
    Vec3 A = {0.0f, 0.0f, 0.0f};
    Vec3 B = {2.0f, 0.0f, 1.0f};
    Vec3 C = {0.0f, 2.0f, 1.0f};

    Vec2 P = {0.5f, 0.5f};

    if (pointOnTriangle3D(P, A, B, C)) {
        printf("El punto ESTA dentro del triangulo\n");
    } else {
        printf("El punto NO esta dentro del triangulo\n");
    }

    return 0;
}

//Esto va en zxvision.h
extern void zxvision_draw_filled_triangle_3D(zxvision_window *w,int color_relleno,int color_aristas,Vec3 A,Vec3 B,Vec3 C,zbuffer *buffz,
    void (*fun_putpixel) (zxvision_window *w,int parm_x,int parm_y,int parm_color),int debug);

//Esto va en zxvision.c



//Coordenadas en el plano 3D
//void zxvision_draw_filled_triangle_3D(zxvision_window *w,int color_relleno,int color_aristas,int x1,int y1,int x2,int y2,int x3,int y3)
void zxvision_draw_filled_triangle_3D(zxvision_window *w,int color_relleno,int color_aristas,Vec3 A,Vec3 B,Vec3 C,zbuffer *buffz,
    void (*fun_putpixel) (zxvision_window *w,int parm_x,int parm_y,int parm_color),int debug)
{



    //Buscamos minimo x
    int min_x,min_y,max_x,max_y;


    //Temporal. TODO. de momento minimos y maximos puestos a pelo
    //Habria que sacar esos minimos y maximos proyectando coordenadas X,Y,Z a X,Y a cada vertice con x1,y1, x2,y2, x3,y3 y descomentar trozo código anterior
    min_x=0;
    min_y=0;
    max_x=199;
    max_y=199;


    //Ahora vamos a hacer render desde posicion Y menor (en coordenadas de zxvision el 0,0 esta arriba del todo)
    int y;
    for (y=min_y;y<=max_y;y++) {
        int x;

        //Vamos a rellenar mirando si ese pixel corresponde o no al triangulo en 3d
        for (x=min_x;x<=max_x;x++) {



            Vec2 P = {x, y};


            if (pointOnTriangle3D(P, A, B, C)) {

                //Obtener el punto 3d de coordenada 2d, sabiendo en que triangulo está
                Vec3 P3D = barycentricTo3D(P, A, B, C);

                //printf("1 en %d %d  en 3D: %f,%f,%f\n",x,y,P3D.x,P3D.y,P3D.z);


                //Ver si ese pixel no está en el zbuffer
                //Obtener coordenadas 3D para ese pixel 2D

                int offset_zbuffer=200*y+x;

                int dibujar=1;


                if (buffz[offset_zbuffer].usado) {


                    //TODO esto esta MAL. Aquí hay que tener una función "mágica" que diga si un punto 3D está detrás de otro
                    if (-P3D.x/2.0f + P3D.y/2.0f + P3D.z > -buffz[offset_zbuffer].x_3d/2.0f + buffz[offset_zbuffer].y_3d/2.0f + buffz[offset_zbuffer].z_3d) {

                        //Está detrás
                        //printf("No dibujar\n");
                        dibujar=0;
                    }
                }


                if (dibujar) {
                    if (debug && buffz[offset_zbuffer].usado) {
                        //printf("Dibujar. buffer Y: %f interpolada Y: %f resta: %f\n",buffz[offset_zbuffer].y_3d,P3D.y,resta);
                    }
                    buffz[offset_zbuffer].usado=1;
                    buffz[offset_zbuffer].x_3d=P3D.x;
                    buffz[offset_zbuffer].y_3d=P3D.y;
                    buffz[offset_zbuffer].z_3d=P3D.z;
                    fun_putpixel(w,x,y,color_relleno);
                }
            }

        }
    }



}




//Ejemplo de uso en ventana


void menu_debug_3d_test_overlay_putpixel_pyramid(zxvision_window *w,int x,int y,int color)
{
    zxvision_putpixel(w,x,150-y,color);
}

zxvision_window *menu_debug_3d_test_window;

zbuffer prueba_zbuffer[200][200];



void menu_debug_3d_test_overlay(void)
{

    menu_speech_set_tecla_pulsada(); //Si no, envia continuamente todo ese texto a speech

    //si ventana minimizada, no ejecutar todo el codigo de overlay
    if (menu_debug_3d_test_window->is_minimized) return;





    // prueba
    //inicializar zbuffer
    //prueba_zbuffer
    int i,j;
    for (i=0;i<199;i++) {
        for (j=0;j<199;j++) {
            prueba_zbuffer[i][j].usado=0;
        }
    }



    Vec3 Punto_alto = {25.0f, 25.0f, 130.0f}; //75.0f
    Vec3 Punto1 = {0.0f, 0.0f, 50.0f};
    Vec3 Punto2 = {50.0f, 0.0f, 50.0f};
    Vec3 Punto3 = {50.0f, 50.0f, 50.0f};

    //1
    zxvision_draw_filled_triangle_3D(menu_debug_3d_test_window,6,0,Punto1,Punto_alto,Punto2,&prueba_zbuffer[0][0],menu_debug_3d_test_overlay_putpixel_pyramid,0);

    //2
    zxvision_draw_filled_triangle_3D(menu_debug_3d_test_window,2,0,Punto2,Punto_alto,Punto3,&prueba_zbuffer[0][0],menu_debug_3d_test_overlay_putpixel_pyramid,0);

    //3
    zxvision_draw_filled_triangle_3D(menu_debug_3d_test_window,0,0,Punto3,Punto_alto,Punto1,&prueba_zbuffer[0][0],menu_debug_3d_test_overlay_putpixel_pyramid,1);






    //Mostrar contenido
    zxvision_draw_window_contents(menu_debug_3d_test_window);

    menu_debug_3d_test_window->must_clear_cache_on_draw=1;


}




//Almacenar la estructura de ventana aqui para que se pueda referenciar desde otros sitios
zxvision_window zxvision_window_debug_3d_test;


void menu_debug_3d_test(MENU_ITEM_PARAMETERS)
{
    menu_espera_no_tecla();

    if (!menu_multitarea) {
        menu_warn_message("This window needs multitask enabled");
        return;
    }

    zxvision_window *ventana;
    ventana=&zxvision_window_debug_3d_test;

    //IMPORTANTE! no crear ventana si ya existe. Esto hay que hacerlo en todas las ventanas que permiten background.
    //si no se hiciera, se crearia la misma ventana, y en la lista de ventanas activas , al redibujarse,
    //la primera ventana repetida apuntaria a la segunda, que es el mismo puntero, y redibujaria la misma, y se quedaria en bucle colgado
    //zxvision_delete_window_if_exists(ventana);

    //Crear ventana si no existe
    if (!zxvision_if_window_already_exists(ventana)) {
        int xventana,yventana,ancho_ventana,alto_ventana,is_minimized,is_maximized,ancho_antes_minimize,alto_antes_minimize;

        if (!util_find_window_geometry("debug3dtest",&xventana,&yventana,&ancho_ventana,&alto_ventana,&is_minimized,&is_maximized,&ancho_antes_minimize,&alto_antes_minimize)) {
            ancho_ventana=30;
            alto_ventana=20;

            xventana=menu_center_x()-ancho_ventana/2;
            yventana=menu_center_y()-alto_ventana/2;
        }


        zxvision_new_window_gn_cim(ventana,xventana,yventana,ancho_ventana,alto_ventana,ancho_ventana-1,alto_ventana-2,"3D Test",
            "debug3dtest",is_minimized,is_maximized,ancho_antes_minimize,alto_antes_minimize);

        ventana->can_be_backgrounded=1;

    }

    //Si ya existe, activar esta ventana
    else {
        zxvision_activate_this_window(ventana);
    }

    zxvision_draw_window(ventana);

    z80_byte tecla;


    int salir=0;


    menu_debug_3d_test_window=ventana; //Decimos que el overlay lo hace sobre la ventana que tenemos aqui


    //cambio overlay
    zxvision_set_window_overlay(ventana,menu_debug_3d_test_overlay);


    //Toda ventana que este listada en zxvision_known_window_names_array debe permitir poder salir desde aqui
    //Se sale despues de haber inicializado overlay y de cualquier otra variable que necesite el overlay
    if (zxvision_currently_restoring_windows_on_start) {
        //printf ("Saliendo de ventana ya que la estamos restaurando en startup\n");
        return;
    }

    do {


        tecla=zxvision_common_getkey_refresh();


        switch (tecla) {

            case 11:
                //arriba
                //blablabla
            break;



            //Salir con ESC
            case 2:
                salir=1;
            break;

            //O tecla background
            case 3:
                salir=1;
            break;
        }


    } while (salir==0);


    util_add_window_geometry_compact(ventana);

    if (tecla==3) {
        zxvision_message_put_window_background();
    }

    else {
        zxvision_destroy_window(ventana);
    }


}


