FROM buildpack-deps:bionic

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        libc++-dev \
        cmake \
        liblua5.1-0-dev && \
    rm -rf /var/lib/apt/lists/*

RUN git clone https://github.com/pvpgn/pvpgn-server.git && \
    cd pvpgn-server && cmake -G "Unix Makefiles" -H./ -B./build && \
    cd build && make -j$(nproc) && make install && \
    rm -rf /pvpgn-server

CMD bnetd -f
