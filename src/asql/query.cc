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
#include "src/asql/query_internal.h"
#include "src/ascore/ascore.h"

attachsql_error_st *attachsql_query(attachsql_connect_t *con, size_t length, const char *statement, uint16_t parameter_count, attachsql_query_parameter_st *parameters)
{
  attachsql_error_st *err= NULL;
  size_t pos;
  size_t buffer_pos= 0;
  size_t out_len;
  uint16_t param;
  ascore_command_status_t ret;

  if (con == NULL)
  {
    attachsql_error_client_create(&err, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Connection parameter not valid");
    return err;
  }

  if (con->in_query)
  {
    attachsql_error_client_create(&err, ATTACHSQL_ERROR_CODE_OUT_OF_SYNC, ATTACHSQL_ERROR_LEVEL_ERROR, "08002", "Connection already used for query");
    return err;
  }
  con->in_query= true;

  /* No parameters so we can send now */
  if (parameter_count == 0)
  {
    if (con->core_con->status == ASCORE_CON_STATUS_NOT_CONNECTED)
    {
      con->query_buffer= (char*)statement;
      con->query_buffer_length= length;
      con->query_buffer_alloc= false;
      return attachsql_connect(con);
    }
    ret= ascore_command_send(con->core_con, ASCORE_COMMAND_QUERY, (char*)statement, length);
    if (ret == ASCORE_COMMAND_STATUS_SEND_FAILED)
    {
      attachsql_error_client_create(&err, ATTACHSQL_ERROR_CODE_SERVER_GONE, ATTACHSQL_ERROR_LEVEL_ERROR, "08006", con->core_con->errmsg);
      return err;
    }
    return NULL;
  }

  /* Work out buffer size */
  out_len= length + 1;

  for (param= 0; param < parameter_count; param++)
  {
    switch (parameters[param].type)
    {
      case ATTACHSQL_ESCAPE_TYPE_NONE:
        out_len+= parameters[param].length;
        break;
      case ATTACHSQL_ESCAPE_TYPE_CHAR:
        out_len+= (parameters[param].length * 2) + 2;
        break;
      case ATTACHSQL_ESCAPE_TYPE_CHAR_LIKE:
        out_len+= (parameters[param].length * 2);
        break;
      case ATTACHSQL_ESCAPE_TYPE_INT:
        out_len+= 10;
        break;
      case ATTACHSQL_ESCAPE_TYPE_BIGINT:
        out_len+= 20;
        break;
      case ATTACHSQL_ESCAPE_TYPE_FLOAT:
        out_len+= FLOAT_MAX_LEN;
        break;
      case ATTACHSQL_ESCAPE_TYPE_DOUBLE:
        out_len+= DOUBLE_MAX_LEN;
        break;
    }
  }
  con->query_buffer= new (std::nothrow) char[out_len];

  /* Copy into buffer and replace */
  param= 0;
  for (pos= 0; pos < length; pos++)
  {
    if (statement[pos] != '?')
    {
      con->query_buffer[buffer_pos] = statement[pos];
      buffer_pos++;
    }
    else
    {
      switch (parameters[param].type)
      {
        case ATTACHSQL_ESCAPE_TYPE_NONE:
          memcpy(&con->query_buffer[buffer_pos], parameters[param].data, parameters[param].length);
          buffer_pos+= parameters[param].length;
          break;
        case ATTACHSQL_ESCAPE_TYPE_CHAR:
          con->query_buffer[buffer_pos] = '\'';
          buffer_pos++;
          buffer_pos+= attachsql_query_escape_data(&con->query_buffer[buffer_pos], (char*)parameters[param].data, parameters[param].length);
          con->query_buffer[buffer_pos] = '\'';
          buffer_pos++;
          break;
        case ATTACHSQL_ESCAPE_TYPE_CHAR_LIKE:
          buffer_pos+= attachsql_query_escape_data(&con->query_buffer[buffer_pos], (char*)parameters[param].data, parameters[param].length);
          break;
        case ATTACHSQL_ESCAPE_TYPE_INT:
          if (parameters[param].is_unsigned)
          {
            buffer_pos+= sprintf(&con->query_buffer[buffer_pos], "%u", *(unsigned int*)parameters[param].data);
          }
          else
          {
            buffer_pos+= sprintf(&con->query_buffer[buffer_pos], "%d", *(int*)parameters[param].data);
          }
          break;
        case ATTACHSQL_ESCAPE_TYPE_BIGINT:
          if (parameters[param].is_unsigned)
          {
            buffer_pos+= sprintf(&con->query_buffer[buffer_pos], "%" PRId64, *(int64_t*)parameters[param].data);
          }
          else
          {
            buffer_pos+= sprintf(&con->query_buffer[buffer_pos], "%" PRIu64, *(uint64_t*)parameters[param].data);
          }
          break;
        case ATTACHSQL_ESCAPE_TYPE_FLOAT:
          // Significant digit length from http://msdn.microsoft.com/en-us/library/hd7199ke.aspx
          buffer_pos+= snprintf(&con->query_buffer[buffer_pos], FLOAT_MAX_LEN, "%.7f", *(float*)parameters[param].data);
          break;
        case ATTACHSQL_ESCAPE_TYPE_DOUBLE:
          buffer_pos+= snprintf(&con->query_buffer[buffer_pos], DOUBLE_MAX_LEN, "%.15f", *(double*)parameters[param].data);
          break;
      }
      param++;
    }
  }

  if (con->core_con->status == ASCORE_CON_STATUS_NOT_CONNECTED)
  {
    con->query_buffer_length= buffer_pos;
    con->query_buffer_alloc= true;
    return attachsql_connect(con);
  }
  ret= ascore_command_send(con->core_con, ASCORE_COMMAND_QUERY, con->query_buffer, buffer_pos);
  if (ret == ASCORE_COMMAND_STATUS_SEND_FAILED)
  {
    attachsql_error_client_create(&err, ATTACHSQL_ERROR_CODE_SERVER_GONE, ATTACHSQL_ERROR_LEVEL_ERROR, "08006", con->core_con->errmsg);
    return err;
  }
  return NULL;
}

size_t attachsql_query_escape_data(char *buffer, char *data, size_t length)
{
  size_t buffer_pos= 0;
  size_t pos;
  char newchar;

  for (pos= 0; pos < length; pos++)
  {
    newchar= '\0';
    if (not (data[pos] & 0x80))
    {
      switch(data[pos])
      {
        case 0:
          newchar= '0';
          break;
        case '\n':
          newchar= 'n';
          break;
        case '\r':
          newchar= 'r';
          break;
        case '\032':
          newchar= 'Z';
          break;
        case '\\':
          newchar= '\\';
          break;
        case '\'':
          newchar= '\'';
          break;
        case '\"':
          newchar= '\"';
          break;
        default:
          break;
      }
    }
    if (newchar != '\0')
    {
      buffer[buffer_pos]= '\\';
      buffer_pos++;
      buffer[buffer_pos]= newchar;
      buffer_pos++;
    }
    else
    {
      buffer[buffer_pos]= data[pos];
      buffer_pos++;
    }
  }
  return buffer_pos;
}

void attachsql_query_close(attachsql_connect_t *con)
{
  if (con == NULL)
  {
    return;
  }

  if (con->query_buffer_alloc and (con->query_buffer_length != 0))
  {
    delete[] con->query_buffer;
    con->query_buffer= NULL;
  }
  con->query_buffer_alloc= false;
  con->query_buffer_length= 0;

  if (con->columns != NULL)
  {
    delete[] con->columns;
    con->columns= NULL;
  }
  if ((con->row != NULL) and not con->buffer_rows)
  {
    delete[] con->row;
  }
  con->row= NULL;
  /* We are still in query if there are more results */
  if (not (con->core_con->server_status bitand ASCORE_SERVER_STATUS_MORE_RESULTS))
  {
    con->in_query= false;
  }

  if (con->row_buffer_alloc_size > 0)
  {
    for (uint64_t row_pos= 0; row_pos < con->row_buffer_count; row_pos++)
    {
      delete[] con->row_buffer[row_pos];
    }
    free(con->row_buffer);
  }
  con->row_buffer_alloc_size= 0;
  con->row_buffer_position= 0;
  con->row_buffer_count= 0;
  con->all_rows_buffered= false;
}

uint16_t attachsql_query_column_count(attachsql_connect_t *con)
{
  if (con == NULL)
  {
    return 0;
  }

  return con->core_con->result.column_count;
}

attachsql_query_column_st *attachsql_query_column_get(attachsql_connect_t *con, uint16_t column)
{
  uint16_t column_count;
  uint16_t current_col;
  column_t *core_column;
  if (con == NULL)
  {
    return 0;
  }
  column_count= con->core_con->result.column_count;

  if ((column > column_count) or (column < 1))
  {
    return NULL;
  }

  if (con->columns == NULL)
  {
    con->columns= new attachsql_query_column_st[column_count];
    for (current_col= 0; current_col < column_count; current_col++)
    {
      core_column= &con->core_con->result.columns[current_col];
      con->columns[current_col].schema= core_column->schema;
      con->columns[current_col].table= core_column->table;
      con->columns[current_col].origin_table= core_column->origin_table;
      con->columns[current_col].charset= core_column->charset;
      con->columns[current_col].length= core_column->length;
      con->columns[current_col].type= (attachsql_column_type_t) core_column->type;
      con->columns[current_col].flags= (attachsql_column_flags_t) core_column->flags;
      con->columns[current_col].decimals= core_column->decimals;
      con->columns[current_col].default_value= core_column->default_value;
      con->columns[current_col].default_size= core_column->default_size;
    }
  }
  return &con->columns[column - 1];
}

attachsql_query_row_st *attachsql_query_row_get(attachsql_connect_t *con, attachsql_error_st **error)
{
  uint16_t column;
  uint16_t total_columns;
  uint64_t length;
  uint8_t bytes;
  char *raw_row;

  if (con == NULL)
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Connection parameter not valid");
    return NULL;
  }

  if (con->buffer_rows)
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_BUFFERED_MODE, ATTACHSQL_ERROR_LEVEL_ERROR, "42000", "Cannot use function whilst buffering mode is enabled");
    return NULL;
  }

  if (con->core_con->command_status != ASCORE_COMMAND_STATUS_ROW_IN_BUFFER)
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_NO_DATA, ATTACHSQL_ERROR_LEVEL_ERROR, "02000", "No more data to retreive");
    return NULL;
  }

  total_columns= con->core_con->result.column_count;
  if (con->row == NULL)
  {
    con->row= new (std::nothrow) attachsql_query_row_st[total_columns];
  }

  if (con->row == NULL)
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_ALLOC, ATTACHSQL_ERROR_LEVEL_ERROR, "82100", "Allocation failure for row");
    return NULL;
  }

  raw_row= con->core_con->result.row_data;
  for (column= 0; column < total_columns; column++)
  {
    length= ascore_unpack_length(raw_row, &bytes, NULL);
    raw_row+= bytes;
    con->row[column].length= length;
    con->row[column].data= raw_row;
    raw_row+= length;
  }
  return con->row;
}

