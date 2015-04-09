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
#include "connect.h"
#include "sha1.h"
#include "net.h"
#include "query_internal.h"
#include <errno.h>
#include <string.h>
#ifdef HAVE_OPENSSL
# include <openssl/ssl.h>
#endif

attachsql_connect_t *attachsql_connect_create(const char *host, in_port_t port, const char *user, const char *pass, const char *schema, attachsql_error_t **error)
{
  attachsql_connect_t *con;

  con = new (std::nothrow) attachsql_connect_t;
  if (con == NULL)
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_ALLOC, ATTACHSQL_ERROR_LEVEL_ERROR, "82100", "Allocation failure for connection object");
    return NULL;
  }

  con->host= host;
  con->port= port;
  if ((user == NULL) or (strlen(user) > ATTACHSQL_MAX_USER_SIZE))
  {
    con->local_errcode= ATTACHSQL_RET_USER_TOO_LONG;
    con->status= ATTACHSQL_CON_STATUS_PARAMETER_ERROR;
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "User name too long");
    attachsql_connect_destroy(con);
    return NULL;
  }
  con->user= user;
  // We don't really care how long pass is since we itterate though it during
  // SHA1 passes.  Needs to be nul terminated.  NULL is also acceptable.
  con->pass= pass;
  if ((schema == NULL) or (strlen(schema) > ATTACHSQL_MAX_SCHEMA_SIZE))
  {
    con->local_errcode= ATTACHSQL_RET_SCHEMA_TOO_LONG;
    con->status= ATTACHSQL_CON_STATUS_PARAMETER_ERROR;
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Schema too long");
    attachsql_connect_destroy(con);
    return NULL;
  }
  con->schema= schema;

  return con;
}

void attachsql_connect_destroy(attachsql_connect_t *con)
{
  if (con == NULL)
  {
    return;
  }

  if (con->in_query)
  {
    attachsql_query_close(con);
  }

  if (con->read_buffer != NULL)
  {
    attachsql_buffer_free(con->read_buffer);
  }

  if (con->read_buffer_compress != NULL)
  {
    attachsql_buffer_free(con->read_buffer_compress);
  }

  if (con->uncompressed_buffer != NULL)
  {
    free(con->uncompressed_buffer);
  }

  if (con->compressed_buffer != NULL)
  {
    free(con->compressed_buffer);
  }

  if (con->next_packet_queue != NULL)
  {
    free(con->next_packet_queue);
  }

#ifdef HAVE_OPENSSL
  if (con->ssl.bio_buffer != NULL)
  {
    free(con->ssl.bio_buffer);
  }
  if (con->ssl.write_buffer != NULL)
  {
    attachsql_buffer_free(con->ssl.write_buffer);
  }
  /* This frees BIOs as well */
  if (con->ssl.ssl != NULL)
  {
    SSL_free(con->ssl.ssl);
  }
  if (con->ssl.context != NULL)
  {
    SSL_CTX_free(con->ssl.context);
  }
#endif
  // On a net error we already closed the connection
  if ((con->uv_objects.stream != NULL) and (con->status != ATTACHSQL_CON_STATUS_NET_ERROR))
  {
    uv_check_stop(&con->uv_objects.check);
    uv_close((uv_handle_t*)con->uv_objects.stream, NULL);
    if (not con->in_pool)
    {
      uv_run(con->uv_objects.loop, UV_RUN_DEFAULT);
    }
  }
  if (not con->in_pool)
  {
    uv_loop_close(con->uv_objects.loop);
    delete con->uv_objects.loop;
    delete con;
  }
}

void on_resolved(uv_getaddrinfo_t *resolver, int status, struct addrinfo *res)
{
  attachsql_connect_t *con= (attachsql_connect_t *)resolver->data;

  asdebug("Resolver callback");
  if (status < 0)
  {
    asdebug("DNS lookup failure: %s", uv_err_name(status));
    con->status= ATTACHSQL_CON_STATUS_CONNECT_FAILED;
    con->local_errcode= ATTACHSQL_RET_DNS_ERROR;
    snprintf(con->errmsg, ATTACHSQL_ERROR_BUFFER_SIZE, "DNS lookup failure: %s", uv_err_name(status));
    return;
  }
  char addr[17] = {'\0'};

  uv_ip4_name((struct sockaddr_in*) res->ai_addr, addr, 16);
  asdebug("DNS lookup success: %s", addr);
  uv_tcp_init(resolver->loop, &con->uv_objects.socket.tcp);
  con->uv_objects.socket.tcp.data= con;
  con->uv_objects.connect_req.data= (void*) &con->uv_objects.socket.tcp;
  uv_tcp_connect(&con->uv_objects.connect_req, &con->uv_objects.socket.tcp, (const struct sockaddr*) res->ai_addr, on_connect);

  uv_freeaddrinfo(res);
}

