#
# This image is use to build the Android app.
# If /opt/android-build folder is mounted, the app installation package is copied to that folder.
#

FROM gradle:8.1.1-jdk17
USER root

RUN apt update && apt -y install ninja-build

# get Android SDK
ENV ANDROID_HOME /opt/android
RUN mkdir -p ${ANDROID_HOME} && \
    cd ${ANDROID_HOME} && \
    wget -q https://dl.google.com/android/repository/commandlinetools-linux-10406996_latest.zip -O android_tools.zip && \
    unzip android_tools.zip && \
    rm android_tools.zip

# add things to PATH
ENV PATH ${PATH}:${ANDROID_HOME}/tools/bin:${ANDROID_HOME}/platform-tools:${ANDROID_HOME}/cmdline-tools/bin

# accept licenses
RUN yes | sdkmanager --sdk_root=${ANDROID_HOME} --licenses

# install NDK, platform matching "minSdkVersion" and CMake
ENV NDK_VERSION 23.0.7599858
ENV MIN_SDK_VERSION 26
RUN sdkmanager --sdk_root=${ANDROID_HOME} --update
RUN sdkmanager --sdk_root=${ANDROID_HOME} "ndk;${NDK_VERSION}" "platforms;android-${MIN_SDK_VERSION}" "cmake;3.18.1"
ENV ANDROID_NDK_HOME ${ANDROID_HOME}/ndk/${NDK_VERSION}

# add source code
ADD . /opt/beatmup

# build app
ENTRYPOINT cd /opt/beatmup/android/app &&\
    gradle clean assembleDebug &&\
    if [ -d "/opt/android-build" ]; then mv -v build/outputs/apk/debug/app-debug.apk /opt/android-build/beatmupApp.apk; fi
