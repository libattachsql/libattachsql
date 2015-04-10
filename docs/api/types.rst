Data Types
==========

Basic Types
-----------

.. c:type:: attachsql_connect_t

   A connection object allocated by :c:func:`attachsql_connect_create` and used to send and receive data from the MySQL server.

.. c:type:: attachsql_error_t

   An error object allocated by many functions in the library and needs to be freed by the user using :c:func:`attachsql_error_free`.

.. c:type:: attachsql_pool_t

   An object containing a pool of connections to be executed using the same event loop.

Builtin Types
-------------

.. c:type:: in_port_t

   A port number as defined in the standard header ``arpa/inet.h``.  In Windows this is typedef'd to :c:type:`uint16_t`.

.. c:type:: NULL

   A null pointer as defined in the standard header ``string.h``.

.. c:type:: size_t

   An unsigned interget used to represent an object's size as defined in the standard header ``stddef.h``

.. c:type:: bool

   A boolean type ad defined in the standard header ``stdbool.h``

.. c:type:: uint8_t

   An 8-bit unsigned integer as defined in the standard header ``stdint.h``

.. c:type:: uint16_t

   A 16-bit unsigned integer as defined in the standard header ``stdint.h``

.. c:type:: uint32_t

   A 32-bit unsigned integer as defined in the standard header ``stdint.h``

.. c:type:: int32_t

   A 32-bit signed integer as defined in the standard header ``stdint.h``

.. c:type:: uint64_t

   A 64-bit unsigned integer as defined in the standard header ``stdint.h``

.. c:type:: int64_t

   A 64-bit signed integer as defined in the standard header ``stdint.h``

.. c:type:: float

   An IEEE 754 single-precision binary floating-point format

.. c:type:: double

   An IEEE 754 double-precision binary floating-point format

Structs
-------

.. c:type:: attachsql_query_parameter_st

   A struct containing a query parameter for use with :c:func:`attachsql_query`.  An array of this struct should be allocated by the user's application and provided to the function.

   .. note::
      The library will make a copy of this data internally so it can be free'd after :c:func:`attachsql_query` has successfully returned.

   .. c:member:: attachsql_query_parameter_type_t type

      The parameter type

   .. c:member:: void *data

      A pointer to the parameter data

   .. c:member:: size_t length

      The length of the parameter (for non-numeric parameters)

   .. c:member:: bool is_unsigned

      A numeric provided is unsigned (for numeric parameters)

.. c:type:: attachsql_query_column_st

   A struct containing column metadata.

   .. note::
      The library will automatically free this information on :c:func:`attachsql_query_close`

   .. c:member:: char *schema

      The database schema for the column

   .. c:member:: char *table

      The database table for the column

   .. c:member:: char *origin_table

      The original table name (if the query aliases the table)

   .. c:member:: char *column

      The column name

   .. c:member:: char *origin_column

      The original column name (if the query aliases the column)

   .. c:member:: uint16_t charset

      The character set / collation number

   .. c:member:: uint32_t length

      The size of the field (as defined by type and table definition)

   .. c:member:: attachsql_column_type_t type

      The column type

   .. c:member:: attachsql_column_flags_t flags

      Option flags for the column

   .. c:member:: uint8_t decimals

      The number of decimals for a numeric field

   .. c:member:: char *default_value

      The default value of the field

   .. c:member:: size_t default_size

      The length of the :c:member:`attachsql_query_column_st.default_value`

.. c:type:: attachsql_query_row_st

   A struct that contains an item in a row, :c:func:`attachsql_query_row_get` will return an array of these to make up a row.

   .. c:member:: char *data

      The item data (not NUL terminated)

   .. c:member:: size_t length

      The length of the data

Callbacks
---------

