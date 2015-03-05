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
  con->stmt= attachsql_stmt_prepare(con, length, statement);
  if (con->stmt == NULL)
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_ALLOC, ATTACHSQL_ERROR_LEVEL_ERROR, "82100", "Allocation failure for statement object");
    return false;
  }
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
    if (con->local_errcode == ASRET_BAD_STMT_PARAMETER)
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

  if (attachsql_stmt_reset(con->stmt) == ATTACHSQL_COMMAND_STATUS_SEND_FAILED)
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_SERVER_GONE, ATTACHSQL_ERROR_LEVEL_ERROR, "08006", con->errmsg);
    return false;
  }

  return true;
}

bool attachsql_statement_send_long_data(attachsql_connect_t *con, uint16_t param, size_t length, char *data, attachsql_error_t **error)
{
  if (con == NULL)
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "No statement provided");
    return false;
  }
  if (con->stmt == NULL)
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "No statement prepared");
    return false;
  }

  if (attachsql_stmt_send_long_data(con->stmt, param, length, data) == ATTACHSQL_COMMAND_STATUS_SEND_FAILED)
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_SERVER_GONE, ATTACHSQL_ERROR_LEVEL_ERROR, "08006", con->errmsg);
    return false;
  }

  return true;
}

uint16_t attachsql_statement_get_param_count(attachsql_connect_t *con)
{
  if ((con == NULL) || (con->stmt == NULL))
  {
    return 0;
  }
  return con->stmt->param_count;
}

bool attachsql_statement_set_int(attachsql_connect_t *con, uint16_t param, int32_t value, attachsql_error_t **error)
{
  return attachsql_statement_set_param(con, ATTACHSQL_COLUMN_TYPE_LONG, param, 0, &value, false, error);
}

bool attachsql_statement_set_unsigned_int(attachsql_connect_t *con, uint16_t param, uint32_t value, attachsql_error_t **error)
{
  return attachsql_statement_set_param(con, ATTACHSQL_COLUMN_TYPE_LONG, param, 0, &value, true, error);
}

bool attachsql_statement_set_bigint(attachsql_connect_t *con, uint16_t param, int64_t value, attachsql_error_t **error)
{
  return attachsql_statement_set_param(con, ATTACHSQL_COLUMN_TYPE_LONGLONG, param, 0, &value, false, error);
}

bool attachsql_statement_set_unsigned_bigint(attachsql_connect_t *con, uint16_t param, uint64_t value, attachsql_error_t **error)
{
  return attachsql_statement_set_param(con, ATTACHSQL_COLUMN_TYPE_LONGLONG, param, 0, &value, true, error);
}

bool attachsql_statement_set_double(attachsql_connect_t *con, uint16_t param, double value, attachsql_error_t **error)
{
  return attachsql_statement_set_param(con, ATTACHSQL_COLUMN_TYPE_DOUBLE, param, 0, &value, false, error);
}

bool attachsql_statement_set_string(attachsql_connect_t *con, uint16_t param, size_t length, const char *value, attachsql_error_t **error)
{
  return attachsql_statement_set_param(con, ATTACHSQL_COLUMN_TYPE_STRING, param, length, value, false, error);
}

bool attachsql_statement_set_binary(attachsql_connect_t *con, uint16_t param, size_t length, const char *value, attachsql_error_t **error)
{
  return attachsql_statement_set_param(con, ATTACHSQL_COLUMN_TYPE_BLOB, param, length, value, false, error);
}

bool attachsql_statement_set_null(attachsql_connect_t *con, uint16_t param, attachsql_error_t **error)
{
  return attachsql_statement_set_param(con, ATTACHSQL_COLUMN_TYPE_NULL, param, 0, NULL, false, error);
}

bool attachsql_statement_set_datetime(attachsql_connect_t *con, uint16_t param, uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, uint32_t microsecond, attachsql_error_t **error)
{
  if ((con == NULL) || (con->stmt == NULL))
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Connection parameter not valid");
    return false;
  }

  if (param >= con->stmt->param_count)
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Param %d does not exist", param);
    return false;
  }
  con->stmt->param_data[param].data.datetime_data= new attachsql_datetime_st;
  con->stmt->param_data[param].datetime_alloc= true;
  con->stmt->param_data[param].data.datetime_data->year= year;
  con->stmt->param_data[param].data.datetime_data->month= month;
  con->stmt->param_data[param].data.datetime_data->day= day;
  con->stmt->param_data[param].data.datetime_data->hour= hour;
  con->stmt->param_data[param].data.datetime_data->minute= minute;
  con->stmt->param_data[param].data.datetime_data->second= second;
  con->stmt->param_data[param].data.datetime_data->microsecond= microsecond;
  con->stmt->param_data[param].type= ATTACHSQL_COLUMN_TYPE_DATETIME;

  return true;
}

