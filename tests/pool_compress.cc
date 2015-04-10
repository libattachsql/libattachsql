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
#include <libattachsql2/attachsql.h>

static int done= 0;

void callbk(attachsql_connect_t *current_con, uint32_t connection_id, attachsql_events_t events, void *context, attachsql_error_t *error)
{
  (void) context;
  attachsql_query_row_st *row;
  uint16_t columns, col;
  switch(events)
  {
    case ATTACHSQL_EVENT_CONNECTED:
      printf("Connected event on con %d\n", connection_id);
      break;
    case ATTACHSQL_EVENT_ERROR:
      if (error && (attachsql_error_code(error) == 2002))
      {
        SKIP_IF_(true, "No MYSQL server");
      }
      else
      {
        ASSERT_FALSE_(true, "Error exists on con %d: %d", connection_id, attachsql_error_code(error));
      }
      // Normally we would attachsql_error_free() here
      break;
    case ATTACHSQL_EVENT_EOF:
      printf("Connection %d finished\n", connection_id);
      done++;
      attachsql_query_close(current_con);
      break;
    case ATTACHSQL_EVENT_ROW_READY:
      row= attachsql_query_row_get(current_con, &error);
      columns= attachsql_query_column_count(current_con);
      for (col=0; col < columns; col++)
      {
        printf("Con: %d, Column: %d, Length: %zu, Data: %.*s ", connection_id, col, row[col].length, (int)row[col].length, row[col].data);
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
  attachsql_pool_t *pool;
  attachsql_error_t *error= NULL;
  const char *data= "SHOW PROCESSLIST";

  pool= attachsql_pool_create(callbk, NULL, NULL);
  con[0]= attachsql_connect_create("localhost", 3306, "test", "test", "", NULL);
  bool compress= attachsql_connect_set_option(con[0], ATTACHSQL_OPTION_COMPRESS, NULL);
  SKIP_IF_(!compress, "Not compiled with ZLib");
  attachsql_pool_add_connection(pool, con[0], &error);
  con[1]= attachsql_connect_create("localhost", 3306, "test", "test", "", NULL);
  attachsql_connect_set_option(con[1], ATTACHSQL_OPTION_COMPRESS, NULL);
  attachsql_pool_add_connection(pool, con[1], &error);
  con[2]= attachsql_connect_create("localhost", 3306, "test", "test", "", NULL);
  attachsql_connect_set_option(con[2], ATTACHSQL_OPTION_COMPRESS, NULL);
  attachsql_pool_add_connection(pool, con[2], &error);
  attachsql_query(con[0], strlen(data), data, 0, NULL, &error);
  attachsql_query(con[1], strlen(data), data, 0, NULL, &error);
  attachsql_query(con[2], strlen(data), data, 0, NULL, &error);

  while(done < 3)
  {
    attachsql_pool_run(pool);
  }
  attachsql_pool_destroy(pool);
}
