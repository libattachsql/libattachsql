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

#include <libattachsql2/attachsql.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
  attachsql_connect_t *con= NULL;
  attachsql_error_t *error= NULL;
  const char *query= "SELECT * FROM t1 WHERE name = ? AND age > ?";
  attachsql_return_t ret= ATTACHSQL_RETURN_NONE;

  con= attachsql_connect_create("localhost", 3306, "test", "test", "testdb", NULL);
  attachsql_statement_prepare(con, strlen(query), query, &error);
  while((ret != ATTACHSQL_RETURN_EOF) && (error == NULL))
  {
    ret= attachsql_connect_poll(con, &error);
  }
  if (error != NULL)
  {
    printf("Error occurred: %s", attachsql_error_message(error));
    attachsql_error_free(error);
    attachsql_statement_close(con);
    attachsql_connect_destroy(con);
    return 1;
  }

  const char *name= "fred";
  uint32_t age= 30;
  attachsql_statement_set_string(con, 0, strlen(name), name, NULL);
  attachsql_statement_set_int(con, 1, age, NULL);
  attachsql_statement_execute(con, &error);
  ret= ATTACHSQL_RETURN_NONE;
  while ((ret != ATTACHSQL_RETURN_EOF) && (error == NULL))
  {
    ret= attachsql_connect_poll(con, &error);
    if (ret != ATTACHSQL_RETURN_ROW_READY)
    {
      continue;
    }
    attachsql_statement_row_get(con, &error);
    printf("ID: %d, ", attachsql_statement_get_int(con, 0, &error));
    size_t len;
    char *name_data= attachsql_statement_get_char(con, 1, &len, &error);
    printf("Name: %.*s, ", (int)len, name_data);
    printf("Age: %d\n", attachsql_statement_get_int(con, 2, &error));
    attachsql_statement_row_next(con);
  }
  if (error != NULL)
  {
    printf("Error occurred: %s", attachsql_error_message(error));
    attachsql_error_free(error);
  }
  attachsql_statement_close(con);
  attachsql_connect_destroy(con);
}
