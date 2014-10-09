Server-side Prepared Statement Functions
========================================

attachsql_statement_prepare()
-----------------------------

.. c:function:: bool attachsql_statement_prepare(attachsql_connect_t *con, size_t length, const char *statement, attachsql_error_t **error)

   Asyncronusly sends the statement to be prepared to the server, with ``?`` placeholders to be filled in with bound parameters.

   .. warning::
      The ``statement`` parameter needs to stay in scope until a result is received from the MySQL server.

   :param con: The connection object to prepare the statement on
   :param length: The length of the statement
   :param statement: The statement itself
   :param error: A pointer to a pointer of an error object which is created if an error occurs
   :returns: ``true`` on success or ``false`` on failure

   .. versionadded:: 0.4.0
   .. versionchanged:: 0.5.0

attachsql_statement_execute()
-----------------------------

.. c:function:: bool attachsql_statement_execute(attachsql_connect_t *con, attachsql_error_t **error)

   Executes a prepared statement

   :param con: The connection the statement is on
   :param error: A pointer to a pointer of an error object which is created if an error occurs
   :returns: ``true`` on success or ``false`` on failure

   .. versionadded:: 0.4.0
   .. versionchanged:: 0.5.0

attachsql_statement_reset()
---------------------------

.. c:function:: bool attachsql_statement_reset(attachsql_connect_t *con, attachsql_error_t **error)

   Resets a prepared statement.  Polling will be required to send the reset command.

   :param con: The connection the statement is on
   :param error: A pointer to a pointer of an error object which is created if an error occurs
   :returns: ``true`` on success or ``false`` on failure

   .. versionadded:: 0.4.0
   .. versionchanged:: 0.5.0

attachsql_statement_close()
---------------------------

.. c:function:: void attachsql_statement_close(attachsql_connect_t *con)

   Closes a prepared statement.  Polling will be required to send the close command.

   :param con: The connection the statement is on

   .. versionadded:: 0.4.0

attachsql_statement_send_long_data()
------------------------------------

.. c:function:: bool attachsql_statement_send_long_data(attachsql_connect_t *con, uint16_t param, size_t length, char *data, attachsql_error_t **error)

   Send a large amount of data for a given prepared statement parameter

   :param con: The connection the statement is on
   :param param: The parameter number (starting with 0)
   :param length: The length of the data to send
   :param data: The data to send
   :param error: A pointer to a pointer of an error object which is created if an error occurs
   :returns: ``true`` on success or ``false`` on failure

   .. versionadded:: 0.4.0
   .. versionchanged:: 0.5.0

attachsql_statement_get_param_count()
-------------------------------------

.. c:function:: uint16_t attachsql_statement_get_param_count(attachsql_connect_t *con)

   Returns the number of parameters for a prepared statement

   :param con: The connection the statement is on
   :returns: The number of parameters for the statement

   .. versionadded:: 0.4.0

attachsql_statement_set_int()
-----------------------------

.. c:function:: bool attachsql_statement_set_int(attachsql_connect_t *con, uint16_t param, int32_t value, attachsql_error_t **error)

   Sets a signed int value for a given parameter

   :param con: The connection the statement is on
   :param param: The parameter to set (starting at 0)
   :param value: The value for the parameter
   :param error: A pointer to a pointer of an error object which is created if an error occurs
   :returns: ``true`` on success or ``false`` on failure

   .. versionadded:: 0.4.0
   .. versionchanged:: 0.5.0

attachsql_statement_set_unsigned_int()
--------------------------------------

.. c:function:: bool attachsql_statement_set_unsigned_int(attachsql_connect_t *con, uint16_t param, uint32_t value, attachsql_error_t **error)

   Sets an unsigned int value for a given parameter

   :param con: The connection the statement is on
   :param param: The parameter to set (starting at 0)
   :param value: The value for the parameter
   :param error: A pointer to a pointer of an error object which is created if an error occurs
   :returns: ``true`` on success or ``false`` on failure

   .. versionadded:: 0.4.0
   .. versionchanged:: 0.5.0

