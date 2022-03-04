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

//Archivo para generar funciones comunes de peek_byte y peek_byte_no_time nested

//
//INICIO Funciones de anidacion de peek_byte mediante listas nested
//



//
//Punteros de funciones nested
//
//
//Para peek_byte
//
//puntero a peek_byte normal sin lista
z80_byte (*peek_byte_no_nested) (z80_int dir);
//puntero a primer item en lista de funciones de peek_byte
//Si es NULL quiere decir que no existe lista
debug_nested_function_element *nested_list_peek_byte;




//Funcion que gestiona las llamadas a los peek_bytes anidados
z80_byte peek_byte_nested_handler(z80_int dir)
{
	return debug_nested_generic_handler(nested_list_peek_byte,dir,0);
}


//Agregar un peek_byte sobre el actual. Devuelve id de elemento de lista que la funcion que llama debe guardar
int debug_nested_peek_byte_add(debug_nested_function funcion,char *nombre)
{
	//Si es el primero, crear elemento inicial y cambio de peek_byte
	//if (nested_list_peek_byte==NULL) {
	if (peek_byte!=peek_byte_nested_handler) {

		//printf ("Adding first peek_byte to nested list\n");

        	//Creamos el inicial
	        nested_list_peek_byte=debug_nested_alloc_element();

        	//Le metemos datos
        	//Primer identificador cero
	        debug_nested_fill(nested_list_peek_byte,nombre,0,funcion, NULL, NULL);

		peek_byte_no_nested=peek_byte;
		peek_byte=peek_byte_nested_handler;

		return 0;
	}

	else {
		return debug_nested_add(nested_list_peek_byte,nombre,funcion);
	}

}

void debug_nested_peek_byte_del(int id)
{
	//Eliminar id
	//Si se elimina el primero de la lista, hay que reasignar puntero inicial y poner el no_nested a NULL

        //Si esta a NULL, no hacer nada
        if (peek_byte!=peek_byte_nested_handler) {
                debug_printf (VERBOSE_DEBUG,"peek_byte nested is not enabled. Not deleting anything");
                return;
        }


	debug_nested_del(&nested_list_peek_byte,id);

	if (nested_list_peek_byte==NULL) {
		//lista vacia. asignar peek_byte normal
		debug_printf (VERBOSE_DEBUG,"peek_byte nested empty. Assign normal peek_byte");
		peek_byte=peek_byte_no_nested;
	}		
}


//Llama a peek_byte anterior, llamando por numero de id
z80_byte debug_nested_peek_byte_call_previous(int id,z80_int dir)
{

	//if (t_estados<20) printf ("Calling previous peek_byte to id %d\n",id);

	//Si no hay anterior, quiere decir que hay que llamar al peek_byte original
	//Ver si solo 1 elemento en la lista (esto acelera la busqueda)
	if (nested_list_peek_byte->next==NULL) {
		//Solo un elemento. Llamar al peek_byte original
		//if (t_estados<20) printf ("Only one element in list. Calling original function\n");
		return peek_byte_no_nested(dir);
	}

	else {
		debug_nested_function_element *actual;
		actual=debug_nested_find_id(nested_list_peek_byte,id);
		//Si no existe id, error grave
		if (actual==NULL) cpu_panic ("peek_byte id does not exist when searching previous on list");

		//Llamar a funcion de elemento anterior
		actual=actual->previous;

		//Hay anterior?
		if (actual==NULL) {
			//No hay anterior. Llamar al peek_byte original
			//if (t_estados<20) printf ("No previous element in list. Calling original function\n");
	                return peek_byte_no_nested(dir);
        	}

		else {
			//Hay anterior. Llamarlo
			//if (t_estados<20) printf ("Calling previous element in list. Name: %s\n",actual->function_name);
			return actual->funcion(dir,0); 
		}
	}
}


//
//FIN Funciones de anidacion de peek_byte mediante listas nested
//
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

//Archivo para generar funciones comunes de peek_byte y peek_byte_no_time nested

