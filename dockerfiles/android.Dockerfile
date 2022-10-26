#
# This image is use to build the Android app.
# If /opt/android-build folder is mounted, the app installation package is copied to that folder.
#

FROM gradle:4.10.0-jdk8
USER root

RUN apt update && apt -y install ninja-build

# get Android SDK
ENV ANDROID_HOME /opt/android-sdk-linux
RUN mkdir -p ${ANDROID_HOME} && \
    cd ${ANDROID_HOME} && \
    wget -q https://dl.google.com/android/repository/sdk-tools-linux-3859397.zip -O android_tools.zip && \
    unzip android_tools.zip && \
    rm android_tools.zip

# add things to PATH
ENV PATH ${PATH}:${ANDROID_HOME}/tools:${ANDROID_HOME}/tools/bin:${ANDROID_HOME}/platform-tools

# accept licenses
RUN yes | sdkmanager --licenses

# install NDK, platform matching "minSdkVersion" and CMake
ENV NDK_VERSION 18.1.5063045
ENV MIN_SDK_VERSION 21
RUN ${ANDROID_HOME}/tools/bin/sdkmanager --update
RUN ${ANDROID_HOME}/tools/bin/sdkmanager "ndk;${NDK_VERSION}" "platforms;android-${MIN_SDK_VERSION}" "cmake;3.10.2.4988404"
ENV ANDROID_NDK_HOME /opt/android-sdk-linux/ndk/${NDK_VERSION}

# add source code
ADD . /opt/beatmup

# build app
ENTRYPOINT cd /opt/beatmup/android/app &&\
    gradle clean assembleDebug &&\
    if [ -d "/opt/android-build" ]; then mv -v build/outputs/apk/debug/app-debug.apk /opt/android-build/beatmupApp.apk; fi
