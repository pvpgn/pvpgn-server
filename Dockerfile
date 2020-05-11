FROM buildpack-deps:bionic

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        libc++-dev \
        cmake \
        liblua5.1-0-dev && \
    rm -rf /var/lib/apt/lists/*

ARG with-bnetd=true
ARG with-d2cs=true
ARG with-d2dbs=true
ARG with-lua=false

ARG with-mysql=false
ARG with-sqlite3=false
ARG with-pgsql=false
ARG with-odbc=false

ARG march-native=false

RUN git clone https://github.com/pvpgn/pvpgn-server.git && \
    cd pvpgn-server && \
    cmake -G "Unix Makefiles" -H./ -B./build \
          -D WITH_BNETD=$with-bnetd \
          -D WITH_D2CS=$with-d2cs \
          -D WITH_D2DBS=$with-d2dbs \
          -D WITH_LUA=$with-lua \
          -D WITH_MYSQL=$with-mysql \
          -D WITH_SQLITE3=$with-sqlite3 \
          -D WITH_PGSQL=$with-pgsql \
          -D WITH_ODBC=$with-odbc \
          -D MARCH_NATIVE=$march-native && \
    cd build && make -j$(nproc) && make install && \
    rm -rf /pvpgn-server

CMD bnetd -f