//
//INICIO Funciones de anidacion de peek_byte_no_time mediante listas nested
//



//
//Punteros de funciones nested
//
//
//Para peek_byte_no_time
//
//puntero a peek_byte_no_time normal sin lista
z80_byte (*peek_byte_no_time_no_nested) (z80_int dir);
//puntero a primer item en lista de funciones de peek_byte_no_time
//Si es NULL quiere decir que no existe lista
debug_nested_function_element *nested_list_peek_byte_no_time;




//Funcion que gestiona las llamadas a los peek_byte_no_times anidados
z80_byte peek_byte_no_time_nested_handler(z80_int dir)
{
	return debug_nested_generic_handler(nested_list_peek_byte_no_time,dir,0);
}


//Agregar un peek_byte_no_time sobre el actual. Devuelve id de elemento de lista que la funcion que llama debe guardar
int debug_nested_peek_byte_no_time_add(debug_nested_function funcion,char *nombre)
{
	//Si es el primero, crear elemento inicial y cambio de peek_byte_no_time
	//if (nested_list_peek_byte_no_time==NULL) {
	if (peek_byte_no_time!=peek_byte_no_time_nested_handler) {

		//printf ("Adding first peek_byte_no_time to nested list\n");

        	//Creamos el inicial
	        nested_list_peek_byte_no_time=debug_nested_alloc_element();

        	//Le metemos datos
        	//Primer identificador cero
	        debug_nested_fill(nested_list_peek_byte_no_time,nombre,0,funcion, NULL, NULL);

		peek_byte_no_time_no_nested=peek_byte_no_time;
		peek_byte_no_time=peek_byte_no_time_nested_handler;

		return 0;
	}

	else {
		return debug_nested_add(nested_list_peek_byte_no_time,nombre,funcion);
	}

}

void debug_nested_peek_byte_no_time_del(int id)
{
	//Eliminar id
	//Si se elimina el primero de la lista, hay que reasignar puntero inicial y poner el no_nested a NULL

        //Si esta a NULL, no hacer nada
        if (peek_byte_no_time!=peek_byte_no_time_nested_handler) {
                debug_printf (VERBOSE_DEBUG,"peek_byte_no_time nested is not enabled. Not deleting anything");
                return;
        }


	debug_nested_del(&nested_list_peek_byte_no_time,id);

	if (nested_list_peek_byte_no_time==NULL) {
		//lista vacia. asignar peek_byte_no_time normal
		debug_printf (VERBOSE_DEBUG,"peek_byte_no_time nested empty. Assign normal peek_byte_no_time");
		peek_byte_no_time=peek_byte_no_time_no_nested;
	}		
}


//Llama a peek_byte_no_time anterior, llamando por numero de id
z80_byte debug_nested_peek_byte_no_time_call_previous(int id,z80_int dir)
{

	//if (t_estados<20) printf ("Calling previous peek_byte_no_time to id %d\n",id);

	//Si no hay anterior, quiere decir que hay que llamar al peek_byte_no_time original
	//Ver si solo 1 elemento en la lista (esto acelera la busqueda)
	if (nested_list_peek_byte_no_time->next==NULL) {
		//Solo un elemento. Llamar al peek_byte_no_time original
		//if (t_estados<20) printf ("Only one element in list. Calling original function\n");
		return peek_byte_no_time_no_nested(dir);
	}

	else {
		debug_nested_function_element *actual;
		actual=debug_nested_find_id(nested_list_peek_byte_no_time,id);
		//Si no existe id, error grave
		if (actual==NULL) cpu_panic ("peek_byte_no_time id does not exist when searching previous on list");

		//Llamar a funcion de elemento anterior
		actual=actual->previous;

		//Hay anterior?
		if (actual==NULL) {
			//No hay anterior. Llamar al peek_byte_no_time original
			//if (t_estados<20) printf ("No previous element in list. Calling original function\n");
	                return peek_byte_no_time_no_nested(dir);
        	}

		else {
			//Hay anterior. Llamarlo
			//if (t_estados<20) printf ("Calling previous element in list. Name: %s\n",actual->function_name);
			return actual->funcion(dir,0); 
		}
	}
}


