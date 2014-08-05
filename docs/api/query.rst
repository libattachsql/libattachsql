Query Functions
===============

.. c:function:: attachsql_error_st *attachsql_query(attachsql_connect_t *con, size_t length, const char *statement, uint16_t parameter_count, attachsql_query_parameter_st *parameters)

   Asyncronusly sends a query to the MySQL server.  The query will not be sent until :c:func:`attachsql_connect_poll` is called.  The call to :c:func:`attachsql_connect_poll` should be repeated until an error has returned or ``ATTACHSQL_RETURN_ROW_READY``.

   Queries can use ``?`` placeholders and have those filled in using the ``parameter_count`` and ``parameters`` options.  This is so that data in queries can be escaped appropriately.  See :c:type:`attachsql_query_parameter_st` for more information.

   .. note::
      If the connection object has not yet connected to MySQL a blocking connection will be made upon the first query.

   :param con: The connection object to send the query on
   :param length: The length of the statement
   :param statement: The statement itself
   :param parameter_count: The number of parameters provided and therefore ``?`` placeholders in the query
   :param parameters: An array of parameter fillers
   :returns: An error structure or :c:type:`NULL` upon success

   .. versionadded:: 0.1.0

.. c:function:: void attachsql_query_close(attachsql_connect_t *con)

   Closes a query on a connection

   :param con: The connection object the query is on

   .. versionadded:: 0.1.0

.. c:function:: uint16_t attachsql_query_column_count(attachsql_connect_t *con)

   Returns the number of columns in a query result

   :param con: The connection object the query is on
   :returns: The column count or ``0`` if there is no active query

   .. versionadded:: 0.1.0

.. c:function:: attachsql_query_column_st *attachsql_query_column_get(attachsql_connect_t *con, uint16_t column)

   Gets column information for a specified column from a given query

   :param con: The connection object the query is on
   :param column: The column number to retrieve
   :returns: The column data or :c:type:`NULL` if there is no such column

   .. versionadded:: 0.1.0

.. c:function:: attachsql_query_row_st *attachsql_query_row_get(attachsql_connect_t *con, attachsql_error_st **error)

   Retrieves row data from a query.  Should be called when :c:func:`attachsql_connect_poll` returns ``ATTACHSQL_RETURN_ROW_READY``.

   .. note::
      MySQL returns all row data for standard queries as char/binary, even the numerical data.

   :param con: The connection object the query is on
   :param error: A pointer to a pointer of an error struct which is created if an error occurs
   :returns: An array of row data, the number of elements in the array can be found with :c:func:`attachsql_query_column_count`

   .. versionadded:: 0.1.0

.. c:function:: void attachsql_query_row_next(attachsql_connect_t *con)

   Start retrieving the next row in a query.  This will return immediately and calls to :c:func:`attachsql_connect_poll` will retrieve the row until ``ATTACHSQL_RETURN_ROW_READY`` is returned.  This should not be used for the first row, but every subsequent row.

   .. warning::
      Row data from the previous row should be copied at this point, calling this function will erase it.

   :param con: The connection object the query is on

   .. versionadded:: 0.1.0

.. c:function:: uint64_t attachsql_connection_last_insert_id(attachsql_connect_t *con)

   Returns the insert ID for the previous query (if applicable).

   :param con: The connection object the query was on
   :returns: The insert ID or ``0`` if there was none

   .. versionadded:: 0.1.0

.. c:function:: uint64_t attachsql_query_affected_rows(attachsql_connect_t *con)

   Returns the number of affected rows from an UPDATE query.

   :param con: The connection object the query was on
   :returns: The number of affected rows

   .. versionadded:: 0.1.0

.. c:function:: const char *attachsql_query_info(attachsql_connect_t *con)

   Returns a string of information on the previous query.

   :param con: The connection object the query was on
   :returns: A string of the info or :c:type:`NULL` if there is none

   .. versionadded:: 0.1.0
