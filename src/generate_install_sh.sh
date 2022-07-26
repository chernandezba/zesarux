#!/usr/bin/env bash


INSTALLPREFIX=`cat compileoptions.h |grep INSTALL_PREFIX|cut -d '"' -f2`
EDITION_NAME_GAME=`cat cpu.h|grep EDITION_NAME_GAME|cut -d '"' -f2`


cat > install.sh << _EOF
#!/usr/bin/env bash

echo "Installing ZEsarUX under $INSTALLPREFIX ..."

mkdir -p $INSTALLPREFIX
mkdir -p $INSTALLPREFIX/bin
mkdir -p $INSTALLPREFIX/share/zesarux/

COMMONFILES="ACKNOWLEDGEMENTS LICENSE LICENSES_info licenses Changelog TODO* README HISTORY FEATURES FEATURES_es EXCLUSIVEFEATURES INSTALL INSTALLWINDOWS ALTERNATEROMS INCLUDEDTAPES DONATE DONORS FAQ *.odt mantransfev3.bin *.rom zxuno.flash tbblue.mmc speech_filters my_soft docs zesarux.mp3 zesarux.xcf $EDITION_NAME_GAME* bin_sprite_to_c.sh keyboard_*.bmp zesarux.pdf"

# -f to force overwrite already existing share files which are set to 444
cp -f -a \$COMMONFILES $INSTALLPREFIX/share/zesarux/
cp zesarux $INSTALLPREFIX/bin/


# Default permissions for files: read only
find $INSTALLPREFIX/share/zesarux/ -type f -print0| xargs -0 chmod 444

# Speech filters can be run
chmod +x $INSTALLPREFIX/share/zesarux/speech_filters/*

echo "Install done"

_EOF


chmod 755 install.sh

