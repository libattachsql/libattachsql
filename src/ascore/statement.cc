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
#include "statement.h"
#include "command.h"
#include "net.h"

ascore_stmt_st *ascore_stmt_prepare(ascon_st *con, size_t length, const char *statement)
{
  con->stmt= new (std::nothrow) ascore_stmt_st;

  if (con->stmt == NULL)
  {
    con->local_errcode= ASRET_OUT_OF_MEMORY_ERROR;
    con->command_status= ASCORE_COMMAND_STATUS_SEND_FAILED;
    return NULL;
  }

  con->stmt->con= con;
  asdebug("Sending MySQL prepare");
  ascore_command_send(con, ASCORE_COMMAND_STMT_PREPARE, (char*)statement, length);
  return con->stmt;
}

bool ascore_stmt_execute(ascore_stmt_st *stmt)
{
  char *buffer_pos= NULL;
  uint16_t param_count= 0;

  /* Need minimum of 2K plus a bit extra for packet header */
  if (not ascore_stmt_check_buffer_size(stmt, 2060))
  {
    return false;
  }
  buffer_pos= stmt->exec_buffer;

  /* statement ID */
  ascore_pack_int4(buffer_pos, stmt->id);
  buffer_pos+= 4;
  /* cursor flags */
  buffer_pos[0]= ASCORE_STMT_CURSOR_NONE;
  buffer_pos++;
  /* iteration count (always 1) */
  ascore_pack_int4(buffer_pos, 1);
  buffer_pos+= 4;
  /* NULL bitmask */
  if (stmt->param_count > 0)
  {
    uint16_t null_bytes= (stmt->param_count + 7) / 8;
    memset(buffer_pos, 0, null_bytes);
    param_count= stmt->param_count;
    for (uint16_t param= 0; param < stmt->param_count; param++)
    {
      if (stmt->param_data[param].type == ASCORE_COLUMN_TYPE_NULL)
      {
        buffer_pos[param/8] |= (1 << (param % 8));
        param_count--;
      }
    }
    buffer_pos+= null_bytes;
  }
  /* New bind? */
  buffer_pos[0]= stmt->new_bind;
  buffer_pos++;
  /* Bind params
   * First part is type of each parameter, 2 bytes per type
   * Second part is the parameters themselves
   * param_count is the number of params without NULL since they don't store type data
   */
  char *param_type_pos= buffer_pos;
  size_t param_bytes= 0;
  size_t buffer_bytes= 0;
  buffer_pos+= (param_count * 2);
  for (uint16_t param= 0; param < stmt->param_count; param++)
  {
    ascore_stmt_param_st *param_data= &stmt->param_data[param];

    uint16_t type= (uint16_t)param_data->type;
    if (type == ASCORE_COLUMN_TYPE_NULL)
    {
      continue;
    }
    if (param_data->is_unsigned)
    {
      type|= ASCORE_STMT_PARAM_UNSIGNED_BIT;
    }
    ascore_pack_int2(&param_type_pos[param * 2], type);

    /* Long data skipped */
    if (param_data->is_long_data)
    {
      continue;
    }

    /* Check buffer for at least ascore_datetime_st bytes
     * restore pointers too
     */
    param_bytes= param_type_pos - stmt->exec_buffer;
    buffer_bytes= buffer_pos - stmt->exec_buffer;
    if (not ascore_stmt_check_buffer_size(stmt, sizeof(ascore_datetime_st)))
    {
      return false;
    }
    param_type_pos= stmt->exec_buffer + param_bytes;
    buffer_pos= stmt->exec_buffer + buffer_bytes;

    switch (param_data->type)
    {
      case ASCORE_COLUMN_TYPE_TINY:
        buffer_pos[0]= param_data->data.tinyint_data;
        buffer_pos++;
        break;
      case ASCORE_COLUMN_TYPE_SHORT:
        ascore_pack_int2(buffer_pos, param_data->data.smallint_data);
        buffer_pos+= 2;
        break;
      case ASCORE_COLUMN_TYPE_LONG:
        ascore_pack_int4(buffer_pos, param_data->data.int_data);
        buffer_pos+= 4;
        break;
      case ASCORE_COLUMN_TYPE_LONGLONG:
        ascore_pack_int8(buffer_pos, param_data->data.bigint_data);
        buffer_pos+= 8;
        break;
      case ASCORE_COLUMN_TYPE_FLOAT:
        memcpy(buffer_pos, &param_data->data.float_data, 4);
        buffer_pos+= 4;
        break;
      case ASCORE_COLUMN_TYPE_DOUBLE:
        memcpy(buffer_pos, &param_data->data.double_data, 8);
        buffer_pos+= 8;
        break;
      case ASCORE_COLUMN_TYPE_TIME:
        buffer_pos= ascore_pack_time(buffer_pos, param_data->data.datetime_data);
        break;
      case ASCORE_COLUMN_TYPE_DATE:
        buffer_pos= ascore_pack_datetime(buffer_pos, param_data->data.datetime_data, true);
        break;
      case ASCORE_COLUMN_TYPE_DATETIME:
      case ASCORE_COLUMN_TYPE_TIMESTAMP:
        buffer_pos= ascore_pack_datetime(buffer_pos, param_data->data.datetime_data, false);
        break;
      case ASCORE_COLUMN_TYPE_STRING:
      case ASCORE_COLUMN_TYPE_BLOB:
        /* check buffer for size */
        param_bytes= param_type_pos - stmt->exec_buffer;
        buffer_bytes= buffer_pos - stmt->exec_buffer;
        if (not ascore_stmt_check_buffer_size(stmt, param_data->length))
        {
          return false;
        }
        param_type_pos= stmt->exec_buffer + param_bytes;
        buffer_pos= stmt->exec_buffer + buffer_bytes;
        buffer_pos= ascore_pack_data(buffer_pos, param_data->length, param_data->data.string_data);
        break;
      case ASCORE_COLUMN_TYPE_NULL:
        /* Already handled in the NULL bitmask we should not get here */
      /* The following should never happen, but are listed to make compilers happy bunnies */
      case ASCORE_COLUMN_TYPE_INT24:
      case ASCORE_COLUMN_TYPE_YEAR:
      case ASCORE_COLUMN_TYPE_NEWDATE:
      case ASCORE_COLUMN_TYPE_VARCHAR:
      case ASCORE_COLUMN_TYPE_BIT:
      case ASCORE_COLUMN_TYPE_TIMESTAMP2:
      case ASCORE_COLUMN_TYPE_DATETIME2:
      case ASCORE_COLUMN_TYPE_TIME2:
      case ASCORE_COLUMN_TYPE_NEWDECIMAL:
      case ASCORE_COLUMN_TYPE_ENUM:
      case ASCORE_COLUMN_TYPE_SET:
      case ASCORE_COLUMN_TYPE_TINY_BLOB:
      case ASCORE_COLUMN_TYPE_MEDIUM_BLOB:
      case ASCORE_COLUMN_TYPE_LONG_BLOB:
      case ASCORE_COLUMN_TYPE_VARSTRING:
      case ASCORE_COLUMN_TYPE_GEOMETRY:
      case ASCORE_COLUMN_TYPE_DECIMAL:
      default:
        stmt->con->local_errcode= ASRET_BAD_STMT_PARAMETER;
        asdebug("Bad stmt parameter type provided: %d", param_data->type);
        stmt->con->command_status= ASCORE_COMMAND_STATUS_SEND_FAILED;
        stmt->con->next_packet_queue_used= 0;
        return false;
        break;
    }
  }
  if (ascore_command_send(stmt->con, ASCORE_COMMAND_STMT_EXECUTE, stmt->exec_buffer, buffer_pos - stmt->exec_buffer) != ASCORE_COMMAND_STATUS_SEND)
  {
    return false;
  }
  return true;
}

