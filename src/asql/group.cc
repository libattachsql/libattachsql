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

#include "config.h"
#include "common.h"

attachsql_group_t *attachsql_group_create(attachsql_error_t **error)
{
  attachsql_group_t *group= NULL;

  group= new (std::nothrow) attachsql_group_t;

  if (group == NULL)
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_ALLOC, ATTACHSQL_ERROR_LEVEL_ERROR, "82100", "Allocation failure for group object");
    return NULL;
  }
  group->loop= uv_loop_new();
  if (group->loop == NULL)
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_ALLOC, ATTACHSQL_ERROR_LEVEL_ERROR, "82100", "Allocation failure for group event loop");
    delete group;
    return NULL;
  }
  return group;
}

void attachsql_group_destroy(attachsql_group_t *group)
{
  size_t connection;
  if (group == NULL)
  {
    return;
  }
  for (connection= 0; connection < group->connection_count; connection++)
  {
    attachsql_connect_destroy(group->connections[connection]);
  }
  uv_run(group->loop, UV_RUN_DEFAULT);
  uv_loop_delete(group->loop);
  for (connection= 0; connection < group->connection_count; connection++)
  {
    delete group->connections[connection]->core_con;
    delete group->connections[connection];
  }
  if (group->connections != NULL)
  {
    free(group->connections);
  }
  delete group;
}

void attachsql_group_add_connection(attachsql_group_t *group, attachsql_connect_t *con, attachsql_error_t **error)
{
  attachsql_connect_t **tmp_cons;

  if ((group == NULL) || (con == NULL))
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Bad parameter");
    return;
  }

  tmp_cons= (attachsql_connect_t**)realloc(group->connections, sizeof(attachsql_connect_t*) * (group->connection_count + 1));
  if (tmp_cons != NULL)
  {
    group->connections= tmp_cons;
    group->connections[group->connection_count]= con;
    con->core_con->in_group= true;
    con->core_con->uv_objects.loop= group->loop;
    group->connection_count++;
  }
  else
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_ALLOC, ATTACHSQL_ERROR_LEVEL_ERROR, "82100", "Allocation failure for group connection add");
  }
}

void attachsql_group_run(attachsql_group_t *group)
{
  size_t connection;
  attachsql_error_t *error= NULL;
  if (group == NULL)
  {
    return;
  }
  uv_run(group->loop, UV_RUN_NOWAIT);
  for (connection= 0; connection < group->connection_count; connection++)
  {
    attachsql_connect_poll(group->connections[connection], &error);
  }
}
