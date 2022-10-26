#
# A minimum linux environment to build linux Beatmup binaries and Python package and run few tests using OpenGL software emulation
#

FROM ubuntu:20.04

# install stuff
ENV DEBIAN_FRONTEND=noninteractive
RUN apt update && apt install -y \
        g++ cmake freeglut3-dev python3 python3-pip \
        libegl1-mesa-dev libgles2-mesa-dev xvfb

# add source code
ADD . /opt/beatmup

# compile beatmup
RUN cd /opt/beatmup && mkdir -p build && cd build &&\
    cmake -DUSE_EGL=ON -DGLES_VERSION=20 -DDEBUG=ON .. &&\
    make -j`nproc`

# run tests
RUN cd /opt/beatmup/build && xvfb-run ./Tests

# set python path
ENV PYTHONPATH=/opt/beatmup/build

# run python tests
RUN python3 -m pip install numpy
RUN cd /opt/beatmup/python/tests && xvfb-run -a python3 test.py
