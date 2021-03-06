Version History
===============

Version 2.0
-----------

Version 2.0.0 Beta (Not yet released)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

* Removed the IDLE return state, it was causing confusion and the EOF state did the same (`Issue #179 <https://github.com/libattachsql/libattachsql/issues/179>`_)
* Removed ``attachsql_statement_set_float()`` and ``attachsql_statement_get_float()``.  The double functions should be used instead (`Issue #181 <https://github.com/libattachsql/libattachsql/issues/181>`_)
* All `group` functions and types have been renamed to `pool`
* Configure now checks version numbers for dependencies
* libAttachSQL now requires libuv 1.4 or above
* Build system cleanups
* Callbacks are now in the connection pool rather than individual connections (`Issue #131 <https://github.com/libattachsql/libattachsql/issues/131>`_)


Version 1.0
-----------

Version 1.0.3 GA (2015-02-19)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

* Fixed compiling Mac universal in debug mode (`Issue #167 <https://github.com/libattachsql/libattachsql/issues/167>`_)
* Fixed test cases when MySQL is in a alternative timezone (`Issue #168 <https://github.com/libattachsql/libattachsql/issues/168>`_)
* Fixed self-signed certificates in test suite (`Issue #170 <https://github.com/libattachsql/libattachsql/issues/170>`_)
* Fixed Ubuntu 12.04 test verbosity breaking Travis CI (`Issue #174 <https://github.com/libattachsql/libattachsql/issues/174>`_)
* Fixed expired SSL certificates in test suite (`Issue #176 <https://github.com/libattachsql/libattachsql/issues/176>`_)
* Added many examples to the documentation (`Issue #172 <https://github.com/libattachsql/libattachsql/issues/172>`_)
* Slightly improved performance of prepared statement prepare phase (`Issue #166 <https://github.com/libattachsql/libattachsql/issues/166>`_)

Version 1.0.2 GA (2014-12-02)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

* Fixed compiler issues in Mac OS X 10.10 (`Issue #151 <https://github.com/libattachsql/libattachsql/issues/151>`_)
* Added universal (x86 and x86_64) binary support for Mac OS X (`Issue #153 <https://github.com/libattachsql/libattachsql/issues/153>`_)
* Fixed cleanup for distcheck (`Issue #155 <https://github.com/libattachsql/libattachsql/issues/155>`_)
* Fixed debuginfo RPM building (`Issue #157 <https://github.com/libattachsql/libattachsql/issues/157>`_)
* Fixed crash on statement cleanup with unused connection (`Issue #159 <https://github.com/libattachsql/libattachsql/issues/159>`_)
* Fixed incorrect column count check on prepared statement get_* functions (`Issue #162 <https://github.com/libattachsql/libattachsql/issues/162>`_)

Version 1.0.1 GA (2014-11-16)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

* Removed libuv calls that were not needed (`Issue #139 <https://github.com/libattachsql/libattachsql/issues/139>`_)
* Allow batched command sending which removes the need to stop the entire event loop when a command is sent (`Issue #140 <https://github.com/libattachsql/libattachsql/issues/140>`_)
* Add a next packet stack for further batch command support (`Issue #143 <https://github.com/libattachsql/libattachsql/issues/143>`_)
* Fix segfault when destroying an unused connection (`Issue #147 <https://github.com/libattachsql/libattachsql/issues/147>`_)

Version 1.0.0 GA (2014-10-21)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

* Fixed a lockup with many connections using connection groups (`Issue #124 <https://github.com/libattachsql/libattachsql/issues/124>`_)
* Fixed prepared statement with semi-block hangs (`Issue #128 <https://github.com/libattachsql/libattachsql/issues/128>`_)
* Fixed SSL+compression on the same connection (`Issue #127 <https://github.com/libattachsql/libattachsql/issues/127>`_)
* Fixed compression and SSL when used with Connection Groups (`Issue #125 <https://github.com/libattachsql/libattachsql/issues/125>`_)
* Improved error handling for connect and network errors (`Issue #133 <https://github.com/libattachsql/libattachsql/issues/133>`_)
* Some clang static analysis based code cleanups

Version 0.9
-----------

Version 0.9.0 RC (2014-10-14)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

