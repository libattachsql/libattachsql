Connection Group Functions
==========================

attachsql_group_create()
------------------------

.. c:function:: attachsql_group_t *attachsql_group_create(attachsql_error_t **error)

   Creates and initializes a connection group object

   :param error: A pointer to a pointer of an error object which is created if an error occurs
   :returns: A newly created group object or :c:type:`NULL` on failure

   .. versionadded:: 0.9.0

attachsql_group_destroy()
-------------------------

.. c:function:: void attachsql_group_destroy(attachsql_group_t *group)

   Destroys a connection group along with all the underlying connections.

   .. warning::
      Do NOT use :c:func:`attachsql_connect_destroy` on any connection used by a connection group.  This will cause a double-free to occur.

   :param group: The group object to destroy

   .. versionadded:: 0.9.0

attachsql_group_add_connection()
--------------------------------

.. c:function:: void attachsql_group_add_connection(attachsql_group_t *group, attachsql_connect_t *con, attachsql_error_t **error)

   Adds a connection to the group object.

   .. warning::
      * This should be a newly created connection with :c:func:`attachsql_connect_create` that has not yet connected to the server.
      * You should NOT use :c:func:`attachsql_connect_poll` with a connection in a group
      * You should NOT use :c:func:`attachsql_connect_destroy` with a connection in a group
      * See the examples in this documentation for more information

   :param group: The connection group object to add to
   :param con: The connection object to add
   :param error: A pointer to a pointer of an error object which is created if an error occurs

   .. versionadded:: 0.9.0

attachsql_group_run()
---------------------

.. c:function:: void attachsql_group_run(attachsql_group_t *group)

   Runs the event loop for the connection group, firing the callbacks if any event has occurred.

   :param group: The connection group to run

   .. versionadded:: 0.9.0
