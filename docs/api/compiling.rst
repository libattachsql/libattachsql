Compiling Your Application
==========================

Include Files
-------------

Make sure that your application includes the main libAttachSQL include as follows:

.. code-block:: c

   #include <libattachsql2/attachsql.h>

This will pull in all the libAttachSQL functions and constants you may require for your application.

Package Config
--------------

libAttachSQL includes a utility called ``libattachsql-config``.  This can give you all the options used to compile the library as well as the compiler options to link the library.  For a full list of what it providesrun:

.. code-block:: bash

   libattachsql-config --help

Compiling
---------

If the library is installed correctly in your Linux distribution compiling your application with libAttachSQL should be a simple matter of adding the library to link to as follows:

.. code-block:: bash

   gcc -o basic_query basic_query.c -lattachsql

And likewise for CLang:

.. code-block:: bash

   clang -o basic_query basic_query.c -lattachsql

When compiling and linking your application in MinGW to create a Windows application things are a little different.  The C++ compiler is needed and several dependencies need to be linked in too:

.. code-block:: bash

   x86_64-w64-mingw32-g++ -o escaped_query.exe escaped_query.c -lattachsql -luv -lws2_32 -lpsapi -liphlpapi -lssp
