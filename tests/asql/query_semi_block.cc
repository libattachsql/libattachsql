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
  attachsql_error_t *error= NULL;
  const char *data= "SHOW PROCESSLIST";
  const char *data2= "SELECT ? as a, '?' as b, ? as c, ? as d, ? as e";
  attachsql_return_t aret= ATTACHSQL_RETURN_NONE;
  attachsql_query_row_st *row;
  attachsql_query_parameter_st param[5];
  uint16_t columns, col;

  con= attachsql_connect_create("localhost", 3306, "test", "test", "", NULL);
  attachsql_connect_set_option(con, ATTACHSQL_OPTION_SEMI_BLOCKING, NULL);
  attachsql_query(con, strlen(data), data, 0, NULL, &error);
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
  const char *td= "test";
  uint32_t tn= 45768;
  float tf= 3.14159;
  double tlf= 3.1415926;
  param[0].type= ATTACHSQL_ESCAPE_TYPE_CHAR;
  param[0].data= (char*)td;
  param[0].length= strlen(td);
  param[1].type= ATTACHSQL_ESCAPE_TYPE_CHAR_LIKE;
  param[1].data= (char*)td;
  param[1].length= strlen(td);
  param[2].type= ATTACHSQL_ESCAPE_TYPE_INT;
  param[2].data= &tn;
  param[2].is_unsigned= true;
  param[3].type= ATTACHSQL_ESCAPE_TYPE_FLOAT;
  param[3].data= &tf;
  param[4].type= ATTACHSQL_ESCAPE_TYPE_DOUBLE;
  param[4].data= &tlf;
  attachsql_query(con, strlen(data2), data2, 5, param, &error);
  ASSERT_NULL_(error, "Error not NULL");
  aret= ATTACHSQL_RETURN_NONE;
  while(aret != ATTACHSQL_RETURN_EOF)
  {
    aret= attachsql_connect_poll(con, &error);
    if (aret == ATTACHSQL_RETURN_ROW_READY)
    {
      row= attachsql_query_row_get(con, &error);
      columns= attachsql_query_column_count(con);
      ASSERT_EQ_(5, columns, "Column count unexpected");
      ASSERT_STREQL_("test", row[0].data, 4, "Bad row data");
      ASSERT_STREQL_("test", row[1].data, 4, "Bad row data");
      ASSERT_STREQL_("45768", row[2].data, 5, "Bad row data");
      ASSERT_STREQL_("3.14159", row[3].data, 7, "Bad float data");
      ASSERT_STREQL_("3.1415926", row[4].data, 9, "Bad double data");
      attachsql_query_row_next(con);
      printf("\n");
    }
  }
  attachsql_query_close(con);
  attachsql_connect_destroy(con);
}
