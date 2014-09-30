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

#include "command.h"
#include "net.h"

#ifdef HAVE_ZLIB
ascore_command_status_t ascore_command_send_compressed(ascon_st *con, ascore_command_t command, char *data, size_t length)
{
  ascore_send_compressed_packet(con, data, length, command);
  if (con->status == ASCORE_CON_STATUS_IDLE)
  {
    if (command == ASCORE_COMMAND_STMT_PREPARE)
    {
      con->next_packet_type= ASCORE_PACKET_TYPE_PREPARE_RESPONSE;
    }
    else
    {
      con->next_packet_type= ASCORE_PACKET_TYPE_RESPONSE;
    }
    uv_read_start(con->uv_objects.stream, on_alloc, ascore_read_data_cb);
    con->command_status= ASCORE_COMMAND_STATUS_SEND;
    con->status= ASCORE_CON_STATUS_BUSY;
    if (con->options.semi_block)
    {
      uv_run(con->uv_objects.loop, UV_RUN_ONCE);
    }
    else
    {
      uv_run(con->uv_objects.loop, UV_RUN_NOWAIT);
    }
    return ASCORE_COMMAND_STATUS_SEND;
  }
  return con->command_status;
}
#endif

ascore_command_status_t ascore_command_send(ascon_st *con, ascore_command_t command, char *data, size_t length)
{
  uv_buf_t send_buffer[3];
  int ret;

  if (con->status != ASCORE_CON_STATUS_IDLE)
  {
    /* Server not ready */
    return con->command_status;
  }

  /* Reset a bunch of internals */
  con->result.current_column= 0;
  con->affected_rows= 0;
  con->insert_id= 0;
  con->server_status= 0;
  con->warning_count= 0;
  con->server_errno= 0;

  asdebug("Sending command %02X to sever", command);
  con->local_errcode= ASRET_OK;
  con->errmsg[0]= '\0';

  ascore_pack_int3(con->packet_header, length + 1);
  con->packet_number= 0;
  con->packet_header[3] = con->packet_number;

#ifdef HAVE_ZLIB
  if (con->client_capabilities & ASCORE_CAPABILITY_COMPRESS)
  {
    return ascore_command_send_compressed(con, command, data, length);
  }
#endif

  con->write_buffer[0]= command;
  send_buffer[0].base= con->packet_header;
  send_buffer[0].len= 4;
  send_buffer[1].base= con->write_buffer;
  send_buffer[1].len= 1 + con->write_buffer_extra;
  con->write_buffer_extra= 0;
  if (length > 0)
  {
    send_buffer[2].base= data;
    send_buffer[2].len= length;
    asdebug("Sending %zd bytes with command to server", length);
    asdebug_hex(data, length);
#ifdef HAVE_OPENSSL
    if (con->ssl.handshake_done)
    {
      ret= ascore_ssl_buffer_write(con, send_buffer, 3);
    }
    else
#endif
    {
      uv_write_t *req= new (std::nothrow) uv_write_t;
      ret= uv_write(req, con->uv_objects.stream, send_buffer, 3, on_write);
    }
  }
  else
  {
    asdebug("Sending command with no data");
#ifdef HAVE_OPENSSL
    if (con->ssl.handshake_done)
    {
      ret= ascore_ssl_buffer_write(con, send_buffer, 2);
    }
    else
#endif
    {
      uv_write_t *req= new (std::nothrow) uv_write_t;
      ret= uv_write(req, con->uv_objects.stream, send_buffer, 2, on_write);
    }
  }
  if (ret != 0)
  {
    con->local_errcode= ASRET_NET_WRITE_ERROR;
    asdebug("Write fail: %s", uv_err_name(uv_last_error(con->uv_objects.loop)));
    con->command_status= ASCORE_COMMAND_STATUS_SEND_FAILED;
    con->next_packet_type= ASCORE_PACKET_TYPE_NONE;
    con->local_errcode= ASRET_NET_WRITE_ERROR;
    snprintf(con->errmsg, ASCORE_ERROR_BUFFER_SIZE, "Query send failed: %s", uv_err_name(uv_last_error(con->uv_objects.loop)));
    return con->command_status;
  }

  if (command == ASCORE_COMMAND_STMT_PREPARE)
  {
    con->next_packet_type= ASCORE_PACKET_TYPE_PREPARE_RESPONSE;
  }
  else if (command == ASCORE_COMMAND_STMT_RESET)
  {
    con->next_packet_type= ASCORE_PACKET_TYPE_NONE;
  }
  else
  {
    con->next_packet_type= ASCORE_PACKET_TYPE_RESPONSE;
  }
  if (command != ASCORE_COMMAND_STMT_RESET)
  {
    uv_read_start(con->uv_objects.stream, on_alloc, ascore_read_data_cb);
  }
  con->command_status= ASCORE_COMMAND_STATUS_SEND;
  con->status= ASCORE_CON_STATUS_BUSY;
  if (con->options.semi_block)
  {
    uv_run(con->uv_objects.loop, UV_RUN_ONCE);
  }
  else
  {
    uv_run(con->uv_objects.loop, UV_RUN_NOWAIT);
  }
  return ASCORE_COMMAND_STATUS_SEND;
}

ascore_command_status_t ascore_get_next_row(ascon_st *con)
{
  ascore_buffer_packet_read_end(con->read_buffer);
  con->next_packet_type= ASCORE_PACKET_TYPE_ROW;
  con->command_status= ASCORE_COMMAND_STATUS_READ_ROW;
  ascore_con_process_packets(con);
  return con->command_status;
}

bool ascore_command_next_result(ascon_st *con)
{
  if (con == NULL)
  {
    return false;
  }
  if (con->server_status & ASCORE_SERVER_STATUS_MORE_RESULTS)
  {
    /* Reset a bunch of internals */
    con->result.current_column= 0;
    con->affected_rows= 0;
    con->insert_id= 0;
    con->server_status= 0;
    con->warning_count= 0;
    con->server_errno= 0;
    con->next_packet_type= ASCORE_PACKET_TYPE_RESPONSE;
    uv_read_start(con->uv_objects.stream, on_alloc, ascore_read_data_cb);
    con->command_status= ASCORE_COMMAND_STATUS_READ_RESPONSE;
    con->status= ASCORE_CON_STATUS_BUSY;
    if (con->options.semi_block)
    {
      uv_run(con->uv_objects.loop, UV_RUN_ONCE);
    }
    else
    {
      uv_run(con->uv_objects.loop, UV_RUN_NOWAIT);
    }
    return true;
  }
  return false;
}

void ascore_command_free(ascon_st *con)
{
  if (con->result.columns != NULL)
  {
    delete[] con->result.columns;
  }
  con->result.columns= NULL;
}