bool attachsql_statement_set_time(attachsql_connect_t *con, uint16_t param, uint8_t hour, uint8_t minute, uint8_t second, uint32_t microsecond, bool is_negative, attachsql_error_t **error)
{
  if ((con == NULL) || (con->stmt == NULL))
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Connection parameter not valid");
    return false;
  }

  if (param >= con->stmt->param_count)
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Param %d does not exist", param);
    return false;
  }
  con->stmt->param_data[param].data.datetime_data= new attachsql_datetime_st;
  con->stmt->param_data[param].datetime_alloc= false;
  con->stmt->param_data[param].data.datetime_data->hour= hour;
  con->stmt->param_data[param].data.datetime_data->minute= minute;
  con->stmt->param_data[param].data.datetime_data->second= second;
  con->stmt->param_data[param].data.datetime_data->microsecond= microsecond;
  con->stmt->param_data[param].data.datetime_data->is_negative= is_negative;
  con->stmt->param_data[param].type= ATTACHSQL_COLUMN_TYPE_TIME;

  return true;
}

bool attachsql_statement_set_param(attachsql_connect_t *con, attachsql_column_type_t type, uint16_t param, size_t length, const void *value, bool is_unsigned, attachsql_error_t **error)
{
  if ((con == NULL) || (con->stmt == NULL))
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Connection parameter not valid");
    return false;
  }

  if (param >= con->stmt->param_count)
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Param %d does not exist", param);
    return false;
  }

  switch (type)
  {
    case ATTACHSQL_COLUMN_TYPE_LONG:
      con->stmt->param_data[param].data.int_data= *(uint32_t*)value;
      con->stmt->param_data[param].is_unsigned= is_unsigned;
      con->stmt->param_data[param].type= type;
      break;
    case ATTACHSQL_COLUMN_TYPE_LONGLONG:
      con->stmt->param_data[param].data.bigint_data= *(uint64_t*)value;
      con->stmt->param_data[param].is_unsigned= is_unsigned;
      con->stmt->param_data[param].type= type;
      break;
    case ATTACHSQL_COLUMN_TYPE_FLOAT:
      con->stmt->param_data[param].data.float_data= *(float*)value;
      con->stmt->param_data[param].is_unsigned= false;
      con->stmt->param_data[param].type= type;
      break;
    case ATTACHSQL_COLUMN_TYPE_DOUBLE:
      con->stmt->param_data[param].data.double_data= *(double*)value;
      con->stmt->param_data[param].is_unsigned= false;
      con->stmt->param_data[param].type= type;
      break;
    case ATTACHSQL_COLUMN_TYPE_STRING:
    case ATTACHSQL_COLUMN_TYPE_BLOB:
      con->stmt->param_data[param].data.string_data= (char*)value;
      con->stmt->param_data[param].length= length;
      con->stmt->param_data[param].type= type;
      break;
    case ATTACHSQL_COLUMN_TYPE_NULL:
      con->stmt->param_data[param].type= type;;
      break;
    /* the following won't happen, but this will keep the compiler happy */
    case ATTACHSQL_COLUMN_TYPE_DECIMAL:
    case ATTACHSQL_COLUMN_TYPE_TINY:
    case ATTACHSQL_COLUMN_TYPE_SHORT:
    case ATTACHSQL_COLUMN_TYPE_TIMESTAMP:
    case ATTACHSQL_COLUMN_TYPE_INT24:
    case ATTACHSQL_COLUMN_TYPE_DATE:
    case ATTACHSQL_COLUMN_TYPE_TIME:
    case ATTACHSQL_COLUMN_TYPE_DATETIME:
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
    case ATTACHSQL_COLUMN_TYPE_ERROR:
      break;
  }

  return true;
}

