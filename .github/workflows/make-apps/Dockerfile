FROM gradle:4.10.0-jdk8

USER root

RUN apt-get -y update
RUN apt-get -y install g++ cmake ninja-build freeglut3-dev

# get Android SDK
ENV ANDROID_HOME /opt/android-sdk-linux
RUN mkdir -p ${ANDROID_HOME} && \
    cd ${ANDROID_HOME} && \
    wget -q https://dl.google.com/android/repository/sdk-tools-linux-3859397.zip -O android_tools.zip && \
    unzip android_tools.zip && \
    rm android_tools.zip

# add things to PATH
ENV PATH ${PATH}:${ANDROID_HOME}/tools:${ANDROID_HOME}/tools/bin:${ANDROID_HOME}/platform-tools
ENV ANDROID_VERSION 29
ENV ANDROID_BUILD_TOOLS_VERSION 27.0.3

# accept licenses
RUN yes | sdkmanager --licenses

# install tools and NDK
RUN $ANDROID_HOME/tools/bin/sdkmanager --update
RUN $ANDROID_HOME/tools/bin/sdkmanager "build-tools;${ANDROID_BUILD_TOOLS_VERSION}" \
    "platforms;android-${ANDROID_VERSION}" \
    "platform-tools" "ndk-bundle"

ADD . /beatmup
COPY script.sh /script.sh
ENTRYPOINT ["/script.sh"]