//
//FIN Funciones de anidacion de peek_byte_no_time mediante listas nested
//
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

//Archivo para generar funciones comunes de peek_byte y peek_byte_no_time nested

//
//INICIO Funciones de anidacion de poke_byte mediante listas nested
//

//
//Punteros de funciones nested
//
//
//Para poke_byte
//
//puntero a poke_byte normal sin lista
void (*poke_byte_no_nested) (z80_int dir,z80_byte value);
//puntero a primer item en lista de funciones de poke_byte
//Si es NULL quiere decir que no existe lista
debug_nested_function_element *nested_list_poke_byte;

//Funcion que gestiona las llamadas a los poke_bytes anidados
void poke_byte_nested_handler(z80_int dir,z80_byte value)
{
	debug_nested_generic_handler(nested_list_poke_byte,dir,value);
}


//Agregar un poke_byte sobre el actual. Devuelve id de elemento de lista que la funcion que llama debe guardar
int debug_nested_poke_byte_add(debug_nested_function funcion,char *nombre)
{
	//Si es el primero, crear elemento inicial y cambio de poke_byte
	//if (nested_list_poke_byte==NULL) {
	if (poke_byte!=poke_byte_nested_handler) {

		//printf ("Adding first poke_byte to nested list\n");

        	//Creamos el inicial
	        nested_list_poke_byte=debug_nested_alloc_element();

        	//Le metemos datos
        	//Primer identificador cero
	        debug_nested_fill(nested_list_poke_byte,nombre,0,funcion, NULL, NULL);

		poke_byte_no_nested=poke_byte;
		poke_byte=poke_byte_nested_handler;

		return 0;
	}

	else {
		return debug_nested_add(nested_list_poke_byte,nombre,funcion);
	}

}

void debug_nested_poke_byte_del(int id)
{
	//Eliminar id
	//Si se elimina el primero de la lista, hay que reasignar puntero inicial y poner el no_nested a NULL

        //Si esta a NULL, no hacer nada
        if (poke_byte!=poke_byte_nested_handler) {
                debug_printf (VERBOSE_DEBUG,"poke_byte nested is not enabled. Not deleting anything");
                return;
        }

	debug_nested_del(&nested_list_poke_byte,id);

	if (nested_list_poke_byte==NULL) {
		//lista vacia. asignar poke_byte normal
		debug_printf (VERBOSE_DEBUG,"poke_byte nested empty. Assign normal poke_byte normal");
		poke_byte=poke_byte_no_nested;
	}		
}


//Llama a poke_byte anterior, llamando por numero de id
void debug_nested_poke_byte_call_previous(int id,z80_int dir,z80_byte value)
{

	//if (t_estados<20) printf ("Calling previous poke_byte to id %d\n",id);

	//Si no hay anterior, quiere decir que hay que llamar al poke_byte original
	//Ver si solo 1 elemento en la lista (esto acelera la busqueda)
	if (nested_list_poke_byte->next==NULL) {
		//Solo un elemento. Llamar al poke_byte original
		//if (t_estados<20) printf ("Only one element in list. Calling original function\n");
		poke_byte_no_nested(dir,value);
		return;
	}

	else {
		debug_nested_function_element *actual;
		actual=debug_nested_find_id(nested_list_poke_byte,id);
		//Si no existe id, error grave
		if (actual==NULL) cpu_panic ("poke_byte id does not exist when searching previous on list");

		//Llamar a funcion de elemento anterior
		actual=actual->previous;

		//Hay anterior?
		if (actual==NULL) {
			//No hay anterior. Llamar al poke_byte original
			//if (t_estados<20) printf ("No previous element in list. Calling original function\n");
	                poke_byte_no_nested(dir,value);
			return;
        	}

		else {
			//Hay anterior. Llamarlo
			//if (t_estados<20) printf ("Calling previous element in list. Name: %s\n",actual->function_name);
			actual->funcion(dir,value); 
			return;
		}
	}
}


