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

attachsql_pool_t *attachsql_pool_create(attachsql_error_t **error)
{
  attachsql_pool_t *pool= NULL;

  pool= new (std::nothrow) attachsql_pool_t;

  if (pool == NULL)
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_ALLOC, ATTACHSQL_ERROR_LEVEL_ERROR, "82100", "Allocation failure for pool object");
    return NULL;
  }
  pool->loop= new (std::nothrow) uv_loop_t;
  uv_loop_init(pool->loop);
  if (pool->loop == NULL)
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_ALLOC, ATTACHSQL_ERROR_LEVEL_ERROR, "82100", "Allocation failure for pool event loop");
    delete pool;
    return NULL;
  }
  return pool;
}

void attachsql_pool_destroy(attachsql_pool_t *pool)
{
  size_t connection;
  if (pool == NULL)
  {
    return;
  }
  for (connection= 0; connection < pool->connection_count; connection++)
  {
    attachsql_connect_destroy(pool->connections[connection]);
  }
  uv_run(pool->loop, UV_RUN_DEFAULT);
  uv_loop_close(pool->loop);
  delete pool->loop;
  for (connection= 0; connection < pool->connection_count; connection++)
  {
    delete pool->connections[connection];
  }
  if (pool->connections != NULL)
  {
    free(pool->connections);
  }
  delete pool;
}

void attachsql_pool_add_connection(attachsql_pool_t *pool, attachsql_connect_t *con, attachsql_error_t **error)
{
  attachsql_connect_t **tmp_cons;

  if ((pool == NULL) || (con == NULL))
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Bad parameter");
    return;
  }

  tmp_cons= (attachsql_connect_t**)realloc(pool->connections, sizeof(attachsql_connect_t*) * (pool->connection_count + 1));
  if (tmp_cons != NULL)
  {
    pool->connections= tmp_cons;
    pool->connections[pool->connection_count]= con;
    con->in_pool= true;
    con->uv_objects.loop= pool->loop;
    pool->connection_count++;
  }
  else
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_ALLOC, ATTACHSQL_ERROR_LEVEL_ERROR, "82100", "Allocation failure for pool connection add");
  }
}

void attachsql_pool_run(attachsql_pool_t *pool)
{
  size_t connection;
  attachsql_error_t *error= NULL;
  if (pool == NULL)
  {
    return;
  }
  uv_run(pool->loop, UV_RUN_NOWAIT);
  for (connection= 0; connection < pool->connection_count; connection++)
  {
    attachsql_connect_poll(pool->connections[connection], &error);
  }
}
