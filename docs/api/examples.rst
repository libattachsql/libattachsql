API Examples
============

.. _basic-query-example:

A Basic Query
-------------

This is a basic query example which can be found at ``examples/basic_query.c`` in the libAttachSQL source.

It will connect to a MySQL server with the username *test*, password *test* and default database *testdb*.  It will then execute the query:

.. code-block:: mysql

   SELECT * FROM t1 WHERE name='fred';

Source Code
^^^^^^^^^^^

.. literalinclude:: ../../examples/basic_query.c
   :language: c

Breaking it Down
^^^^^^^^^^^^^^^^

First we create the connection object, the connection does not happen at this stage but everything required for the connection is set up:

.. code-block:: c

   con= attachsql_connect_create("localhost", 3306, "test", "test", "testdb", NULL);

Next we send a query to the MySQL server, at this stage the query will only be put on the network buffer, it will not be sent immediately.  If there is a problem at this stage an error will be returned:

.. code-block:: c

   attachsql_query(con, strlen(query), query, 0, NULL, &error);

We want to loop until all the rows have been retrieved (``ATTACHSQL_RETURN_EOF``) or an error occurs:

.. code-block:: c

   while ((ret != ATTACHSQL_RETURN_EOF) && (error == NULL))
   {

Now we poll, the poll will send data when possible and retrieve data when available.  It will return with a status immediately every time:

.. code-block:: c

   ret= attachsql_connect_poll(con, &error);

If the status is that we do not have a complete row yet, loop again:

.. code-block:: c

   if (ret != ATTACHSQL_RETURN_ROW_READY)
   {
     continue;
   }

Getting this far means there is a row ready for us on the network buffer, lets get this:

.. code-block:: c

   row= attachsql_query_row_get(con, &error);

We also need to know how many columns are in the result set.  Technically this only needs to be run on the first loop but in this example it is being retrieved on every loop:

.. code-block:: c

   columns= attachsql_query_column_count(con);

Now data for all the columns is sent to *stdout*.  The column data is **not** NULL terminated so we use the length parameter to tell ``printf`` how long the data is.:

.. code-block:: c

   for (current_column= 0; current_column < columns; current_column++)
   {
     printf("%.*s ", (int)row[current_column].length, row[current_column].data);
   }

Finally we tell the library we are done with this row and to grab another:

.. code-block:: c

   attachsql_query_row_next(con);

At the end of the program everything is cleaned up nicely:

.. code-block:: c

   attachsql_query_close(con);
   attachsql_connect_destroy(con);

.. _escaped-query-example:

An Escaped Query
----------------

The :c:func:`attachsql_query` function can automatically escape data before sending it to the MySQL server in a similar way to prepared statements inside a database server.  In this example we will do a slight variation of the basic query application above.  This example can be found at ``examples/escaped_query.c`` in the source.

Source Code
^^^^^^^^^^^

.. literalinclude:: ../../examples/escaped_query.c
   :language: c

Breaking it Down
^^^^^^^^^^^^^^^^

This time the query we have defined is as follows, with the "?" characters representing what we want libAttachSQL to fill in:

.. code-block:: c

   const char *query= "SELECT * FROM t1 WHERE name = ? AND age > ?";

We therefore need to define a parameter structure with 2 elements in the array:

.. code-block:: c

   attachsql_query_parameter_st param[2];

The first parameter is a char, we shall set this as the first element of the array.  When the query is run libAttachSQL will automatically add quote marks around the text:

.. code-block:: c

   char *name= "fred";
   param[0].type= ATTACHSQL_ESCAPE_TYPE_CHAR;
   param[0].data= name;
   param[0].length= strlen(name);

The second parameter is an int.  This is done in a very similar way, libAttchSQL will automatically conver the C int type to text for the query:

.. code-block:: c

   uint32_t age= 30;
   param[1].type= ATTACHSQL_ESCAPE_TYPE_INT;
   param[1].data= &age;
   param[1].is_unsigned= true;

The query is then executed in a similar way as before, but this time we are telling libAttachSQL that there are two parameters as well as passing the parameters to the query:

.. code-block:: c

   attachsql_query(con, strlen(query), query, 2, param, &error)

The resulting query generated before sending to the MySQL server is:

.. code-block:: mysql

   SELECT * FROM t1 WHERE name = 'fred' AND age > 30;

.. _buffered-results-example:

Buffered Results
----------------

libAttachSQL supports buffered result sets instead of processing row by row.  The API usage is slightly different here so for comparisson we will do the same query and output as the basic query example.

Source Code
^^^^^^^^^^^

.. literalinclude:: ../../examples/buffered_query.c
   :language: c

Breaking it Down
^^^^^^^^^^^^^^^^

After initialising the connection object the command is given to enable result buffering:

.. code-block:: c

   attachsql_query_buffer_rows(con, true);

This time when we are looping we need to wait until we get ``ATTACHSQL_RETURN_EOF`` before we can start using the results:

.. code-block:: c

   while ((ret != ATTACHSQL_RETURN_EOF) && (error == NULL))
   {
     ret= attachsql_connect_poll(con, &error);
   }

We can now loop through :c:func:`attachsql_query_buffer_row_get` to retrieve the rows from the buffer.  When there are no more rows this function will return :c:type:`NULL` and the application can finish and clean up:

.. code-block:: c

   while((row= attachsql_query_buffer_row_get(con)))
   {
     for (current_column= 0; current_column < columns; current_column++)
     {
       printf("%.*s ", (int)row[current_column].length, row[current_column].data);
     }
     printf("\n");
   }

.. _prepared-statements-example:

Prepared Statements
-------------------

The API has server-side Prepared Statement functionality.  The implementation is more similar to JDBC than MySQL's own C implementation.

This example implements the same query as the escaped query example, but uses prepared statements instead.  It can be found at ``examples/prepared_statement.c`` in the source.

Source Code
^^^^^^^^^^^

.. literalinclude:: ../../examples/prepared_statement.c
   :language: c

Breaking it Down
^^^^^^^^^^^^^^^^

As before we have the query using "?" placeholders representing the values to be filled in.  This time it will be the server filling in the values rather than libAttachSQL:

.. code-block:: c

   const char *query= "SELECT * FROM t1 WHERE name = ? AND age > ?";

This time we need to send the query to the server in the prepare phase, we will loop until we know that the prepare is complete:

.. code-block:: c

   attachsql_statement_prepare(con, strlen(query), query, &error);
   while((ret != ATTACHSQL_RETURN_EOF) && (error == NULL))
   {
     ret= attachsql_connect_poll(con, &error);
   }

Now to fill in the blanks, we use :c:func:`attachsql_statement_set_int` and :c:func:`attachsql_statement_set_string` to set the parameter data.  It is important to note that data pointed to in :c:func:`attachsql_statement_set_string` needs to stay in scope until after the execute phase has started returning results.

.. code-block:: c

   const char *name= "fred";
   uint32_t age= 30;
   attachsql_statement_set_string(con, 0, strlen(name), name, NULL);
   attachsql_statement_set_int(con, 1, age, NULL);

Once all the parameters have been set, the statement needs executing:

.. code-block:: c

   attachsql_statement_execute(con, &error);

As with previous examples we loop until we get the result ``ATTACHSQL_RETURN_ROW_READY``.  At this point we use :c:func:`attachsql_statement_row_get` to process the row ready for retrieval.  The :c:func:`attachsql_statement_get_int` and :c:func:`attachsql_statement_get_char` functions will return the data, converting it if needed into the required type (if possible).

.. code-block:: c

   attachsql_statement_row_get(con, &error);
   printf("ID: %d, ", attachsql_statement_get_int(con, 0, &error));
   size_t len;
   char *name_data= attachsql_statement_get_char(con, 1, &len, &error);
   printf("Name: %.*s, ", (int)len, name_data);
   printf("Age: %d\n", attachsql_statement_get_int(con, 2, &error));

When we have finished with the row, a new one is requested.

.. code-block:: c

   attachsql_statement_row_next(con);

And finally instead of closing a query, we are closing a statement.

.. code-block:: c

   attachsql_statement_close(con);

.. _pool-connections-example:

Pool Connections
----------------

libAttachSQL has the ability to pool several connections into a single event loop.  This makes this more efficient for many connections on a single thread.  There is a slightly different access pattern for this which relies on callback.

In this example we will make three simultaneous queries to the server on three connections in a single connection pool.  All three will be executed in parallel and as such will return in a random order.  The example file can be found at ``examples/pool_query.c``.

Source Code
^^^^^^^^^^^

.. literalinclude:: ../../examples/pool_query.c
   :language: c

Breaking it Down
^^^^^^^^^^^^^^^^

First we create the pool object which the connections will be attached to:

.. code-block:: c

   pool= attachsql_pool_create(callbk, NULL, NULL);

Then we create the connections, add the connections to the pool and set the callback function for the connections.  This is repeated 3 times, once for every connection:

.. code-block:: c

   con[0]= attachsql_connect_create("localhost", 3306, "test", "test", "testdb", NULL);
   attachsql_pool_add_connection(pool, con[0], &error);

We can now send a query to each of these connections:

.. code-block:: c

   const char *query1= "SELECT * FROM t1 WHERE name='fred'";
   const char *query2= "SELECT * FROM t1 WHERE age >= 40";
   const char *query3= "SELECT * FROM t1 WHERE age < 40";
   ...
   attachsql_query(con[0], strlen(query1), query1, 0, NULL, &error);
   attachsql_query(con[1], strlen(query2), query2, 0, NULL, &error);
   attachsql_query(con[2], strlen(query3), query3, 0, NULL, &error);

The pool method uses callbacks instead of polling and checking the results.  So you only need to run the pool event loop whenever ready.  This is non-blocking and will only fire a callback if there is data ready:

.. code-block:: c

   while(done_count < 3)
   {
     attachsql_pool_run(pool);
   }

When there is an event to be triggered such as a row ready in the buffer the callback is triggered:

.. code-block:: c

   void callbk(attachsql_connect_t *current_con, uint32_t connection_id, attachsql_events_t events, void *context, attachsql_error_t *error)

In this callback function we are using a switch statement to find out which event was fired and act appropriately.  The connected event fires when connection and handshake is complete:

.. code-block:: c

   case ATTACHSQL_EVENT_CONNECTED:
     printf("Connected event on con %d\n", connection_id);
     break;

The error event fires when an error occurs.  It is up to the application to clean up the error:

.. code-block:: c

   case ATTACHSQL_EVENT_ERROR:
     printf("Error exists on con %d: %d", connection_id, attachsql_error_code(error));
     attachsql_error_free(error);
     break;

The EOF event fires when we have reached the end of the query results:

.. code-block:: c

   case ATTACHSQL_EVENT_EOF:
     printf("Connection %d finished\n", connection_id);
     done_count++;
     attachsql_query_close(current_con);

Finally the row ready event fires when a row is ready for processing in the buffer:

.. code-block:: c

   case ATTACHSQL_EVENT_ROW_READY:
     row= attachsql_query_row_get(current_con, &error);
     columns= attachsql_query_column_count(current_con);
     for (col=0; col < columns; col++)
     {
       printf("Con: %d, Column: %d, Length: %zu, Data: %.*s ", connection_id, col, row[col].length, (int)row[col].length, row[col].data);
     }
     attachsql_query_row_next(current_con);
     printf("\n");
     break;

After the main while() loop has finished the pool needs to be destroyed.  This will cleanup all underlying connections:

.. code-block:: c

   attachsql_pool_destroy(pool);
    break;
