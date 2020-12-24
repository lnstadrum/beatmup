#
# A minimum linux environment to build linux Beatmup binaries and Python package
#

FROM ubuntu:18.04

RUN apt update && apt -y install g++ cmake freeglut3-dev python3 python3-pip

ADD . /opt/beatmup
ENTRYPOINT cd /opt/beatmup && mkdir -p build && cd build &&\
    cmake -DUSE_GLX=ON -DDEBUG=ON .. && make -j`nproc` &&\
    PYTHONPATH=$(pwd) python3 -c "import beatmup; beatmup.say_hi()"

