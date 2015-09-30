#!/bin/bash

mkdir ~/tmp
cd ~/tmp
wget https://github.com/libuv/libuv/archive/v1.4.2.tar.gz
tar -zxvf v1.4.2.tar.gz
cd libuv-1.4.2
./autogen.sh
./configure --prefix=/usr
make
sudo make install
