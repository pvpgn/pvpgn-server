FROM buildpack-deps:bionic

LABEL maintainer="Aperture <aperture147@gmail.com>"

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        libc++-dev \
        cmake \
        liblua5.1-0-dev && \
    rm -rf /var/lib/apt/lists/*

ARG with_bnetd=true
ARG with_d2cs=false
ARG with_d2dbs=false
ARG with_lua=false

ARG with_mysql=false
ARG with_sqlite3=false
ARG with_pgsql=false
ARG with_odbc=false

RUN git clone https://github.com/pvpgn/pvpgn-server.git && \
    cd pvpgn-server && \
    cmake -G "Unix Makefiles" -H./ -B./build \
          -DWITH_BNETD=${with_bnetd} \
          -DWITH_D2CS=${with_d2cs} \
          -DWITH_D2DBS=${with_d2dbs} \
          -DWITH_LUA=${with_lua} \
          -DWITH_MYSQL=${with_mysql} \
          -DWITH_SQLITE3=${with_sqlite3} \
          -DWITH_PGSQL=${with_pgsql} \
          -DWITH_ODBC=${with_odbc} && \
    cd build && make -j$(nproc) && make install && \
    rm -rf /pvpgn-server

CMD bnetd -f
