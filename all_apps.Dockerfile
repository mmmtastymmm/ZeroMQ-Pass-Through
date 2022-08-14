FROM ubuntu:latest as build
ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update

RUN apt-get install -y \
    cmake \
    gcc \
    gdb \
    g++ \
    git \
    libboost-all-dev \
    libssl-dev \
    make \
    ninja-build \
    pkg-config

# Make the git packages directory
RUN mkdir -p packages
# ZeroMQ
RUN cd /packages/ &&\
    git clone --depth 1 --recursive https://github.com/zeromq/libzmq.git /packages/libzmq/ &&\
    cd libzmq/ &&\
    mkdir build &&\
    cd build/ &&\
    cmake -G Ninja .. &&\
    cmake --build . --target all -- install -j $(nproc)

RUN cd /packages/ &&\
    git clone --depth 1 --recursive https://github.com/zeromq/cppzmq.git /packages/cppzmq/ &&\
    cd cppzmq/ &&\
    mkdir build &&\
    cd build/ &&\
    cmake -G Ninja -DCPPZMQ_BUILD_TESTS=OFF .. &&\
    cmake --build . --target all -- install -j $(nproc)

RUN \
    git clone --depth 1 https://github.com/catchorg/Catch2.git && \
    cd Catch2 && \
    cmake -G Ninja -Bbuild -H. -DBUILD_TESTING=OFF && \
    cmake --build build/ --target install && \
    cd .. && \
    rm -rf Catch2


COPY . /project
WORKDIR /project
RUN mkdir build && cd build && cmake -G Ninja .. && cmake --build . --target all -j $(nproc)

FROM ubuntu:latest as release-base
# copy all libs used by the program into the new image.
COPY --from=build /usr/local/lib/libzmq.so* /usr/local/lib/

FROM release-base as release-tests

COPY --from=build /project/build/tests /
ENTRYPOINT ["/tests"]

FROM release-base as release-publisher

# Copy in the executable
COPY --from=build /project/build/zmq_publisher /

ENTRYPOINT [ "/zmq_publisher" ]


FROM release-base as release-subscriber

# Copy in the executable
COPY --from=build /project/build/zmq_subscriber /

ENTRYPOINT [ "/zmq_subscriber" ]
