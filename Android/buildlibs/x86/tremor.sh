export NDK_PLATFORM_ROOT=$NDK_ROOT/platforms/android-9/arch-x86/usr
export NDK_ADDITIONAL_LIBRARY_PATH=$(pwd)/../../nativelibs/x86
export NDK_HOST_NAME=i686-android-linux

FILENAME=libtremor-svn

svn co http://svn.xiph.org/trunk/Tremor $FILENAME

cd $FILENAME

# remove call to ./configure from the script
head --lines=-1 autogen.sh > autogenmod.sh

chmod +x ./autogenmod.sh

./autogenmod.sh

rm ./config.sub
rm ./config.guess

wget http://git.savannah.gnu.org/cgit/config.git/plain/config.sub -O config.sub
wget http://git.savannah.gnu.org/cgit/config.git/plain/config.guess -O config.guess

LDFLAGS="-Wl,-rpath-link=$NDK_PLATFORM_ROOT/lib,-L$NDK_ADDITIONAL_LIBRARY_PATH/lib,-L$NDK_PLATFORM_ROOT/lib" CFLAGS="-nostdlib -I$NDK_ADDITIONAL_LIBRARY_PATH/include -I$NDK_PLATFORM_ROOT/include -DLITTLE_ENDIAN -DBYTE_ORDER=LITTLE_ENDIAN" LIBS="-lc" ./configure --host=$NDK_HOST_NAME --prefix=$NDK_ADDITIONAL_LIBRARY_PATH
make
make install