bool attachsql_statement_row_get(attachsql_connect_t *con, attachsql_error_t **error)
{
  char *raw_row;
  uint16_t column;
  uint16_t total_columns;
  uint8_t bytes= 0;
  uint64_t length;

  if (con == NULL)
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Connection parameter not valid");
    return false;
  }

  total_columns= con->result.column_count;
  if (con->stmt_row == NULL)
  {
    con->stmt_row= new (std::nothrow) attachsql_stmt_row_st[total_columns];
  }

  if (con->stmt_row == NULL)
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_ALLOC, ATTACHSQL_ERROR_LEVEL_ERROR, "82100", "Allocation failure for row");
    return false;
  }

  raw_row= con->result.row_data;
  /* packet header */
  raw_row++;
  con->stmt_null_bitmap_length= ((total_columns+7+2)/8);
  con->stmt_null_bitmap= raw_row;
  raw_row+= con->stmt_null_bitmap_length;

  for (column= 0; column < total_columns; column++)
  {
    attachsql_column_type_t type= con->result.columns[column].type;
    /* Really complex way of saying "if this is in the NULL bitmap */
    if (con->stmt_null_bitmap[(column+2)/8] & (1 << ((column+2) % 8)))
    {
      type= ATTACHSQL_COLUMN_TYPE_NULL;
    }
    switch(type)
    {
      case ATTACHSQL_COLUMN_TYPE_STRING:
      case ATTACHSQL_COLUMN_TYPE_VARCHAR:
      case ATTACHSQL_COLUMN_TYPE_VARSTRING:
      case ATTACHSQL_COLUMN_TYPE_ENUM:
      case ATTACHSQL_COLUMN_TYPE_SET:
      case ATTACHSQL_COLUMN_TYPE_LONG_BLOB:
      case ATTACHSQL_COLUMN_TYPE_MEDIUM_BLOB:
      case ATTACHSQL_COLUMN_TYPE_TINY_BLOB:
      case ATTACHSQL_COLUMN_TYPE_BLOB:
      case ATTACHSQL_COLUMN_TYPE_GEOMETRY:
      case ATTACHSQL_COLUMN_TYPE_BIT:
      case ATTACHSQL_COLUMN_TYPE_DECIMAL:
      case ATTACHSQL_COLUMN_TYPE_NEWDECIMAL:
        length= attachsql_unpack_length(raw_row, &bytes, NULL);
        raw_row+= bytes;
        break;
      case ATTACHSQL_COLUMN_TYPE_DATE:
      case ATTACHSQL_COLUMN_TYPE_DATETIME:
      case ATTACHSQL_COLUMN_TYPE_TIMESTAMP:
      case ATTACHSQL_COLUMN_TYPE_TIME:
        length= raw_row[0];
        raw_row++;
        break;
      case ATTACHSQL_COLUMN_TYPE_LONGLONG:
      case ATTACHSQL_COLUMN_TYPE_DOUBLE:
        length= 8;
        break;
      case ATTACHSQL_COLUMN_TYPE_LONG:
      case ATTACHSQL_COLUMN_TYPE_INT24:
      case ATTACHSQL_COLUMN_TYPE_FLOAT:
        length= 4;
        break;
      case ATTACHSQL_COLUMN_TYPE_SHORT:
      case ATTACHSQL_COLUMN_TYPE_YEAR:
        length= 2;
        break;
      case ATTACHSQL_COLUMN_TYPE_TINY:
        length= 1;
        break;
      case ATTACHSQL_COLUMN_TYPE_NULL:
        /* in NULL bitmask only */
        length= 0;
        break;
      case ATTACHSQL_COLUMN_TYPE_ERROR:
      default:
        attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_UNKNOWN, ATTACHSQL_ERROR_LEVEL_ERROR, "60000", "Bad data in statement result");
        return false;
    }
    con->stmt_row[column].data= raw_row;
    con->stmt_row[column].length= (size_t)length;
    con->stmt_row[column].type= type;
    raw_row+= length;
  }
  return true;
}

int32_t attachsql_statement_get_int(attachsql_connect_t *con, uint16_t column, attachsql_error_t **error)
{
  if (con == NULL)
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Connection parameter not valid");
    return 0;
  }

  if (con->stmt_row == NULL)
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Statement row has not been processed");
    return 0;
  }

  if (column >= con->result.column_count)
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Column %d does not exist", column);
    return 0;
  }

  attachsql_stmt_row_st *column_data= &con->stmt_row[column];
  switch (column_data->type)
  {
    case ATTACHSQL_COLUMN_TYPE_TINY:
      return (int8_t)column_data->data[0];
      break;
    case ATTACHSQL_COLUMN_TYPE_YEAR:
    case ATTACHSQL_COLUMN_TYPE_SHORT:
      return (int16_t)attachsql_unpack_int2(column_data->data);
      break;
    case ATTACHSQL_COLUMN_TYPE_LONG:
      return (int32_t)attachsql_unpack_int4(column_data->data);
      break;
    case ATTACHSQL_COLUMN_TYPE_LONGLONG:
      return (int32_t)attachsql_unpack_int8(column_data->data);
      break;
    case ATTACHSQL_COLUMN_TYPE_FLOAT:
      float f;
      memcpy(&f, column_data->data, 4);
      return (int32_t)f;
      break;
    case ATTACHSQL_COLUMN_TYPE_DOUBLE:
      double d;
      memcpy(&d, column_data->data, 8);
      return (int32_t)d;
      break;
    case ATTACHSQL_COLUMN_TYPE_NULL:
      return 0;
      break;
    case ATTACHSQL_COLUMN_TYPE_INT24:
      return (int32_t)attachsql_unpack_int3(column_data->data);
      break;

    case ATTACHSQL_COLUMN_TYPE_DECIMAL:
    case ATTACHSQL_COLUMN_TYPE_TIMESTAMP:
    case ATTACHSQL_COLUMN_TYPE_DATE:
    case ATTACHSQL_COLUMN_TYPE_TIME:
    case ATTACHSQL_COLUMN_TYPE_DATETIME:
    case ATTACHSQL_COLUMN_TYPE_VARCHAR:
    case ATTACHSQL_COLUMN_TYPE_BIT:
    case ATTACHSQL_COLUMN_TYPE_NEWDECIMAL:
    case ATTACHSQL_COLUMN_TYPE_ENUM:
    case ATTACHSQL_COLUMN_TYPE_SET:
    case ATTACHSQL_COLUMN_TYPE_TINY_BLOB:
    case ATTACHSQL_COLUMN_TYPE_MEDIUM_BLOB:
    case ATTACHSQL_COLUMN_TYPE_LONG_BLOB:
    case ATTACHSQL_COLUMN_TYPE_BLOB:
    case ATTACHSQL_COLUMN_TYPE_VARSTRING:
    case ATTACHSQL_COLUMN_TYPE_STRING:
    case ATTACHSQL_COLUMN_TYPE_GEOMETRY:
    case ATTACHSQL_COLUMN_TYPE_ERROR:
      attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Cannot convert to int");
      return 0;
      break;
  }
  /* Should never hit here, but lets make compilers happy */
  return 0;
}


