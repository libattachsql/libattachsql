Utility Functions
=================

attachsql_get_library_version()
-------------------------------

.. c:function:: const char *attachsql_get_library_version(void)

   Returns a string respresentation of the library version

   :returns: The string of the library version

   .. versionadded:: 0.1.0

attachsql_get_library_version_major()
-------------------------------------

.. c:function:: uint8_t attachsql_get_library_version_major(void)

   Returns the integer of the major version number for the library

   :returns: The major version number

   .. versionadded:: 0.1.0

attachsql_get_library_version_minor()
-------------------------------------

.. c:function:: uint8_t attachsql_get_library_version_minor(void)

   Returns the integer of the minor version number for the library

   :returns: The minor version number

   .. versionadded:: 0.1.0

attachsql_get_library_version_patch()
-------------------------------------

.. c:function:: uint8_t attachsql_get_library_version_patch(void)

   Returns the integer of the patch level for the library

   :returns: The patch level

   .. versionadded:: 0.1.0

attachsql_library_init()
------------------------

.. c:function:: void attachsql_library_init(void)

   Sets up the libAttachSQL library, currently only required for SSL connections.

   .. note::
      Should only be called once at the start of an application, before any other libAttachSQL function.

   .. versionadded:: 0.3.0