attachsql_con_status_t attachsql_do_poll(attachsql_connect_t *con)
{
  //asdebug("Connection poll");
  if (con == NULL)
  {
    return ATTACHSQL_CON_STATUS_PARAMETER_ERROR;
  }

  if ((con->status == ATTACHSQL_CON_STATUS_NOT_CONNECTED) or (con->status == ATTACHSQL_CON_STATUS_CONNECT_FAILED) or (con->status == ATTACHSQL_CON_STATUS_IDLE) or (con->status == ATTACHSQL_CON_STATUS_SSL_ERROR))
  {
    return con->status;
  }
#ifdef HAVE_OPENSSL
  if (con->ssl.handshake_done)
  {
    attachsql_ssl_run(con);
  }
#endif
  attachsql_run_uv_loop(con);

  return con->status;
}

attachsql_return_t attachsql_connect_poll(attachsql_connect_t *con, attachsql_error_t **error)
{
  attachsql_con_status_t status;

  if (con == NULL)
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Invalid connection object");
    return ATTACHSQL_RETURN_ERROR;
  }
  if (not con->in_pool)
  {
    status= attachsql_do_poll(con);
  }
  else
  {
    status= con->status;
  }

  switch (status)
  {
    case ATTACHSQL_CON_STATUS_PARAMETER_ERROR:
      attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Bad parameter");
      if (con->callback_fn != NULL)
      {
        con->callback_fn(con, ATTACHSQL_EVENT_ERROR, con->callback_context, *error);
      }
      return ATTACHSQL_RETURN_ERROR;
      break;
    case ATTACHSQL_CON_STATUS_NOT_CONNECTED:
      return ATTACHSQL_RETURN_NOT_CONNECTED;
      break;
    case ATTACHSQL_CON_STATUS_CONNECTING:
      return ATTACHSQL_RETURN_CONNECTING;
      break;
    case ATTACHSQL_CON_STATUS_CONNECT_FAILED:
      if (con->local_errcode == ATTACHSQL_RET_DNS_ERROR)
      {
        attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_HOST_UNKNOWN, ATTACHSQL_ERROR_LEVEL_ERROR, "08000", con->errmsg);
      }
      else if (con->server_errno != 0)
      {
        attachsql_error_server_create(con, error);
      }
      else
      {
        if (con->errmsg[0] != '\0')
        {
          attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_CONNECT, ATTACHSQL_ERROR_LEVEL_ERROR, "08000", con->errmsg);
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
    case ATTACHSQL_CON_STATUS_BUSY:
      return ATTACHSQL_RETURN_PROCESSING;
      break;
    case ATTACHSQL_CON_STATUS_SSL_ERROR:
      attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_SSL, ATTACHSQL_ERROR_LEVEL_ERROR, "08000", con->errmsg);
      if (con->callback_fn != NULL)
      {
        con->callback_fn(con, ATTACHSQL_EVENT_ERROR, con->callback_context, *error);
      }
      return ATTACHSQL_RETURN_ERROR;
      break;
    case ATTACHSQL_CON_STATUS_IDLE:
      if (con->server_errno != 0)
      {
        // Server error during query
        attachsql_error_server_create(con, error);
        if (con->callback_fn != NULL)
        {
          con->callback_fn(con, ATTACHSQL_EVENT_ERROR, con->callback_context, *error);
        }
        return ATTACHSQL_RETURN_ERROR;
      }
      else if (con->command_status == ATTACHSQL_COMMAND_STATUS_EOF)
      {
        if (con->callback_fn != NULL)
        {
          con->callback_fn(con, ATTACHSQL_EVENT_EOF, con->callback_context, NULL);
        }
        con->all_rows_buffered= true;
        con->command_status= ATTACHSQL_COMMAND_STATUS_EOF;
        return ATTACHSQL_RETURN_EOF;
      }
      else if (con->command_status == ATTACHSQL_COMMAND_STATUS_ROW_IN_BUFFER)
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
      else if (con->command_status == ATTACHSQL_COMMAND_STATUS_CONNECTED)
      {
        if (con->callback_fn != NULL)
        {
          con->callback_fn(con, ATTACHSQL_EVENT_CONNECTED, con->callback_context, NULL);
        }
        if (con->query_buffer_length > 0)
        {
          return attachsql_connect_query(con, error);
        }
        else
        {
          return ATTACHSQL_RETURN_EOF;
        }
      }
      /* If we hit here something went wrong */
      assert(0);
      return ATTACHSQL_RETURN_EOF;
      break;
    case ATTACHSQL_CON_STATUS_NET_ERROR:
      attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_SERVER_LOST, ATTACHSQL_ERROR_LEVEL_ERROR, "08006", con->errmsg);
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

