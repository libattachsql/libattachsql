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
  attachsql_query_buffer_rows(con, true);
  error= attachsql_query(con, strlen(query), query, 0, NULL);

  while ((ret != ATTACHSQL_RETURN_EOF) && (error == NULL))
  {
    ret= attachsql_connect_poll(con, &error);
  }
  if (error != NULL)
  {
    printf("Error occurred: %s", error->msg);
    return;
  }

  columns= attachsql_query_column_count(con);
  while((row= attachsql_query_buffer_row_get(con)))
  {
    for (current_column= 0; current_column < columns; current_column++)
    {
      printf("%.*s ", (int)row[current_column].length, row[current_column].data);
    }
    printf("\n");
  }
  if (error != NULL)
  {
    printf("Error occurred: %s", error->msg);
  }
  attachsql_query_close(con);
  attachsql_connect_destroy(con);
}
