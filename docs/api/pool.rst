Connection Pool Functions
=========================

attachsql_pool_create()
------------------------

.. c:function:: attachsql_pool_t *attachsql_pool_create(attachsql_error_t **error)

   Creates and initializes a connection pool object

   :param error: A pointer to a pointer of an error object which is created if an error occurs
   :returns: A newly created pool object or :c:type:`NULL` on failure

   .. versionadded:: 0.9.0

   .. versionchanged:: 2.0.0
      Renamed `group` to `pool`

Example
^^^^^^^

.. code-block:: c

   attachsql_pool_t *pool= NULL;
   attachsql_error_t *error= NULL;

   pool= attachsql_pool_create(&error);

   if (error != NULL)
   {
     printf("Error occurred: %s\n", attachsql_error_message(error));
     attachsql_error_free(error);
     return -1;
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

   pool= attachsql_pool_create(&error);

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

   pool= attachsql_pool_create(NULL);
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
