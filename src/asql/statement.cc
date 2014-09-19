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

uint16_t attachsql_statement_get_param_count(attachsql_connect_t *con)
{
  if ((con == NULL) || (con->stmt == NULL))
  {
    return 0;
  }
  return con->stmt->param_count;
}

attachsql_error_st *attachsql_statement_set_int(attachsql_connect_t *con, uint16_t param, int32_t value)
{
  return attachsql_statement_set_param(con, ASCORE_COLUMN_TYPE_LONG, param, 0, &value, false);
}

attachsql_error_st *attachsql_statement_set_unsigned_int(attachsql_connect_t *con, uint16_t param, uint32_t value)
{
  return attachsql_statement_set_param(con, ASCORE_COLUMN_TYPE_LONG, param, 0, &value, true);
}

attachsql_error_st *attachsql_statement_set_bigint(attachsql_connect_t *con, uint16_t param, int64_t value)
{
  return attachsql_statement_set_param(con, ASCORE_COLUMN_TYPE_LONGLONG, param, 0, &value, false);
}

attachsql_error_st *attachsql_statement_set_unsigned_bigint(attachsql_connect_t *con, uint16_t param, uint64_t value)
{
  return attachsql_statement_set_param(con, ASCORE_COLUMN_TYPE_LONGLONG, param, 0, &value, true);
}

attachsql_error_st *attachsql_statement_set_float(attachsql_connect_t *con, uint16_t param, float value)
{
  return attachsql_statement_set_param(con, ASCORE_COLUMN_TYPE_FLOAT, param, 0, &value, false);
}

attachsql_error_st *attachsql_statement_set_double(attachsql_connect_t *con, uint16_t param, double value)
{
  return attachsql_statement_set_param(con, ASCORE_COLUMN_TYPE_DOUBLE, param, 0, &value, false);
}

attachsql_error_st *attachsql_statement_set_string(attachsql_connect_t *con, uint16_t param, size_t length, const char *value)
{
  return attachsql_statement_set_param(con, ASCORE_COLUMN_TYPE_STRING, param, length, value, false);
}

attachsql_error_st *attachsql_statement_set_binary(attachsql_connect_t *con, uint16_t param, size_t length, const char *value)
{
  return attachsql_statement_set_param(con, ASCORE_COLUMN_TYPE_BLOB, param, length, value, false);
}

attachsql_error_st *attachsql_statement_set_null(attachsql_connect_t *con, uint16_t param)
{
  return attachsql_statement_set_param(con, ASCORE_COLUMN_TYPE_NULL, param, 0, NULL, false);
}

attachsql_error_st *attachsql_statement_set_datetime(attachsql_connect_t *con, uint16_t param, uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, uint32_t microsecond)
{
  attachsql_error_st *error;

  if ((con == NULL) || (con->stmt == NULL))
  {
    attachsql_error_client_create(&error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Connection parameter not valid");
    return error;
  }

  if (param >= con->stmt->param_count)
  {
    attachsql_error_client_create(&error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Param %d does not exist", param);
    return error;
  }
  con->stmt->param_data[param].data.datetime_data= new ascore_datetime_st;
  con->stmt->param_data[param].data.datetime_data->year= year;
  con->stmt->param_data[param].data.datetime_data->month= month;
  con->stmt->param_data[param].data.datetime_data->day= day;
  con->stmt->param_data[param].data.datetime_data->hour= hour;
  con->stmt->param_data[param].data.datetime_data->minute= minute;
  con->stmt->param_data[param].data.datetime_data->second= second;
  con->stmt->param_data[param].data.datetime_data->microsecond= microsecond;
  con->stmt->param_data[param].type= ASCORE_COLUMN_TYPE_DATETIME;

  return NULL;
}

attachsql_error_st *attachsql_statement_set_time(attachsql_connect_t *con, uint16_t param, uint8_t hour, uint8_t minute, uint8_t second, uint32_t microsecond, bool is_negative)
{
  attachsql_error_st *error;

  if ((con == NULL) || (con->stmt == NULL))
  {
    attachsql_error_client_create(&error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Connection parameter not valid");
    return error;
  }

  if (param >= con->stmt->param_count)
  {
    attachsql_error_client_create(&error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Param %d does not exist", param);
    return error;
  }
  con->stmt->param_data[param].data.datetime_data= new ascore_datetime_st;
  con->stmt->param_data[param].data.datetime_data->hour= hour;
  con->stmt->param_data[param].data.datetime_data->minute= minute;
  con->stmt->param_data[param].data.datetime_data->second= second;
  con->stmt->param_data[param].data.datetime_data->microsecond= microsecond;
  con->stmt->param_data[param].data.datetime_data->is_negative= is_negative;
  con->stmt->param_data[param].type= ASCORE_COLUMN_TYPE_TIME;

  return NULL;
}

attachsql_error_st *attachsql_statement_set_param(attachsql_connect_t *con, ascore_column_type_t type, uint16_t param, size_t length, const void *value, bool is_unsigned)
{
  attachsql_error_st *error;

  if ((con == NULL) || (con->stmt == NULL))
  {
    attachsql_error_client_create(&error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Connection parameter not valid");
    return error;
  }

  if (param >= con->stmt->param_count)
  {
    attachsql_error_client_create(&error, ATTACHSQL_ERROR_CODE_PARAMETER, ATTACHSQL_ERROR_LEVEL_ERROR, "22023", "Param %d does not exist", param);
    return error;
  }

  switch (type)
  {
    case ASCORE_COLUMN_TYPE_LONG:
      con->stmt->param_data[param].data.int_data= *(uint32_t*)value;
      con->stmt->param_data[param].is_unsigned= is_unsigned;
      con->stmt->param_data[param].type= type;
      break;
    case ASCORE_COLUMN_TYPE_LONGLONG:
      con->stmt->param_data[param].data.bigint_data= *(uint64_t*)value;
      con->stmt->param_data[param].is_unsigned= is_unsigned;
      con->stmt->param_data[param].type= type;
      break;
    case ASCORE_COLUMN_TYPE_FLOAT:
      con->stmt->param_data[param].data.float_data= *(float*)value;
      con->stmt->param_data[param].is_unsigned= false;
      con->stmt->param_data[param].type= type;
      break;
    case ASCORE_COLUMN_TYPE_DOUBLE:
      con->stmt->param_data[param].data.double_data= *(double*)value;
      con->stmt->param_data[param].is_unsigned= false;
      con->stmt->param_data[param].type= type;
      break;
    case ASCORE_COLUMN_TYPE_STRING:
    case ASCORE_COLUMN_TYPE_BLOB:
      con->stmt->param_data[param].data.string_data= (char*)value;
      con->stmt->param_data[param].length= length;
      con->stmt->param_data[param].type= type;
      break;
    case ASCORE_COLUMN_TYPE_NULL:
      con->stmt->param_data[param].type= type;;
      break;
    /* the following won't happen, but this will keep the compiler happy */
    case ASCORE_COLUMN_TYPE_DECIMAL:
    case ASCORE_COLUMN_TYPE_TINY:
    case ASCORE_COLUMN_TYPE_SHORT:
    case ASCORE_COLUMN_TYPE_TIMESTAMP:
    case ASCORE_COLUMN_TYPE_INT24:
    case ASCORE_COLUMN_TYPE_DATE:
    case ASCORE_COLUMN_TYPE_TIME:
    case ASCORE_COLUMN_TYPE_DATETIME:
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
      break;
  }

  return NULL;
}