void attachsql_check_for_data_cb(uv_check_t *handle)
{
  asdebug("Check called");
  struct attachsql_connect_t *con= (struct attachsql_connect_t*)handle->data;
  attachsql_con_process_packets(con);
}

attachsql_con_status_t attachsql_do_connect(attachsql_connect_t *con)
{
  int ret;

  if (con == NULL)
  {
    return ATTACHSQL_CON_STATUS_PARAMETER_ERROR;
  }

  con->uv_objects.hints.ai_family = PF_INET;
  con->uv_objects.hints.ai_socktype = SOCK_STREAM;
  con->uv_objects.hints.ai_protocol = IPPROTO_TCP;
  con->uv_objects.hints.ai_flags = 0;

  if (con->status != ATTACHSQL_CON_STATUS_NOT_CONNECTED)
  {
    return con->status;
  }

  if (not con->in_pool)
  {
    con->uv_objects.loop= new (std::nothrow) uv_loop_t;
    ret= uv_loop_init(con->uv_objects.loop);
    if (ret < 0)
    {
      asdebug("Loop initalize failure");
      con->local_errcode= ATTACHSQL_RET_OUT_OF_MEMORY_ERROR;
      snprintf(con->errmsg, ATTACHSQL_ERROR_BUFFER_SIZE, "Loop initialization failure, either out of memory or out of file descripitors (usually the latter)");
      con->status= ATTACHSQL_CON_STATUS_CONNECT_FAILED;
      return con->status;
    }
  }

  snprintf(con->str_port, 6, "%d", con->port);
  // If port is 0 and no explicit option set then assume we mean UDS
  // instead of TCP
  if (con->options.protocol == ATTACHSQL_CON_PROTOCOL_UNKNOWN)
  {
    if (con->port == 0)
    {
      con->options.protocol= ATTACHSQL_CON_PROTOCOL_UDS;
    }
    else
    {
      con->options.protocol= ATTACHSQL_CON_PROTOCOL_TCP;
    }
  }
  switch(con->options.protocol)
  {
    case ATTACHSQL_CON_PROTOCOL_TCP:
      asdebug("TCP connection");
      asdebug("Async DNS lookup: %s", con->host);
      con->uv_objects.resolver.data= con;
      ret= uv_getaddrinfo(con->uv_objects.loop, &con->uv_objects.resolver, on_resolved, con->host, con->str_port, &con->uv_objects.hints);
      if (ret < 0)
      {
        asdebug("DNS lookup fail: %s", uv_err_name(ret));
        con->local_errcode= ATTACHSQL_RET_DNS_ERROR;
        snprintf(con->errmsg, ATTACHSQL_ERROR_BUFFER_SIZE, "DNS lookup failure: %s", uv_err_name(ret));
        con->status= ATTACHSQL_CON_STATUS_CONNECT_FAILED;
        return con->status;
      }
      con->status= ATTACHSQL_CON_STATUS_CONNECTING;
      attachsql_run_uv_loop(con);
      break;
    case ATTACHSQL_CON_PROTOCOL_UDS:
      asdebug("UDS connection");
      uv_pipe_init(con->uv_objects.loop, &con->uv_objects.socket.uds, 1);
      con->uv_objects.socket.uds.data= con;
      con->uv_objects.connect_req.data= (void*) &con->uv_objects.socket.uds;
      con->status= ATTACHSQL_CON_STATUS_CONNECTING;
      uv_pipe_connect(&con->uv_objects.connect_req, &con->uv_objects.socket.uds, con->host, on_connect);
      attachsql_run_uv_loop(con);
      break;
    case ATTACHSQL_CON_PROTOCOL_UNKNOWN:
      asdebug("Unknown protocol, this shouldn't happen");
      con->status= ATTACHSQL_CON_STATUS_CONNECT_FAILED;
  }

  return con->status;
}

