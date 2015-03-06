Introduction to Contributing
============================

There are many ways to contribute to libAttachSQL.  Simply using it and creating an issue report when you found a bug or have a suggestion is a great contribution.  Documentation and code contribituions are also greatly appreciated.

Layout
------

The code for libAttachSQL in several parts:

+--------------------+-------------------------------+
| Directory          | Contents                      |
+====================+===============================+
| ``/src``           | The API source code           |
+--------------------+-------------------------------+
| ``/libattachsql2`` | The public API headers        |
+--------------------+-------------------------------+
| ``/tests/asql``    | Unit tests for the public API |
+--------------------+-------------------------------+
| ``/examples``      | Usage examples for the API    |
+--------------------+-------------------------------+

In each case if any files are added or removed the ``include.am`` file in that directory will require updating to reflect the change.

The protocol is kept separate so that in the future we can add additional APIs such as one similar to libmysqlclient.

Submitting to Github
--------------------

The main hub for the code is `GitHub <https://github.com/>`_.  The main tree is the `libAttachSQL GitHub tree <https://github.com/libattachsql/libattachsql>`_.  Anyone is welcome to submit pull requests or issues.  All requests will be considered and appropriate feedback given.

Continuous Integration
----------------------

libAttachSQL uses Travis CI for testing of every pull request prior to merge and the tree itself after a change.

The test configuration can be found in ``.travis.yml``.  It is setup to build using GCC and CLang on Ubuntu 12.04 hosts.  From there it will run the test suite and attempt to build HTML docs in nitpick mode with warnings set to errors.

Messages from the build system can be found on the `#libAttachSQL Freenode channel <irc://chat.freenode.net/libAttachSQL>`_.

Modifying the Build System
--------------------------

The build system is an m4 template system called `DDM4 <https://github.com/BrianAker/DDM4>`_.  If any changes are made to the scripts in ``m4`` directory the *serial* line will need incrementing in that file.  You should look for a line near the top that looks like:

.. code-block:: makefile

   #serial 7

Shared Library Version
^^^^^^^^^^^^^^^^^^^^^^

If any of the source code has changed please see ``LIBATTACHSQL_LIBRARY_VERSION`` in ``configure.ac``.  This gives rules on bumping the shared library versioning, not to be confused with the API public version which follows similar rules as described in the next section.

API Version
-----------

API versioning is stored in the ``VERSION`` file which is used by the build system to version the API and docs.  The versioning scheme follows the `Semantic Versioning Rules <http://semver.org/>`_.

Function Visibility
-------------------

The code and build system only exposes public API functions as usable symbols in the finished binary.  This cuts down on binary size quite significantly and also discourages use of undocumented functionality that was not designed for public use.

When adding a new API function to ``/libattachsql2`` always add ``ASQL_API`` on its own on the line above the function definition in the header.  This tells the build system this is an API function to be included.

License Headers
---------------

Please make sure before committing that all new files have appropriate license headers in.  Only add to the copyright of older headers if you have made a significant contribution to that file (25 - 50 lines is typically classed as significant for Open Souce projects).

