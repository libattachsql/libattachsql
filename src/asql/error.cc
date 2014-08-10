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
#include "src/asql/common.h"
#include <stdio.h>
#include <stdarg.h>

/* At a later date we can probably optimise away the new/deletes with a buffer.
 * For now we will copy and delete */
void attachsql_error_free(attachsql_error_st *err)
{
  if (err == NULL)
  {
    return;
  }

  delete err;
}
__attribute__((__format__ (__printf__, 5, 6)))
void attachsql_error_client_create(attachsql_error_st **error, int code, attachsql_error_level_t level, const char *sqlstate, const char *msg, ...)
{
  va_list args;
  attachsql_error_st *new_err= NULL;

  if (error == NULL)
  {
    /* NULL has been passed, so error disabled */
    return;
  }

  new_err= new (std::nothrow) attachsql_error_st;

  if (new_err == NULL)
  {
    /* No good way of handling this scenario */
    return;
  }
  va_start(args, msg);
  new_err->code= code;
  new_err->level= level;
  vsnprintf(new_err->msg, ATTACHSQL_MESSAGE_SIZE, msg, args);
  if (sqlstate != NULL)
  {
    memcpy(new_err->sqlstate, sqlstate, ATTACHSQL_SQLSTATE_SIZE - 1);
    new_err->sqlstate[ATTACHSQL_SQLSTATE_SIZE - 1]= '\0';
  }

  *error= new_err;
  va_end(args);
}

void attachsql_error_server_create(attachsql_connect_t *con, attachsql_error_st **error)
{
  attachsql_error_st *new_err= NULL;

  if (con == NULL)
  {
    return;
  }
  if (error == NULL)
  {
    /* NULL has been passed, so error disabled */
    return;
  }
  new_err= new (std::nothrow) attachsql_error_st;

  if (new_err == NULL)
  {
    /* No good way of handling this scenario */
    return;
  }
  new_err->code= con->core_con->server_errno;
  strncpy(new_err->msg, con->core_con->server_message, ATTACHSQL_MESSAGE_SIZE - 1);
  new_err->msg[ATTACHSQL_MESSAGE_SIZE - 1]= '\0';
  memcpy(new_err->sqlstate, con->core_con->sqlstate, ATTACHSQL_SQLSTATE_SIZE - 1);
  new_err->sqlstate[ATTACHSQL_SQLSTATE_SIZE - 1]= '\0';
  *error= new_err;
}
