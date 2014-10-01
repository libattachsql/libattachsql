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

#include <yatl/lite.h>
#include "version.h"
#include <libattachsql-1.0/attachsql.h>

int main(int argc, char *argv[])
{
  (void) argc;
  (void) argv;
  attachsql_connect_t *con;
  attachsql_error_t *error;
  const char *data= "SHOW PROCESSLIST";
  attachsql_return_t aret= ATTACHSQL_RETURN_NONE;
  attachsql_query_row_st *row;
  uint16_t columns, col;

  con= attachsql_connect_create("localhost", 3306, "test", "test", "", NULL);
  attachsql_query_buffer_rows(con, true);
  error= attachsql_query(con, strlen(data), data, 0, NULL);
  while(aret != ATTACHSQL_RETURN_EOF)
  {
    aret= attachsql_connect_poll(con, &error);
    if (error && (attachsql_error_code(error) == 2002))
    {
      SKIP_IF_(true, "No MYSQL server");
    }
    else if (error)
    {
      ASSERT_FALSE_(true, "Error exists: %d", attachsql_error_code(error));
    }
  }
  columns= attachsql_query_column_count(con);
  ASSERT_TRUE(attachsql_query_row_count(con) > 0);
  while((row= attachsql_query_buffer_row_get(con)))
  {
    for (col=0; col < columns; col++)
    {
      printf("Column: %d, Length: %zu, Data: %.*s ", col, row[col].length, (int)row[col].length, row[col].data);
    }
    printf("\n");
  }

  attachsql_query_close(con);
  attachsql_connect_destroy(con);
}