.. c:type:: attachsql_callback_fn

   A callback function template for use with :c:func:`attachsql_pool_create`.  Defined as:

   .. c:function:: void (attachsql_callback_fn)(attachsql_connect_t *con, uint32_t connection_id, attachsql_events_t events, void *context, attachsql_error_t *error)

      :param con: The connection object
      :param connection_id: A unique ID given to the connection by libAttachSQL, *NOT* the MySQL thread ID
      :param events: The event triggered
      :param context: A user defined pointer which is set along with the callback
      :param error: An error object (if an error occurred)

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

   +-------------------------------+-----------------------------------+
   | Value                         | Description                       |
   +===============================+===================================+
   | ``ATTACHSQL_EVENT_NONE``      | No event                          |
   +-------------------------------+-----------------------------------+
   | ``ATTACHSQL_EVENT_CONNECTED`` | Connection and handshake complete |
   +-------------------------------+-----------------------------------+
   | ``ATTACHSQL_EVENT_ERROR``     | An error has occurred             |
   +-------------------------------+-----------------------------------+
   | ``ATTACHSQL_EVENT_EOF``       | Query EOF, no more rows           |
   +-------------------------------+-----------------------------------+
   | ``ATTACHSQL_EVENT_ROW_READY`` | A row is ready in the buffer      |
   +-------------------------------+-----------------------------------+

.. c:type:: attachsql_error_level_t

   The severity of an error.  This is an ENUM with the following values:

   +-----------------------------------+-------------+
   | Value                             | Description |
   +===================================+=============+
   | ``ATTACHSQL_ERROR_LEVEL_NOTICE``  | A notice    |
   +-----------------------------------+-------------+
   | ``ATTACHSQL_ERROR_LEVEL_WARNING`` | A warning   |
   +-----------------------------------+-------------+
   | ``ATTACHSQL_ERROR_LEVEL_ERROR``   | An error    |
   +-----------------------------------+-------------+

.. c:type:: attachsql_column_type_t

   The column type for a given column.  This is an ENUM with the following values:

   +--------------------------------------+------------------------------------------------------------+
   | Value                                | Description                                                |
   +======================================+============================================================+
   | ``ATTACHSQL_COLUMN_TYPE_DECIMAL``    | A decimal column type (for MySQL versions less than 5.0.3) |
   +--------------------------------------+------------------------------------------------------------+
   | ``ATTACHSQL_COLUMN_TYPE_TINY``       | A tinyint column type                                      |
   +--------------------------------------+------------------------------------------------------------+
   | ``ATTACHSQL_COLUMN_TYPE_SHORT``      | A smallint column type                                     |
   +--------------------------------------+------------------------------------------------------------+
   | ``ATTACHSQL_COLUMN_TYPE_LONG``       | An int column type                                         |
   +--------------------------------------+------------------------------------------------------------+
   | ``ATTACHSQL_COLUMN_TYPE_FLOAT``      | A float column type                                        |
   +--------------------------------------+------------------------------------------------------------+
   | ``ATTACHSQL_COLUMN_TYPE_DOUBLE``     | A double/real column type                                  |
   +--------------------------------------+------------------------------------------------------------+
   | ``ATTACHSQL_COLUMN_TYPE_NULL``       | A NULL column                                              |
   +--------------------------------------+------------------------------------------------------------+
   | ``ATTACHSQL_COLUMN_TYPE_TIMESTAMP``  | A timestamp column type                                    |
   +--------------------------------------+------------------------------------------------------------+
   | ``ATTACHSQL_COLUMN_TYPE_LONGLONG``   | A bigint column type                                       |
   +--------------------------------------+------------------------------------------------------------+
   | ``ATTACHSQL_COLUMN_TYPE_INT24``      | A mediumint column type                                    |
   +--------------------------------------+------------------------------------------------------------+
   | ``ATTACHSQL_COLUMN_TYPE_DATE``       | A date column type                                         |
   +--------------------------------------+------------------------------------------------------------+
   | ``ATTACHSQL_COLUMN_TYPE_TIME``       | A time column type                                         |
   +--------------------------------------+------------------------------------------------------------+
   | ``ATTACHSQL_COLUMN_TYPE_DATETIME``   | A datetime column type                                     |
   +--------------------------------------+------------------------------------------------------------+
   | ``ATTACHSQL_COLUMN_TYPE_YEAR``       | A year column type                                         |
   +--------------------------------------+------------------------------------------------------------+
   | ``ATTACHSQL_COLUMN_TYPE_BIT``        | A bit column type                                          |
   +--------------------------------------+------------------------------------------------------------+
   | ``ATTACHSQL_COLUMN_TYPE_NEWDECIMAL`` | A decimal column type                                      |
   +--------------------------------------+------------------------------------------------------------+
   | ``ATTACHSQL_COLUMN_TYPE_ENUM``       | An enum column type                                        |
   +--------------------------------------+------------------------------------------------------------+
   | ``ATTACHSQL_COLUMN_TYPE_SET``        | A set column type                                          |
   +--------------------------------------+------------------------------------------------------------+
   | ``ATTACHSQL_COLUMN_TYPE_BLOB``       | A blob column type                                         |
   +--------------------------------------+------------------------------------------------------------+
   | ``ATTACHSQL_COLUMN_TYPE_VARSTRING``  | A varchar or varbinary column type                         |
   +--------------------------------------+------------------------------------------------------------+
   | ``ATTACHSQL_COLUMN_TYPE_STRING``     | A char or binary column type                               |
   +--------------------------------------+------------------------------------------------------------+
   | ``ATTACHSQL_COLUMN_TYPE_GEOMETRY``   | A geometry column type                                     |
   +--------------------------------------+------------------------------------------------------------+
   | ``ATTACHSQL_COLUMN_TYPE_ERROR``      | An error occurred                                          |
   +--------------------------------------+------------------------------------------------------------+