uint32_t attachsql_statement_get_int_unsigned(attachsql_connect_t *con, uint16_t column, attachsql_error_t **error)
{
  if (con == NULL)
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Connection parameter not valid");
    return 0;
  }

  if (con->stmt_row == NULL)
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Statement row has not been processed");
    return 0;
  }

  if (column >= con->result.column_count)
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Column %d does not exist", column);
    return 0;
  }

  attachsql_stmt_row_st *column_data= &con->stmt_row[column];
  switch (column_data->type)
  {
    case ATTACHSQL_COLUMN_TYPE_TINY:
      return (uint8_t)column_data->data[0];
      break;
    case ATTACHSQL_COLUMN_TYPE_YEAR:
    case ATTACHSQL_COLUMN_TYPE_SHORT:
      return (uint16_t)attachsql_unpack_int2(column_data->data);
      break;
    case ATTACHSQL_COLUMN_TYPE_LONG:
      return (uint32_t)attachsql_unpack_int4(column_data->data);
      break;
    case ATTACHSQL_COLUMN_TYPE_LONGLONG:
      return (uint32_t)attachsql_unpack_int8(column_data->data);
      break;
    case ATTACHSQL_COLUMN_TYPE_FLOAT:
      float f;
      memcpy(&f, column_data->data, 4);
      return (uint32_t)f;
      break;
    case ATTACHSQL_COLUMN_TYPE_DOUBLE:
      double d;
      memcpy(&d, column_data->data, 8);
      return (uint32_t)d;
      break;
    case ATTACHSQL_COLUMN_TYPE_NULL:
      return 0;
      break;
    case ATTACHSQL_COLUMN_TYPE_INT24:
      return (uint32_t)attachsql_unpack_int3(column_data->data);
      break;

    case ATTACHSQL_COLUMN_TYPE_DECIMAL:
    case ATTACHSQL_COLUMN_TYPE_TIMESTAMP:
    case ATTACHSQL_COLUMN_TYPE_DATE:
    case ATTACHSQL_COLUMN_TYPE_TIME:
    case ATTACHSQL_COLUMN_TYPE_DATETIME:
    case ATTACHSQL_COLUMN_TYPE_VARCHAR:
    case ATTACHSQL_COLUMN_TYPE_BIT:
    case ATTACHSQL_COLUMN_TYPE_NEWDECIMAL:
    case ATTACHSQL_COLUMN_TYPE_ENUM:
    case ATTACHSQL_COLUMN_TYPE_SET:
    case ATTACHSQL_COLUMN_TYPE_TINY_BLOB:
    case ATTACHSQL_COLUMN_TYPE_MEDIUM_BLOB:
    case ATTACHSQL_COLUMN_TYPE_LONG_BLOB:
    case ATTACHSQL_COLUMN_TYPE_BLOB:
    case ATTACHSQL_COLUMN_TYPE_VARSTRING:
    case ATTACHSQL_COLUMN_TYPE_STRING:
    case ATTACHSQL_COLUMN_TYPE_GEOMETRY:
    case ATTACHSQL_COLUMN_TYPE_ERROR:
      attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Cannot convert to int");
      return 0;
      break;
  }
  /* Should never hit here, but lets make compilers happy */
  return 0;
}