void attachsql_query_row_next(attachsql_connect_t *con)
{
  if (con == NULL)
  {
    return;
  }

  if (con->buffer_rows)
  {
    return;
  }
  ascore_get_next_row(con->core_con);
}

uint64_t attachsql_connection_last_insert_id(attachsql_connect_t *con)
{
  if (con == NULL)
  {
    return 0;
  }

  return con->core_con->insert_id;
}

uint64_t attachsql_query_affected_rows(attachsql_connect_t *con)
{
  if (con == NULL)
  {
    return 0;
  }

  return con->core_con->affected_rows;
}

const char *attachsql_query_info(attachsql_connect_t *con)
{
  if (con == NULL)
  {
    return NULL;
  }

  return con->core_con->server_message;
}

uint32_t attachsql_query_warning_count(attachsql_connect_t *con)
{
  if (con == NULL)
  {
    return 0;
  }

  return con->core_con->warning_count;
}

attachsql_return_t attachsql_query_next_result(attachsql_connect_t *con)
{
  if (con == NULL)
  {
    return ATTACHSQL_RETURN_ERROR;
  }

  if (ascore_command_next_result(con->core_con))
  {
    return ATTACHSQL_RETURN_PROCESSING;
  }
  return ATTACHSQL_RETURN_EOF;
}

