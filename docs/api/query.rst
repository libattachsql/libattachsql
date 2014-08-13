Query Functions
===============

attachsql_query()
-----------------

.. c:function:: attachsql_error_st *attachsql_query(attachsql_connect_t *con, size_t length, const char *statement, uint16_t parameter_count, attachsql_query_parameter_st *parameters)

   Asyncronusly sends a query to the MySQL server.  The query will not be sent until :c:func:`attachsql_connect_poll` is called.  The call to :c:func:`attachsql_connect_poll` should be repeated until an error has returned or ``ATTACHSQL_RETURN_ROW_READY``.  When buffered results are enabled with :c:func:`attachsql_query_buffer_rows` the polling will return ``ATTACHSQL_RETURN_EOF`` when ready.

   Queries can use ``?`` placeholders and have those filled in using the ``parameter_count`` and ``parameters`` options.  This is so that data in queries can be escaped appropriately.  See :c:type:`attachsql_query_parameter_st` for more information.

   .. note::
      If the connection object has not yet connected to MySQL a non-blocking connect to MySQL will be made first.

   .. warning::
      A copy of the statement parameter is only made when parameter_count > 0.  Otherwise the data the statement parameter points to needs to stay in scope until results are returned.

   :param con: The connection object to send the query on
   :param length: The length of the statement
   :param statement: The statement itself
   :param parameter_count: The number of parameters provided and therefore ``?`` placeholders in the query
   :param parameters: An array of parameter fillers
   :returns: An error structure or :c:type:`NULL` upon success

   .. versionadded:: 0.1.0

attachsql_query_close()
-----------------------

.. c:function:: void attachsql_query_close(attachsql_connect_t *con)

   Closes a query on a connection

   :param con: The connection object the query is on

   .. versionadded:: 0.1.0

attachsql_query_column_count()
------------------------------

.. c:function:: uint16_t attachsql_query_column_count(attachsql_connect_t *con)

   Returns the number of columns in a query result

   :param con: The connection object the query is on
   :returns: The column count or ``0`` if there is no active query

   .. versionadded:: 0.1.0

attachsql_query_column_get()
----------------------------

.. c:function:: attachsql_query_column_st *attachsql_query_column_get(attachsql_connect_t *con, uint16_t column)

   Gets column information for a specified column from a given query

   :param con: The connection object the query is on
   :param column: The column number to retrieve
   :returns: The column data or :c:type:`NULL` if there is no such column

   .. versionadded:: 0.1.0

attachsql_query_row_get()
-------------------------

.. c:function:: attachsql_query_row_st *attachsql_query_row_get(attachsql_connect_t *con, attachsql_error_st **error)

   Retrieves row data from a query.  Should be called when :c:func:`attachsql_connect_poll` returns ``ATTACHSQL_RETURN_ROW_READY``.

   .. note::
      MySQL returns all row data for standard queries as char/binary, even the numerical data.

   .. warning::
      Do not use this function when using row buffering, it will return an error, instead use :c:func:`attachsql_query_buffer_row_get`

   :param con: The connection object the query is on
   :param error: A pointer to a pointer of an error struct which is created if an error occurs
   :returns: An array of row data, the number of elements in the array can be found with :c:func:`attachsql_query_column_count`

   .. versionadded:: 0.1.0

attachsql_query_row_next()
--------------------------

.. c:function:: void attachsql_query_row_next(attachsql_connect_t *con)

   Start retrieving the next row in a query.  This will return immediately and calls to :c:func:`attachsql_connect_poll` will retrieve the row until ``ATTACHSQL_RETURN_ROW_READY`` is returned.  This should not be used for the first row, but every subsequent row.

   .. warning::
      Row data from the previous row should be copied at this point, calling this function will erase it.

   .. note::
      This function does nothing when row buffering is enabled.

   :param con: The connection object the query is on

   .. versionadded:: 0.1.0

attachsql_connection_last_insert_id()
-------------------------------------

.. c:function:: uint64_t attachsql_connection_last_insert_id(attachsql_connect_t *con)

   Returns the insert ID for the previous query (if applicable).

   :param con: The connection object the query was on
   :returns: The insert ID or ``0`` if there was none

   .. versionadded:: 0.1.0

attachsql_query_affected_rows()
-------------------------------

.. c:function:: uint64_t attachsql_query_affected_rows(attachsql_connect_t *con)

   Returns the number of affected rows from an UPDATE query.

   :param con: The connection object the query was on
   :returns: The number of affected rows

   .. versionadded:: 0.1.0

attachsql_query_info()
----------------------

.. c:function:: const char *attachsql_query_info(attachsql_connect_t *con)

   Returns a string of information on the previous query.

   :param con: The connection object the query was on
   :returns: A string of the info or :c:type:`NULL` if there is none

   .. versionadded:: 0.1.0

attachsql_query_next_result()
-----------------------------

.. c:function:: attachsql_return_t attachsql_query_next_result(attachsql_connect_t *con)

   Checks to see if there is another result waiting and starts the process to receive the result if there is.  Once this has been run :c:func:`attachsql_connect_poll` can be used to retrieve the data as normal.

   :param con: The connection object the query was on
   :returns: ``ATTACHSQL_RETURN_PROCESSING`` for more results, ``ATTACHSQL_RETURN_EOF`` for no more results.

   .. versionadded:: 0.1.0

attachsql_query_buffer_rows()
-----------------------------

.. c:function:: bool attachsql_query_buffer_rows(attachsql_connect_t *con, bool enable)

   Enable or disable row buffering mode

   .. warning::
      This cannot be enable whilst a query is executing and it will return ``false`` if you try this

   :param con: The connection the queries will be on
   :param enable: ``true`` to enable, ``false`` to disable
   :returns: Whether or not the status change was successful

   .. versionadded:: 0.2.0

attachsql_query_row_count()
---------------------------

.. c:function:: uint64_t attachsql_query_row_count(attachsql_connect_t *con)

   Returns the number of rows returned in a query when row buffering is enabled.  Will return 0 if row buffering is not enabled or the entire result set has not yet been retrieved.

   :param con: The connection the query was on
   :returns: The number of rows or 0 if not possible

   .. versionadded:: 0.2.0

attachsql_query_buffer_row_get()
--------------------------------

.. c:function:: attachsql_query_row_st *attachsql_query_buffer_row_get(attachsql_connect_t *con)

   Retrieves a row from a buffered result set

   :param con: The connection the query was on
   :returns: An array of row data, the number of elements in the array can be found with :c:func:`attachsql_query_column_count`

   .. versionadded:: 0.2.0

attachsql_query_row_get_offset()
--------------------------------

.. c:function:: attachsql_query_row_get_offset(attachsql_connect_t *con, uint64_t row_number)

   Retrieves a row from a buffered result set as a specified row number.  This is the row in the order they were retrieved from the MySQL server, not related to any key.

   :param con: The connection the query was on
   :param row_number: The row number to retrieve
   :returns: An array of row data, the number of elements in the array can be found with :c:func:`attachsql_query_column_count`

   .. versionadded:: 0.2.0
