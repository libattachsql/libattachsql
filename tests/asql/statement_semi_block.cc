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
  const char *data= "SELECT ? as a, ? as b, CONVERT_TZ(FROM_UNIXTIME(1196440219),@@session.time_zone,'+00:00') as c";
  const char *data2= "hello world";
  attachsql_return_t aret= ATTACHSQL_RETURN_NONE;
  uint16_t columns;

  con= attachsql_connect_create("localhost", 3306, "test", "test", "", NULL);
  attachsql_connect_set_option(con, ATTACHSQL_OPTION_SEMI_BLOCKING, NULL);
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
  ASSERT_EQ_(2, attachsql_statement_get_param_count(con), "Wrong number of params");
  attachsql_statement_set_string(con, 0, 11, data2, NULL);
  attachsql_statement_set_int(con, 1, 123456, NULL);
  attachsql_statement_execute(con, &error);
  aret= ATTACHSQL_RETURN_NONE;
  while(aret != ATTACHSQL_RETURN_EOF)
  {
    aret= attachsql_connect_poll(con, &error);
    if (aret == ATTACHSQL_RETURN_ROW_READY)
    {
      columns= attachsql_statement_get_column_count(con);
      attachsql_statement_row_get(con, &error);
      printf("Got %d columns\n", columns);
      size_t len;
      char *col_data= attachsql_statement_get_char(con, 0, &len, &error);
      printf("Column 0: %.*s\n", (int)len, col_data);
      printf("Column 1: %d\n", attachsql_statement_get_int(con, 1, &error));
      ASSERT_EQ_(123456, attachsql_statement_get_int(con, 1, &error), "Column 1 result match fail");
      ASSERT_EQ_(ATTACHSQL_COLUMN_TYPE_LONG, attachsql_statement_get_column_type(con, 1), "Column 1 type match fail");
      ASSERT_STREQL_("hello world", col_data, len, "Column 0 result match fail");
      col_data= attachsql_statement_get_char(con, 1, &len, &error);
      ASSERT_STREQL_("123456", col_data, len, "Column 0 str conversion fail");
      col_data= attachsql_statement_get_char(con, 2, &len, &error);
      printf("Column 2: %.*s\n", (int)len, col_data);
      ASSERT_STREQL_("2007-11-30 16:30:19", col_data, len, "Column 2 str conversion fail");
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