attachsql_return_t attachsql_connect_query(attachsql_connect_t *con, attachsql_error_t **error)
{
  attachsql_command_status_t ret;

  if (con->query_buffer_statement)
  {
    attachsql_statement_prepare(con, con->query_buffer_length, con->query_buffer, NULL);
    ret= con->command_status;
  }
  else
  {
    ret= attachsql_command_send(con, ATTACHSQL_COMMAND_QUERY, con->query_buffer, con->query_buffer_length);
  }
  if (ret == ATTACHSQL_COMMAND_STATUS_SEND_FAILED)
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_SERVER_GONE, ATTACHSQL_ERROR_LEVEL_ERROR, "08006", con->errmsg);
    return ATTACHSQL_RETURN_ERROR;
  }
  return ATTACHSQL_RETURN_PROCESSING;
}

bool attachsql_connect(attachsql_connect_t *con, attachsql_error_t **error)
{
  attachsql_con_status_t status;

  /* TODO: Merge this in? */
  status= attachsql_do_connect(con);

  switch(status)
  {
    case ATTACHSQL_CON_STATUS_PARAMETER_ERROR:
      attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Bad parameter");
      if (con->callback_fn != NULL)
      {
        con->callback_fn(con, ATTACHSQL_EVENT_ERROR, con->callback_context, *error);
      }
      return false;
      break;
    case ATTACHSQL_CON_STATUS_NOT_CONNECTED:
      /* Shouldn't happen */
      return true;
      break;
    case ATTACHSQL_CON_STATUS_CONNECTING:
      return true;
      break;
    case ATTACHSQL_CON_STATUS_CONNECT_FAILED:
      if (con->local_errcode == ATTACHSQL_RET_DNS_ERROR)
      {
        attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_HOST_UNKNOWN, ATTACHSQL_ERROR_LEVEL_ERROR, "08000", con->errmsg);
      }
      else if (con->server_errno != 0)
      {
        attachsql_error_server_create(con, error);
      }
      else
      {
        if (con->errmsg[0] != '\0')
        {
          attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_CONNECT, ATTACHSQL_ERROR_LEVEL_ERROR, "08000", con->errmsg);
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
      break;
    case ATTACHSQL_CON_STATUS_BUSY:
      /* Should never be hit */
      return false;
      break;
    case ATTACHSQL_CON_STATUS_IDLE:
      if ((con->command_status == ATTACHSQL_COMMAND_STATUS_CONNECTED) and (con->callback_fn != NULL))
      {
        con->callback_fn(con, ATTACHSQL_EVENT_CONNECTED, con->callback_context, NULL);
      }
      return true;
      break;
    case ATTACHSQL_CON_STATUS_SSL_ERROR:
      attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_SSL, ATTACHSQL_ERROR_LEVEL_ERROR, "08000", con->errmsg);
      if (con->callback_fn != NULL)
      {
        con->callback_fn(con, ATTACHSQL_EVENT_ERROR, con->callback_context, *error);
      }

      return false;
      break;
    case ATTACHSQL_CON_STATUS_NET_ERROR:
      attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_SERVER_LOST, ATTACHSQL_ERROR_LEVEL_ERROR, "08006", con->errmsg);
      if (con->callback_fn != NULL)
      {
        con->callback_fn(con, ATTACHSQL_EVENT_ERROR, con->callback_context, *error);
      }
      return false;
      break;
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
      con->client_capabilities|= ATTACHSQL_CAPABILITY_COMPRESS;
#else
      return false;
