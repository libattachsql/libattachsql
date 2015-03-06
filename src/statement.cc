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

bool attachsql_statement_prepare(attachsql_connect_t *con, size_t length, const char *statement, attachsql_error_t **error)
{
  if (con->status == ATTACHSQL_CON_STATUS_NOT_CONNECTED)
  {
    con->query_buffer= (char*)statement;
    con->query_buffer_length= length;
    con->query_buffer_alloc= false;
    con->query_buffer_statement= true;
    return attachsql_connect(con, error);
  }
  con->stmt= new (std::nothrow) attachsql_stmt_st;

  if (con->stmt == NULL)
  {
    con->local_errcode= ATTACHSQL_RET_OUT_OF_MEMORY_ERROR;
    con->command_status= ATTACHSQL_COMMAND_STATUS_SEND_FAILED;
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_ALLOC, ATTACHSQL_ERROR_LEVEL_ERROR, "82100", "Allocation failure for statement object");
    return false;
  }

  con->stmt->con= con;
  asdebug("Sending MySQL prepare");
  attachsql_command_send(con, ATTACHSQL_COMMAND_STMT_PREPARE, (char*)statement, length);

  return true;
}

bool attachsql_statement_execute(attachsql_connect_t *con, attachsql_error_t **error)
{
  if (con == NULL)
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "No connection provided");
    return false;
  }
  if (con->stmt == NULL)
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "No statement prepared");
    return false;
  }
  /* Free anything left over from last exec */
  attachsql_command_free(con);
  if (not attachsql_stmt_execute(con->stmt))
  {
    if (con->local_errcode == ATTACHSQL_RET_BAD_STMT_PARAMETER)
    {
      attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Bad parameter bound to statement");
      return false;
    }
    else
    {
      attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_ALLOC, ATTACHSQL_ERROR_LEVEL_ERROR, "82100", "Allocation failure for statement object");
      return false;
    }
  }
  return true;
}