//
//FIN Funciones de anidacion de poke_byte mediante listas nested
//
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

//Archivo para generar funciones comunes de peek_byte y peek_byte_no_time nested

//
//INICIO Funciones de anidacion de poke_byte_no_time mediante listas nested
//

//
//Punteros de funciones nested
//
//
//Para poke_byte_no_time
//
//puntero a poke_byte_no_time normal sin lista
void (*poke_byte_no_time_no_nested) (z80_int dir,z80_byte value);
//puntero a primer item en lista de funciones de poke_byte_no_time
//Si es NULL quiere decir que no existe lista
debug_nested_function_element *nested_list_poke_byte_no_time;

//Funcion que gestiona las llamadas a los poke_byte_no_times anidados
void poke_byte_no_time_nested_handler(z80_int dir,z80_byte value)
{
	debug_nested_generic_handler(nested_list_poke_byte_no_time,dir,value);
}


//Agregar un poke_byte_no_time sobre el actual. Devuelve id de elemento de lista que la funcion que llama debe guardar
int debug_nested_poke_byte_no_time_add(debug_nested_function funcion,char *nombre)
{
	//Si es el primero, crear elemento inicial y cambio de poke_byte_no_time
	//if (nested_list_poke_byte_no_time==NULL) {
	if (poke_byte_no_time!=poke_byte_no_time_nested_handler) {

		//printf ("Adding first poke_byte_no_time to nested list\n");

        	//Creamos el inicial
	        nested_list_poke_byte_no_time=debug_nested_alloc_element();

        	//Le metemos datos
        	//Primer identificador cero
	        debug_nested_fill(nested_list_poke_byte_no_time,nombre,0,funcion, NULL, NULL);

		poke_byte_no_time_no_nested=poke_byte_no_time;
		poke_byte_no_time=poke_byte_no_time_nested_handler;

		return 0;
	}

	else {
		return debug_nested_add(nested_list_poke_byte_no_time,nombre,funcion);
	}

}

void debug_nested_poke_byte_no_time_del(int id)
{
	//Eliminar id
	//Si se elimina el primero de la lista, hay que reasignar puntero inicial y poner el no_nested a NULL

        //Si esta a NULL, no hacer nada
        if (poke_byte_no_time!=poke_byte_no_time_nested_handler) {
                debug_printf (VERBOSE_DEBUG,"poke_byte_no_time nested is not enabled. Not deleting anything");
                return;
        }

	debug_nested_del(&nested_list_poke_byte_no_time,id);

	if (nested_list_poke_byte_no_time==NULL) {
		//lista vacia. asignar poke_byte_no_time normal
		debug_printf (VERBOSE_DEBUG,"poke_byte_no_time nested empty. Assign normal poke_byte_no_time normal");
		poke_byte_no_time=poke_byte_no_time_no_nested;
	}		
}


//Llama a poke_byte_no_time anterior, llamando por numero de id
void debug_nested_poke_byte_no_time_call_previous(int id,z80_int dir,z80_byte value)
{

	//if (t_estados<20) printf ("Calling previous poke_byte_no_time to id %d\n",id);

	//Si no hay anterior, quiere decir que hay que llamar al poke_byte_no_time original
	//Ver si solo 1 elemento en la lista (esto acelera la busqueda)
	if (nested_list_poke_byte_no_time->next==NULL) {
		//Solo un elemento. Llamar al poke_byte_no_time original
		//if (t_estados<20) printf ("Only one element in list. Calling original function\n");
		poke_byte_no_time_no_nested(dir,value);
		return;
	}

	else {
		debug_nested_function_element *actual;
		actual=debug_nested_find_id(nested_list_poke_byte_no_time,id);
		//Si no existe id, error grave
		if (actual==NULL) cpu_panic ("poke_byte_no_time id does not exist when searching previous on list");

		//Llamar a funcion de elemento anterior
		actual=actual->previous;

		//Hay anterior?
		if (actual==NULL) {
			//No hay anterior. Llamar al poke_byte_no_time original
			//if (t_estados<20) printf ("No previous element in list. Calling original function\n");
	                poke_byte_no_time_no_nested(dir,value);
			return;
        	}

		else {
			//Hay anterior. Llamarlo
			//if (t_estados<20) printf ("Calling previous element in list. Name: %s\n",actual->function_name);
			actual->funcion(dir,value); 
			return;
		}
	}
}


