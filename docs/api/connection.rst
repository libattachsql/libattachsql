Connection Functions
====================

attachsql_connect_create()
--------------------------

.. c:function:: attachsql_connect_t *attachsql_connect_create(const char *host, in_port_t port, const char *user, const char *pass, const char *schema, attachsql_error_t **error)

   Creates a connection object with the requested parameters.  If port ``0`` is given then the library will assume a Unix Domain Socket (UDS) connection is required unless explicitly specified.

   .. note::
      The connection is not established until the first query or it is explicitly requested using :c:func:`attachsql_connect`.

   :param host: The host name / IP of the server or socket path for UDS
   :param port: The port number of the server (for TCP/IP)
   :param user: The user name for the connection
   :param pass: The password for the user
   :param schema: The default schema for the connection
   :param error: A pointer to a pointer of an error object which is created if an error occurs
   :returns: A newly allocated connection object or :c:type:`NULL` on error

   .. versionadded:: 0.1.0

Example
^^^^^^^

.. code-block:: c

   attachsql_connect_t *con= NULL;
   attachsql_error_t *error= NULL;

   // Note: we are not connecting here, just defining the connection
   con= attachsql_connect_create("localhost", 3306, "test", "test", "testdb", &error);

   if (error != NULL)
   {
     printf("Error occurred: %s\n", attachsql_error_message(error));
     attachsql_error_free(error);
     return -1;
   }


attachsql_connect_destroy()
---------------------------

.. c:function:: void attachsql_connect_destroy(attachsql_connect_t *con)

   Disconnects from the MySQL server (if connected), frees allocated memory associated with the connection object and frees the connection object.

   :param con: The connection object

   .. versionadded:: 0.1.0

Example
^^^^^^^

.. code-block:: c

   attachsql_connect_t *con= NULL;
   attachsql_error_t *error= NULL;

   con= attachsql_connect_create("localhost", 3306, "test", "test", "testdb", error);
   // Do lots with the connection here
   ...
   attachsql_connect_destroy(con);



attachsql_connect()
-------------------

.. c:function:: bool attachsql_connect(attachsql_connect_t *con, attachsql_error_t **error)

   Starts an asyncronus connection to a MySQL server and returns immediately.  Call :c:func:`attachsql_connect_poll` until connected test to see if the connection has been established yet.

   :param con: The connetion object to use for the connection
   :param error: A pointer to a pointer of an error object which is created if an error occurs
   :returns: ``true`` on success, ``false`` on failure

   .. versionadded:: 0.1.0
   .. versionchanged:: 0.5.0

Example
^^^^^^^

.. code-block:: c

   attachsql_connect_t *con= NULL;
   attachsql_error_t *error= NULL;

   // Note: we are not connecting here, just defining the connection
   con= attachsql_connect_create("localhost", 3306, "test", "test", "testdb", &error);

   if (error != NULL)
   {
     printf("Error occurred: %s\n", attachsql_error_message(error));
     attachsql_error_free(error);
     return -1;
   }

   // Note: we are not connecting here either, just requesting the connection takes place
   if (!attachsql_connect(con, &error))
   {
     printf("Error occurred\n");
     if (error != NULL)
     {
       printf("Message: %s\n", attachsql_error_message(error));
       attachsql_error_free(error);
       return -1;
     }
   }

attachsql_connect_set_callback()
--------------------------------

.. c:function:: void attachsql_connect_set_callback(attachsql_connect_t *con, attachsql_callback_fn *function, void *context)

   Sets a callback function which will be executed on connection complete, error, eof or row ready events.

   .. note::
      The callback will only be called as part of an execution of another function such as :c:func:`attachsql_connect_poll`

   :param con: The connection object to bind the callback functino to
   :param function: The callback function
   :param context: A pointer to some data which will be passed to the callback function upon execution

   .. versionadded:: 0.1.0

Example
^^^^^^^

.. code-block:: c

   // User defined callback function
   void my_callback(attachsql_t *current_con, attachsql_events_t events, void *context, attachsql_error_t *error)
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
     attachsql_connect_t *con= NULL;
     attachsql_error_t *error= NULL;
     char arbitrary_data[]= "Callback context data";

     // Note: we are not connecting here, just defining the connection
     con= attachsql_connect_create("localhost", 3306, "test", "test", "testdb", &error);
     attachsql_connect_set_callback(con, my_callback, arbitrary_data);

     if (error != NULL)
     {
       printf("Error occurred: %s". attachsql_error_message(error));
       attachsql_error_free(error);
     }
     // Execute a query here
     ...
   }


