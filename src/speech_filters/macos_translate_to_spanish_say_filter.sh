#!/usr/bin/env bash

# Script para enviar texto a speech y tambien traduccion
# Entrada: texto por stdin a traducir
# Salida: a partir de ZEsarUX 9.3, texto traducido, que se recoge desde ventana Debug console window.

LOGFILE=/tmp/traduccion.log
TEXTO=`cat`

echo "Linea" >> $LOGFILE
echo "$TEXTO" >> $LOGFILE
echo  >> $LOGFILE

#sleep 3
#echo "Pruebas" `date +%s`
#exit

#salir de momento
#echo "á é í ó ú ñ Ñ ñ ñ ñ ñ ñ ñ « » "
#exit

# si texto no vacio
if [ "$TEXTO" != "" ]; then
	#con translate shell
	#https://github.com/soimort/translate-shell
	#TRADUCIDO=`./trans -b "$TEXTO"`

	#con aws, traducir english a spanish
	#https://docs.aws.amazon.com/translate/latest/dg/get-started-cli.html
	#eliminamos las " del propio mensaje de retorno
	#quitamos la , del final
	TRADUCIDO=`aws translate translate-text --region eu-west-3 --source-language-code "en" --target-language-code "es" --text "$TEXTO"|grep TranslatedText|cut -d ':' -f2|sed 's/"//g'|sed 's/,$//'`


	echo "$TRADUCIDO" >> $LOGFILE

	#sacarlo por consola de vuelta a ZEsarUX. Requiere ZEsarUX version >= 9.3
	#el script se puede ejecutar en ZEsarUX version < 9.3, aunque para ver el texto traducido en ZEsarUX, requiere >=9.3
	# se puede simplemente hacer un tail -f de LOGFILE para verlo en console
	echo "$TRADUCIDO"

	# enviar texto a speech para escuchar la traduccion
	# de momento desactivo el envio a speech para que sea mas rapido
	echo "$TRADUCIDO"|say -f -


	#contar caracteres para saber si llegamos a un limite de la api y/o facturacion
	CARACTERES=`echo "$TRADUCIDO"|wc -c|awk '{printf $1}'`

	#sumar el total
	TOTALCARACTERES=0

	TOTALCHARSFILE=totalcaracteres.txt

	if [ -e $TOTALCHARSFILE ]; then
		. $TOTALCHARSFILE
	fi

	TOTALCARACTERES=$(($TOTALCARACTERES+CARACTERES))

	cat > $TOTALCHARSFILE << _EOF
TOTALCARACTERES=$TOTALCARACTERES
_EOF


fi

#echo "--fin--"  >> $LOGFILE
