Data Types
==========

Basic Types
-----------

.. c:type:: attachsql_connect_t

   A connection object allocated by :c:func:`attachsql_connect_create` and used to send and receive data from the MySQL server.

Builtin Types
-------------

.. c:type:: in_port_t

   A port number as defined in the standard header ``arpa/inet.h``.  In Windows this is typedef'd to ``uint16_t``.

.. c:type:: NULL

   A null pointer as defined in the standard header ``string.h``.

.. c:type:: uint8_t

   An 8-bit unsigned integer as define in the standard header ``stdint.h``

Structs
-------

.. c:type:: attachsql_error_st

   An error struct.

   .. c:member:: int code

      An integer container the error code.

   .. c:member:: char *msg

      The error message

   .. c:member:: char *sqlstate

      The SQLSTATE of the error

   .. c:member:: attachsql_error_level_t level

      The severity of the error

Callbacks
---------

.. c:type:: attachsql_callback_fn

   A callback function template for use with :c:func:`attachsql_connect_set_callback`.  Defined as:

   .. c:function:: void (attachsql_callback_fn)(attachsql_connect_t *con, attachsql_events_t events, void *context)

      :param con: The connection object
      :param events: The event triggered
      :param context: A user defined pointer which is set along with the callback

ENUMs
-----

.. c:type:: attachsql_return_t

   The connection return status after :c:func:`attachsql_connect_poll` is used.  This is an ENUM with the following values:

   +------------------------------------+--------------------------------------------------------------------+
   | Value                              | Description                                                        |
   +====================================+====================================================================+
   | ``ATTACHSQL_RETURN_NONE``          | Empty return code (functions will not return this)                 |
   +------------------------------------+--------------------------------------------------------------------+
   | ``ATTACHSQL_RETURN_NOT_CONNECTED`` | Poll was attempted on a connection object that is currently unused |
   +------------------------------------+--------------------------------------------------------------------+
   | ``ATTACHSQL_RETURN_CONNECTING``    | Connection/handshake currently in-progress                         |
   +------------------------------------+--------------------------------------------------------------------+
   | ``ATTACHSQL_RETURN_IDLE``          | Connection is idle and waiting for a command                       |
   +------------------------------------+--------------------------------------------------------------------+
   | ``ATTACHSQL_RETURN_PROCESSING``    | The query is currently processing                                  |
   +------------------------------------+--------------------------------------------------------------------+
   | ``ATTACHSQL_RETURN_ROW_READY``     | A row is ready for retrieval from the buffer                       |
   +------------------------------------+--------------------------------------------------------------------+
   | ``ATTACHSQL_RETURN_ERROR``         | An error has occurred                                              |
   +------------------------------------+--------------------------------------------------------------------+
   | ``ATTACHSQL_RETURN_EOF``           | There are no more rows to retrieve                                 |
   +------------------------------------+--------------------------------------------------------------------+

.. c:type:: attachsql_events_t

   The event that trigger the execution of the callback function.  This is an ENUM with the following values:

.. c:type:: attachsql_error_level_t

   The severity of an error.  This is an ENUM with the following values:
