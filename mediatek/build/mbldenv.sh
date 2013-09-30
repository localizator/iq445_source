#!/bin/bash
# ##########################################################
# ALPS(Android4.1 based) build environment profile setting
# ##########################################################
# Overwrite JAVA_HOME environment variable setting if already exists
JAVA_HOME=/usr/lib/jvm/java-6-sun
export JAVA_HOME

# Overwrite ANDROID_JAVA_HOME environment variable setting if already exists
ANDROID_JAVA_HOME=/usr/lib/jvm/java-6-sun
export ANDROID_JAVA_HOME

# Overwrite PATH environment setting for JDK & arm-eabi if already exists
PATH=/usr/lib/jvm/java-6-sun/bin:$PWD/prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.6/bin:$PATH
export PATH

# Use prebuilt host gcc
alias gcc=$PWD'/prebuilts/gcc/linux-x86/x86/i686-android-linux-4.4.3/bin/i686-android-linux-gcc'

# Add MediaTek developed Python libraries path into PYTHONPATH
if [ -z "$PYTHONPATH" ]; then
  PYTHONPATH=$PWD/mediatek/build/tools
else
  PYTHONPATH=$PWD/mediatek/build/tools:$PYTHONPATH
fi
export PYTHONPATH