#endif
      break;
    case ATTACHSQL_OPTION_FOUND_ROWS:
      con->client_capabilities|= ATTACHSQL_CAPABILITY_FOUND_ROWS;
      break;
    case ATTACHSQL_OPTION_IGNORE_SIGPIPE:
      con->client_capabilities|= ATTACHSQL_CAPABILITY_IGNORE_SIGPIPE;
      break;
    case ATTACHSQL_OPTION_INTERACTIVE:
      con->client_capabilities|= ATTACHSQL_CAPABILITY_INTERACTIVE;
      break;
    case ATTACHSQL_OPTION_LOCAL_FILES:
      con->client_capabilities|= ATTACHSQL_CAPABILITY_LOCAL_FILES;
      break;
    case ATTACHSQL_OPTION_MULTI_STATEMENTS:
      con->client_capabilities|= ATTACHSQL_CAPABILITY_MULTI_STATEMENTS;
      break;
    case ATTACHSQL_OPTION_NO_SCHEMA:
      con->client_capabilities|= ATTACHSQL_CAPABILITY_NO_SCHEMA;
      break;
    case ATTACHSQL_OPTION_SSL_NO_VERIFY:
#ifdef HAVE_OPENSSL
      con->ssl.no_verify= true;
#else
      return false;
#endif
      break;
    case ATTACHSQL_OPTION_SEMI_BLOCKING:
      con->options.semi_block= true;
      break;
    case ATTACHSQL_OPTION_NONE:
      return false;
      break;
    default:
      return false;
  }
  return true;
}

void on_connect(uv_connect_t *req, int status)
{
  attachsql_connect_t *con= (attachsql_connect_t*)req->handle->data;
  asdebug("Connect event callback");
  if (status < 0)
  {
    asdebug("Connect fail: %s", uv_err_name(status));
    con->local_errcode= ATTACHSQL_RET_CONNECT_ERROR;
    con->status= ATTACHSQL_CON_STATUS_CONNECT_FAILED;
    snprintf(con->errmsg, ATTACHSQL_ERROR_BUFFER_SIZE, "Connection failed: %s", uv_err_name(status));
    return;
  }
  asdebug("Connection succeeded!");
  attachsql_packet_queue_push(con, ATTACHSQL_PACKET_TYPE_HANDSHAKE);
  // maybe move the set con->stream to connect function
  con->uv_objects.stream= (uv_stream_t*)req->data;
  uv_check_init(con->uv_objects.loop, &con->uv_objects.check);
  con->uv_objects.check.data= con;
  uv_check_start(&con->uv_objects.check, attachsql_check_for_data_cb);
  uv_read_start((uv_stream_t*)req->data, on_alloc, attachsql_read_data_cb);
}

void on_alloc(uv_handle_t *client, size_t suggested_size, uv_buf_t *buf)
{
  size_t buffer_free;
  attachsql_connect_t *con= (attachsql_connect_t*) client->data;

#ifdef HAVE_OPENSSL
  if (con->ssl.handshake_done && (con->ssl.bio_buffer_size < suggested_size))
  {
    asdebug("Increasing SSL read buffer to %zd", suggested_size);
    char *realloc_buffer= (char*)realloc(con->ssl.bio_buffer, suggested_size);
    if (realloc_buffer)
    {
      con->ssl.bio_buffer= realloc_buffer;
      con->ssl.bio_buffer_size= suggested_size;
    }
    else
    {
      con->local_errcode= ATTACHSQL_RET_OUT_OF_MEMORY_ERROR;
      asdebug("SSL buffer realloc failure");
      con->command_status= ATTACHSQL_COMMAND_STATUS_SEND_FAILED;
      con->next_packet_queue_used= 0;
    }
  }
#endif

  if (not con->options.compression)
  {
    asdebug("%zd bytes requested for read buffer", suggested_size);

    if (con->read_buffer == NULL)
    {
      asdebug("Creating read buffer");
      con->read_buffer= attachsql_buffer_create();
    }
    buffer_free= attachsql_buffer_get_available(con->read_buffer);
    if (buffer_free < suggested_size)
    {
      asdebug("Enlarging buffer, free: %zd, requested: %zd", buffer_free, suggested_size);
      attachsql_buffer_increase(con->read_buffer);
      buffer_free= attachsql_buffer_get_available(con->read_buffer);
    }
    buf->base= con->read_buffer->buffer_write_ptr;
    buf->len= buffer_free;
  }
  else
  {
    asdebug("%zd bytes requested for compressed read buffer", suggested_size);

    if (con->read_buffer_compress == NULL)
    {
      asdebug("Creating compressed read buffer");
      con->read_buffer_compress= attachsql_buffer_create();
    }
    buffer_free= attachsql_buffer_get_available(con->read_buffer_compress);
    if (buffer_free < suggested_size)
    {
      asdebug("Enlarging compress buffer, free: %zd, requested: %zd", buffer_free, suggested_size);
      attachsql_buffer_increase(con->read_buffer_compress);
      buffer_free= attachsql_buffer_get_available(con->read_buffer_compress);
    }
    buf->base= con->read_buffer_compress->buffer_write_ptr;
    buf->len= buffer_free;
  }

#ifdef HAVE_OPENSSL
  if (con->ssl.handshake_done)
  {
    buf->base= con->ssl.bio_buffer;
    buf->len= con->ssl.bio_buffer_size;
  }
#endif
}

