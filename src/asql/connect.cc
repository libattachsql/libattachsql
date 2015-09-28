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
#include "src/asql/query_internal.h"
#include "src/asql/connect_internal.h"

attachsql_connect_t *attachsql_connect_create(const char *host, in_port_t port, const char *user, const char *pass, const char *schema, attachsql_error_t **error)
{
  attachsql_connect_t *con= NULL;

  con= new (std::nothrow) attachsql_connect_t;
  if (con == NULL)
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "82100", "Allocation failure for connection object");
    return NULL;
  }
  con->core_con= ascore_con_create(host, port, user, pass, schema);
  if (con->core_con == NULL)
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_ALLOC, ATTACHSQL_ERROR_LEVEL_ERROR, "82100", "Allocation failure for connection object");
    delete con;
    return NULL;
  }

  if (con->core_con->status == ASCORE_CON_STATUS_PARAMETER_ERROR)
  {
    if (con->core_con->local_errcode == ASRET_USER_TOO_LONG)
    {
      attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "User name too long");
    }
    else if (con->core_con->local_errcode == ASRET_SCHEMA_TOO_LONG)
    {
      attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Schema too long");
    }
    attachsql_connect_destroy(con);
    return NULL;
  }

  return con;
}

void attachsql_connect_destroy(attachsql_connect_t *con)
{
  bool in_group= false;
  if (con == NULL)
  {
    return;
  }

  if (con->in_query)
  {
    attachsql_query_close(con);
  }

  if (con->core_con != NULL)
  {
    in_group= con->core_con->in_group;
    ascore_con_destroy(con->core_con);
  }
  if (!in_group)
  {
    delete con;
  }
}

void attachsql_connect_set_callback(attachsql_connect_t *con, attachsql_callback_fn *function, void *context)
{
  con->callback_fn= function;
  con->callback_context= context;
}

const char *attachsql_connect_get_server_version(attachsql_connect_t *con)
{
  if (con == NULL)
  {
    return NULL;
  }

  return con->core_con->server_version;
}

uint32_t attachsql_connect_get_connection_id(attachsql_connect_t *con)
{
  if (con == NULL)
  {
    return 0;
  }
  return con->core_con->thread_id;
}

