sudo apt-get install -y cppcheck

autoreconf -fi
./configure --enable-debug
cppcheck --quiet --enable=all --error-exitcode=1 src tests examples