int64_t attachsql_statement_get_bigint(attachsql_connect_t *con, uint16_t column, attachsql_error_t **error)
{
  if (con == NULL)
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Connection parameter not valid");
    return 0;
  }

  if (con->stmt_row == NULL)
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Statement row has not been processed");
    return 0;
  }

  if (column >= con->result.column_count)
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Column %d does not exist", column);
    return 0;
  }

  attachsql_stmt_row_st *column_data= &con->stmt_row[column];
  switch (column_data->type)
  {
    case ATTACHSQL_COLUMN_TYPE_TINY:
      return (int8_t)column_data->data[0];
      break;
    case ATTACHSQL_COLUMN_TYPE_YEAR:
    case ATTACHSQL_COLUMN_TYPE_SHORT:
      return (int16_t)attachsql_unpack_int2(column_data->data);
      break;
    case ATTACHSQL_COLUMN_TYPE_LONG:
      return (int32_t)attachsql_unpack_int4(column_data->data);
      break;
    case ATTACHSQL_COLUMN_TYPE_LONGLONG:
      return (int64_t)attachsql_unpack_int8(column_data->data);
      break;
    case ATTACHSQL_COLUMN_TYPE_FLOAT:
      float f;
      memcpy(&f, column_data->data, 4);
      return (int64_t)f;
      break;
    case ATTACHSQL_COLUMN_TYPE_DOUBLE:
      double d;
      memcpy(&d, column_data->data, 8);
      return (int64_t)d;
      break;
    case ATTACHSQL_COLUMN_TYPE_NULL:
      return 0;
      break;
    case ATTACHSQL_COLUMN_TYPE_INT24:
      return (int32_t)attachsql_unpack_int3(column_data->data);
      break;

    case ATTACHSQL_COLUMN_TYPE_DECIMAL:
    case ATTACHSQL_COLUMN_TYPE_TIMESTAMP:
    case ATTACHSQL_COLUMN_TYPE_DATE:
    case ATTACHSQL_COLUMN_TYPE_TIME:
    case ATTACHSQL_COLUMN_TYPE_DATETIME:
    case ATTACHSQL_COLUMN_TYPE_VARCHAR:
    case ATTACHSQL_COLUMN_TYPE_BIT:
    case ATTACHSQL_COLUMN_TYPE_NEWDECIMAL:
    case ATTACHSQL_COLUMN_TYPE_ENUM:
    case ATTACHSQL_COLUMN_TYPE_SET:
    case ATTACHSQL_COLUMN_TYPE_TINY_BLOB:
    case ATTACHSQL_COLUMN_TYPE_MEDIUM_BLOB:
    case ATTACHSQL_COLUMN_TYPE_LONG_BLOB:
    case ATTACHSQL_COLUMN_TYPE_BLOB:
    case ATTACHSQL_COLUMN_TYPE_VARSTRING:
    case ATTACHSQL_COLUMN_TYPE_STRING:
    case ATTACHSQL_COLUMN_TYPE_GEOMETRY:
    case ATTACHSQL_COLUMN_TYPE_ERROR:
      attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Cannot convert to int");
      return 0;
      break;
  }
  /* Should never hit here, but lets make compilers happy */
  return 0;
}

