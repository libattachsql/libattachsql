API Examples
============

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

   error= attachsql_query(con, strlen(query), query, 0, NULL);

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

   error= attachsql_query(con, strlen(query), query, 2, param)

The resulting query generated before sending to the MySQL server is:

.. code-block:: mysql

   SELECT * FROM t1 WHERE name = 'fred' AND age > 30;

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



