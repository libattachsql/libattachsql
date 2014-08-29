sudo apt-get install -y libc6-dev libc6-dev:i386 libuv-dev:i386 zlib1g-dev:i386

sudo apt-get install -y gcc-multilib g++-multilib

export CFLAGS="-m32"
export CXXFLAGS="-m32"
./bootstrap.sh
make check