* Added three new API calls for prepared statements (`Issue #113 <https://github.com/libattachsql/libattachsql/issues/113>`_)

  * :c:func:`attachsql_statement_get_column_type`
  * :c:func:`attachsql_statement_row_next`
  * :c:func:`attachsql_statement_get_column_count`

* Added new connection groups feature (`Issue #115 <https://github.com/libattachsql/libattachsql/issues/115>`_)

  * Changed :c:type:`attachsql_callback_fn` to include an error object. **THIS BREAKS API OVER 0.5.0**
  * Added ``attachsql_group_t``
  * Added ``attachsql_group_create()``
  * Added ``attachsql_group_destroy()``
  * Added ``attachsql_group_add_connection()``
  * Added ``attachsql_group_run()``
  * Packet processing is now part of the event loop
  * Added example

* Remove bootstrap.sh file, autoreconf should be used instead (`Issue #66 <https://github.com/libattachsql/libattachsql/issues/66>`_)
* Fix MinGW compiling issues (`Issue #118 <https://github.com/libattachsql/libattachsql/issues/118>`_)
* Cleaned up header files (`Issue #53 <https://github.com/libattachsql/libattachsql/issues/53>`_)

Version 0.5
-----------

Version 0.5.0 Beta (2014-10-08)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

* Removed unused network blocking code (`Issue #72 <https://github.com/libattachsql/libattachsql/issues/72>`_)
* Added support for ``NO_BACKSLASH_ESCAPES`` SQL mode (`Issue #63 <https://github.com/libattachsql/libattachsql/issues/63>`_)
* Fixed wrong return state for empty result set (`Issue #83 <https://github.com/libattachsql/libattachsql/issues/83>`_)
* Fixed crash when using multiple threads (`Issue #86 <https://github.com/libattachsql/libattachsql/issues/86>`_)
* Fixed possible double-free on :c:func:`attachsql_query_close` (`Issue #85 <https://github.com/libattachsql/libattachsql/issues/85>`_)
* Added a semi-blocking mode for one connection per thread access (`Issue #89 <https://github.com/libattachsql/libattachsql/issues/89>`_)
* Attempt to error when file descriptors are exhausted (`Issue #92 <https://github.com/libattachsql/libattachsql/issues/92>`_)
* Added the start of troubleshooting documentation
* Fix statement and statement_param test cases when a MySQL server is not present (`Issue #99 <https://github.com/libattachsql/libattachsql/issues/99>`_)
* Examples are now compiled to ensure API compatibility (`Issue #97 <https://github.com/libattachsql/libattachsql/issues/97>`_)
* Added basic API usage documentation (`Issue #96 <https://github.com/libattachsql/libattachsql/issues/96>`_)
* Moved some documentation around to make the sections clearer
* Fixed bad send/receive packets for :c:func:`attachsql_statement_close` (`Issue #102 <https://github.com/libattachsql/libattachsql/issues/102>`_ and `Issue #103 <https://github.com/libattachsql/libattachsql/issues/103>`_)
* Fixed ``./configure --enable-debug`` breaking linkage with C applications (`Issue #104 <https://github.com/libattachsql/libattachsql/issues/104>`_)
* Fixed bad internal state after :c:func:`attachsql_statement_close` (`Issue #106 <https://github.com/libattachsql/libattachsql/issues/106>`_)
* Fixed double-free on close of second call to :c:func:`attachsql_statement_close` (`Issue #107 <https://github.com/libattachsql/libattachsql/issues/107>`_)
* Fixed minor memory leaks in prepared statements (`Issue #109 <https://github.com/libattachsql/libattachsql/issues/109>`_)
* Added a prepared statement example (`Issue #80 <https://github.com/libattachsql/libattachsql/issues/80>`_)

Incompatible changes
""""""""""""""""""""

  .. warning::

     These changes are API incompatible with version 0.4.0 and below

