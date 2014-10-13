sudo apt-get install -y python-sphinx libuv-dev

autoreconf -fi
./configure
make html