void attachsql_packet_read_handshake(attachsql_connect_t *con)
{
  asdebug("Connect handshake packet");
  buffer_st *buffer= con->read_buffer;

  // Rejection error before handshake
  if ((unsigned char)buffer->buffer_read_ptr[0] == 0xff)
  {
    attachsql_packet_read_response(con);
  }

  // Protocol version
  if (buffer->buffer_read_ptr[0] != 10)
  {
    // Note that 255 is a special immediate auth fail case
    asdebug("Bad protocol version");
    con->local_errcode= ATTACHSQL_RET_BAD_PROTOCOL;
    snprintf(con->errmsg, ATTACHSQL_ERROR_BUFFER_SIZE, "Incompatible protocol version");
    return;
  }

  // Server version (null-terminated string)
  buffer->buffer_read_ptr++;
  strncpy(con->server_version, buffer->buffer_read_ptr, ATTACHSQL_MAX_SERVER_VERSION_LEN);
  con->server_version[ATTACHSQL_MAX_SERVER_VERSION_LEN - 1]= '\0';
  buffer->buffer_read_ptr+= strlen(con->server_version) + 1;

  // Thread ID
  con->thread_id= attachsql_unpack_int4(buffer->buffer_read_ptr);
  buffer->buffer_read_ptr+= 4;

  // Scramble buffer and 1 byte filler
  memcpy(con->scramble_buffer, buffer->buffer_read_ptr, 8);
  buffer->buffer_read_ptr+= 9;

  // Server capabilities
  con->server_capabilities= (attachsql_capabilities_t)attachsql_unpack_int2(buffer->buffer_read_ptr);
  buffer->buffer_read_ptr+= 2;
  // Check MySQL 4.1 protocol capability is on, we won't support old auth
  if (not (con->server_capabilities & ATTACHSQL_CAPABILITY_PROTOCOL_41))
  {
    asdebug("MySQL <4.1 Auth not supported");
    con->local_errcode= ATTACHSQL_RET_NO_OLD_AUTH;
    snprintf(con->errmsg, ATTACHSQL_ERROR_BUFFER_SIZE, "MySQL 4.1 protocol and higher required");
  }

  con->charset= buffer->buffer_read_ptr[0];
  buffer->buffer_read_ptr++;

  con->server_status= attachsql_unpack_int2(buffer->buffer_read_ptr);
  // 13 byte filler and unrequired scramble length (until auth plugins)
  buffer->buffer_read_ptr+= 15;

  memcpy(con->scramble_buffer + 8, buffer->buffer_read_ptr, 12);
  // '\0' scramble terminator
  buffer->buffer_read_ptr+= 13;

  // MySQL 5.5 onwards has more password plugin stuff here, ignore for now
  attachsql_packet_read_end(con);

  // Create response packet
  attachsql_handshake_response(con);
}

