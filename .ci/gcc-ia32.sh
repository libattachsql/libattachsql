sudo apt-get install -y libc6-dev libc6-dev:i386 libuv-dev:i386 zlib1g-dev:i386 build-essential g++ gcc libtool

sudo apt-get install -y gcc-multilib g++-multilib

export CFLAGS="-m32"
export CXXFLAGS="-m32"
export LDFLAGS="-m32"
export CPPFLAGS="-m32"
autoreconf -fi
./configure --enable-debug
make check
