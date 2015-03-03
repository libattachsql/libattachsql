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

#include <libattachsql-2.0/attachsql.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

uint8_t done_count= 0;

void callbk(attachsql_connect_t *current_con, attachsql_events_t events, void *context, attachsql_error_t *error);

void callbk(attachsql_connect_t *current_con, attachsql_events_t events, void *context, attachsql_error_t *error)
{
  uint8_t *con_no= (uint8_t*)context;
  attachsql_query_row_st *row;
  uint16_t columns, col;
  switch(events)
  {
    case ATTACHSQL_EVENT_CONNECTED:
      printf("Connected event on con %d\n", *con_no);
      break;
    case ATTACHSQL_EVENT_ERROR:
      printf("Error exists on con %d: %d", *con_no, attachsql_error_code(error));
      attachsql_error_free(error);
      break;
    case ATTACHSQL_EVENT_EOF:
      printf("Connection %d finished\n", *con_no);
      done_count++;
      attachsql_query_close(current_con);
      break;
    case ATTACHSQL_EVENT_ROW_READY:
      row= attachsql_query_row_get(current_con, &error);
      columns= attachsql_query_column_count(current_con);
      for (col=0; col < columns; col++)
      {
        printf("Con: %d, Column: %d, Length: %zu, Data: %.*s ", *con_no, col, row[col].length, (int)row[col].length, row[col].data);
      }
      attachsql_query_row_next(current_con);
      printf("\n");
      break;
    case ATTACHSQL_EVENT_NONE:
      break;
  }
}

int main(int argc, char *argv[])
{
  (void) argc;
  (void) argv;
  attachsql_connect_t *con[3];
  attachsql_group_t *group;
  attachsql_error_t *error= NULL;
  const char *query1= "SELECT * FROM t1 WHERE name='fred'";
  const char *query2= "SELECT * FROM t1 WHERE age >= 40";
  const char *query3= "SELECT * FROM t1 WHERE age < 40";
  uint8_t con_no[3]= {0, 1, 2};

  group= attachsql_group_create(NULL);
  con[0]= attachsql_connect_create("localhost", 3306, "test", "test", "testdb", NULL);
  attachsql_group_add_connection(group, con[0], &error);
  attachsql_connect_set_callback(con[0], callbk, &con_no[0]);
  con[1]= attachsql_connect_create("localhost", 3306, "test", "test", "testdb", NULL);
  attachsql_group_add_connection(group, con[1], &error);
  attachsql_connect_set_callback(con[1], callbk, &con_no[1]);
  con[2]= attachsql_connect_create("localhost", 3306, "test", "test", "testdb", NULL);
  attachsql_group_add_connection(group, con[2], &error);
  attachsql_connect_set_callback(con[2], callbk, &con_no[2]);
  attachsql_query(con[0], strlen(query1), query1, 0, NULL, &error);
  attachsql_query(con[1], strlen(query2), query2, 0, NULL, &error);
  attachsql_query(con[2], strlen(query3), query3, 0, NULL, &error);

  while(done_count < 3)
  {
    attachsql_group_run(group);
  }
  attachsql_group_destroy(group);
}