bool attachsql_stmt_execute(attachsql_stmt_st *stmt)
{
  char *buffer_pos= NULL;
  uint16_t param_count= 0;

  /* Need minimum of 2K plus a bit extra for packet header */
  if (not attachsql_stmt_check_buffer_size(stmt, 2060))
  {
    return false;
  }
  buffer_pos= stmt->exec_buffer;

  /* statement ID */
  attachsql_pack_int4(buffer_pos, stmt->id);
  buffer_pos+= 4;
  /* cursor flags */
  buffer_pos[0]= ATTACHSQL_STMT_CURSOR_NONE;
  buffer_pos++;
  /* iteration count (always 1) */
  attachsql_pack_int4(buffer_pos, 1);
  buffer_pos+= 4;
  /* NULL bitmask */
  if (stmt->param_count > 0)
  {
    uint16_t null_bytes= (stmt->param_count + 7) / 8;
    memset(buffer_pos, 0, null_bytes);
    param_count= stmt->param_count;
    for (uint16_t param= 0; param < stmt->param_count; param++)
    {
      if (stmt->param_data[param].type == ATTACHSQL_COLUMN_TYPE_NULL)
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
    attachsql_stmt_param_st *param_data= &stmt->param_data[param];

    uint16_t type= (uint16_t)param_data->type;
    if (type == ATTACHSQL_COLUMN_TYPE_NULL)
    {
      continue;
    }
    if (param_data->is_unsigned)
    {
      type|= ATTACHSQL_STMT_PARAM_UNSIGNED_BIT;
    }
    attachsql_pack_int2(&param_type_pos[param * 2], type);

    /* Long data skipped */
    if (param_data->is_long_data)
    {
      continue;
    }

    /* Check buffer for at least attachsql_datetime_st bytes
     * restore pointers too
     */
    param_bytes= param_type_pos - stmt->exec_buffer;
    buffer_bytes= buffer_pos - stmt->exec_buffer;
    if (not attachsql_stmt_check_buffer_size(stmt, sizeof(attachsql_datetime_st)))
    {
      return false;
    }
    param_type_pos= stmt->exec_buffer + param_bytes;
    buffer_pos= stmt->exec_buffer + buffer_bytes;

    switch (param_data->type)
    {
      case ATTACHSQL_COLUMN_TYPE_TINY:
        buffer_pos[0]= param_data->data.tinyint_data;
        buffer_pos++;
        break;
      case ATTACHSQL_COLUMN_TYPE_SHORT:
        attachsql_pack_int2(buffer_pos, param_data->data.smallint_data);
        buffer_pos+= 2;
        break;
      case ATTACHSQL_COLUMN_TYPE_LONG:
        attachsql_pack_int4(buffer_pos, param_data->data.int_data);
        buffer_pos+= 4;
        break;
      case ATTACHSQL_COLUMN_TYPE_LONGLONG:
        attachsql_pack_int8(buffer_pos, param_data->data.bigint_data);
        buffer_pos+= 8;
        break;
      case ATTACHSQL_COLUMN_TYPE_FLOAT:
        memcpy(buffer_pos, &param_data->data.float_data, 4);
        buffer_pos+= 4;
        break;
      case ATTACHSQL_COLUMN_TYPE_DOUBLE:
        memcpy(buffer_pos, &param_data->data.double_data, 8);
        buffer_pos+= 8;
        break;
      case ATTACHSQL_COLUMN_TYPE_TIME:
        buffer_pos= attachsql_pack_time(buffer_pos, param_data->data.datetime_data);
        break;
      case ATTACHSQL_COLUMN_TYPE_DATE:
        buffer_pos= attachsql_pack_datetime(buffer_pos, param_data->data.datetime_data, true);
        break;
      case ATTACHSQL_COLUMN_TYPE_DATETIME:
      case ATTACHSQL_COLUMN_TYPE_TIMESTAMP:
        buffer_pos= attachsql_pack_datetime(buffer_pos, param_data->data.datetime_data, false);
        break;
      case ATTACHSQL_COLUMN_TYPE_STRING:
      case ATTACHSQL_COLUMN_TYPE_BLOB:
        /* check buffer for size */
        param_bytes= param_type_pos - stmt->exec_buffer;
        buffer_bytes= buffer_pos - stmt->exec_buffer;
        if (not attachsql_stmt_check_buffer_size(stmt, param_data->length))
        {
          return false;
        }
        param_type_pos= stmt->exec_buffer + param_bytes;
        buffer_pos= stmt->exec_buffer + buffer_bytes;
        buffer_pos= attachsql_pack_data(buffer_pos, param_data->length, param_data->data.string_data);
        break;
      case ATTACHSQL_COLUMN_TYPE_NULL:
        /* Already handled in the NULL bitmask we should not get here */
      /* The following should never happen, but are listed to make compilers happy bunnies */
      case ATTACHSQL_COLUMN_TYPE_INT24:
      case ATTACHSQL_COLUMN_TYPE_YEAR:
      case ATTACHSQL_COLUMN_TYPE_VARCHAR:
      case ATTACHSQL_COLUMN_TYPE_BIT:
      case ATTACHSQL_COLUMN_TYPE_NEWDECIMAL:
      case ATTACHSQL_COLUMN_TYPE_ENUM:
      case ATTACHSQL_COLUMN_TYPE_SET:
      case ATTACHSQL_COLUMN_TYPE_TINY_BLOB:
      case ATTACHSQL_COLUMN_TYPE_MEDIUM_BLOB:
      case ATTACHSQL_COLUMN_TYPE_LONG_BLOB:
      case ATTACHSQL_COLUMN_TYPE_VARSTRING:
      case ATTACHSQL_COLUMN_TYPE_GEOMETRY:
      case ATTACHSQL_COLUMN_TYPE_DECIMAL:
      case ATTACHSQL_COLUMN_TYPE_ERROR:
      default:
        stmt->con->local_errcode= ATTACHSQL_RET_BAD_STMT_PARAMETER;
        asdebug("Bad stmt parameter type provided: %d", param_data->type);
        stmt->con->command_status= ATTACHSQL_COMMAND_STATUS_SEND_FAILED;
        stmt->con->next_packet_queue_used= 0;
        return false;
        break;
    }
  }
  if (attachsql_command_send(stmt->con, ATTACHSQL_COMMAND_STMT_EXECUTE, stmt->exec_buffer, buffer_pos - stmt->exec_buffer) != ATTACHSQL_COMMAND_STATUS_SEND)
  {
    return false;
  }
  return true;
}

bool attachsql_stmt_check_buffer_size(attachsql_stmt_st *stmt, size_t required)
{
  char *realloc_buffer= NULL;

  if (stmt->exec_buffer_length < required)
  {
    size_t new_size= 0;
    if (stmt->exec_buffer_length == 0)
    {
      new_size= ATTACHSQL_STMT_EXEC_DEFAULT_SIZE;
    }
    else
    {
      new_size= stmt->exec_buffer_length * 2;
    }
    realloc_buffer= (char*)realloc(stmt->exec_buffer, new_size);
    if (realloc_buffer == NULL)
    {
      stmt->con->local_errcode= ATTACHSQL_RET_OUT_OF_MEMORY_ERROR;
      asdebug("Exec buffer realloc failure");
      stmt->con->command_status= ATTACHSQL_COMMAND_STATUS_SEND_FAILED;
      stmt->con->next_packet_queue_used= 0;
      return false;
    }
    stmt->exec_buffer= realloc_buffer;
    stmt->exec_buffer_length= new_size;
  }
  return true;
}

attachsql_command_status_t attachsql_stmt_fetch(attachsql_stmt_st *stmt)
{
  attachsql_buffer_packet_read_end(stmt->con->read_buffer);
  attachsql_packet_queue_push(stmt->con, ATTACHSQL_PACKET_TYPE_STMT_ROW);
  stmt->con->command_status= ATTACHSQL_COMMAND_STATUS_READ_STMT_ROW;
  attachsql_con_process_packets(stmt->con);
  return stmt->con->command_status;
}

void attachsql_statement_close(attachsql_connect_t *con)
{
  attachsql_stmt_st *stmt= con->stmt;
  if (con == NULL)
  {
    return;
  }

  if (con->stmt_row != NULL)
  {
    delete[] con->stmt_row;
  }
  con->stmt_row= NULL;

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

  con->stmt= NULL;
  con->write_buffer_extra= 4;
  attachsql_command_free(con);
  attachsql_pack_int4(&con->write_buffer[1], stmt->id);
  attachsql_command_send(con, ATTACHSQL_COMMAND_STMT_CLOSE, NULL, 0);
  delete stmt;
}

bool attachsql_statement_reset(attachsql_connect_t *con, attachsql_error_t **error)
{
  if (con == NULL)
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "No connection provided");
    return false;
  }
  if (con->stmt == NULL)
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "No statement prepared");
    return false;
  }

  con->write_buffer_extra= 4;
  attachsql_pack_int4(&con->write_buffer[1], con->stmt->id);

  if (attachsql_command_send(con, ATTACHSQL_COMMAND_STMT_RESET, NULL, 0) == ATTACHSQL_COMMAND_STATUS_SEND_FAILED)
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_SERVER_GONE, ATTACHSQL_ERROR_LEVEL_ERROR, "08006", con->errmsg);
    return false;
  }

  return true;
}

