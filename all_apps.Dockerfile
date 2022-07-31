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
 git clone --recursive\
 https://github.com/zeromq/libzmq.git\
 /packages/libzmq/ &&\
 cd libzmq/ &&\
 mkdir build &&\
 cd build/ &&\
 cmake .. &&\
 make install -j $(nproc)

RUN cd /packages/ &&\
 git clone --recursive\
 https://github.com/zeromq/cppzmq.git\
 /packages/cppzmq/ &&\
 cd cppzmq/ &&\
 mkdir build &&\
 cd build/ &&\
 cmake -DCPPZMQ_BUILD_TESTS=OFF .. &&\
 make install -j $(nproc)

RUN \
    git clone https://github.com/catchorg/Catch2.git && \
    cd Catch2 && \
    cmake -Bbuild -H. -DBUILD_TESTING=OFF && \
    cmake --build build/ --target install


COPY . /project
WORKDIR /project
RUN mkdir build && cd build && cmake -G Ninja .. && cmake --build . --target all -j $(nproc)


FROM ubuntu:latest as release-publisher

COPY --from=build /project/build/zmq_publisher /

ENTRYPOINT [ "/zmq_publisher" ]


FROM ubuntu:latest as release-subscriber

# copy all libs used by the program into the new image.
COPY --from=build /usr/lib/x86_64-linux-gnu/libboost_program_options.so.* /usr/lib/x86_64-linux-gnu/
COPY --from=build /usr/lib/x86_64-linux-gnu/libboost_log.so.* /usr/lib/x86_64-linux-gnu/
COPY --from=build /usr/lib/x86_64-linux-gnu/libboost_thread.so.* /usr/lib/x86_64-linux-gnu/
COPY --from=build /usr/lib/x86_64-linux-gnu/libboost_filesystem.so.* /usr/lib/x86_64-linux-gnu/
COPY --from=build /usr/local/lib/libzmq.so* /usr/local/lib/

# Also copy in the executable
COPY --from=build /project/build/zmq_subscriber /

ENTRYPOINT [ "/zmq_subscriber" ]
