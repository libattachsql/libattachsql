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

void cycle_query(attachsql_connect_t *con)
{
  attachsql_error_t *error= NULL;
  attachsql_return_t aret= ATTACHSQL_RETURN_NONE;
  while (aret != ATTACHSQL_RETURN_EOF)
  {
    aret= attachsql_connect_poll(con, &error);
    if (aret == ATTACHSQL_RETURN_ROW_READY)
    {
      attachsql_query_row_next(con);
    }
    if (error && (attachsql_error_code(error) == 2002))
    {
      SKIP_IF_(true, "No MYSQL server");
    }
    else if (error != NULL)
    {
      ASSERT_FALSE_(true, "Error exists: %s", attachsql_error_message(error));
    }
  }
}

int main(int argc, char *argv[])
{
  (void) argc;
  (void) argv;
  attachsql_connect_t *con;
  attachsql_error_t *error= NULL;
  const char data[]= "CREATE DATABASE IF NOT EXISTS testdb";
  const char data2[]= "CREATE TABLE IF NOT EXISTS testdb.t1 (a int, b int)";
  const char data3[]= "START TRANSACTION";
  const char data4[]= "INSERT INTO testdb.t1 VALUES (1,2),(2,3),(3,4)";
  const char data5[]= "COMMIT";
  const char data6[]= "SELECT * FROM testdb.t1";
  attachsql_return_t aret= ATTACHSQL_RETURN_NONE;
  attachsql_query_row_st *row;

  con= attachsql_connect_create("localhost", 3306, "test", "test", "", NULL);
  attachsql_query(con, strlen(data), data, 0, NULL, &error);
  cycle_query(con);
  attachsql_query_close(con);

  attachsql_query(con, strlen(data2), data2, 0, NULL, &error);
  cycle_query(con);
  attachsql_query_close(con);

  attachsql_query(con, strlen(data3), data3, 0, NULL, &error);
  cycle_query(con);
  attachsql_query_close(con);

  attachsql_query(con, strlen(data4), data4, 0, NULL, &error);
  cycle_query(con);
  attachsql_query_close(con);

  attachsql_query(con, strlen(data5), data5, 0, NULL, &error);
  cycle_query(con);
  attachsql_query_close(con);

  attachsql_query(con, strlen(data6), data6, 0, NULL, &error);

  uint8_t current_row= 0;
  while(aret != ATTACHSQL_RETURN_EOF)
  {
    aret= attachsql_connect_poll(con, &error);
    if (aret == ATTACHSQL_RETURN_ROW_READY)
    {
      row= attachsql_query_row_get(con, &error);
      if (current_row == 0)
      {
        ASSERT_STREQL_("1", row[0].data, 1, "Bad row data");
        ASSERT_STREQL_("2", row[1].data, 1, "Bad row data");
      }
      if (current_row == 1)
      {
        ASSERT_STREQL_("2", row[0].data, 1, "Bad row data");
        ASSERT_STREQL_("3", row[1].data, 1, "Bad row data");
      }
      if (current_row == 2)
      {
        ASSERT_STREQL_("3", row[0].data, 1, "Bad row data");
        ASSERT_STREQL_("4", row[1].data, 1, "Bad row data");
      }
      current_row++;
      attachsql_query_row_next(con);
    }
    if (error && (attachsql_error_code(error) == 2002))
    {
      SKIP_IF_(true, "No MYSQL server");
    }
    else if (error)
    {
      ASSERT_FALSE_(true, "Error exists: %d", attachsql_error_code(error));
    }
  }
  attachsql_query_close(con);
  attachsql_connect_destroy(con);
}
