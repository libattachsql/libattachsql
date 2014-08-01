Data Types
==========

.. c:type:: attachsql_connect_t

   A connection object allocated by :c:func:`attachsql_connect_create` and used to send and receive data from the MySQL server.

.. c:type:: in_port_t

   A port number as defined in the standard header ``arpa/inet.h``.

.. c:type:: attachsql_error_st

   An error struct.

   .. c:member:: int code

      An integer container the error code.

.. c:type:: NULL

   A null pointer as defined in the standard header ``string.h``.
