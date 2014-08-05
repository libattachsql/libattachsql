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
#include "net.h"
#include "pack.h"
#include "pack_macros.h"

void ascore_send_data(ascon_st *con, char *data, size_t length)
{
  uv_buf_t send_buffer[2];

  asdebug("Sending %zd bytes to server", length);
  ascore_pack_int3(con->packet_header, length);
  con->packet_number++;
  con->packet_header[3]= con->packet_number;

  send_buffer[0].base= con->packet_header;
  send_buffer[0].len= 4;
  send_buffer[1].base= data;
  send_buffer[1].len= length;
  asdebug_hex(data, length);
  con->command_status= ASCORE_COMMAND_STATUS_READ_RESPONSE;

  if (uv_write(&con->uv_objects.write_req, con->uv_objects.stream, send_buffer, 2, on_write) != 0)
  {
    con->local_errcode= ASRET_NET_WRITE_ERROR;
    asdebug("Write fail: %s", uv_err_name(uv_last_error(con->uv_objects.loop)));
    con->command_status= ASCORE_COMMAND_STATUS_SEND_FAILED;
    con->next_packet_type= ASCORE_PACKET_TYPE_NONE;
  }
}

void on_write(uv_write_t *req, int status)
{
  ascon_st *con= (ascon_st*)req->handle->loop->data;
  asdebug("Write callback, status: %d", status);

  if (status != 0)
  {
    con->local_errcode= ASRET_NET_WRITE_ERROR;
    asdebug("Write fail: %s", uv_err_name(uv_last_error(con->uv_objects.loop)));
    con->command_status= ASCORE_COMMAND_STATUS_SEND_FAILED;
    con->next_packet_type= ASCORE_PACKET_TYPE_NONE;
  }
}

void ascore_read_data_cb(uv_stream_t* tcp, ssize_t read_size, const uv_buf_t buf)
{
  (void) buf;
  struct ascon_st *con= (struct ascon_st*)tcp->loop->data;

  if (read_size < 0)
  {
    con->local_errcode= ASRET_NET_READ_ERROR;
    asdebug("Read fail: %s", uv_err_name(uv_last_error(con->uv_objects.loop)));
    con->command_status= ASCORE_COMMAND_STATUS_READ_FAILED;
    con->next_packet_type= ASCORE_PACKET_TYPE_NONE;
    return;
  }
  asdebug("Got data, %zd bytes", read_size);
  ascore_buffer_move_write_ptr(con->read_buffer, read_size);
  ascore_con_process_packets(con);
}

bool ascore_con_process_packets(ascon_st *con)
{
  uint32_t packet_len;
  size_t data_size;

  while (con->next_packet_type != ASCORE_PACKET_TYPE_NONE)
  {
    // Packet header is 4 bytes, if we don't have that, then we don't have enough
    data_size= ascore_buffer_unread_data(con->read_buffer);
    if (data_size < 4)
    {
      asdebug("Read less than 4 bytes, waiting for more");
      return false;
    }

    // First 3 bytes are packet size
    packet_len= ascore_unpack_int3(con->read_buffer->buffer_read_ptr);
    con->packet_size= packet_len;

    if ((packet_len + 4) > data_size)
    {
      asdebug("Don't have whole packet, expected %u bytes, got %zu", packet_len, data_size - 4);
      return false;
    }

    // If initial read handshake packet_number is 0, so don't increment
    if (con->next_packet_type != ASCORE_PACKET_TYPE_HANDSHAKE)
    {
      con->packet_number++;
    }
    // Fourth byte is packet number
    asdebug("Got packet %d, expected %d", con->read_buffer->buffer_read_ptr[3], con->packet_number);
    if (con->packet_number != con->read_buffer->buffer_read_ptr[3])
    {
      asdebug("Packet out of sequence!");
      con->local_errcode= ASRET_PACKET_OUT_OF_SEQUENCE;
      con->command_status= ASCORE_COMMAND_STATUS_READ_FAILED;
      con->next_packet_type= ASCORE_PACKET_TYPE_NONE;
      return true;
    }
    con->read_buffer->buffer_read_ptr+= 4;
    con->read_buffer->packet_end_ptr= con->read_buffer->buffer_read_ptr + packet_len;
    asdebug_hex(con->read_buffer->buffer_read_ptr, packet_len);
    switch(con->next_packet_type)
    {
      case ASCORE_PACKET_TYPE_NONE:
        // Shoudn't happen
        return true;
      case ASCORE_PACKET_TYPE_HANDSHAKE:
        ascore_packet_read_handshake(con);
        break;
      case ASCORE_PACKET_TYPE_RESPONSE:
        ascore_packet_read_response(con);
        break;
      case ASCORE_PACKET_TYPE_COLUMN:
        ascore_packet_read_column(con);
        break;
      case ASCORE_PACKET_TYPE_ROW:
        ascore_packet_read_row(con);
        return true;
        break;
    }
  }

  return true;
}

