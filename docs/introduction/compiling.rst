Compiling libAttachSQL
======================

libAttachSQL is designed to be compiled with GCC or CLang on a modern Linux distrubition or Mac OSX.

On a Mac you should have XCode along with XCode's console tools installed along with `Homebrew <http://brew.sh/>`_. You will need to following once homebrew is installed to get the proper dev environment:

.. code-block:: bash

   brew install autoconf automake libtool pkg-config

Prerequisites
-------------

libAttachSQL requires *libuv 1.x* to be installed. For Fedora 22 this can be installed using:

.. code-block:: bash

   sudo dnf install libuv-devel

On a Mac we recommend using Homebrew to install this:

.. code-block:: bash

   brew install libuv

For most other Linux operating systems version 0.10 or 0.11 is still in the repositories so this currently needs to be compiled from source:

.. code-block:: bash

   git clone git://github.com/libuv/libuv
   cd libuv
   ./autogen.sh
   ./configure --prefix=/usr
   make
   sudo make install

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

On most systems you can use the following commands, this is especially useful for customising your install::

   autoreconf -fi
   ./configure
   make
   make install

The build system will automatically detect how many processor cores you have (physicaly and virtual) and set the ``--jobs`` options of make accordingly.

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

#. Configure and build in MinGW mode::

      autoreconf -fi
      mingw64-configure --disable-shared --enable-static
      mingw64-make

With Arch Linux the MinGW sysroot to copy files to is ``/usr/x86_64-w64-mingw32/`` without the ``sys-root/mingw`` on the end and to compile::

   autoreconf -fi
   ./configure --prefix=/usr/x86_64-w64-mingw32/ --with-sysroot=/usr/x86_64-w64-mingw32/ --host=x86_64-w64-mingw32
   make

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

   autoreconf -fi
   ./configure
   make dist-rpm

.. note::
   The package ``redhat-rpm-config`` is required for building the RPM because this generates the debuginfo RPM.
