Troubleshooting
===============

SIGABRT when connecting
-----------------------

If you get "``Aborted (core dumped)``" whilst connecting this is either due to an out of memory error or far more likely the file descriptor limit has been hit.  In some cases there will be an error for this instead depending on where in the connect phase it happened.

libAttachSQL requires two file descriptors per connection due to the internal event loop.

Poor performance for multi-threads
----------------------------------

libAttachSQL was designed to run many connections in a single thread and the technologies used to do that do not scale out for one connection per thread.  The library does, however, have a mode for threaded applications.  If you enable the semi-blocking mode you will find much better performance in these scenarios, at the cost of losing some of the non-blocking benefits.  You can enable it by running this before connecting or queries:

.. code-block:: cpp

   attachsql_connect_set_option(con, ATTACHSQL_OPTION_SEMI_BLOCKING, NULL);

Semi-blocking mode will block until some data is sent or received, so polling will still be required but will be needed less often.