* Changed error struct to a hidden error type (``attachsql_error_st`` has been replaced by :c:type:`attachsql_error_t`) and added access functions (:c:func:`attachsql_error_code`, :c:func:`attachsql_error_message` and :c:func:`attachsql_error_sqlstate`).
* Changed several functions for a more consistent error handling API.  These are:

  * :c:func:`attachsql_connect`
  * :c:func:`attachsql_connect_set_ssl`
  * :c:func:`attachsql_query`
  * :c:func:`attachsql_statement_prepare`
  * :c:func:`attachsql_statement_execute`
  * :c:func:`attachsql_statement_reset`
  * :c:func:`attachsql_statement_send_long_data`
  * :c:func:`attachsql_statement_set_int`
  * :c:func:`attachsql_statement_set_unsigned_int`
  * :c:func:`attachsql_statement_set_bigint`
  * :c:func:`attachsql_statement_set_unsigned_bigint`
  * ``attachsql_statement_set_float()``
  * :c:func:`attachsql_statement_set_double`
  * :c:func:`attachsql_statement_set_string`
  * :c:func:`attachsql_statement_set_binary`
  * :c:func:`attachsql_statement_set_null`
  * :c:func:`attachsql_statement_set_datetime`
  * :c:func:`attachsql_statement_set_time`
  * :c:func:`attachsql_statement_row_get`

Version 0.4
-----------

Version 0.4.0 Beta (2014-09-22)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

* Fixed compiler flag for GCC 4.9 (`Issue #65 <https://github.com/libattachsql/libattachsql/issues/65>`_)
* Fix docs building under Python 3.x
* Documentation fixes
* Fixed compiling in RedHat/CentOS 6.x
* Fixed parallel building
* Fixed SSL connect with no server support (`Issue #68 <https://github.com/libattachsql/libattachsql/issues/68>`_)
* Fixed possible buffer pointer breakage (`Issue #67 <https://github.com/libattachsql/libattachsql/issues/67>`_)
* Fixed potential crash on query with empty table/schema (`Issue #75 <https://github.com/libattachsql/libattachsql/issues/75>`_)
* Fixed double packet clear corrupts read buffer (`Issue #73 <https://github.com/libattachsql/libattachsql/issues/73>`_)
* Added prepared statement support (`Issue #32 <https://github.com/libattachsql/libattachsql/issues/32>`_)

Version 0.3
-----------

Version 0.3.0 Alpha (2014-09-01)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

* Fixed infinite loop memory eater for test run when MySQL server not present (`Issue #48 <https://github.com/libattachsql/libattachsql/issues/48>`_)
* Add UDS test case (`Issue #46 <https://github.com/libattachsql/libattachsql/issues/46>`_).  Note, the ``MYSQL_SOCK`` env variable needs to be set to use it.
* Add protocol compression support (`Issue #9 <https://github.com/libattachsql/libattachsql/issues/9>`_)
* Additional documentation (`Issue #44 <https://github.com/libattachsql/libattachsql/issues/44>`_, `Issue #58 <https://github.com/libattachsql/libattachsql/issues/58>`_ and `Issue #59 <https://github.com/libattachsql/libattachsql/issues/59>`_)
* Fixed case where multiple network writes could overwrite each other
* Add optional SSL support using OpenSSL (`Issue #10 <https://github.com/libattachsql/libattachsql/issues/10>`_)
* Fix valgrind reported problems (`Issue #55 <https://github.com/libattachsql/libattachsql/issues/55>`_)
* Fix cppcheck reported problems
* Fix 32bit CLang compiling
* Overhaul Travis configuration to test many more things

Version 0.2
-----------

Version 0.2.0 Alpha (2014-08-15)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

* Fixed 2 issues found by Coverity (`Issue #25 <https://github.com/libattachsql/libattachsql/issues/25>`_)
* Cleanup m4 files (`Issue #28 <https://github.com/libattachsql/libattachsql/issues/28>`_)
* Fixed in_query flag status (`Issue #31 <https://github.com/libattachsql/libattachsql/issues/31>`_)
* Add row buffering support (`Issue #8 <https://github.com/libattachsql/libattachsql/issues/8>`_)
* Connect on first query is now non-blocking (`Issue #39 <https://github.com/libattachsql/libattachsql/issues/39>`_)
* Memory alloc/free cleanups
* Default read buffer is 1MB instead of 1GB (big oops!)
* Additional documentation (`Issue #38 <https://github.com/libattachsql/libattachsql/issues/38>`_ and `Issue #30 <https://github.com/libattachsql/libattachsql/issues/30>`_)
* Fixed linking libAttachSQL to C applications

Version 0.1
-----------

Version 0.1.1 Alpha (Never released)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

* Fixed 2 issues found by Coverity (`Issue #25 <https://github.com/libattachsql/libattachsql/issues/25>`_)

Version 0.1.0 Alpha (2014-08-08)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

* Initial Alpha version