bool ascore_stmt_check_buffer_size(ascore_stmt_st *stmt, size_t required)
{
  char *realloc_buffer= NULL;

  if (stmt->exec_buffer_length < required)
  {
    size_t new_size= 0;
    if (stmt->exec_buffer_length == 0)
    {
      new_size= ASCORE_STMT_EXEC_DEFAULT_SIZE;
    }
    else
    {
      new_size= stmt->exec_buffer_length * 2;
    }
    realloc_buffer= (char*)realloc(stmt->exec_buffer, new_size);
    if (realloc_buffer == NULL)
    {
      stmt->con->local_errcode= ASRET_OUT_OF_MEMORY_ERROR;
      asdebug("Exec buffer realloc failure");
      stmt->con->command_status= ASCORE_COMMAND_STATUS_SEND_FAILED;
      stmt->con->next_packet_queue_used= 0;
      return false;
    }
    stmt->exec_buffer= realloc_buffer;
    stmt->exec_buffer_length= new_size;
  }
  return true;
}

ascore_command_status_t ascore_stmt_fetch(ascore_stmt_st *stmt)
{
  ascore_buffer_packet_read_end(stmt->con->read_buffer);
  ascore_packet_queue_push(stmt->con, ASCORE_PACKET_TYPE_STMT_ROW);
  stmt->con->command_status= ASCORE_COMMAND_STATUS_READ_STMT_ROW;
  ascore_con_process_packets(stmt->con);
  return stmt->con->command_status;
}

void ascore_stmt_destroy(ascore_stmt_st *stmt)
{
  if (stmt == NULL)
  {
    return;
  }

  if (stmt->param_count > 0)
  {
    delete[] stmt->params;
  }

  if (stmt->exec_buffer_length > 0)
  {
    free(stmt->exec_buffer);
  }

  if (stmt->param_data != NULL)
  {
    for (uint16_t param= 0; param < stmt->param_count; param++)
    {
      if (stmt->param_data[param].datetime_alloc)
      {
        delete stmt->param_data[param].data.datetime_data;
      }
    }
    delete[] stmt->param_data;
  }

  stmt->con->stmt= NULL;
  stmt->con->write_buffer_extra= 4;
  ascore_command_free(stmt->con);
  ascore_pack_int4(&stmt->con->write_buffer[1], stmt->id);
  ascore_command_send(stmt->con, ASCORE_COMMAND_STMT_CLOSE, NULL, 0);
  delete stmt;
}

ascore_command_status_t ascore_stmt_reset(ascore_stmt_st *stmt)
{
  stmt->con->write_buffer_extra= 4;
  ascore_pack_int4(&stmt->con->write_buffer[1], stmt->id);

  return ascore_command_send(stmt->con, ASCORE_COMMAND_STMT_RESET, NULL, 0);
}

ascore_command_status_t ascore_stmt_send_long_data(ascore_stmt_st *stmt, uint16_t param, size_t length, char *data)
{
  stmt->con->write_buffer_extra= 6;
  ascore_pack_int4(&stmt->con->write_buffer[1], stmt->id);
  ascore_pack_int2(&stmt->con->write_buffer[5], param);

  return ascore_command_send(stmt->con, ASCORE_COMMAND_STMT_SEND_LONG_DATA, data, length);
}