void attachsql_handshake_response(attachsql_connect_t *con)
{
  unsigned char *buffer_ptr;
  uint32_t capabilities;

  asdebug("Sending handshake response");
  buffer_ptr= (unsigned char*)con->write_buffer;

  capabilities= con->server_capabilities & ATTACHSQL_CAPABILITY_CLIENT;
  capabilities|= ATTACHSQL_CAPABILITY_MULTI_RESULTS;
  capabilities|= con->client_capabilities;

#ifdef HAVE_OPENSSL
  if (con->ssl.ssl != NULL)
  {
    if (not (con->server_capabilities & ATTACHSQL_CAPABILITY_SSL))
    {
      asdebug("SSL disabled on server");
      con->local_errcode= ATTACHSQL_RET_NET_SSL_ERROR;
      snprintf(con->errmsg, ATTACHSQL_ERROR_BUFFER_SIZE, "SSL auth not supported enabled on server");
      con->command_status= ATTACHSQL_COMMAND_STATUS_SEND_FAILED;
      con->next_packet_queue_used= 0;
      con->status= ATTACHSQL_CON_STATUS_CONNECT_FAILED;
      return;
    }
    capabilities |= ATTACHSQL_CAPABILITY_SSL;
  }
#endif

  attachsql_pack_int4(buffer_ptr, capabilities);
  buffer_ptr+= 4;

  // Set max packet size to our buffer size for now
  attachsql_pack_int4(buffer_ptr, ATTACHSQL_DEFAULT_BUFFER_SIZE);
  buffer_ptr+= 4;

  // Change this when we support charsets
  buffer_ptr[0]= 0;
  buffer_ptr++;

  // 0x00 padding for 23 bytes
  memset(buffer_ptr, 0, 23);
  buffer_ptr+= 23;

#ifdef HAVE_OPENSSL
  if ((con->ssl.ssl != NULL) and (attachsql_packet_queue_peek(con) != ATTACHSQL_PACKET_TYPE_HANDSHAKE_SSL))
  {
    /* for SSL we do a short handshake ending here in plain text,
     * no user/password sent.
     * The enablement of the SSL capability tells the server that OpenSSL
     * handshake happens next followed by a full MySQL handshake packet with
     * user/pass encrypted
     */
    attachsql_send_data(con, con->write_buffer, (size_t)(buffer_ptr - (unsigned char*)con->write_buffer));
    attachsql_packet_queue_push(con, ATTACHSQL_PACKET_TYPE_HANDSHAKE_SSL);
    attachsql_handshake_response(con);
    return;
  }

  if ((con->ssl.ssl != NULL) and (attachsql_packet_queue_peek(con) == ATTACHSQL_PACKET_TYPE_HANDSHAKE_SSL))
  {
    /* second entry into handshake for SSL, this response and all other send /
     * receives should be encrypted from now on
     */
    con->ssl.enabled= true;
    attachsql_packet_queue_pop(con);
  }
#endif

  // User name
  memcpy(buffer_ptr, con->user, strlen(con->user));
  buffer_ptr+= strlen(con->user);
  buffer_ptr[0]= '\0';
  buffer_ptr++;

  // Password
  // TODO: add support for password plugins
  if (con->pass[0] != '\0')
  {
    attachsql_ret_t ret;
    buffer_ptr[0]= SHA1_DIGEST_LENGTH; // probably should use char packing?
    buffer_ptr++;
    ret= scramble_password(con, (unsigned char*)buffer_ptr);
    if (ret != ATTACHSQL_RET_OK)
    {
      asdebug("Scramble problem!");
      con->local_errcode= ATTACHSQL_RET_BAD_SCRAMBLE;
      return;
    }
    buffer_ptr+= SHA1_DIGEST_LENGTH;
  }
  else
  {
    buffer_ptr[0]= '\0';
    buffer_ptr++;
  }

  if (con->schema != NULL)
  {
    memcpy(buffer_ptr, con->schema, strlen(con->schema));
    buffer_ptr+= strlen(con->schema);
  }
  buffer_ptr[0]= '\0';
  buffer_ptr++;
  attachsql_send_data(con, con->write_buffer, (size_t)(buffer_ptr - (unsigned char*)con->write_buffer));
  attachsql_packet_queue_push(con, ATTACHSQL_PACKET_TYPE_RESPONSE);
}

