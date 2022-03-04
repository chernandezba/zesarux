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

//Archivo para generar funciones de push_valor

//
//INICIO Funciones de anidacion de NOMBRE_FUNCION mediante listas nested
//

//
//Punteros de funciones nested
//
//
//Para NOMBRE_FUNCION
//
//puntero a NOMBRE_FUNCION normal sin lista
void (*NOMBRE_FUNCION_no_nested) (z80_int valor,z80_byte tipo);
//puntero a primer item en lista de funciones de NOMBRE_FUNCION
//Si es NULL quiere decir que no existe lista
debug_nested_function_element *nested_list_NOMBRE_FUNCION;

//Funcion que gestiona las llamadas a los NOMBRE_FUNCIONs anidados
void NOMBRE_FUNCION_nested_handler(z80_int valor,z80_byte tipo)
{
	debug_nested_generic_handler(nested_list_NOMBRE_FUNCION,valor,tipo);
}


//Agregar un NOMBRE_FUNCION sobre el actual. Devuelve id de elemento de lista que la funcion que llama debe guardar
int debug_nested_NOMBRE_FUNCION_add(debug_nested_function funcion,char *nombre)
{
	//Si es el primero, crear elemento inicial y cambio de NOMBRE_FUNCION
	//if (nested_list_NOMBRE_FUNCION==NULL) {
	if (NOMBRE_FUNCION!=NOMBRE_FUNCION_nested_handler) {

		//printf ("Adding first NOMBRE_FUNCION to nested list\n");

        	//Creamos el inicial
	        nested_list_NOMBRE_FUNCION=debug_nested_alloc_element();

        	//Le metemos datos
        	//Primer identificador cero
	        debug_nested_fill(nested_list_NOMBRE_FUNCION,nombre,0,funcion, NULL, NULL);

		NOMBRE_FUNCION_no_nested=NOMBRE_FUNCION;
		NOMBRE_FUNCION=NOMBRE_FUNCION_nested_handler;

		return 0;
	}

	else {
		return debug_nested_add(nested_list_NOMBRE_FUNCION,nombre,funcion);
	}

}

void debug_nested_NOMBRE_FUNCION_del(int id)
{
	//Eliminar id
	//Si se elimina el primero de la lista, hay que reasignar puntero inicial y poner el no_nested a NULL

        //Si esta a NULL, no hacer nada
        if (NOMBRE_FUNCION!=NOMBRE_FUNCION_nested_handler) {
                debug_printf (VERBOSE_DEBUG,"NOMBRE_FUNCION nested is not enabled. Not deleting anything");
                return;
        }

	debug_nested_del(&nested_list_NOMBRE_FUNCION,id);

	if (nested_list_NOMBRE_FUNCION==NULL) {
		//lista vacia. asignar NOMBRE_FUNCION normal
		debug_printf (VERBOSE_DEBUG,"NOMBRE_FUNCION nested empty. Assign normal NOMBRE_FUNCION normal");
		NOMBRE_FUNCION=NOMBRE_FUNCION_no_nested;
	}		
}


//Llama a NOMBRE_FUNCION anterior, llamando por numero de id
void debug_nested_NOMBRE_FUNCION_call_previous(int id,z80_int valor,z80_byte tipo)
{

	//if (t_estados<20) printf ("Calling previous NOMBRE_FUNCION to id %d\n",id);

	//Si no hay anterior, quiere decir que hay que llamar al NOMBRE_FUNCION original
	//Ver si solo 1 elemento en la lista (esto acelera la busqueda)
	if (nested_list_NOMBRE_FUNCION->next==NULL) {
		//Solo un elemento. Llamar al NOMBRE_FUNCION original
		//if (t_estados<20) printf ("Only one element in list. Calling original function\n");
		NOMBRE_FUNCION_no_nested(valor,tipo);
		return;
	}

	else {
		debug_nested_function_element *actual;
		actual=debug_nested_find_id(nested_list_NOMBRE_FUNCION,id);
		//Si no existe id, error grave
		if (actual==NULL) cpu_panic ("NOMBRE_FUNCION id does not exist when searching previous on list");

		//Llamar a funcion de elemento anterior
		actual=actual->previous;

		//Hay anterior?
		if (actual==NULL) {
			//No hay anterior. Llamar al NOMBRE_FUNCION original
			//if (t_estados<20) printf ("No previous element in list. Calling original function\n");
	                NOMBRE_FUNCION_no_nested(valor,tipo);
			return;
        	}

		else {
			//Hay anterior. Llamarlo
			//if (t_estados<20) printf ("Calling previous element in list. Name: %s\n",actual->function_name);
			actual->funcion(valor,tipo); 
			return;
		}
	}
}


//
//FIN Funciones de anidacion de NOMBRE_FUNCION mediante listas nested
//