void ascore_packet_read_row(ascon_st *con)
{
  // If we hit an EOF instead
  if ((unsigned char)con->read_buffer->buffer_read_ptr[0] == 0xfe)
  {
    con->result.row_data= NULL;
    con->result.row_length= 0;
    ascore_packet_read_response(con);
    return;
  }
  asdebug("Row read");
  con->result.row_data= con->read_buffer->buffer_read_ptr;
  con->result.row_length= con->packet_size;
  con->next_packet_type= ASCORE_PACKET_TYPE_NONE;
  con->command_status= ASCORE_COMMAND_STATUS_ROW_IN_BUFFER;
  con->status= ASCORE_CON_STATUS_IDLE;
  uv_read_stop(con->uv_objects.stream);
}

void ascore_packet_read_end(ascon_st *con)
{
  asdebug("Packet end");

  uv_read_stop(con->uv_objects.stream);
  ascore_buffer_packet_read_end(con->read_buffer);
}

void ascore_packet_read_response(ascon_st *con)
{
  asdebug("Response packet");
  uint8_t bytes;
  uint32_t data_read= 0;
  buffer_st *buffer= con->read_buffer;

  if (buffer->buffer_read_ptr[0] == 0x00)
  {
    // This is an OK packet
    asdebug("Got OK packet");
    buffer->buffer_read_ptr++;
    data_read++;
    con->affected_rows= ascore_unpack_length(buffer->buffer_read_ptr, &bytes, NULL);
    buffer->buffer_read_ptr+= bytes;
    data_read+= bytes;
    con->insert_id= ascore_unpack_length(buffer->buffer_read_ptr, &bytes, NULL);
    buffer->buffer_read_ptr+= bytes;
    data_read+= bytes;
    con->server_status= ascore_unpack_int2(buffer->buffer_read_ptr);
    buffer->buffer_read_ptr+= 2;
    data_read+= 2;
    con->warning_count= ascore_unpack_int2(buffer->buffer_read_ptr);
    buffer->buffer_read_ptr+= 2;
    data_read+= 2;
    snprintf(con->server_message, ASCORE_MAX_MESSAGE_LEN, "%.*s", (con->packet_size - data_read), buffer->buffer_read_ptr);
    con->server_message[ASCORE_MAX_MESSAGE_LEN - 1]= '\0';
    buffer->buffer_read_ptr+= (con->packet_size - data_read);
    if (con->status == ASCORE_CON_STATUS_CONNECTING)
    {
      con->command_status= ASCORE_COMMAND_STATUS_CONNECTED;
    }
    else
    {
      con->command_status= ASCORE_COMMAND_STATUS_NONE;
    }
    con->status= ASCORE_CON_STATUS_IDLE;
    con->next_packet_type= ASCORE_PACKET_TYPE_NONE;
    ascore_packet_read_end(con);
  }
  else if ((unsigned char)buffer->buffer_read_ptr[0] == 0xff)
  {
    // This is an Error packet
    asdebug("Got Error packet");
    buffer->buffer_read_ptr++;
    data_read++;
    con->server_errno= ascore_unpack_int2(buffer->buffer_read_ptr);
    // Also skip the SQLSTATE marker, always a '#'
    buffer->buffer_read_ptr+= 3;
    data_read+= 3;
    memcpy(con->sqlstate, buffer->buffer_read_ptr, 5);
    buffer->buffer_read_ptr+= 5;
    data_read+= 5;
    snprintf(con->server_message, ASCORE_MAX_MESSAGE_LEN, "%.*s", (con->packet_size - data_read), buffer->buffer_read_ptr);
    con->server_message[ASCORE_MAX_MESSAGE_LEN - 1]= '\0';
    buffer->buffer_read_ptr+= (con->packet_size - data_read);
    if (con->command_status == ASCORE_COMMAND_STATUS_READ_RESPONSE)
    {
      con->status= ASCORE_CON_STATUS_CONNECT_FAILED;
    }
    else
    {
      con->status= ASCORE_CON_STATUS_IDLE;
    }
    con->next_packet_type= ASCORE_PACKET_TYPE_NONE;
    con->command_status= ASCORE_COMMAND_STATUS_NONE;
    ascore_packet_read_end(con);
  }
  else if ((unsigned char)buffer->buffer_read_ptr[0] == 0xfe)
  {
    // This is an EOF packet
    asdebug("Got EOF packet");
    buffer->buffer_read_ptr++;
    con->warning_count= ascore_unpack_int2(buffer->buffer_read_ptr);
    buffer->buffer_read_ptr+= 2;
    con->server_status= ascore_unpack_int2(buffer->buffer_read_ptr);
    buffer->buffer_read_ptr+= 2;
    if (con->command_status == ASCORE_COMMAND_STATUS_READ_COLUMN)
    {
      con->command_status= ASCORE_COMMAND_STATUS_READ_ROW;
      con->next_packet_type= ASCORE_PACKET_TYPE_ROW;
      ascore_buffer_packet_read_end(con->read_buffer);
    }
    else
    {
      con->next_packet_type= ASCORE_PACKET_TYPE_NONE;
      con->command_status= ASCORE_COMMAND_STATUS_EOF;
      con->status= ASCORE_CON_STATUS_IDLE;
      ascore_packet_read_end(con);
    }
  }
  else
  {
    // This is a result packet
    asdebug("Got result packet");
    con->result.column_count= ascore_unpack_length(buffer->buffer_read_ptr, &bytes, NULL);
    buffer->buffer_read_ptr+= bytes;
    con->result.columns= new (std::nothrow) column_t[con->result.column_count];
    ascore_buffer_packet_read_end(con->read_buffer);
    con->next_packet_type= ASCORE_PACKET_TYPE_COLUMN;
    con->command_status= ASCORE_COMMAND_STATUS_READ_COLUMN;
  }
}

