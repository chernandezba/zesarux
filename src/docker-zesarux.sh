#!/bin/bash

#    ZEsarUX  ZX Second-Emulator And Released for UniX
#    Copyright (C) 2013 Cesar Hernandez Bano
#
#    This file is part of ZEsarUX.
#
# Just a simple utility to test ZEsarUX in docker

docker-build() {
	docker build --tag=zesarux .
}

help() {
	echo "$0 [build|codetests|run|run-curses|run-mac-xorg|run-xorg]"
}

if [ $# == 0 ]; then
	help
	exit 1
fi

case $1 in
	


	build)
		docker-build
	;;

	run)
		docker-build
		docker run -it zesarux
	;;

	run-curses)
		docker-build
		docker run -it zesarux --ao null --vo curses
	;;

	run-xorg)
		docker-build
		docker run -it -e DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix --user="$(id --user):$(id --group)" zesarux --disableshm --ao null
	;;

	run-mac-xorg)
		docker-build
		export HOSTNAME=`hostname`
		xhost +
		echo "Be sure that xquarz preference Allow connections from network clients is enabled"
		docker run -it -e DISPLAY=${HOSTNAME}:0 -v /tmp/.X11-unix:/tmp/.X11-unix  zesarux --disableshm --ao null
	;;


	codetests)
		docker-build
		docker run -it zesarux --codetests
	;;

	*)
		help
	;;

esac