attachsql_return_t attachsql_connect_poll(attachsql_connect_t *con, attachsql_error_t **error)
{
  ascore_con_status_t status;

  if (con == NULL)
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Invalid connection object");
    return ATTACHSQL_RETURN_ERROR;
  }
  if (!con->core_con->in_group)
  {
    status= ascore_con_poll(con->core_con);
  }
  else
  {
    status= con->core_con->status;
  }

  switch (status)
  {
    case ASCORE_CON_STATUS_PARAMETER_ERROR:
      attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Bad parameter");
      if (con->callback_fn != NULL)
      {
        con->callback_fn(con, ATTACHSQL_EVENT_ERROR, con->callback_context, *error);
      }
      return ATTACHSQL_RETURN_ERROR;
      break;
    case ASCORE_CON_STATUS_NOT_CONNECTED:
      return ATTACHSQL_RETURN_NOT_CONNECTED;
      break;
    case ASCORE_CON_STATUS_CONNECTING:
      return ATTACHSQL_RETURN_CONNECTING;
      break;
    case ASCORE_CON_STATUS_CONNECT_FAILED:
      if (con->core_con->local_errcode == ASRET_DNS_ERROR)
      {
        attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_HOST_UNKNOWN, ATTACHSQL_ERROR_LEVEL_ERROR, "08000", con->core_con->errmsg);
      }
      else if (con->core_con->server_errno != 0)
      {
        attachsql_error_server_create(con, error);
      }
      else
      {
        if (con->core_con->errmsg[0] != '\0')
        {
          attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_CONNECT, ATTACHSQL_ERROR_LEVEL_ERROR, "08000", con->core_con->errmsg);
        }
        else
        {
          attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_CONNECT, ATTACHSQL_ERROR_LEVEL_ERROR, "08000", "Unknown connection failure");
        }
      }
      if (con->callback_fn != NULL)
      {
        con->callback_fn(con, ATTACHSQL_EVENT_ERROR, con->callback_context, *error);
      }
      return ATTACHSQL_RETURN_ERROR;
      break;
    case ASCORE_CON_STATUS_BUSY:
      return ATTACHSQL_RETURN_PROCESSING;
      break;
    case ASCORE_CON_STATUS_SSL_ERROR:
      attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_SSL, ATTACHSQL_ERROR_LEVEL_ERROR, "08000", con->core_con->errmsg);
      if (con->callback_fn != NULL)
      {
        con->callback_fn(con, ATTACHSQL_EVENT_ERROR, con->callback_context, *error);
      }
      return ATTACHSQL_RETURN_ERROR;
      break;
    case ASCORE_CON_STATUS_IDLE:
      if (con->core_con->server_errno != 0)
      {
        // Server error during query
        attachsql_error_server_create(con, error);
        if (con->callback_fn != NULL)
        {
          con->callback_fn(con, ATTACHSQL_EVENT_ERROR, con->callback_context, *error);
        }
        return ATTACHSQL_RETURN_ERROR;
      }
      else if (con->core_con->command_status == ASCORE_COMMAND_STATUS_EOF)
      {
        if (con->callback_fn != NULL)
        {
          con->callback_fn(con, ATTACHSQL_EVENT_EOF, con->callback_context, NULL);
        }
        con->all_rows_buffered= true;
        con->core_con->command_status= ASCORE_COMMAND_STATUS_NONE;
        return ATTACHSQL_RETURN_EOF;
      }
      else if (con->core_con->command_status == ASCORE_COMMAND_STATUS_ROW_IN_BUFFER)
      {
        if (con->buffer_rows)
        {
          return attachsql_query_row_buffer(con, error);
        }
        else
        {
          if (con->callback_fn != NULL)
          {
            con->callback_fn(con, ATTACHSQL_EVENT_ROW_READY, con->callback_context, NULL);
          }
          return ATTACHSQL_RETURN_ROW_READY;
        }
      }
      else if (con->core_con->command_status == ASCORE_COMMAND_STATUS_CONNECTED)
      {
        if (con->callback_fn != NULL)
        {
          con->callback_fn(con, ATTACHSQL_EVENT_CONNECTED, con->callback_context, NULL);
        }
        if (con->query_buffer_length > 0)
        {
          return attachsql_connect_query(con, error);
        }
      }
      return ATTACHSQL_RETURN_IDLE;
      break;
    case ASCORE_CON_STATUS_NET_ERROR:
      attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_SERVER_LOST, ATTACHSQL_ERROR_LEVEL_ERROR, "08006", con->core_con->errmsg);
      if (con->callback_fn != NULL)
      {
        con->callback_fn(con, ATTACHSQL_EVENT_ERROR, con->callback_context, *error);
      }
      return ATTACHSQL_RETURN_ERROR;
      break;
  }
  /* If we get this far 1. our compiler needs shooting for not catching a
   * missing case statement and 2. something bad probably happened anyway */
  attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_UNKNOWN, ATTACHSQL_ERROR_LEVEL_ERROR, "99999", "Unkown error occurred");
  return ATTACHSQL_RETURN_ERROR;
}

attachsql_return_t attachsql_connect_query(attachsql_connect_t *con, attachsql_error_t **error)
{
  ascore_command_status_t ret;

  if (con->query_buffer_statement)
  {
    con->stmt= ascore_stmt_prepare(con->core_con, con->query_buffer_length, con->query_buffer);
    ret= con->core_con->command_status;
  }
  else
  {
    ret= ascore_command_send(con->core_con, ASCORE_COMMAND_QUERY, con->query_buffer, con->query_buffer_length);
  }
  if (ret == ASCORE_COMMAND_STATUS_SEND_FAILED)
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_SERVER_GONE, ATTACHSQL_ERROR_LEVEL_ERROR, "08006", con->core_con->errmsg);
    return ATTACHSQL_RETURN_ERROR;
  }
  return ATTACHSQL_RETURN_PROCESSING;
}