void ascore_packet_read_column(ascon_st *con)
{
  asdebug("Column packet callback");
  uint8_t bytes;
  uint64_t str_len, str_read;
  buffer_st *buffer= con->read_buffer;
  column_t *column;

  column= &con->result.columns[con->result.current_column];
  // Skip catalog since no MySQL version actually uses this yet
  str_len= ascore_unpack_length(buffer->buffer_read_ptr, &bytes, NULL);
  buffer->buffer_read_ptr+= bytes;
  buffer->buffer_read_ptr+= str_len;

  // Schema
  str_len= ascore_unpack_length(buffer->buffer_read_ptr, &bytes, NULL);
  buffer->buffer_read_ptr+= bytes;
  if (str_len >= ASCORE_MAX_SCHEMA_SIZE)
  {
    str_read= ASCORE_MAX_SCHEMA_SIZE - 1;
  }
  else
  {
    str_read= str_len;
  }
  memcpy(column->schema, buffer->buffer_read_ptr, str_read);
  column->schema[str_read + 1]= '\0';
  buffer->buffer_read_ptr+= str_len;

  // Table
  str_len= ascore_unpack_length(buffer->buffer_read_ptr, &bytes, NULL);
  buffer->buffer_read_ptr+= bytes;
  if (str_len >= ASCORE_MAX_TABLE_SIZE)
  {
    str_read= ASCORE_MAX_TABLE_SIZE -1;
  }
  else
  {
    str_read= str_len;
  }
  memcpy(column->table, buffer->buffer_read_ptr, str_read);
  column->table[str_read + 1]= '\0';
  buffer->buffer_read_ptr+= str_len;

  // Origin table
  str_len= ascore_unpack_length(buffer->buffer_read_ptr, &bytes, NULL);
  buffer->buffer_read_ptr+= bytes;
  if (str_len >= ASCORE_MAX_TABLE_SIZE)
  {
    str_read= ASCORE_MAX_TABLE_SIZE -1;
  }
  else
  {
    str_read= str_len;
  }
  memcpy(column->origin_table, buffer->buffer_read_ptr, str_read);
  column->origin_table[str_read + 1]= '\0';
  buffer->buffer_read_ptr+= str_len;

  // Column
  str_len= ascore_unpack_length(buffer->buffer_read_ptr, &bytes, NULL);
  buffer->buffer_read_ptr+= bytes;
  if (str_len >= ASCORE_MAX_COLUMN_SIZE)
  {
    str_read= ASCORE_MAX_COLUMN_SIZE -1;
  }
  else
  {
    str_read= str_len;
  }
  memcpy(column->column, buffer->buffer_read_ptr, str_read);
  column->column[str_read + 1]= '\0';
  buffer->buffer_read_ptr+= str_len;

  // Origin column
  str_len= ascore_unpack_length(buffer->buffer_read_ptr, &bytes, NULL);
  buffer->buffer_read_ptr+= bytes;
  if (str_len >= ASCORE_MAX_COLUMN_SIZE)
  {
    str_read= ASCORE_MAX_COLUMN_SIZE -1;
  }
  else
  {
    str_read= str_len;
  }
  memcpy(column->origin_column, buffer->buffer_read_ptr, str_read);
  column->origin_column[str_read + 1]= '\0';
  buffer->buffer_read_ptr+= str_len;

  // Padding
  buffer->buffer_read_ptr++;

  // Charset
  column->charset= ascore_unpack_int2(buffer->buffer_read_ptr);
  buffer->buffer_read_ptr+= 2;

  // Length
  column->length= ascore_unpack_int4(buffer->buffer_read_ptr);
  buffer->buffer_read_ptr+= 4;

  // Type
  column->type= (ascore_column_type_t)buffer->buffer_read_ptr[0];
  buffer->buffer_read_ptr++;

  // Flags
  column->flags= (ascore_column_flags_t)ascore_unpack_int2(buffer->buffer_read_ptr);
  buffer->buffer_read_ptr+= 2;

  // Decimals
  column->decimals= buffer->buffer_read_ptr[0];
  buffer->buffer_read_ptr++;

  // Padding
  buffer->buffer_read_ptr+= 2;

  // Default value
  str_len= ascore_unpack_length(buffer->buffer_read_ptr, &bytes, NULL);
  buffer->buffer_read_ptr+= bytes;
  if (str_len >= ASCORE_MAX_DEFAULT_VALUE_SIZE)
  {
    str_read= ASCORE_MAX_DEFAULT_VALUE_SIZE -1;
  }
  else
  {
    str_read= str_len;
  }
  memcpy(column->default_value, buffer->buffer_read_ptr, str_read);
  column->default_size= str_read;
  buffer->buffer_read_ptr+= str_len;
  asdebug("Got column %s.%s.%s", column->schema, column->table, column->column);
  con->result.current_column++;
  if (con->result.current_column == con->result.column_count)
  {
    con->next_packet_type= ASCORE_PACKET_TYPE_RESPONSE;
  }
  ascore_buffer_packet_read_end(con->read_buffer);
}
