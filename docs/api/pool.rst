Connection Pool Functions
=========================

attachsql_pool_create()
------------------------

.. c:function:: attachsql_pool_t *attachsql_pool_create(attachsql_callback_fn *function, void *context, attachsql_error_t **error)

   Creates and initializes a connection pool object

   :param function: The callback function
   :param context: A pointer to some data which will be passed to the callback function upon execution
   :param error: A pointer to a pointer of an error object which is created if an error occurs
   :returns: A newly created pool object or :c:type:`NULL` on failure

   .. versionadded:: 0.9.0

   .. versionchanged:: 2.0.0
      * Renamed `group` to `pool`
      * Callback API added

Example
^^^^^^^

.. code-block:: c

   // User defined callback function
   void my_callback(attachsql_t *current_con, uint32_t connection_id, attachsql_events_t events, void *context, attachsql_error_t *error)
   {
     // User data which needs typecasting back to the type that was sent
     char *my_data= (char*)context;
     attachsql_query_row_st *row;
     uint16_t columns, col;

     switch(events)
     {
       case ATTACHSQL_EVENT_CONNECTED:
         printf("Connected event!\n");
         break;
       case ATTACHSQL_EVENT_ERROR:
         printf("Error occurred: %d\n", attachsql_error_code(error));
         attachsql_error_free(error);
         break;
       case ATTACHSQL_EVENT_EOF:
         printf("Connection has finished query");
         break;
       case ATTACHSQL_EVENT_ROW_READY:
         row= attachsql_query_row_get(current_con, &error);
         columns= attachsql_query_column_count(current_con);
         for (col=0; col < columns; col++)
         {
           printf("Con: %d, Column: %d, Length: %zu, Data: %.*s ", *con_no, col, row[col].length, (int)row[col].length, row[col].data);
         }
         attachsql_query_row_next(current_con);
         printf("\n");
         break;
       case ATTACHSQL_EVENT_NONE:
         // This should never happen!
         break;
     }
   }

   void main(int argc, char *argv[])
   {
     attachsql_pool_t *pool= NULL;
     attachsql_error_t *error= NULL;
     char arbitrary_data[]= "Callback context data";

     pool= attachsql_pool_create(my_callback, arbitrary_data, &error);

     if (error != NULL)
     {
       printf("Error occurred: %s\n", attachsql_error_message(error));
       attachsql_error_free(error);
       return -1;
     }
     // Do stuff here
     ...
   }

.. seealso:: :ref:`pool-connections-example` example

attachsql_pool_destroy()
-------------------------

.. c:function:: void attachsql_pool_destroy(attachsql_pool_t *pool)

   Destroys a connection pool along with all the underlying connections.

   .. warning::
      Do NOT use :c:func:`attachsql_connect_destroy` on any connection used by a connection pool.  This will cause a double-free to occur.

   :param pool: The pool object to destroy

   .. versionadded:: 0.9.0

   .. versionchanged:: 2.0.0
      Renamed `group` to `pool`

Example
^^^^^^^

.. code-block:: c

   attachsql_pool_t *pool= NULL;
   attachsql_error_t *error= NULL;

   pool= attachsql_pool_create(my_callback, NULL, &error);

   // Do stuff with the pool
   ...

   attachsql_pool_destroy(pool);

.. seealso:: :ref:`pool-connections-example` example

attachsql_pool_add_connection()
--------------------------------

.. c:function:: void attachsql_pool_add_connection(attachsql_pool_t *pool, attachsql_connect_t *con, attachsql_error_t **error)

   Adds a connection to the pool object.

   .. warning::
      * This should be a newly created connection with :c:func:`attachsql_connect_create` that has not yet connected to the server.
      * You should NOT use :c:func:`attachsql_connect_poll` with a connection in a pool
      * You should NOT use :c:func:`attachsql_connect_destroy` with a connection in a pool
      * See the examples in this documentation for more information

   :param pool: The connection pool object to add to
   :param con: The connection object to add
   :param error: A pointer to a pointer of an error object which is created if an error occurs

   .. versionadded:: 0.9.0

   .. versionchanged:: 2.0.0
      Renamed `group` to `pool`

Example
^^^^^^^

.. code-block:: c

   attachsql_connect_t *con1= NULL;
   attachsql_pool_t *pool= NULL;
   attachsql_error_t *error= NULL;

   pool= attachsql_pool_create(my_callback, NULL, NULL);
   con1= attachsql_connect_create("localhost", 3306, "test", "test", "testdb", NULL);
   attachsql_pool_add_connection(pool, con1, &error);

   // Do things with the pool here
   ...

.. seealso:: :ref:`pool-connections-example` example

attachsql_pool_run()
---------------------

.. c:function:: void attachsql_pool_run(attachsql_pool_t *pool)

   Runs the event loop for the connection pool, firing the callbacks if any event has occurred.

   .. warning::
      This function is not reentrant, trying to call it on the same pool with two threads will invoke undefined behaviour — it may block the process indefinitely, it may eat all your laundry, it will probably crash

   :param pool: The connection pool to run

   .. versionadded:: 0.9.0

   .. versionchanged:: 2.0.0
      Renamed `group` to `pool`

Example
^^^^^^^

See the :ref:`pool-connections-example` example