bool attachsql_connect(attachsql_connect_t *con, attachsql_error_t **error)
{
  ascore_con_status_t status;

  status= ascore_connect(con->core_con);

  switch(status)
  {
    case ASCORE_CON_STATUS_PARAMETER_ERROR:
      attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Bad parameter");
      if (con->callback_fn != NULL)
      {
        con->callback_fn(con, ATTACHSQL_EVENT_ERROR, con->callback_context, *error);
      }
      return false;
    case ASCORE_CON_STATUS_NOT_CONNECTED:
      /* Shouldn't happen */
      return true;
    case ASCORE_CON_STATUS_CONNECTING:
      return true;
    case ASCORE_CON_STATUS_CONNECT_FAILED:
      if (con->core_con->local_errcode == ASRET_DNS_ERROR)
      {
        attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_HOST_UNKNOWN, ATTACHSQL_ERROR_LEVEL_ERROR, "08000", con->core_con->errmsg);
      }
      else if (con->core_con->server_errno != 0)
      {
        attachsql_error_server_create(con, error);
      }
      else
      {
        if (con->core_con->errmsg[0] != '\0')
        {
          attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_CONNECT, ATTACHSQL_ERROR_LEVEL_ERROR, "08000", con->core_con->errmsg);
        }
        else
        {
          attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_CONNECT, ATTACHSQL_ERROR_LEVEL_ERROR, "08000", "Unknown connection failure");
        }
      }
      if (con->callback_fn != NULL)
      {
        con->callback_fn(con, ATTACHSQL_EVENT_ERROR, con->callback_context, *error);
      }
      return false;
    case ASCORE_CON_STATUS_BUSY:
      /* Should never be hit */
      return false;
    case ASCORE_CON_STATUS_IDLE:
      if ((con->core_con->command_status == ASCORE_COMMAND_STATUS_CONNECTED) && (con->callback_fn != NULL))
      {
        con->callback_fn(con, ATTACHSQL_EVENT_CONNECTED, con->callback_context, NULL);
      }
      return true;
    case ASCORE_CON_STATUS_SSL_ERROR:
      attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_SSL, ATTACHSQL_ERROR_LEVEL_ERROR, "08000", con->core_con->errmsg);
      if (con->callback_fn != NULL)
      {
        con->callback_fn(con, ATTACHSQL_EVENT_ERROR, con->callback_context, *error);
      }

      return false;
    case ASCORE_CON_STATUS_NET_ERROR:
      attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_SERVER_LOST, ATTACHSQL_ERROR_LEVEL_ERROR, "08006", con->core_con->errmsg);
      if (con->callback_fn != NULL)
      {
        con->callback_fn(con, ATTACHSQL_EVENT_ERROR, con->callback_context, *error);
      }
      return false;
  }

  return true;
}

bool attachsql_connect_set_option(attachsql_connect_t *con, attachsql_options_t option, const void *arg)
{
  // arg option is for later
  (void) arg;
  if (con == NULL)
  {
    return false;
  }

  switch (option)
  {
    case ATTACHSQL_OPTION_COMPRESS:
#ifdef HAVE_ZLIB
      con->core_con->client_capabilities|= ASCORE_CAPABILITY_COMPRESS;
#else
      return false;
#endif
      break;
    case ATTACHSQL_OPTION_FOUND_ROWS:
      con->core_con->client_capabilities|= ASCORE_CAPABILITY_FOUND_ROWS;
      break;
    case ATTACHSQL_OPTION_IGNORE_SIGPIPE:
      con->core_con->client_capabilities|= ASCORE_CAPABILITY_IGNORE_SIGPIPE;
      break;
    case ATTACHSQL_OPTION_INTERACTIVE:
      con->core_con->client_capabilities|= ASCORE_CAPABILITY_INTERACTIVE;
      break;
    case ATTACHSQL_OPTION_LOCAL_FILES:
      con->core_con->client_capabilities|= ASCORE_CAPABILITY_LOCAL_FILES;
      break;
    case ATTACHSQL_OPTION_MULTI_STATEMENTS:
      con->core_con->client_capabilities|= ASCORE_CAPABILITY_MULTI_STATEMENTS;
      break;
    case ATTACHSQL_OPTION_NO_SCHEMA:
      con->core_con->client_capabilities|= ASCORE_CAPABILITY_NO_SCHEMA;
      break;
    case ATTACHSQL_OPTION_SSL_NO_VERIFY:
#ifdef HAVE_OPENSSL
      con->core_con->ssl.no_verify= true;
#else
      return false;
#endif
      break;
    case ATTACHSQL_OPTION_SEMI_BLOCKING:
      con->core_con->options.semi_block= true;
      break;
    case ATTACHSQL_OPTION_NONE:
      return false;
      break;
    default:
      return false;
  }
  return true;
}

#ifdef HAVE_OPENSSL
bool attachsql_connect_set_ssl(attachsql_connect_t *con, const char *key, const char *cert, const char *ca, const char *capath, const char *cipher, bool verify, attachsql_error_t **error)
{
  if (con == NULL)
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Connection parameter not valid");
    return false;
  }

  if (!ascore_con_set_ssl(con->core_con, key, cert, ca, capath, cipher, verify))
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_SSL, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", con->core_con->errmsg);
    return false;
  }
  return true;
}
#else
bool attachsql_connect_set_ssl(attachsql_connect_t *con, const char *key, const char *cert, const char *ca, const char *capath, const char *cipher, bool verify, attachsql_error_t **error)
{
  (void) con;
  (void) key;
  (void) cert;
  (void) ca;
  (void) capath;
  (void) cipher;
  (void) verify;

  attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_NO_SSL, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "SSL support has not been compiled in");
  return false;
}
#endif
