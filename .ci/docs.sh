sudo apt-get install -y python-sphinx

.ci/install-libuv.sh
autoreconf -fi
./configure
make html