bool attachsql_query_buffer_rows(attachsql_connect_t *con, bool enable)
{
  if (con == NULL)
  {
    return false;
  }

  /* Can't switch whilst already executing a query */
  if (con->in_query)
  {
    return false;
  }

  con->buffer_rows= enable;
  return true;
}

uint64_t attachsql_query_row_count(attachsql_connect_t *con)
{
  if (con == NULL)
  {
    return 0;
  }
  if (not con->buffer_rows)
  {
    return 0;
  }
  if (not con->all_rows_buffered)
  {
    return 0;
  }

  return con->row_buffer_count;
}

attachsql_return_t attachsql_query_row_buffer(attachsql_connect_t *con, attachsql_error_st **error)
{
  uint16_t column;
  uint16_t total_columns;
  uint64_t length;
  uint8_t bytes;
  char *raw_row;
  attachsql_query_row_st *row= NULL;

  do
  {
    if (con->row_buffer_alloc_size <= con->row_buffer_count)
    {
      con->row_buffer_alloc_size+= ATTACHSQL_BUFFER_ROW_ALLOC_SIZE;
      attachsql_query_row_st **realloc_buffer= (attachsql_query_row_st**)realloc(con->row_buffer, con->row_buffer_alloc_size * sizeof(attachsql_query_row_st*));
      if (realloc_buffer == NULL)
      {
        attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_ALLOC, ATTACHSQL_ERROR_LEVEL_ERROR, "82100", "Allocation failure for row buffer");
        return ATTACHSQL_RETURN_ERROR;
      }
      con->row_buffer= realloc_buffer;
    }

    total_columns= con->core_con->result.column_count;
    row= new (std::nothrow) attachsql_query_row_st[total_columns];

    if (row == NULL)
    {
      attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_ALLOC, ATTACHSQL_ERROR_LEVEL_ERROR, "82100", "Allocation failure for row");
      return ATTACHSQL_RETURN_ERROR;
    }

    raw_row= con->core_con->result.row_data;
    for (column= 0; column < total_columns; column++)
    {
      length= ascore_unpack_length(raw_row, &bytes, NULL);
      raw_row+= bytes;
      row[column].length= length;
      row[column].data= raw_row;
      raw_row+= length;
    }

    con->row_buffer[con->row_buffer_count]= row;
    con->row_buffer_count++;
    ascore_get_next_row(con->core_con);
  } while (ascore_con_process_packets(con->core_con) and (con->core_con->status != ASCORE_CON_STATUS_IDLE));
  return ATTACHSQL_RETURN_PROCESSING;
}

attachsql_query_row_st *attachsql_query_buffer_row_get(attachsql_connect_t *con)
{
  if (con == NULL)
  {
    return NULL;
  }

  if (con->row_buffer_position >= con->row_buffer_count)
  {
    /* All rows retrieved */
    return NULL;
  }
  con->row= con->row_buffer[con->row_buffer_position];
  con->row_buffer_position++;
  return con->row;
}

attachsql_query_row_st *attachsql_query_row_get_offset(attachsql_connect_t *con, uint64_t row_number)
{
  if (con == NULL)
  {
    return NULL;
  }

  if (row_number >= con->row_buffer_count)
  {
    /* No such row */
    return NULL;
  }
  return con->row_buffer[con->row_buffer_position];
}
