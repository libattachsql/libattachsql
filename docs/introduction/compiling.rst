Compiling libAttachSQL
======================

libAttachSQL is designed to be compiled with GCC or CLang on a modern Linux distrubition or Mac OSX.

Prerequisites
-------------

libAttachSQL requires *libuv 0.10* to be installed.  For RedHat 6.x this is in the EPEL repositories so make sure those are enabled first.  In RedHat/Fedora this is installed using:

.. code-block:: bash

   sudo yum install libuv-devel

On a Mac we recommend using `Homebrew <http://brew.sh/>`_ to install this:

.. code-block:: bash

   brew install libuv

Ubuntu 12.04 does not have libuv in its repositories so LinuxJedi created a PPA for this dependency (simply a backport from 14.04) which is used for the Travis CI tests.  It can be found at: `<https://launchpad.net/~linuxjedi/+archive/ubuntu/ppa>`_.  To install it simply do:

.. code-block:: bash

   sudo apt-add-repository ppa:linuxjedi/ppa
   sudo apt-get update
   sudo apt-get install libuv-dev

More current versions of Ubuntu have libuv and it can be installed using:

.. code-block:: bash

   sudo apt-get install libuv-dev

Optional Libraries
------------------

You can optionally install zlib and OpenSSL development libraries to get compression and encryption protocol functions.  If these aren't installed then libAttachSQL will compile without these functions.  In RedHat/Fedora:

.. code-block:: bash

   sudo yum install zlib-devel openssl-devel

And Ubuntu:

.. code-block:: bash

   sudo apt-get install zlib1g-dev libssl-dev

Building
--------

To build libAttachSQL simply run::

   ./bootstrap.sh

This will generate required Makefiles and build the library ready for installation.

Aternatively on most systems you can use the following commands, this is especially useful for customising your install::

   autoreconf -fi
   ./configure
   make
   make install

Building for Windows
--------------------

It is possible to use Fedora Linux to cross-compile using MinGW for 64bit Windows targets.  Unfortunately the Ubuntu MinGW packages are broken at the time of writing so it is not possible to do this in Ubuntu yet.  The following steps will show you how to do this:

#. Install MinGW, the following packages are required::

      mingw64-gcc
      mingw64-gcc-c++

#. The libuv dependency needs to be built in MinGW for libAttachSQL to use.  Unfortunately there is no pre-package for this, so we have to build one (adjust version numbers as appropriate)::

      yumdownloader --source libuv
      rpm -Uvh libuv-0.10.27-1.fc20.src.rpm
      mkdir libuv-mingw
      cd libuv-mingw
      tar -zxvf ~/rpmbuild/SOURCES/libuv-v0.10.27.tar.gz
      cd libuv-v0.10.27
      PREFIX=x86_64-w64-mingw32- mingw64-make PLATFORM=mingw
      sudo cp libuv.a /usr/x86_64-w64-mingw32/sys-root/mingw/lib/
      sudo cp include/uv.h /usr/x86_64-w64-mingw32/sys-root/mingw/include/
      sudo cp -a include/uv-private/ /usr/x86_64-w64-mingw32/sys-root/mingw/include/

#. Execute the bootstrap file in MinGW mode::

      ./bootstrap.sh mingw

Testing
-------

libAttachSQL comes with a basic test suite which we recommend executing, especially if you are building for a new platform.

The test suite is automatically built along with the library and can be executed with ``make check`` or ``make distcheck``.  If you wish to test with valgrind you can use::

      TESTS_ENVIRONMENT="./libtool --mode=execute valgrind --error-exitcode=1 --leak-check=yes --track-fds=yes --malloc-fill=A5 --free-fill=DE" make check

Testing in Windows
------------------

The test suite can still be executed by using `wine <http://www.winehq.org/>`_:

#. Install wine.  On most platforms this is the ``wine`` package.

#. Setup wine to find the MinGW dlls as follows:

   #. Run wineconsole as follows to create the required wine home directory (and exit it afterwards)::

         wineconsole

   #. Open the ``~/.wine/system.reg`` file for editing
   #. Find the section called ``[System\\CurrentControlSet\\Control\\Session Manager\\Environment]``
   #. Under this find the ``PATH`` setting and add the path to MinGW's ``bin`` directory using the ``Z:`` drive.  For Fedora 20 64bit this makes the entry::

         "PATH"=str(2):"C:\\windows\\system32;C:\\windows;C:\\windows\\system32\\wbem;Z:\\usr\\x86_64-w64-mingw32\\sys-root\\mingw\\bin"

#. Run the test suite as follows::

      LOG_COMPILER=wine make check

Building RPMs
-------------

The build system for libAttachSQL has the capability to build RPMs.  To build RPMs simply do the following:

.. code-block:: bash

   ./bootstrap.sh
   make dist-rpm

