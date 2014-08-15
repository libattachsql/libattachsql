API Examples
============

A Basic Query
-------------

This is a basic query example which can be found at ``examples/basic_query.c`` in the libAttachSQL source.

It will connect to a MySQL server with the username *test*, password *test* and default database *testdb*.  It will then execute the query:

.. code-block:: mysql

   SELECT * FROM t1 WHERE name='fred';

Source Code
^^^^^^^^^^^

.. code-block:: c

   /* vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
    * Copyright 2014 Hewlett-Packard Development Company, L.P.
    *
    * Licensed under the Apache License, Version 2.0 (the "License"); you may
    * not use this file except in compliance with the License. You may obtain 
    * a copy of the License at
    *
    *      http://www.apache.org/licenses/LICENSE-2.0
    *
    * Unless required by applicable law or agreed to in writing, software
    * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
    * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
    * License for the specific language governing permissions and limitations
    * under the License.
    *
    */

   #include <libattachsql-1.0/attachsql.h>
   #include <stddef.h>
   #include <stdio.h>
   #include <string.h>

   int main(int argc, char *argv[])
   {
     attachsql_connect_t *con= NULL;
     attachsql_error_st *error= NULL;
     const char *query= "SELECT * FROM t1 WHERE name='fred'";
     attachsql_return_t ret= ATTACHSQL_RETURN_NONE;
     attachsql_query_row_st *row;
     uint16_t columns, current_column;

     con= attachsql_connect_create("localhost", 3306, "test", "test", "testdb", NULL);
     error= attachsql_query(con, strlen(query), query, 0, NULL);

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
     if (error != NULL)
     {
       printf("Error occurred: %s", error->msg);
     }
     attachsql_query_close(con);
     attachsql_connect_destroy(con);
   }

Breaking it Down
^^^^^^^^^^^^^^^^

First we create the connection object, the connection does not happen at this stage but everything required for the connection is set up:

.. code-block:: c

   con= attachsql_connect_create("localhost", 3306, "test", "test", "testdb", NULL);

Next we send a query to the MySQL server, at this stage the query will only be put on the network buffer, it will not be sent immediately.  If there is a problem at this stage an error will be returned:

.. code-block:: c

   error= attachsql_query(con, strlen(query), query, 0, NULL);

We want to loop until all the rows have been retrieved (``ATTACHSQL_RETURN_EOF``) or an error occurs:

.. code-block:: c

   while ((ret != ATTACHSQL_RETURN_EOF) && (error == NULL))
   {

Now we poll, the poll will send data when possible and retrieve data when available.  It will return with a status immediately every time:

.. code-block:: c

   ret= attachsql_connect_poll(con, &error);

If the status is that we do not have a complete row yet, loop again:

.. code-block:: c

   if (ret != ATTACHSQL_RETURN_ROW_READY)
   {
     continue;
   }

Getting this far means there is a row ready for us on the network buffer, lets get this:

.. code-block:: c

   row= attachsql_query_row_get(con, &error);

We also need to know how many columns are in the result set.  Technically this only needs to be run on the first loop but in this example it is being retrieved on every loop:

.. code-block:: c

   columns= attachsql_query_column_count(con);

Now data for all the columns is sent to *stdout*.  The column data is **not** NULL terminated so we use the length parameter to tell ``printf`` how long the data is.:

.. code-block:: c

   for (current_column= 0; current_column < columns; current_column++)
   {
     printf("%.*s ", (int)row[current_column].length, row[current_column].data);
   }

Finally we tell the library we are done with this row and to grab another:

.. code-block:: c

   attachsql_query_row_next(con);

At the end of the program everything is cleaned up nicely:

.. code-block:: c

   attachsql_query_close(con);
   attachsql_connect_destroy(con);