.. c:type:: attachsql_column_flags_t

   Zero or more bit flags that help to describe more information about a field. This is an ENUM with the following values:

   +---------------------------------------------+----------------------------------------------------+
   | Value                                       | Description                                        |
   +=============================================+====================================================+
   | ``ATTACHSQL_COLUMN_FLAGS_NONE``             | No flags are set                                   |
   +---------------------------------------------+----------------------------------------------------+
   | ``ATTACHSQL_COLUMN_FLAGS_NOT_NULL``         | The column has not null set                        |
   +---------------------------------------------+----------------------------------------------------+
   | ``ATTACHSQL_COLUMN_FLAGS_PRIMARY_KEY``      | The column is part of a primary key                |
   +---------------------------------------------+----------------------------------------------------+
   | ``ATTACHSQL_COLUMN_FLAGS_UNIQUE_KEY``       | The column is part of a unique key                 |
   +---------------------------------------------+----------------------------------------------------+
   | ``ATTACHSQL_COLUMN_FLAGS_MULTIPLE_KEY``     | The column is part of a non-unique key             |
   +---------------------------------------------+----------------------------------------------------+
   | ``ATTACHSQL_COLUMN_FLAGS_BLOB``             | The column is a blob or text (deprecated in MySQL) |
   +---------------------------------------------+----------------------------------------------------+
   | ``ATTACHSQL_COLUMN_FLAGS_UNSIGNED``         | The numeric column is unsigned                     |
   +---------------------------------------------+----------------------------------------------------+
   | ``ATTACHSQL_COLUMN_FLAGS_ZEROFILL``         | The column has zerofill set                        |
   +---------------------------------------------+----------------------------------------------------+
   | ``ATTACHSQL_COLUMN_FLAGS_BINARY``           | The column is a binary                             |
   +---------------------------------------------+----------------------------------------------------+
   | ``ATTACHSQL_COLUMN_FLAGS_ENUM``             | The column is an enum                              |
   +---------------------------------------------+----------------------------------------------------+
   | ``ATTACHSQL_COLUMN_FLAGS_AUTO_INCREMENT``   | The column is an auto-increment                    |
   +---------------------------------------------+----------------------------------------------------+
   | ``ATTACHSQL_COLUMN_FLAGS_TIMESTAMP``        | The column is a timestamp (deprecated in MySQL)    |
   +---------------------------------------------+----------------------------------------------------+
   | ``ATTACHSQL_COLUMN_FLAGS_SET``              | The column is a set column type                    |
   +---------------------------------------------+----------------------------------------------------+
   | ``ATTACHSQL_COLUMN_FLAGS_NO_DEFAULT_VALUE`` | The column has no default value                    |
   +---------------------------------------------+----------------------------------------------------+
   | ``ATTACHSQL_COLUMN_FLAGS_NUM``              | The column is numeric                              |
   +---------------------------------------------+----------------------------------------------------+