//
//FIN Funciones de anidacion de poke_byte_no_time mediante listas nested
//
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
//INICIO Funciones de anidacion de push_valor mediante listas nested
//

//
//Punteros de funciones nested
//
//
//Para push_valor
//
//puntero a push_valor normal sin lista
void (*push_valor_no_nested) (z80_int valor,z80_byte tipo);
//puntero a primer item en lista de funciones de push_valor
//Si es NULL quiere decir que no existe lista
debug_nested_function_element *nested_list_push_valor;

//Funcion que gestiona las llamadas a los push_valors anidados
void push_valor_nested_handler(z80_int valor,z80_byte tipo)
{
	debug_nested_generic_handler(nested_list_push_valor,valor,tipo);
}


//Agregar un push_valor sobre el actual. Devuelve id de elemento de lista que la funcion que llama debe guardar
int debug_nested_push_valor_add(debug_nested_function funcion,char *nombre)
{
	//Si es el primero, crear elemento inicial y cambio de push_valor
	//if (nested_list_push_valor==NULL) {
	if (push_valor!=push_valor_nested_handler) {

		//printf ("Adding first push_valor to nested list\n");

        	//Creamos el inicial
	        nested_list_push_valor=debug_nested_alloc_element();

        	//Le metemos datos
        	//Primer identificador cero
	        debug_nested_fill(nested_list_push_valor,nombre,0,funcion, NULL, NULL);

		push_valor_no_nested=push_valor;
		push_valor=push_valor_nested_handler;

		return 0;
	}

	else {
		return debug_nested_add(nested_list_push_valor,nombre,funcion);
	}

}

void debug_nested_push_valor_del(int id)
{
	//Eliminar id
	//Si se elimina el primero de la lista, hay que reasignar puntero inicial y poner el no_nested a NULL

        //Si esta a NULL, no hacer nada
        if (push_valor!=push_valor_nested_handler) {
                debug_printf (VERBOSE_DEBUG,"push_valor nested is not enabled. Not deleting anything");
                return;
        }

	debug_nested_del(&nested_list_push_valor,id);

	if (nested_list_push_valor==NULL) {
		//lista vacia. asignar push_valor normal
		debug_printf (VERBOSE_DEBUG,"push_valor nested empty. Assign normal push_valor normal");
		push_valor=push_valor_no_nested;
	}		
}


//Llama a push_valor anterior, llamando por numero de id
void debug_nested_push_valor_call_previous(int id,z80_int valor,z80_byte tipo)
{

	//if (t_estados<20) printf ("Calling previous push_valor to id %d\n",id);

	//Si no hay anterior, quiere decir que hay que llamar al push_valor original
	//Ver si solo 1 elemento en la lista (esto acelera la busqueda)
	if (nested_list_push_valor->next==NULL) {
		//Solo un elemento. Llamar al push_valor original
		//if (t_estados<20) printf ("Only one element in list. Calling original function\n");
		push_valor_no_nested(valor,tipo);
		return;
	}

	else {
		debug_nested_function_element *actual;
		actual=debug_nested_find_id(nested_list_push_valor,id);
		//Si no existe id, error grave
		if (actual==NULL) cpu_panic ("push_valor id does not exist when searching previous on list");

		//Llamar a funcion de elemento anterior
		actual=actual->previous;

		//Hay anterior?
		if (actual==NULL) {
			//No hay anterior. Llamar al push_valor original
			//if (t_estados<20) printf ("No previous element in list. Calling original function\n");
	                push_valor_no_nested(valor,tipo);
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
//FIN Funciones de anidacion de push_valor mediante listas nested
//
