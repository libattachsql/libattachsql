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
#include <libattachsql-2.0/attachsql.h>

int main(int argc, char *argv[])
{
  (void) argc;
  (void) argv;
  attachsql_connect_t *con;
  attachsql_error_t *error= NULL;
  const char *data= "SHOW PROCESSLIST";
  attachsql_return_t aret= ATTACHSQL_RETURN_NONE;
  uint16_t columns;

  con= attachsql_connect_create("localhost", 3306, "test", "test", "", NULL);
  attachsql_statement_prepare(con, strlen(data), data, &error);
  ASSERT_FALSE_(error, "Statement creation error");
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

  attachsql_statement_execute(con, &error);
  aret= ATTACHSQL_RETURN_NONE;
  while(aret != ATTACHSQL_RETURN_EOF)
  {
    aret= attachsql_connect_poll(con, &error);
    if (aret == ATTACHSQL_RETURN_ROW_READY)
    {
      columns= attachsql_query_column_count(con);
      attachsql_statement_row_get(con, &error);
      printf("Got %d columns\n", columns);
      attachsql_statement_row_next(con);
    }
    if (error)
    {
      ASSERT_FALSE_(true, "Error exists: %d", attachsql_error_code(error));
    }
  }
  attachsql_statement_close(con);
  attachsql_connect_destroy(con);
}
