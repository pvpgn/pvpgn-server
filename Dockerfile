FROM ubuntu:bionic

LABEL maintainer="Aperture <aperture147@gmail.com>"

ARG with_mysql=true
ARG with_sqlite3=false
ARG with_pgsql=false
ARG with_odbc=false
ARG with_lua=false

ARG with_bnetd=true
ARG with_d2cs=false
ARG with_d2dbs=false

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        git \
        netbase \
        ca-certificates \
        gcc g++ \
        libc6-dev libc6 \
        libc++-dev libc++1  \
        zlib1g-dev zlib1g \
        libcurl4-openssl-dev libcurl4 \
        cmake make \
        $(if ${with_mysql}; then echo "libmysqlclient-dev libmysqlclient20"; fi) \
        $(if ${with_sqlite3}; then echo "libsqlite3-dev libsqlite3-0"; fi) \
        $(if ${with_pgsql}; then echo "libpq-dev libpq5"; fi) \
        $(if ${with_pgsql}; then echo "unixodbc-dev libodbc1"; fi) \
        $(if ${with_lua}; then echo "liblua5.1-0-dev liblua5.1-0"; fi) && \
    git clone https://github.com/pvpgn/pvpgn-server.git && \
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
    apt-get autoremove --purge -y \
        git \
        gcc g++ \
        libc6-dev \
        libc++-dev \
        zlib1g-dev \
        libcurl4-openssl-dev \
        cmake make \
        $(if ${with_mysql}; then echo "libmysqlclient-dev "; fi) \
        $(if ${with_sqlite3}; then echo "libsqlite3-dev "; fi) \
        $(if ${with_pgsql}; then echo "libpq-dev "; fi) \
        $(if ${with_pgsql}; then echo "unixodbc-dev"; fi) \
        $(if ${with_lua}; then echo "liblua5.1-0-dev"; fi) && \
    rm -rf /var/lib/apt/lists/* /pvpgn-server && \
    apt-get clean

CMD bnetd -f
