Query Functions
===============

attachsql_query()
-----------------

.. c:function:: bool attachsql_query(attachsql_connect_t *con, size_t length, const char *statement, uint16_t parameter_count, attachsql_query_parameter_st *parameters, attachsql_error_t **error)

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
   :patam error: A pointer to a pointer of an error object which is created if an error occurs
   :returns: ``true`` on success or ``false`` on failure

   .. versionadded:: 0.1.0
   .. versionchanged:: 0.5.0

Example
^^^^^^^

See the following examples: :ref:`basic-query-example` and :ref:`escaped-query-example`

attachsql_query_close()
-----------------------

.. c:function:: void attachsql_query_close(attachsql_connect_t *con)

   Closes a query on a connection

   :param con: The connection object the query is on

   .. versionadded:: 0.1.0

Example
^^^^^^^

.. code-block:: c

   attachsql_connect_t *con;

   // Connect, send a query and process results
   ...
   attachsql_query_close(con);

.. seealso:: :ref:`basic-query-example` example

attachsql_query_column_count()
------------------------------

.. c:function:: uint16_t attachsql_query_column_count(attachsql_connect_t *con)

   Returns the number of columns in a query result

   .. note:: at least the first row must be received to access this metadata

   :param con: The connection object the query is on
   :returns: The column count or ``0`` if there is no active query

   .. versionadded:: 0.1.0

Example
^^^^^^^

