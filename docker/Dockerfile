FROM ubuntu:xenial

RUN apt-get update \
    && apt-get install -y wget git cmake make build-essential zlib1g-dev liblua5.1-0-dev libmysqlclient-dev \
    && apt-get clean
    
COPY buildPvPGN.sh /opt/
RUN /opt/buildPvPGN.sh

CMD [ "/usr/local/sbin/bnetd", "-f" ] 