#!/bin/bash

cd /home
git clone https://github.com/pvpgn/pvpgn-server.git
cd pvpgn-server
cmake -G "Unix Makefiles" -H./ -B./build
cd build
make
make install
rm -rf /home/pvpgn-server