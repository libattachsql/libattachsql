Connection Functions
====================

.. c:function:: attachsql_connect_t *attachsql_connect_create(const char *host, in_port_t port, const char *user, const char *pass, const char *schema, attachsql_error_st **error)

   Creates a connection object with the requested parameters.  If port ``0`` is given then the library will assume a Unix Domain Socket (UDS) connection is required unless explicitly specified.

   .. note::
      The connection is not established until the first query or it is explicitly requested using :c:func:`attachsql_connect`.

   :param host: The host name / IP of the server or socket path for UDS
   :param port: The port number of the server (for TCP/IP)
   :param user: The user name for the connection
   :param pass: The password for the user
   :param schema: The default schema for the connection
   :param error: A pointer to a pointer of an error struct which is created if an error occurs
   :returns: A newly allocated connection object or :c:type:`NULL` on error

   .. versionadded:: 0.1.0

.. c:function:: void attachsql_connect_destroy(attachsql_connect_t *con)

   Disconnects from the MySQL server (if connected), frees allocated memory associated with the connection object and frees the connection object.

   :param con: The connection object

   .. versionadded:: 0.1.0

.. c:function:: attachsql_error_st *attachsql_connect(attachsql_connect_t *con)

   Starts an asyncronus connection to a MySQL server and returns immediately.  Call :c:func:`attachsql_connect_poll` until connected test to see if the connection has been established yet.

   :param con: The connetion object to use for the connection
   :returns: An error struct or NULL if there is no error

   .. versionadded:: 0.1.0

.. c:function:: void attachsql_connect_set_callback(attachsql_connect_t *con, attachsql_callback_fn *function, void *context)

   Sets a callback function which will be executed on connection complete, error, eof or row ready events.

   .. note::
      The callback will only be called as part of an execution of another function such as :c:func:`attachsql_connect_poll`

   :param con: The connection object to bind the callback functino to
   :param function: The callback function
   :param context: A pointer to some data which will be passed to the callback function upon execution

   .. versionadded:: 0.1.0

.. c:function:: const char *attachsql_connect_get_server_version(attachsql_connect_t *con)

   Gets the version sting of the MySQL server libAttachSQL is connected to.

   :param con: The connection object to get the version string from
   :returns: The version string or :c:type:`NULL` if not connected

   .. versionadded:: 0.1.0

.. c:function:: attachsql_return_t attachsql_connect_poll(attachsql_connect_t *con, attachsql_error_st **error)

   Polls the connection to check if new data is ready.  If there is the new data will automatically be processed ready for use.

   :param con: The connection object to poll
   :param error: A pointer to a pointer of an error struct which is created if an error occurs
   :returns: The status of the connection after the poll

   .. versionadded:: 0.1.0