attachsql_statement_set_bigint()
--------------------------------

.. c:function:: bool attachsql_statement_set_bigint(attachsql_connect_t *con, uint16_t param, int64_t value, attachsql_error_t **error)

   Sets a signed bigint value for a given parameter

   :param con: The connection the statement is on
   :param param: The parameter to set (starting at 0)
   :param value: The value for the parameter
   :param error: A pointer to a pointer of an error object which is created if an error occurs
   :returns: ``true`` on success or ``false`` on failure

   .. versionadded:: 0.4.0
   .. versionchanged:: 0.5.0

attachsql_statement_set_unsigned_bigint()
-----------------------------------------

.. c:function:: bool attachsql_statement_set_unsigned_bigint(attachsql_connect_t *con, uint16_t param, uint64_t value, attachsql_error_t **error)

   Sets an unsigned bigint value for a given parameter

   :param con: The connection the statement is on
   :param param: The parameter to set (starting at 0)
   :param value: The value for the parameter
   :param error: A pointer to a pointer of an error object which is created if an error occurs
   :returns: ``true`` on success or ``false`` on failure

   .. versionadded:: 0.4.0
   .. versionchanged:: 0.5.0

attachsql_statement_set_float()
-------------------------------

.. c:function:: bool attachsql_statement_set_float(attachsql_connect_t *con, uint16_t param, float value, attachsql_error_t **error)

   Sets a float value for a given parameter

   :param con: The connection the statement is on
   :param param: The parameter to set (starting at 0)
   :param value: The value for the parameter
   :param error: A pointer to a pointer of an error object which is created if an error occurs
   :returns: ``true`` on success or ``false`` on failure

   .. versionadded:: 0.4.0
   .. versionchanged:: 0.5.0

attachsql_statement_set_double()
--------------------------------

.. c:function:: bool attachsql_statement_set_double(attachsql_connect_t *con, uint16_t param, double value, attachsql_error_t **error)

   Sets a double value for a given parameter

   :param con: The connection the statement is on
   :param param: The parameter to set (starting at 0)
   :param value: The value for the parameter
   :param error: A pointer to a pointer of an error object which is created if an error occurs
   :returns: ``true`` on success or ``false`` on failure

   .. versionadded:: 0.4.0
   .. versionchanged:: 0.5.0

attachsql_statement_set_string()
--------------------------------

.. c:function:: bool attachsql_statement_set_string(attachsql_connect_t *con, uint16_t param, size_t length, const char *value, attachsql_error_t **error)

   Sets a string value for a given parameter

   :param con: The connection the statement is on
   :param param: The parameter to set (starting at 0)
   :param length: The length of the value
   :param value: The value for the parameter
   :param error: A pointer to a pointer of an error object which is created if an error occurs
   :returns: ``true`` on success or ``false`` on failure

   .. versionadded:: 0.4.0
   .. versionchanged:: 0.5.0

attachsql_statement_set_binary()
--------------------------------

.. c:function:: bool attachsql_statement_set_binary(attachsql_connect_t *con, uint16_t param, size_t length, const char *value, attachsql_error_t **error)

   Sets a binary value for a given parameter

   :param con: The connection the statement is on
   :param param: The parameter to set (starting at 0)
   :param length: The length of the value
   :param value: The value for the parameter
   :param error: A pointer to a pointer of an error object which is created if an error occurs
   :returns: ``true`` on success or ``false`` on failure

   .. versionadded:: 0.4.0
   .. versionchanged:: 0.5.0

attachsql_statement_set_null()
------------------------------

.. c:function:: bool attachsql_statement_set_null(attachsql_connect_t *con, uint16_t param, attachsql_error_t **error)

   Sets a ``NULL`` value for a given parameter

   :param con: The connection the statement is on
   :param param: The parameter to set (starting at 0)
   :param error: A pointer to a pointer of an error object which is created if an error occurs
   :returns: ``true`` on success or ``false`` on failure

   .. versionadded:: 0.4.0
   .. versionchanged:: 0.5.0

