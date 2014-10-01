Version History
===============

Version 0.5
-----------

Version 0.5.0 Beta (Not yet released)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


* Remove unused network blocking code (`Issue #72 <https://github.com/libattachsql/libattachsql/issues/72>`_)
* Add support for ``NO_BACKSLASH_ESCAPES`` SQL mode (`Issue #63 <https://github.com/libattachsql/libattachsql/issues/63>`_)
* Fix wrong return state for empty result set (`Issue #83 <https://github.com/libattachsql/libattachsql/issues/83>`_)
* Fix crash when using multiple threads (`Issue #86 <https://github.com/libattachsql/libattachsql/issues/86>`_)
* Fix possible double-free on :c:func`attachsql_query_close (`Issue #85 <https://github.com/libattachsql/libattachsql/issues/85>`_)
* Add a semi-blocking mode for one connection per thread access (`Issue #89 <https://github.com/libattachsql/libattachsql/issues/89>`_)
* Attempt to error when file descriptors are exhausted (`Issue #92 <https://github.com/libattachsql/libattachsql/issues/92>`_)
* Add the start of troubleshooting documentation
* Changed error struct to a hidden error type (``attachsql_error_st`` has been replaced by :c:type:`attachsql_error_t`) and added access functions (:c:func:`attachsql_error_code`, :c:func:`attachsql_error_message` and :c:func:`attachsql_error_sqlstate`).

  .. warning::

     This is an API incompatible change over 0.4.0


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
