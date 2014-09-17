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
#include "src/ascore/ascore.h"
#include "src/ascore/statement.h"
#include "src/asql/statement_internal.h"

attachsql_error_st *attachsql_statement_prepare(attachsql_connect_t *con, size_t length, const char *statement)
{
  if (con->core_con->status == ASCORE_CON_STATUS_NOT_CONNECTED)
  {
    con->query_buffer= (char*)statement;
    con->query_buffer_length= length;
    con->query_buffer_alloc= false;
    con->query_buffer_statement= true;
    return attachsql_connect(con);
  }
  attachsql_error_st *error= NULL;
  con->stmt= ascore_stmt_prepare(con->core_con, length, statement);
  if (con->stmt == NULL)
  {
    attachsql_error_client_create(&error, ATTACHSQL_ERROR_CODE_ALLOC, ATTACHSQL_ERROR_LEVEL_ERROR, "82100", "Allocation failure for statement object");
  }
  return error;
}

attachsql_error_st *attachsql_statement_execute(attachsql_connect_t *con)
{
  attachsql_error_st *error;
  if (con == NULL)
  {
    attachsql_error_client_create(&error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "No connection provided");
    return error;
  }
  if (con->stmt == NULL)
  {
    attachsql_error_client_create(&error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "No statement prepared");
    return error;
  }
  if (not ascore_stmt_execute(con->stmt))
  {
    if (con->core_con->local_errcode == ASRET_BAD_STMT_PARAMETER)
    {
      attachsql_error_client_create(&error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Bad parameter bound to statement");
      return error;
    }
    else
    {
      attachsql_error_client_create(&error, ATTACHSQL_ERROR_CODE_ALLOC, ATTACHSQL_ERROR_LEVEL_ERROR, "82100", "Allocation failure for statement object");
      return error;
    }
  }
  return NULL;
}

attachsql_error_st *attachsql_statement_reset(attachsql_connect_t *con)
{
  attachsql_error_st *error;
  if (con == NULL)
  {
    attachsql_error_client_create(&error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "No connection provided");
    return error;
  }
  if (con->stmt == NULL)
  {
    attachsql_error_client_create(&error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "No statement prepared");
    return error;
  }

  if (ascore_stmt_reset(con->stmt) == ASCORE_COMMAND_STATUS_SEND_FAILED)
  {
    attachsql_error_client_create(&error, ATTACHSQL_ERROR_CODE_SERVER_GONE, ATTACHSQL_ERROR_LEVEL_ERROR, "08006", con->core_con->errmsg);
    return error;
  }

  return NULL;
}

attachsql_error_st *attachsql_statement_send_long_data(attachsql_connect_t *con, uint16_t param, size_t length, char *data)
{
  attachsql_error_st *error;
  if (con == NULL)
  {
    attachsql_error_client_create(&error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "No statement provided");
    return error;
  }
  if (con->stmt == NULL)
  {
    attachsql_error_client_create(&error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "No statement prepared");
    return error;
  }

  if (ascore_stmt_send_long_data(con->stmt, param, length, data) == ASCORE_COMMAND_STATUS_SEND_FAILED)
  {
    attachsql_error_client_create(&error, ATTACHSQL_ERROR_CODE_SERVER_GONE, ATTACHSQL_ERROR_LEVEL_ERROR, "08006", con->core_con->errmsg);
    return error;
  }

  return NULL;
}