attachsql_statement_set_datetime()
----------------------------------

.. c:function:: bool attachsql_statement_set_datetime(attachsql_connect_t *con, uint16_t param, uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, uint32_t microsecond, attachsql_error_t **error)

   Sets a date and optional time for a given parameter

   :param con: The connection the statement is on
   :param param: The parameter to set (starting at 0)
   :param year: The year value for the date
   :param month: The month value for the date
   :param day: The day value for the date
   :param hour: The hour value for the time
   :param minute: The minute value for the time
   :param second: The second value for the time
   :param microsecond: The microsend value for the time
   :param error: A pointer to a pointer of an error object which is created if an error occurs
   :returns: ``true`` on success or ``false`` on failure

   .. versionadded:: 0.4.0
   .. versionchanged:: 0.5.0

attachsql_statement_set_time()
------------------------------

.. c:function:: bool attachsql_statement_set_time(attachsql_connect_t *con, uint16_t param, uint8_t hour, uint8_t minute, uint8_t second, uint32_t microsecond, bool is_negative, attachsql_error_t **error)

   Sets a time for a given parameter

   :param con: The connection the statement is on
   :param param: The parameter to set (starting at 0)
   :param hour: The hour value for the time
   :param minute: The minute value for the time
   :param second: The second value for the time
   :param microsecond: The microsend value for the time
   :param is_negative: Set to ``true`` for a negative time
   :param error: A pointer to a pointer of an error object which is created if an error occurs
   :returns: ``true`` on success or ``false`` on failure

   .. versionadded:: 0.4.0
   .. versionchanged:: 0.5.0

attachsql_statement_row_get()
-----------------------------

.. c:function:: bool attachsql_statement_row_get(attachsql_connect_t *con, attachsql_error_t **error)

   Retrieves row data from a prepared statement.  Should be called when :c:func:`attachsql_connect_poll` returns ``ATTACHSQL_RETURN_ROW_READY``

   :param con: The connection the statement is on
   :param error: A pointer to a pointer of an error object which is created if an error occurs
   :returns: ``true`` on success or ``false`` on failure

   .. versionadded:: 0.4.0
   .. versionchanged:: 0.5.0

attachsql_statement_get_int()
-----------------------------

.. c:function:: int32_t attachsql_statement_get_int(attachsql_connect_t *con, uint16_t column, attachsql_error_t **error)

   Retrieves a signed int value from a column of a result set.  Converting a non-integer where possible.  An error condition will occur if conversion is not possible.

   :param con: The connection the statement is on
   :param column: The column number to retrieve data from (starting at 0)
   :param error: A pointer to a pointer of an error object which is created if an error occurs
   :returns: The signed integer value (or 0 upon error)

   .. versionadded:: 0.4.0

attachsql_statement_get_unsigned_int()
--------------------------------------

.. c:function:: uint32_t attachsql_statement_get_unsigned_int(attachsql_connect_t *con, uint16_t column, attachsql_error_t **error)

   Retrieves an unsigned int value from a column of a result set.  Converting a non-integer where possible.  An error condition will occur if conversion is not possible.

   :param con: The connection the statement is on
   :param column: The column number to retrieve data from (starting at 0)
   :param error: A pointer to a pointer of an error object which is created if an error occurs
   :returns: The unsigned integer value (or 0 upon error)

   .. versionadded:: 0.4.0

attachsql_statement_get_bigint()
--------------------------------

.. c:function:: int64_t attachsql_statement_get_bigint(attachsql_connect_t *con, uint16_t column, attachsql_error_t **error)

   Retrieves a signed bigint value from a column of a result set.  Converting a non-integer where possible.  An error condition will occur if conversion is not possible.

   :param con: The connection the statement is on
   :param column: The column number to retrieve data from (starting at 0)
   :param error: A pointer to a pointer of an error object which is created if an error occurs
   :returns: The signed integer value (or 0 upon error)

   .. versionadded:: 0.4.0

