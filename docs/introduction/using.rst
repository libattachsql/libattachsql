API Basics
==========

If you are used to MySQL's own libmysqlclient or other connectors there are some very major differences with the way that libAttachSQL works.  This section will attempt to explain those differences.

Polling
-------

libAttachSQL is a non-blocking library.  That means that if there is no data from the network yet the API will return immediately instead of waiting for more data.

A typical usage pattern is:

.. code-block:: cpp

  attachsql_query(con, strlen(query), query, 0, NULL, &error);

  while ((ret != ATTACHSQL_RETURN_EOF) && (error == NULL))
  {
    ret= attachsql_connect_poll(con, &error);
    if (ret != ATTACHSQL_RETURN_ROW_READY)
    {
      continue;
    }
    row= attachsql_query_row_get(con, &error);
    columns= attachsql_query_column_count(con);
    ...
    attachsql_query_row_next(con);
  }

The query may not actually be sent on a network when :c:func:`attachsql_query` is called.  In fact the connection to the server may not even happen at this point if this is the first query on the server.

Most of the magic happens with :c:func:`attachsql_connect_poll`.  This polls to see if there is more data to be sent/received on the network and will do some very basic processing of any data it has received.  It will likely take several polls before there is row data returned from a query in this scenario.

This access method is very useful for many connections on a single thread which can be polled in a loop.  This is one typical pattern for modern scaling web services.

There is a semi-blocking mode which can be enabled with the following before connecting:

.. code-block:: cpp

   attachsql_connect_set_option(con, ATTACHSQL_OPTION_SEMI_BLOCKING, NULL);

The semi-blocking mode will wait until there is some data, but this may not be a full row of data.  This improves performance for one-connection-per-thread scenarios where there are many threads.

Error Handling
--------------

The library uses a pointer-to-a-pointer in a function call to set errors.  This has several advantages such as error allocation is handled by the library and can be independent of the connection functions.

The following is an example of how to use it:

.. code-block:: cpp

   attachsql_error_t *error= NULL;

   ...

   attachsql_connect(con, &error);
   if (error)
   {
     printf("Error: %s", attachsql_error_message(error));
     attachsql_error_free(error);
   }

So, if an error occurs the ``error`` pointer will no longer point to :c:type:`NULL` but will point to a new error object.  Once you are finished with this error it should be cleaned up with :c:func:`attachsql_error_free`.

Alternatively if you don't need error handling:

.. code-block:: cpp

   attachsql_connect(con, NULL);

Internally the library will see that a :c:type:`NULL` has been passed to it and disable allocating the error.
