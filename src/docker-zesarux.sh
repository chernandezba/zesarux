#!/bin/bash

#    ZEsarUX  ZX Second-Emulator And Released for UniX
#    Copyright (C) 2013 Cesar Hernandez Bano
#
#    This file is part of ZEsarUX.
#
# Just a simple utility to test ZEsarUX in docker

docker-build() {
	docker build --tag=zesarux --progress plain .
}


docker-build-version() {
	VERSIONNAME=$1
	docker build --tag=zesarux.$VERSIONNAME --progress plain -f Dockerfile.$VERSIONNAME .
}


docker-build-and-get-binary() {
	VERSIONNAME=$1

                echo "-----Building image"
                sleep 1
                docker-build-version $VERSIONNAME

                echo
                echo "-----Running tests"
                sleep 1
                docker rm run-zesarux-$VERSIONNAME
                docker run --name run-zesarux-$VERSIONNAME -it --entrypoint /zesarux/src/automatic_tests.sh zesarux.$VERSIONNAME 
		if [ $? != 0 ]; then
			echo "codetests failed"
			exit 1
		fi

                echo
                echo "-----Running image"
                sleep 1
		docker rm run-zesarux-$VERSIONNAME
                docker run --name run-zesarux-$VERSIONNAME -it zesarux.$VERSIONNAME --vo stdout --ao null --exit-after 1

                echo
                echo "-----Getting executables and install file"
                sleep 1
                docker cp run-zesarux-$VERSIONNAME:/zesarux/src/zesarux zesarux.$VERSIONNAME
                docker cp run-zesarux-$VERSIONNAME:/zesarux/src/install.sh install.sh.$VERSIONNAME

                echo
                echo "-----$VERSIONNAME Binary file is: "
                ls -lha zesarux.$VERSIONNAME
}

help() {
	echo "$0 [build|build-version|build-version-and-get-binary|clean-cache|codetests|localrun|localsh|run|run-curses|run-mac-xorg|run-xorg]"
	echo
	echo "Development actions:"
	echo "--------------------"
	echo "build:        Builds a simple Debian version"
	echo "codetests:    Execute ZEsarUX codetests from a simple Debian version"
	echo "run:          Executes a simple Debian version using stdout driver"
	echo "run-curses:   Executes a simple Debian version using curses driver"
	echo "run-xorg:     Executes a simple Debian version using xwindows driver"
	echo "run-mac-xorg: Executes a simple Debian version using xwindows driver, for Mac"
	echo "localrun:     Builds and execute ZEsarUX using curses driver from a simple Debian version from this local directory, so do not get source from git"
	echo "localsh:      Similar to localrun but executes a Shell inside container"
	echo
	echo "Building actions:"
	echo "-----------------"
	echo "build-version:                Builds a release version"
	echo "build-version-and-get-binary: Same as build-version + get the tar.gz binary package"
	echo "Note: build-version and build-version-and-get-binary require a parameter, one of: [debian|fedora|ubuntu]"
	echo
	echo "Misc:"
	echo "-----"
	echo "clean-cache: Clean docker cache"
	echo "All docker actions get source code from git except localrun and localsh"
}

if [ $# == 0 ]; then
	help
	exit 1
fi

case $1 in
	
	clean-cache)
		docker builder prune
	;;

	build)
		docker-build
	;;

	build-version)
		if [ $# == 1 ]; then
			echo "A parameter version is required"
			exit 1
		fi

		docker-build-version $2
	;;

	build-version-and-get-binary)
		if [ $# == 1 ]; then
			echo "A parameter version is required"
			exit 1
		fi

		docker-build-and-get-binary $2
	;;

	run)
		docker-build
		docker run --name run-zesarux -it zesarux
	;;

	run-curses)
		docker-build
		docker run --name run-zesarux-curses -it zesarux --ao null --vo curses
	;;

	run-xorg)
		docker-build
		docker run --name run-zesarux-xorg -it -e DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix --user="$(id --user):$(id --group)" zesarux --disableshm --ao null
	;;

	run-mac-xorg)
		docker-build
		export HOSTNAME=`hostname`
		xhost +
		echo "Be sure that xquarz preference Allow connections from network clients is enabled"
		docker run --name run-zesarux-mac-xorg -it -e DISPLAY=${HOSTNAME}:0 -v /tmp/.X11-unix:/tmp/.X11-unix  zesarux --disableshm --ao null
	;;


	codetests)
		docker-build
		docker run --name run-zesarux-codetests -it zesarux --codetests
	;;

	localrun)
		docker build -f Dockerfile.local .  --progress plain --tag=zesarux.local
		docker run --name run-zesarux-localrun -it zesarux.local
	;;

	localsh)
		docker build -f Dockerfile.local .  --progress plain --tag=zesarux.local
		docker run --name run-zesarux-localsh -it --entrypoint /bin/bash zesarux.local
	;;


	*)
		help
	;;

esac