attachsql_statement_get_unsigned_bigint()
-----------------------------------------

.. c:function:: uint64_t attachsql_statement_get_unsigned_bigint(attachsql_connect_t *con, uint16_t column, attachsql_error_t **error)

   Retrieves an unsigned bigint value from a column of a result set.  Converting a non-integer where possible.  An error condition will occur if conversion is not possible.

   :param con: The connection the statement is on
   :param column: The column number to retrieve data from (starting at 0)
   :param error: A pointer to a pointer of an error object which is created if an error occurs
   :returns: The unsigned integer value (or 0 upon error)

   .. versionadded:: 0.4.0

attachsql_statement_get_float()
-------------------------------

.. c:function:: float attachsql_statement_get_float(attachsql_connect_t *con, uint16_t column, attachsql_error_t **error)

   Retrieves a float value from a column of a result set.  Converting a non-float where possible.  An error condition will occur if conversion is not possible.

   :param con: The connection the statement is on
   :param column: The column number to retrieve data from (starting at 0)
   :param error: A pointer to a pointer of an error object which is created if an error occurs
   :returns: The float value (or 0 upon error)

   .. versionadded:: 0.4.0

attachsql_statement_get_double()
--------------------------------

.. c:function:: double attachsql_statement_get_double(attachsql_connect_t *con, uint16_t column, attachsql_error_t **error)

   Retrieves a double value from a column of a result set.  Converting a non-double where possible.  An error condition will occur if conversion is not possible.

   :param con: The connection the statement is on
   :param column: The column number to retrieve data from (starting at 0)
   :param error: A pointer to a pointer of an error object which is created if an error occurs
   :returns: The float value (or 0 upon error)

   .. versionadded:: 0.4.0

attachsql_statement_get_char()
------------------------------

.. c:function:: char *attachsql_statement_get_char(attachsql_connect_t *con, uint16_t column, size_t *length, attachsql_error_t **error)

   Retrieves a string/binary value from a column of a result set.  Converting number and date/time values where possible.  An error condition will occur if conversion is not possible.

   :param con: The connection the statement is on
   :param column: The column number to retrieve data from (starting at 0)
   :param error: A pointer to a pointer of an error object which is created if an error occurs
   :param length: An application allocated variable which the API will set the length of the return value into
   :returns: The string/binary value (or 0 upon error).  Not ``NUL`` terminated.

   .. versionadded:: 0.4.0

attachsql_statement_get_column_type()
-------------------------------------

.. c:function:: attachsql_column_type_t attachsql_statement_get_column_type(attachsql_connect_t *con, uint16_t column)

   Retrieves the data type for a given column in a prepared statement result set.

   :param con: The connection the statement is on
   :param column: The column number to get the data type of
   :returns: The column type or ``ATTACHSQL_COLUMN_TYPE_ERROR`` if any parameter is invalid

   .. versionadded:: 0.9.0

attachsql_statement_row_next()
------------------------------

.. c:function:: void attachsql_statement_row_next(attachsql_connect_t *con)

   Start retrieving the next row in a statement result set.  This will return immediately and calls to :c:func:`attachsql_connect_poll` will retrieve the row until ``ATTACHSQL_RETURN_ROW_READY`` is returned.  This should not be used for the first row, but every subsequent row.

   :param con: The connection the statement is on

   .. versionadded:: 0.9.0

attachsql_statement_get_column_count()
--------------------------------------

.. c:function:: uint16_t attachsql_statement_get_column_count(attachsql_connect_t *con)

   Returns the number of columns in a statement result

  :param con: The connection object the statement is on
  :returns: The column count or ``0`` if there is no active statement

  .. versionadded:: 0.9.0