.. c:type:: attachsql_query_parameter_type_t

   The types of parameters for use with :c:type:`attachsql_query_parameter_st`. This is an ENUM with the following values:

   +-------------------------------------+-----------------------------------------------------------------------------------------------------------------------------------+
   | Value                               | Description                                                                                                                       |
   +=====================================+===================================================================================================================================+
   | ``ATTACHSQL_ESCAPE_TYPE_CHAR``      | Value is a char/binary.  The data will be escaped and quote marks added to the beginning and end.                                 |
   +-------------------------------------+-----------------------------------------------------------------------------------------------------------------------------------+
   | ``ATTACHSQL_ESCAPE_TYPE_CHAR_LIKE`` | Value is a char/binary.  The data will be escaped but no quote marks will be added, for use with ``LIKE`` syntax in MySQL queries |
   +-------------------------------------+-----------------------------------------------------------------------------------------------------------------------------------+
   | ``ATTACHSQL_ESCAPE_TYPE_INT``       | Value is an int.  The data will be converted into a character representation of the int.                                          |
   +-------------------------------------+-----------------------------------------------------------------------------------------------------------------------------------+
   | ``ATTACHSQL_ESCAPE_TYPE_BIGINT``    | Value is a bigint.  The data will be converted into a character representation of the bigint.                                     |
   +-------------------------------------+-----------------------------------------------------------------------------------------------------------------------------------+
   | ``ATTACHSQL_ESCAPE_TYPE_FLOAT``     | Value is a float.  The data will be converted into a character representation of the float.                                       |
   +-------------------------------------+-----------------------------------------------------------------------------------------------------------------------------------+
   | ``ATTACHSQL_ESCAPE_TYPE_DOUBLE``    | Value is a double.  The data will be converted into a character representation of the double.                                     |
   +-------------------------------------+-----------------------------------------------------------------------------------------------------------------------------------+

.. c:type:: attachsql_options_t

   The options for use with :c:func:`attachsql_connect_set_option`.  This is an ENUM with the following values:

   +---------------------------------------+-----------------------------------------------------------------------------------------+----------+
   | Value                                 | Description                                                                             | Argument |
   +=======================================+=========================================================================================+==========+
   | ``ATTACHSQL_OPTION_COMPRESS``         | Enable protocol compression (when compiled with zlib support)                           | Not used |
   +---------------------------------------+-----------------------------------------------------------------------------------------+----------+
   | ``ATTACHSQL_OPTION_FOUND_ROWS``       | Return the number of matched rows instead of number of changed rows                     | Not used |
   +---------------------------------------+-----------------------------------------------------------------------------------------+----------+
   | ``ATTACHSQL_OPTION_IGNORE_SIGPIPE``   | Client library ignores SIGPIPE                                                          | Not used |
   +---------------------------------------+-----------------------------------------------------------------------------------------+----------+
   | ``ATTACHSQL_OPTION_INTERACTIVE``      | Client should use interactive timeout instead of wait timeout                           | Not used |
   +---------------------------------------+-----------------------------------------------------------------------------------------+----------+
   | ``ATTACHSQL_OPTION_LOCAL_FILES``      | Enable ``LOAD DATA LOCAL`` (not yet implemented)                                        | Not used |
   +---------------------------------------+-----------------------------------------------------------------------------------------+----------+
   | ``ATTACHSQL_OPTION_MULTI_STATEMENTS`` | Enable multi-statement queries                                                          | Not used |
   +---------------------------------------+-----------------------------------------------------------------------------------------+----------+
   | ``ATTACHSQL_OPTION_NO_SCHEMA``        | Disable the ``schema_name.table_name.column_name`` syntax (for ODBC)                    | Not used |
   +---------------------------------------+-----------------------------------------------------------------------------------------+----------+
   | ``ATTACHSQL_OPTION_SEMI_BLOCKING``    | Block until there is data in the network buffer.  Useful for one connection per thread. | Not used |
   +---------------------------------------+-----------------------------------------------------------------------------------------+----------+



