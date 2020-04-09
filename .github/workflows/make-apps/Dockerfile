FROM ubuntu
RUN apt-get -y update
RUN apt-get -y install g++
RUN apt-get -y install freeglut3-dev
RUN apt-get -y install cmake
ADD . /beatmup
COPY script.sh /script.sh
ENTRYPOINT ["/script.sh"]
