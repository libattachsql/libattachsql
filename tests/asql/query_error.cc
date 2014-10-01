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
  const char *data= "SELECT * FROM NO_SUCH_TABLE";
  attachsql_return_t aret= ATTACHSQL_RETURN_NONE;
  attachsql_query_row_st *row;
  uint16_t columns, col;

  con= attachsql_connect_create("localhost", 3306, "test", "test", "", NULL);
  error= attachsql_query(con, strlen(data), data, 0, NULL);
  while(aret != ATTACHSQL_RETURN_EOF)
  {
    aret= attachsql_connect_poll(con, &error);
    if (aret == ATTACHSQL_RETURN_ROW_READY)
    {
      row= attachsql_query_row_get(con, &error);
      columns= attachsql_query_column_count(con);
      for (col=0; col < columns; col++)
      {
        printf("Column: %d, Length: %zu, Data: %.*s ", col, row[col].length, (int)row[col].length, row[col].data);
      }
      attachsql_query_row_next(con);
      printf("\n");
    }
    else if (aret == ATTACHSQL_RETURN_ERROR)
    {
      break;
    }
  }
  SKIP_IF_((attachsql_error_code(error) == 2002), "No MySQL server");
  ASSERT_EQ_(ATTACHSQL_RETURN_ERROR, aret, "Query should have error'd");
  ASSERT_EQ_(1046, attachsql_error_code(error), "Error code is wrong");
  ASSERT_STREQL_("3D000", attachsql_error_sqlstate(error), 5, "SQLSTATE is wrong");
  ASSERT_STREQ_("No database selected", attachsql_error_message(error), "Message is wrong");
  attachsql_error_free(error);
  attachsql_query_close(con);
  attachsql_connect_destroy(con);
}
