#!/usr/bin/env bash


INSTALLPREFIX=`cat compileoptions.h |grep INSTALL_PREFIX|cut -d '"' -f2`
EDITION_NAME_GAME=`cat cpu.h|grep EDITION_NAME_GAME|cut -d '"' -f2`


cat > install.sh << _EOF
#!/usr/bin/env bash

echo "Installing ZEsarUX under $INSTALLPREFIX ..."

mkdir -p $INSTALLPREFIX
mkdir -p $INSTALLPREFIX/bin
mkdir -p $INSTALLPREFIX/share/zesarux/

COMMONFILES="ACKNOWLEDGEMENTS LICENSE LICENSES_info licenses Changelog Cambios TODO* README HISTORY FEATURES FEATURES_es EXCLUSIVEFEATURES INSTALL INSTALLWINDOWS IN_MEMORIAM* ALTERNATEROMS INCLUDEDTAPES DONATE DONORS FAQ *.odt mantransfev3.bin *.rom copiador.zsf duplitape2.zsf zxuno.flash tbblue.mmc pcw_8x_boot*dsk speech_filters text_image_filters my_soft docs zesarux.mp3 zesarux.xcf $EDITION_NAME_GAME* bin_sprite_to_c.sh keyboards z88_shortcuts.bmp zesarux.pdf"

# -f to force overwrite already existing share files which are set to 444
cp -f -a \$COMMONFILES $INSTALLPREFIX/share/zesarux/

cp zesarux $INSTALLPREFIX/bin/


# Default permissions for files: read only
find $INSTALLPREFIX/share/zesarux/ -type f -print0| xargs -0 chmod 444

#set permissions to all writable for disk images
chmod 666 $INSTALLPREFIX/share/zesarux/zxuno.flash
chmod 666 $INSTALLPREFIX/share/zesarux/tbblue.mmc

# Speech filters can be run
chmod +x $INSTALLPREFIX/share/zesarux/speech_filters/*

echo "Install done"

_EOF


chmod 755 install.sh

