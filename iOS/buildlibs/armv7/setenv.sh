#!/bin/bash

unset DEVROOT SDKROOT CFLAGS CC LD CPP CXX AR AS NM CXXCPP RANLIB LDFLAGS CPPFLAGS CXXFLAGS

SDKVERSION="5.1"
export IOS_DEPLOY_TGT="3.1.3"
ARCH="armv7"

export DEVROOT=$(xcode-select -print-path)/Platforms/iPhoneOS.platform/Developer
export SDKROOT=$DEVROOT/SDKs/iPhoneOS$SDKVERSION.sdk
PREFIX=$(pwd)/../../nativelibs/$ARCH
export IOS_ADDITIONAL_LIBRARY_PATH=$PREFIX
export IOS_HOST_NAME=arm-apple-darwin*

  # export SDKVERSION=$(xcrun --sdk $SDK --show-sdk-version) # current version
  # export SDKROOT=$(xcrun --sdk $SDK --show-sdk-path) # current version

export AR=$(xcrun -sdk iphoneos --find ar)
export AS=$(xcrun -sdk iphoneos --find as)
export ASCPP=$(xcrun -sdk iphoneos --find as)
export CC=$(xcrun -sdk iphoneos --find gcc)
export CPP="$(xcrun -sdk iphoneos --find gcc) -E"
export CXX=$(xcrun -sdk iphoneos --find g++)
export CXXCPP="$(xcrun -sdk iphoneos --find g++) -E"
export LD=$(xcrun -sdk iphoneos --find ld)
export NM=$(xcrun -sdk iphoneos --find nm)
export RANLIB=$(xcrun -sdk iphoneos --find ranlib)
export STRIP=$(xcrun -sdk iphoneos --find strip)

export CPPFLAGS="-miphoneos-version-min=$SDKVERSION -arch $ARCH -isysroot $SDKROOT"
export CFLAGS="-arch $ARCH -miphoneos-version-min=$SDKVERSION -isysroot $SDKROOT -I$PREFIX/include"
export CXXFLAGS="-arch $ARCH -miphoneos-version-min=$SDKVERSION -isysroot $SDKROOT -I$PREFIX/include"
export LDFLAGS="-arch $ARCH -miphoneos-version-min=$SDKVERSION -isysroot $SDKROOT -L$PREFIX/lib"
export PKG_CONFIG_PATH="$PREFIX/lib/pkgconfig:$SDKROOT/usr/lib/pkgconfig:$DEVROOT/usr/lib/pkgconfig"
export ACLOCAL_PATH="$PREFIX/share/aclocal:$SDKROOT/usr/share/aclocal:$DEVROOT/usr/share/aclocal"

export IOS_CONFIGURE_FLAGS="--host=$IOS_HOST_NAME --prefix=$IOS_ADDITIONAL_LIBRARY_PATH --enable-static --disable-shared"