attachsql_connect_get_server_version()
--------------------------------------

.. c:function:: const char *attachsql_connect_get_server_version(attachsql_connect_t *con)

   Gets the version sting of the MySQL server libAttachSQL is connected to.

   :param con: The connection object to get the version string from
   :returns: The version string or :c:type:`NULL` if not connected

   .. versionadded:: 0.1.0

Example
^^^^^^^

.. code-block:: c

   attachsql_connect_t *con= NULL;

   // Connect here
   ...

   printf("Server version: %s\n", attachsql_connect_get_server_version(con));

attachsql_connect_poll()
------------------------

.. c:function:: attachsql_return_t attachsql_connect_poll(attachsql_connect_t *con, attachsql_error_t **error)

   Polls the connection to check if new data is ready.  If there is the new data will automatically be processed ready for use.

   :param con: The connection object to poll
   :param error: A pointer to a pointer of an error object which is created if an error occurs
   :returns: The status of the connection after the poll

   .. versionadded:: 0.1.0

Example
^^^^^^^

.. code-block:: c

   attachsql_return_t ret= ATTACHSQL_RETURN_NONE;
   attachsql_connect_t *con= NULL;
   attachsql_query_row_st *row;
   attachsql_error_t *error;
   const char *query= "SELECT * FROM t1 WHERE name='fred'";
   uint16_t columns, current_column;

   con= attachsql_connect_create("localhost", 3306, "test", "test", "testdb", NULL);

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
     for (current_column= 0; current_column < columns; current_column++)
     {
       printf("%.*s ", (int)row[current_column].length, row[current_column].data);
     }
     printf("\n");
     attachsql_query_row_next(con);
   }
   // Error handling and cleanup here
   ...


attachsql_connect_set_option()
------------------------------

.. c:function:: bool attachsql_connect_set_option(attachsql_connect_t *con, attachsql_options_t option, const void *arg)

   Sets various connection options.  A list of possible options are listed at :c:type:`attachsql_options_t`

   :param con: The connection object to set the option on
   :param option: The option to set
   :param arg: The option argument (if any)
   :returns: true on success, false on failure

   .. versionadded:: 0.1.0

Example
^^^^^^^

.. code-block:: c

   attachsql_connect_t *con= NULL;

   // Note: we are not connecting here, just defining the connection
   con= attachsql_connect_create("localhost", 3306, "test", "test", "", NULL);
   bool compress= attachsql_connect_set_option(con, ATTACHSQL_OPTION_COMPRESS, NULL);

attachsql_connect_set_ssl()
---------------------------

.. c:function:: bool attachsql_connect_set_ssl(attachsql_connect_t *con, const char *key, const char *cert, const char *ca, const char *capath, const char *cipher, bool verify, attachsql_error_t **error)

   Configures SSL for the MySQL connection.  Should be used before any connection is established.

   .. warning::
      The :c:func:`attachsql_library_init` function must have been called before this function

   :param con: The connection to enable SSL on
   :param key: The certificate key file
   :param cert: The certificate file
   :param ca: The certificate authority file
   :param capath: The path to multiple certificate authority files
   :param cipher: The optional list of ciphers to use, see `OpenSSL's cipher examples <https://www.openssl.org/docs/apps/ciphers.html#EXAMPLES>`_ for how to use this
   :param: A pointer to a pointer of an error object which is created if an error occurs
   :returns: ``true`` on success, ``false`` on failure

   .. versionadded:: 0.3.0
   .. versionchanged:: 0.5.0

Example
^^^^^^^

.. code-block:: c

   attachsql_connect_t *con= NULL;
   attachsql_error_t *error= NULL;

   con= attachsql_connect_create("localhost", 3306, "test", "test", "", NULL);
   attachsql_connect_set_ssl(con, "client-key.pem", "client-cert.pem", "ca-cert.pem", NULL, NULL, false, &error);
   if (error and (attachsql_error_code(error) == 3002))
   {
     printf("Server does not support SSL\n");
     attachsql_error_free(error);
     return -1;
   }
