#!/usr/bin/env bash


archs=$1

if [ -z $QT_DIR ]
then
  export QT_DIR=/opt/Qt5.1/5.1.0
fi

if [ -z $QT_DESK_DIR ]
then
  export QT_DESK_DIR=/opt/Qt5.1/5.1.0/gcc
fi

echo "####################### Build androiddeployqt #######################"
export QT_DESK_QMAKE=$QT_DESK_DIR/bin/qmake
export ANDROID_DEPLOY_QT_PATH=./platform/android/androiddeployqt

cd $ANDROID_DEPLOY_QT_PATH
$QT_DESK_QMAKE androiddeployqt.pro -r -spec linux-g++
make
RETVAL=$?
if [ $RETVAL -ne 0 ];
then
  echo "####################### Build androiddeployqt failed !!! #######################"
  exit $RETVAL
fi
cd -

export ANDROID_DEPLOY_QT=$ANDROID_DEPLOY_QT_PATH/androiddeployqt
export ANDROID_PACKAGE=org.webdriver.qt
export ANDROID_JAVA=/home/dmytro/Chromium/wd_working/platform/android/java/
export ANDROID_APP_NAME=AndroidWD
export MINISTRO="--deployment ministro"

export KEY_STORE=`pwd`/platform/android/androidwd.keystore
export ALIAS="qtwd"
export PASSWORD="123456"

if [ -z $archs ];
then
  archs="armv7 x86"
  modes="release release_dbg"
fi

if [ -z $ANDROID_NDK_ROOT];
then
  export ANDROID_NDK_ROOT=/opt/android/android-ndk-r8e
fi

if [ -z $ANDROID_SDK_ROOT]; 
then
  export ANDROID_SDK_ROOT=/opt/android/adt-bundle-linux-x86-20130522/sdk
fi

for arch in $archs
do
  if [ $arch = "x86" ];
  then
    export ANDROID_ARCH=x86
    export ANDROID_LIB_ARCH=x86
    export ANDROID_TOOLCHAIN_VERSION=4.7
    export ANDROID_TOOLCHAIN=x86-4.7
    export ANDROID_TOOL_PREFIX=i686-linux-android
    export ANDROID_TARGET=android-10
    export ANDROID_TOOLCHAIN_PREFIX=x86
    export ANDROID_NDK_HOST=linux-x86
  elif [ $arch = "armv7" ];
  then
    export ANDROID_ARCH=arm
    export ANDROID_LIB_ARCH=armeabi-v7a
    export ANDROID_TOOLCHAIN_VERSION=4.7
    export ANDROID_TOOLCHAIN=arm-linux-androideabi-4.7
    export ANDROID_TOOL_PREFIX=arm-linux-androideabi
    export ANDROID_TARGET=android-10
    export ANDROID_TOOLCHAIN_PREFIX=arm-linux-androideabi
    export ANDROID_NDK_HOST=linux-x86
  else
    echo "We don't support platform " $arch
    exit 1
  fi

  export QT_ROOT=$QT_DIR/android_$arch

  export PREBUILD=$ANDROID_NDK_ROOT/toolchains/$ANDROID_TOOLCHAIN/prebuilt/linux-x86
  export BIN=$PREBUILD/bin

  export CXX=$BIN/$ANDROID_TOOL_PREFIX-g++
  export CC=$BIN/$ANDROID_TOOL_PREFIX-gcc
  export LINK=$BIN/$ANDROID_TOOL_PREFIX-g++
  export AR=$BIN/$ANDROID_TOOL_PREFIX-ar


  export GYP_DEFINES="OS=android"

if [[ -z $modes ]];
then
  modes="release"
fi

  platform="android_"$arch

  for mode in $modes
  do 
    echo "####################### Build "$arch $mode" #######################"

    ./build.sh `pwd`/out $platform $mode

    RETVAL=$?
    if [ $RETVAL -ne 0 ];
    then
      echo "####################### Build "$arch" failed !!! #######################"
      exit $RETVAL
    fi
    
    dist_dir=`pwd`/out/bin/$platform/$mode
    export BINARY_PATH=$dist_dir/libWebDriver_noWebkit_android.so

    #clean android directory
    rm -rf $dist_dir/android

    
    echo "####################### Create apk "$arch" "$mode" #######################"
    mkdir -p $dist_dir/android/libs/$ANDROID_LIB_ARCH
    cp $BINARY_PATH $dist_dir/android/libs/$ANDROID_LIB_ARCH

    export ANDROID_JSON_CONFIG=$dist_dir/android/android_config.json 
    python generate_android_json.py

    if [ $mode = "release" ]
    then
      export RELEASE_ARG="--sign "$KEY_STORE" "$ALIAS" --storepass "$PASSWORD
      echo $RELEASE_ARG
    fi

    $ANDROID_DEPLOY_QT --output $dist_dir/android --input $ANDROID_JSON_CONFIG --verbose $MINISTRO $RELEASE_ARG 
    RETVAL=$?
    if [ $RETVAL -ne 0 ];
    then
      echo "####################### androiddeployqt error!!! #######################"
      exit $RETVAL
    fi

  done

done
