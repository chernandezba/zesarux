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

docker-build-ubuntu() {
	docker build --tag=zesarux-ubuntu -f Dockerfile.ubuntu .
}

help() {
	echo "$0 [build|build-ubuntu|build-ubuntu-and-get-binary|codetests|run|run-curses|run-mac-xorg|run-xorg]"
}

if [ $# == 0 ]; then
	help
	exit 1
fi

case $1 in
	


	build)
		docker-build
	;;

	build-ubuntu)
		docker-build-ubuntu
	;;

	build-ubuntu-and-get-binary)
		echo "-----Building image"
		docker-build-ubuntu
		echo
		echo "-----Running tests"
		echo
		sleep 1
		docker run -it zesarux-ubuntu --codetests
		# TODO: get return code
		echo "-----Running image"
		docker run --name run-zesarux-ubuntu -it zesarux-ubuntu --vo stdout --ao null --exit-after 1
		echo
		echo "-----Getting binary file"
		docker cp run-zesarux-ubuntu:/zesarux/src/zesarux zesarux.ubuntu
		echo
		echo "-----Ubuntu Binary file is: "
		ls -lha zesarux.ubuntu
		docker rm run-zesarux-ubuntu
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

