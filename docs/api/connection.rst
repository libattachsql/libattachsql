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
   :param error: A pointer to a pointer of an error struct which is created if an error occurs.
   :returns: A newly allocated connection structure or :c:type:`NULL` on error