uint64_t attachsql_statement_get_bigint_unsigned(attachsql_connect_t *con, uint16_t column, attachsql_error_t **error)
{
  if (con == NULL)
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Connection parameter not valid");
    return 0;
  }

  if (con->stmt_row == NULL)
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Statement row has not been processed");
    return 0;
  }

  if (column >= con->result.column_count)
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Column %d does not exist", column);
    return 0;
  }

  attachsql_stmt_row_st *column_data= &con->stmt_row[column];
  switch (column_data->type)
  {
    case ATTACHSQL_COLUMN_TYPE_TINY:
      return (uint8_t)column_data->data[0];
      break;
    case ATTACHSQL_COLUMN_TYPE_YEAR:
    case ATTACHSQL_COLUMN_TYPE_SHORT:
      return (uint16_t)attachsql_unpack_int2(column_data->data);
      break;
    case ATTACHSQL_COLUMN_TYPE_LONG:
      return (uint32_t)attachsql_unpack_int4(column_data->data);
      break;
    case ATTACHSQL_COLUMN_TYPE_LONGLONG:
      return (uint64_t)attachsql_unpack_int8(column_data->data);
      break;
    case ATTACHSQL_COLUMN_TYPE_FLOAT:
      float f;
      memcpy(&f, column_data->data, 4);
      return (uint32_t)f;
      break;
    case ATTACHSQL_COLUMN_TYPE_DOUBLE:
      double d;
      memcpy(&d, column_data->data, 8);
      return (uint32_t)d;
      break;
    case ATTACHSQL_COLUMN_TYPE_NULL:
      return 0;
      break;
    case ATTACHSQL_COLUMN_TYPE_INT24:
      return (uint32_t)attachsql_unpack_int3(column_data->data);
      break;

    case ATTACHSQL_COLUMN_TYPE_DECIMAL:
    case ATTACHSQL_COLUMN_TYPE_TIMESTAMP:
    case ATTACHSQL_COLUMN_TYPE_DATE:
    case ATTACHSQL_COLUMN_TYPE_TIME:
    case ATTACHSQL_COLUMN_TYPE_DATETIME:
    case ATTACHSQL_COLUMN_TYPE_VARCHAR:
    case ATTACHSQL_COLUMN_TYPE_BIT:
    case ATTACHSQL_COLUMN_TYPE_NEWDECIMAL:
    case ATTACHSQL_COLUMN_TYPE_ENUM:
    case ATTACHSQL_COLUMN_TYPE_SET:
    case ATTACHSQL_COLUMN_TYPE_TINY_BLOB:
    case ATTACHSQL_COLUMN_TYPE_MEDIUM_BLOB:
    case ATTACHSQL_COLUMN_TYPE_LONG_BLOB:
    case ATTACHSQL_COLUMN_TYPE_BLOB:
    case ATTACHSQL_COLUMN_TYPE_VARSTRING:
    case ATTACHSQL_COLUMN_TYPE_STRING:
    case ATTACHSQL_COLUMN_TYPE_GEOMETRY:
    case ATTACHSQL_COLUMN_TYPE_ERROR:
      attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Cannot convert to int");
      return 0;
      break;
  }
  /* Should never hit here, but lets make compilers happy */
  return 0;
}

double attachsql_statement_get_double(attachsql_connect_t *con, uint16_t column, attachsql_error_t **error)
{
  if (con == NULL)
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Connection parameter not valid");
    return 0;
  }

  if (con->stmt_row == NULL)
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Statement row has not been processed");
    return 0;
  }

  if (column >= con->result.column_count)
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Column %d does not exist", column);
    return 0;
  }

  attachsql_stmt_row_st *column_data= &con->stmt_row[column];
  switch (column_data->type)
  {
    case ATTACHSQL_COLUMN_TYPE_TINY:
      return (double)column_data->data[0];
      break;
    case ATTACHSQL_COLUMN_TYPE_YEAR:
    case ATTACHSQL_COLUMN_TYPE_SHORT:
      return (double)attachsql_unpack_int2(column_data->data);
      break;
    case ATTACHSQL_COLUMN_TYPE_LONG:
      return (double)attachsql_unpack_int4(column_data->data);
      break;
    case ATTACHSQL_COLUMN_TYPE_LONGLONG:
      return (double)attachsql_unpack_int8(column_data->data);
      break;
    case ATTACHSQL_COLUMN_TYPE_FLOAT:
      float f;
      memcpy(&f, column_data->data, 4);
      return (double)f;
      break;
    case ATTACHSQL_COLUMN_TYPE_DOUBLE:
      double d;
      memcpy(&d, column_data->data, 8);
      return d;
      break;
    case ATTACHSQL_COLUMN_TYPE_NULL:
      return 0;
      break;
    case ATTACHSQL_COLUMN_TYPE_INT24:
      return (double)attachsql_unpack_int3(column_data->data);
      break;

    case ATTACHSQL_COLUMN_TYPE_DECIMAL:
    case ATTACHSQL_COLUMN_TYPE_TIMESTAMP:
    case ATTACHSQL_COLUMN_TYPE_DATE:
    case ATTACHSQL_COLUMN_TYPE_TIME:
    case ATTACHSQL_COLUMN_TYPE_DATETIME:
    case ATTACHSQL_COLUMN_TYPE_VARCHAR:
    case ATTACHSQL_COLUMN_TYPE_BIT:
    case ATTACHSQL_COLUMN_TYPE_NEWDECIMAL:
    case ATTACHSQL_COLUMN_TYPE_ENUM:
    case ATTACHSQL_COLUMN_TYPE_SET:
    case ATTACHSQL_COLUMN_TYPE_TINY_BLOB:
    case ATTACHSQL_COLUMN_TYPE_MEDIUM_BLOB:
    case ATTACHSQL_COLUMN_TYPE_LONG_BLOB:
    case ATTACHSQL_COLUMN_TYPE_BLOB:
    case ATTACHSQL_COLUMN_TYPE_VARSTRING:
    case ATTACHSQL_COLUMN_TYPE_STRING:
    case ATTACHSQL_COLUMN_TYPE_GEOMETRY:
    case ATTACHSQL_COLUMN_TYPE_ERROR:
      attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Cannot convert to int");
      return 0;
      break;
  }
  /* Should never hit here, but lets make compilers happy */
  return 0;
}