attachsql_ret_t scramble_password(attachsql_connect_t *con, unsigned char *buffer)
{
  SHA1_CTX ctx;
  unsigned char stage1[SHA1_DIGEST_LENGTH];
  unsigned char stage2[SHA1_DIGEST_LENGTH];
  uint8_t it;

  if (con->scramble_buffer[0] == '\0')
  {
    asdebug("No scramble supplied from server");
    return ATTACHSQL_RET_NO_SCRAMBLE;
  }

  // Double hash the password
  SHA1Init(&ctx);
  SHA1Update(&ctx, (unsigned char*)con->pass, strlen(con->pass));
  SHA1Final(stage1, &ctx);
  SHA1Init(&ctx);
  SHA1Update(&ctx, stage1, SHA1_DIGEST_LENGTH);
  SHA1Final(stage2, &ctx);

  // Hash the scramble with the double hash
  SHA1Init(&ctx);
  SHA1Update(&ctx, con->scramble_buffer, SHA1_DIGEST_LENGTH);
  SHA1Update(&ctx, stage2, SHA1_DIGEST_LENGTH);
  SHA1Final(buffer, &ctx);

  // XOR the hash with the stage1 hash
  for (it= 0; it < SHA1_DIGEST_LENGTH; it++)
  {
    buffer[it]= buffer[it] ^ stage1[it];
  }

  return ATTACHSQL_RET_OK;
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

  return con->server_version;
}

uint32_t attachsql_connect_get_connection_id(attachsql_connect_t *con)
{
  if (con == NULL)
  {
    return 0;
  }
  return con->thread_id;
}

#ifdef HAVE_OPENSSL
bool attachsql_connect_set_ssl(attachsql_connect_t *con, const char *key, const char *cert, const char *ca, const char *capath, const char *cipher, bool verify, attachsql_error_t **error)
{
  if (con == NULL)
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Connection parameter not valid");
    return false;
  }

  con->ssl.context= SSL_CTX_new(TLSv1_client_method());

  if (cipher != NULL)
  {
    if (SSL_CTX_set_cipher_list(con->ssl.context, cipher) != 1)
    {
      strncpy(con->errmsg, "Error setting SSL cipher list", ATTACHSQL_ERROR_BUFFER_SIZE - 1);
      con->errmsg[ATTACHSQL_ERROR_BUFFER_SIZE - 1]= '\0';
      attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_SSL, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", con->errmsg);
      return false;
    }
  }

  if (SSL_CTX_load_verify_locations(con->ssl.context, ca, capath) != 1)
  {
    strncpy(con->errmsg, "Error loading the SSL certificate authority file", ATTACHSQL_ERROR_BUFFER_SIZE -1);
    con->errmsg[ATTACHSQL_ERROR_BUFFER_SIZE - 1]= '\0';
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_SSL, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", con->errmsg);
    return false;
  }

  if (cert != NULL)
  {
    if (SSL_CTX_use_certificate_file(con->ssl.context, cert, SSL_FILETYPE_PEM) != 1)
    {
      strncpy(con->errmsg, "Error loading the SSL certificate file", ATTACHSQL_ERROR_BUFFER_SIZE - 1);
      con->errmsg[ATTACHSQL_ERROR_BUFFER_SIZE - 1]= '\0';
      attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_SSL, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", con->errmsg);
      return false;
    }

    if (key == NULL)
    {
      key= cert;
    }

    if (SSL_CTX_use_PrivateKey_file(con->ssl.context, key, SSL_FILETYPE_PEM) != 1)
    {
      strncpy(con->errmsg, "Cannot load the SSL key file", ATTACHSQL_ERROR_BUFFER_SIZE - 1);
      con->errmsg[ATTACHSQL_ERROR_BUFFER_SIZE - 1]= '\0';
      attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_SSL, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", con->errmsg);
      return false;
    }

    if (SSL_CTX_check_private_key(con->ssl.context) != 1)
    {
      strncpy(con->errmsg, "Error validating the SSL private key", ATTACHSQL_ERROR_BUFFER_SIZE - 1);
      con->errmsg[ATTACHSQL_ERROR_BUFFER_SIZE - 1]= '\0';
      attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_SSL, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", con->errmsg);
      return false;
    }
  }

  if (verify)
  {
    SSL_CTX_set_verify(con->ssl.context, SSL_VERIFY_PEER, NULL);
  }
  con->ssl.ssl= SSL_new(con->ssl.context);
  /* force client handshake mode to allow SSL_write before handshake complete */
  SSL_set_connect_state(con->ssl.ssl);

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
