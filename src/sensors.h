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

#ifndef SENSORS_H
#define SENSORS_H

#define TOTAL_SENSORS 29

#define SENSORS_MAX_SHORT_NAME 32

#define SENSORS_MAX_LONG_NAME 100

struct s_sensor_item {
    //Nombre corto del sensor utilizado como identificador
    char short_name[SENSORS_MAX_SHORT_NAME];
    //Nombre largo del sensor
    char long_name[SENSORS_MAX_LONG_NAME];

    //nombre corto para display
    char display_short_name[20];

    int min_value;
    int max_value;

    //Umbrales de aviso de porcentaje (si excede o si menor que). Para el porcentaje se tiene en cuenta el rango entre min_value y max_value
    int upper_warning_perc;
    int lower_warning_perc;

    //Umbrales de aviso de valor (si excede o si menor que)
    int upper_warning_value;
    int lower_warning_value;


    //Funcion que retorna el valor del sensor
    //...
    int (*f_funcion_get_value)(int id);

    //Id para la funcion usado sobretodo en sensors del mismo tipo: por ejemplo ay_vol_chip0_chan_A, ay_vol_chip0_chan_B, el id sera 0,1, etc
    int id_parameter;


};

typedef struct s_sensor_item sensor_item;

extern sensor_item sensors_array[];

extern int sensor_find(char *short_name);
extern void sensor_list_print(void);
extern int sensor_get_value(char *short_name);
extern int sensor_get_value_by_id(int indice);
extern int sensor_get_percentaje_value(char *short_name);
extern int sensor_get_percentaje_value_by_id(int indice);

#endif