char *attachsql_statement_get_char(attachsql_connect_t *con, uint16_t column, size_t *length, attachsql_error_t **error)
{
  if (con == NULL)
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Connection parameter not valid");
    return NULL;
  }

  if (con->stmt_row == NULL)
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Statement row has not been processed");
    return NULL;
  }

  if (column >= con->result.column_count)
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Column %d does not exist", column);
    return NULL;
  }

  if (length == NULL)
  {
    attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Length parameter not valid");
    return NULL;
  }

  attachsql_stmt_row_st *column_data= &con->stmt_row[column];
  attachsql_datetime_st datetime;
  switch (column_data->type)
  {
    case ATTACHSQL_COLUMN_TYPE_TINY:
      if (con->result.columns[column].flags & ATTACHSQL_COLUMN_FLAGS_UNSIGNED)
      {
        *length= snprintf(con->stmt_tmp_buffer, ATTACHSQL_STMT_CHAR_BUFFER_SIZE, "%" PRIu8, column_data->data[0]);
      }
      else
      {
        *length= snprintf(con->stmt_tmp_buffer, ATTACHSQL_STMT_CHAR_BUFFER_SIZE, "%" PRId8, column_data->data[0]);
      }
      return con->stmt_tmp_buffer;
      break;
    case ATTACHSQL_COLUMN_TYPE_YEAR:
    case ATTACHSQL_COLUMN_TYPE_SHORT:
      if (con->result.columns[column].flags & ATTACHSQL_COLUMN_FLAGS_UNSIGNED)
      {
        *length= snprintf(con->stmt_tmp_buffer, ATTACHSQL_STMT_CHAR_BUFFER_SIZE, "%" PRIu16, attachsql_unpack_int2(column_data->data));
      }
      else
      {
        *length= snprintf(con->stmt_tmp_buffer, ATTACHSQL_STMT_CHAR_BUFFER_SIZE, "%" PRId16, attachsql_unpack_int2(column_data->data));
      }
      return con->stmt_tmp_buffer;
      break;
    case ATTACHSQL_COLUMN_TYPE_LONG:
      if (con->result.columns[column].flags & ATTACHSQL_COLUMN_FLAGS_UNSIGNED)
      {
        *length= snprintf(con->stmt_tmp_buffer, ATTACHSQL_STMT_CHAR_BUFFER_SIZE, "%" PRIu32, attachsql_unpack_int4(column_data->data));
      }
      else
      {
        *length= snprintf(con->stmt_tmp_buffer, ATTACHSQL_STMT_CHAR_BUFFER_SIZE, "%" PRId32, attachsql_unpack_int4(column_data->data));
      }
      return con->stmt_tmp_buffer;
      break;
    case ATTACHSQL_COLUMN_TYPE_LONGLONG:
      if (con->result.columns[column].flags & ATTACHSQL_COLUMN_FLAGS_UNSIGNED)
      {
        *length= snprintf(con->stmt_tmp_buffer, ATTACHSQL_STMT_CHAR_BUFFER_SIZE, "%" PRIu64, attachsql_unpack_int8(column_data->data));
      }
      else
      {
        *length= snprintf(con->stmt_tmp_buffer, ATTACHSQL_STMT_CHAR_BUFFER_SIZE, "%" PRId64, attachsql_unpack_int8(column_data->data));
      }
      return con->stmt_tmp_buffer;
      break;
    case ATTACHSQL_COLUMN_TYPE_FLOAT:
      float f;
      memcpy(&f, column_data->data, 4);
      *length= snprintf(con->stmt_tmp_buffer, ATTACHSQL_STMT_CHAR_BUFFER_SIZE, "%f", f);
      return con->stmt_tmp_buffer;
      break;
    case ATTACHSQL_COLUMN_TYPE_DOUBLE:
      double d;
      memcpy(&d, column_data->data, 8);
      *length= snprintf(con->stmt_tmp_buffer, ATTACHSQL_STMT_CHAR_BUFFER_SIZE, "%f", d);
      return con->stmt_tmp_buffer;
      break;
    case ATTACHSQL_COLUMN_TYPE_NULL:
      return NULL;
      break;
    case ATTACHSQL_COLUMN_TYPE_INT24:
      if (con->result.columns[column].flags & ATTACHSQL_COLUMN_FLAGS_UNSIGNED)
      {
        *length= snprintf(con->stmt_tmp_buffer, ATTACHSQL_STMT_CHAR_BUFFER_SIZE, "%" PRIu32, attachsql_unpack_int3(column_data->data));
      }
      else
      {
        *length= snprintf(con->stmt_tmp_buffer, ATTACHSQL_STMT_CHAR_BUFFER_SIZE, "%" PRId32, attachsql_unpack_int3(column_data->data));
      }
      return con->stmt_tmp_buffer;
      break;

    case ATTACHSQL_COLUMN_TYPE_DECIMAL:
    case ATTACHSQL_COLUMN_TYPE_VARCHAR:
    case ATTACHSQL_COLUMN_TYPE_BIT:
    case ATTACHSQL_COLUMN_TYPE_NEWDECIMAL:
    case ATTACHSQL_COLUMN_TYPE_ENUM:
    case ATTACHSQL_COLUMN_TYPE_SET:
    case ATTACHSQL_COLUMN_TYPE_TINY_BLOB:
    case ATTACHSQL_COLUMN_TYPE_MEDIUM_BLOB:
    case ATTACHSQL_COLUMN_TYPE_LONG_BLOB:
    case ATTACHSQL_COLUMN_TYPE_BLOB:
    case ATTACHSQL_COLUMN_TYPE_VARSTRING:
    case ATTACHSQL_COLUMN_TYPE_STRING:
    case ATTACHSQL_COLUMN_TYPE_GEOMETRY:
      *length= column_data->length;
      return column_data->data;
      break;
    case ATTACHSQL_COLUMN_TYPE_TIME:
      attachsql_unpack_time(column_data->data, column_data->length, &datetime);
      *length= snprintf(con->stmt_tmp_buffer, ATTACHSQL_STMT_CHAR_BUFFER_SIZE, "%s%02u:%02" PRIu8 ":%02" PRIu8, (datetime.is_negative) ? "-" : "", datetime.hour + 24 * datetime.day, datetime.minute, datetime.second);
      if (datetime.microsecond)
      {
        *length+= snprintf(con->stmt_tmp_buffer+(*length), ATTACHSQL_STMT_CHAR_BUFFER_SIZE-(*length), ".%06" PRIu32, datetime.microsecond);
      }
      return con->stmt_tmp_buffer;
      break;
    case ATTACHSQL_COLUMN_TYPE_TIMESTAMP:
    case ATTACHSQL_COLUMN_TYPE_DATE:
    case ATTACHSQL_COLUMN_TYPE_DATETIME:
      attachsql_unpack_datetime(column_data->data, column_data->length, &datetime);
      *length= snprintf(con->stmt_tmp_buffer, ATTACHSQL_STMT_CHAR_BUFFER_SIZE, "%04" PRIu16 "-%02" PRIu8 "-%02" PRIu32, datetime.year, datetime.month, datetime.day);
      if (column_data->type == ATTACHSQL_COLUMN_TYPE_DATE)
      {
        return con->stmt_tmp_buffer;
      }
      *length+= snprintf(con->stmt_tmp_buffer+(*length), ATTACHSQL_STMT_CHAR_BUFFER_SIZE-(*length), " %02" PRIu8 ":%02" PRIu8 ":%02" PRIu8, datetime.hour, datetime.minute, datetime.second);

      if (datetime.microsecond)
      {
        *length+= snprintf(con->stmt_tmp_buffer+(*length), ATTACHSQL_STMT_CHAR_BUFFER_SIZE-(*length), ".%06" PRIu32, datetime.microsecond);
      }
      return con->stmt_tmp_buffer;
      break;
    case ATTACHSQL_COLUMN_TYPE_ERROR:
      attachsql_error_client_create(error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Cannot convert to int");
      return NULL;
      break;
  }
  /* Should never hit here, but lets make compilers happy */
  return NULL;
}

void attachsql_statement_close(attachsql_connect_t *con)
{
  if (con == NULL)
  {
    return;
  }

  if (con->stmt_row != NULL)
  {
    delete[] con->stmt_row;
  }
  con->stmt_row= NULL;
  attachsql_stmt_destroy(con->stmt);
  con->stmt= NULL;
}

attachsql_column_type_t attachsql_statement_get_column_type(attachsql_connect_t *con, uint16_t column)
{
  if (con == NULL)
  {
    return ATTACHSQL_COLUMN_TYPE_ERROR;
  }

  if (column >= con->result.column_count)
  {
    return ATTACHSQL_COLUMN_TYPE_ERROR;
  }

  return (attachsql_column_type_t)con->result.columns[column].type;
}

void attachsql_statement_row_next(attachsql_connect_t *con)
{
  if (con == NULL)
  {
    return;
  }
  attachsql_get_next_row(con);
}

uint16_t attachsql_statement_get_column_count(attachsql_connect_t *con)
{
  if (con == NULL)
  {
    return 0;
  }

  return con->result.column_count;
}

