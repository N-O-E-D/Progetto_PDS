#! /bin/bash
# Boost installation
sudo apt-get install libboost-all-dev
#Openssl installation
gunzip openssl-1.1.1d.tar.gz
tar xvf openssl-1.1.1d.tar
cd openssl-1.1.1d
./config
make
make test   # this command is optional
make install

