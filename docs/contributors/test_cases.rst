Writing Test Cases
==================

libAttachSQL uses DDM4's YATL library to create unit tests, this provides macros to test if the outcomes are as expected.  Tests are designed to have a stock MySQL installation with the user ``test`` and password ``test`` defined by:

.. code-block:: mysql

   GRANT ALL ON *.* TO 'test'@'localhost' IDENTIFIED BY 'test';

The ``.travis.yml`` file sets this up automatically for Travis CI test runs.

Adding a Test Case
------------------

Test cases are basic C/C++ applications in the ``tests/`` directory.  To add a test case to the suite.  To add a test edit the ``include.am`` and add the following (replacing *query* with whatever the test is called):

.. code-block:: makefile

   t_query_SOURCES= tests/query.cc
   t_query_LDADD= src/libattachsql.la
   if BUILD_WIN32
   t_query_LDADD+= -lws2_32
   t_query_LDADD+= -lpsapi
   t_query_LDADD+= -liphlpapi
   endif
   check_PROGRAMS+= t/query
   noinst_PROGRAMS+= t/query


Using YATL
----------

YATL is needed to make sure conditions within the test program are met.  To include it in your test application, add the following:

.. code-block:: c

   #include <yatl/lite.h>

A test skip can be added if certain conditions aren't met:

.. code-block:: c

   SKIP_IF_(!is_connected, "Cannot connected to a database server")

There are many types of assert provided as can be seen in the next section, they can be used as follows:

.. code-block:: c

   ASSERT_EQ_(3, column, "Column count unexpected)
   ASSERT_FALSE_(false_condition, "False condition is not false")
   ASSERT_STREQ_("test", some_data, "Unexpected data")

YATL Library
------------

Parameter Definitions
^^^^^^^^^^^^^^^^^^^^^

.. c:type:: __expression

   An expression typically used in an ``if`` statement.

.. c:type:: __expected

   An expected variable or expression

.. c:type:: __actual

   The actual variable or expression

.. c:type:: __expected_str

   The expected string

.. c:type:: __actual_str

   The actual string to compare with

.. c:type:: __length

   The length of a string for comparison

Function Definitions
^^^^^^^^^^^^^^^^^^^^

.. c:macro:: SKIP_IF(__expression)

   Skips the test if the expression is true

.. c:macro:: SKIP_IF_(__expression, ...)

   Skips the test if the expression is true and uses a printf style format message

.. c:macro:: ASSERT_TRUE(__expression)

   Make sure the expression is true, test will fail if it is false

.. c:macro:: ASSERT_FALSE(__expression)

   Make sure the expression is false, test will fail if it is true

.. c:macro:: ASSERT_FALSE_(__expression, ...)

   Make sure the expression is false and use a printf style format message to fail if it is true.

.. c:macro:: ASSERT_NULL_(__expression, ...)

   Make sure the expression is :c:type:`NULL` and use a printf style format message to fail if it isn't.

.. c:macro:: ASSERT_NOT_NULL(__expression)

   Make sure the expression is not :c:type:`NULL`, test will fail if it is :c:type:`NULL`.

.. c:macro:: ASSERT_NOT_NULL_(__expression, ...)

   Make sure the expression is not :c:type:`NULL` and use a printf style format message to fail if it is.

.. c:macro:: ASSERT_TRUE_(__expression, ...)

   Make sure the expression is ``true`` and use a printf style format message to fail if it is not.

.. c:macro:: ASSERT_EQ(__expected, __actual)

   Make sure that one condition or variable matches another one.

   .. note::
      Not suitable for string matching

.. c:macro:: ASSERT_EQ_(__expected, __actual, ...)

   Make sure that one condition or variable matches another one and use a printf style format message to fail if the do not match.

   .. note::
      Not suitable for string matching

.. c:macro:: ASSERT_NEQ(__expected, __actual)

   Make sure that one condition or variable does not match another one.

   .. note::
      Not suitable for string matching

.. c:macro:: ASSERT_NEQ_(__expected, __actual, ...)

   Make sure that one condition or variable does not match another one and use a printf style format message to fail if they do match.

   .. note::
      Not suitable for string matching

.. c:macro:: ASSERT_STREQ(__expected_str, __actual_str)

   Compare one ``NUL`` terminated string with another one and fail if they do not match.

.. c:macro:: ASSERT_STREQ_(__expected_str, __actual_str, ...)

   Compare one ``NUL`` terminated string with another one and use a printf style format message to fail if they do not match.

.. c:macro:: ASSERT_STREQL_(__expected_str, __actual_str, __length, ...)

   Compare a string of :c:type:`__length` to another one and use a printf style format message to fail if they do not match.

   .. note::
      This is designed for use with non-NUL-terminated strings.

.. c:macro:: ASSERT_STRNE(__expected_str, __actual_str)

   Compare one ``NUL`` terminated string with another one and fail if they match.

.. c:macro:: ASSERT_STRNE_(__expected_str, __actual_str, ...)

   Compare one ``NUL`` terminated string with another one and use a printf style format message to fail if they match.