.. code-block:: c

   attachsql_connect_t *con;
   uint16_t columns;

   con= attachsql_connect_create("localhost", 3306, "test", "test", "testdb", NULL);
   attachsql_query(con, strlen(query), query, 0, NULL, &error);

   while ((ret != ATTACHSQL_RETURN_EOF) && (error == NULL))
   {
     ret= attachsql_connect_poll(con, &error);
     if (ret != ATTACHSQL_RETURN_ROW_READY)
     {
       continue;
     }
     columns= attachsql_query_column_count(con);
     ...


.. seealso:: :ref:`basic-query-example` example

attachsql_query_column_get()
----------------------------

.. c:function:: attachsql_query_column_st *attachsql_query_column_get(attachsql_connect_t *con, uint16_t column)

   Gets column information for a specified column from a given query

   .. note:: The resulting struct is free'd automatically when :c:func:`attachsql_query_close` is called.  The user should not attempt to free it.

   :param con: The connection object the query is on
   :param column: The column number to retrieve
   :returns: The column data or :c:type:`NULL` if there is no such column

   .. versionadded:: 0.1.0

Example
^^^^^^^

.. code-block:: c

   attachsql_connect_t *con;
   attachsql_query_column_st *column_data;

   con= attachsql_connect_create("localhost", 3306, "test", "test", "testdb", NULL);
   attachsql_query(con, strlen(query), query, 0, NULL, &error);
   // Poll here until first row in result is ready
   ...
   column_data= attachsql_query_column_get(con, 0);
   printf("Table: %s, Column: %s\n", column_data->table, column_data->column);


attachsql_query_row_get()
-------------------------

.. c:function:: attachsql_query_row_st *attachsql_query_row_get(attachsql_connect_t *con, attachsql_error_t **error)

   Retrieves row data from a query.  Should be called when :c:func:`attachsql_connect_poll` returns ``ATTACHSQL_RETURN_ROW_READY``.

   .. note::
      MySQL returns all row data for standard queries as char/binary, even the numerical data.

   .. warning::
      Do not use this function when using row buffering, it will return an error, instead use :c:func:`attachsql_query_buffer_row_get`

   :param con: The connection object the query is on
   :param error: A pointer to a pointer of an error object which is created if an error occurs
   :returns: An array of row data, the number of elements in the array can be found with :c:func:`attachsql_query_column_count`

   .. versionadded:: 0.1.0

Example
^^^^^^^

See :ref:`basic-query-example` example

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

Example
^^^^^^^

.. code-block:: c

   attachsql_connect_t *con;
   attachsql_error_t *error= NULL;
   // Connect and execute query
   ...
   while ((ret != ATTACHSQL_RETURN_EOF) && (error == NULL))
   {
     ret= attachsql_connect_poll(con, &error);
     if (ret != ATTACHSQL_RETURN_ROW_READY)
     {
       continue;
     }
     row= attachsql_query_row_get(con, &error);
     columns= attachsql_query_column_count(con);
     // Do something with the row
     ...
     attachsql_query_row_next(con);
   }

.. seealso:: :ref:`basic-query-example` example

attachsql_connection_last_insert_id()
-------------------------------------

.. c:function:: uint64_t attachsql_connection_last_insert_id(attachsql_connect_t *con)

   Returns the insert ID for the previous query (if applicable).

   :param con: The connection object the query was on
   :returns: The insert ID or ``0`` if there was none

   .. versionadded:: 0.1.0

Example
^^^^^^^

.. code-block:: c

   attachsql_connect_t *con;
   uint64_t insert_id;
   // Execute a query
   ...
   attachsql_query_close(con);

   insert_id= attachsql_connection_last_insert_id(con);

attachsql_query_affected_rows()
-------------------------------

.. c:function:: uint64_t attachsql_query_affected_rows(attachsql_connect_t *con)

   Returns the number of affected rows from an UPDATE query.

   :param con: The connection object the query was on
   :returns: The number of affected rows

   .. versionadded:: 0.1.0

Example
^^^^^^^

.. code-block:: c

   attachsql_connect_t *con;
   uint64_t affected_rows;
   // Execute a query
   ...

   affected_rows= attachsql_query_affected_rows(con);
   attachsql_query_close(con);

attachsql_query_warning_count()
-------------------------------

.. c:function:: uint32_t attachsql_query_warning_count(attachsql_connect_t *con)

   Returns the number of warnings for a query.

   :param con: The connection object the query was on
   :returns: The number of warnings for the query result

   .. versionadded:: 0.1.0

Example
^^^^^^^

.. code-block:: c

   attachsql_connect_t *con;
   uint32_t warning_count;
   // Execute a query
   ...

   warning_count= attachsql_query_warning_count(con);
   attachsql_query_close(con);


attachsql_query_info()
----------------------

.. c:function:: const char *attachsql_query_info(attachsql_connect_t *con)

   Returns a string of information on the previous query.

   .. note:: this is freed internally when the connection is destroyed and should not be freed by the application.

   :param con: The connection object the query was on
   :returns: A string of the info or :c:type:`NULL` if there is none

   .. versionadded:: 0.1.0

Example
^^^^^^^

.. code-block:: c

   attachsql_connect_t *con;
   const char *query_info;
   // Execute a query
   ...

   query_info= attachsql_query_info(con);
   printf("%s\n", query_info);
   attachsql_query_close(con);

attachsql_query_next_result()
-----------------------------

.. c:function:: attachsql_return_t attachsql_query_next_result(attachsql_connect_t *con)

   Checks to see if there is another result waiting and starts the process to receive the result if there is.  Once this has been run :c:func:`attachsql_connect_poll` can be used to retrieve the data as normal.

   :param con: The connection object the query was on
   :returns: ``ATTACHSQL_RETURN_PROCESSING`` for more results, ``ATTACHSQL_RETURN_EOF`` for no more results.

   .. versionadded:: 0.1.0

Example
^^^^^^^

.. code-block:: c

   attachsql_connect_t *con;
   attachsql_error_t *error= NULL;
   const char *data= "SHOW PROCESSLIST; SHOW PROCESSLIST";
   attachsql_return_t aret= ATTACHSQL_RETURN_NONE;
   attachsql_query_row_st *row;
   uint16_t columns, col;

   con= attachsql_connect_create("localhost", 3306, "test", "test", "", NULL);
   attachsql_connect_set_option(con, ATTACHSQL_OPTION_MULTI_STATEMENTS, NULL);
   attachsql_query(con, strlen(data), data, 0, NULL, &error);
   while(aret != ATTACHSQL_RETURN_EOF)
   {
     aret= attachsql_connect_poll(con, &error);
     if (aret == ATTACHSQL_RETURN_ROW_READY)
     {
       row= attachsql_query_row_get(con, &error);
       columns= attachsql_query_column_count(con);
       for (col=0; col < columns; col++)
       {
         printf("Column: %d, Length: %zu, Data: %.*s ", col, row[col].length, (int)row[col].length, row[col].data);
       }
       attachsql_query_row_next(con);
       printf("\n");
     }
   }
   attachsql_query_close(con);
   aret= attachsql_query_next_result(con);
   while(aret != ATTACHSQL_RETURN_EOF)
   {
     aret= attachsql_connect_poll(con, &error);
     if (aret == ATTACHSQL_RETURN_ROW_READY)
     {
       row= attachsql_query_row_get(con, &error);
       columns= attachsql_query_column_count(con);
       for (col=0; col < columns; col++)
       {
         printf("Column: %d, Length: %zu, Data: %.*s ", col, row[col].length, (int)row[col].length, row[col].data);
       }
       attachsql_query_row_next(con);
       printf("\n");
     }
   }
   attachsql_query_close(con);
   attachsql_query_next_result(con);
   attachsql_connect_destroy(con);

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

Example
^^^^^^^

.. code-block:: c

   attachsql_connect_t *con= NULL;
   attachsql_error_t *error= NULL;
   const char *query= "SELECT * FROM t1 WHERE name='fred'";

   con= attachsql_connect_create("localhost", 3306, "test", "test", "testdb", NULL);
   attachsql_query_buffer_rows(con, true);
   attachsql_query(con, strlen(query), query, 0, NULL, &error);

.. seealso:: :ref:`buffered-results-example` example

attachsql_query_row_count()
---------------------------

.. c:function:: uint64_t attachsql_query_row_count(attachsql_connect_t *con)

   Returns the number of rows returned in a query when row buffering is enabled.  Will return 0 if row buffering is not enabled or the entire result set has not yet been retrieved.

   :param con: The connection the query was on
   :returns: The number of rows or 0 if not possible

   .. versionadded:: 0.2.0

Example
^^^^^^^

.. code-block:: c

   attachsql_connect_t *con;
   attachsql_error_t *error= NULL;
   const char *query= "SELECT * FROM t1 WHERE name='fred'";
   uint64_t row_count;

   con= attachsql_connect_create("localhost", 3306, "test", "test", "testdb", NULL);
   attachsql_query_buffer_rows(con, true);
   attachsql_query(con, strlen(query), query, 0, NULL, &error);

   while ((ret != ATTACHSQL_RETURN_EOF) && (error == NULL))
   {
     ret= attachsql_connect_poll(con, &error);
   }
   if (error != NULL)
   {
     printf("Error occurred: %s", attachsql_error_message(error));
     return 1;
   }
   row_count= attachsql_query_row_count(con);
   ...

.. seealso:: :ref:`buffered-results-example` example

attachsql_query_buffer_row_get()
--------------------------------

.. c:function:: attachsql_query_row_st *attachsql_query_buffer_row_get(attachsql_connect_t *con)

   Retrieves a row from a buffered result set

   :param con: The connection the query was on
   :returns: An array of row data, the number of elements in the array can be found with :c:func:`attachsql_query_column_count`

   .. versionadded:: 0.2.0

Example
^^^^^^^

See :ref:`buffered-results-example` example

attachsql_query_row_get_offset()
--------------------------------

.. c:function:: attachsql_query_row_get_offset(attachsql_connect_t *con, uint64_t row_number)

   Retrieves a row from a buffered result set as a specified row number.  This is the row in the order they were retrieved from the MySQL server, not related to any key.

   :param con: The connection the query was on
   :param row_number: The row number to retrieve
   :returns: An array of row data, the number of elements in the array can be found with :c:func:`attachsql_query_column_count`

   .. versionadded:: 0.2.0

Example
^^^^^^^

.. code-block:: c

   attachsql_connect_t *con= NULL;
   attachsql_error_t *error= NULL;
   const char *query= "SELECT * FROM t1 WHERE name='fred'";
   attachsql_return_t ret= ATTACHSQL_RETURN_NONE;
   attachsql_query_row_st *row;
   uint16_t columns, current_column;

   con= attachsql_connect_create("localhost", 3306, "test", "test", "testdb", NULL);
   attachsql_query_buffer_rows(con, true);
   attachsql_query(con, strlen(query), query, 0, NULL, &error);

   while ((ret != ATTACHSQL_RETURN_EOF) && (error == NULL))
   {
     ret= attachsql_connect_poll(con, &error);
   }
   if (error != NULL)
   {
     printf("Error occurred: %s", attachsql_error_message(error));
     return 1;
   }

   columns= attachsql_query_column_count(con);
   // Get row 2 (the third row in a result set)
   row= attachsql_query_buffer_row_get(con, 2);
   for (current_column= 0; current_column < columns; current_column++)
   {
     printf("%.*s ", (int)row[current_column].length, row[current_column].data);
   }
   printf("\n");
   attachsql_query_close(con);
   attachsql_connect_destroy(con);

