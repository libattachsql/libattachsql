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

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

struct attachsql_query_parameter_st
{
  attachsql_query_parameter_type_t type;
  void *data;
  size_t length;
  bool is_unsigned;
};

typedef struct attachsql_query_parameter_st attachsql_query_parameter_st;

struct attachsql_query_column_st
{
  char *schema;
  char *table;
  char *origin_table;
  char *column;
  char *origin_column;
  uint16_t charset;
  uint32_t length;
  attachsql_column_type_t type;
  attachsql_column_flags_t flags;
  uint8_t decimals;
  char *default_value;
  size_t default_size;
};

typedef struct attachsql_query_column_st attachsql_query_column_st;

struct attachsql_query_row_st
{
  char *data;
  size_t length;
};

typedef struct attachsql_query_row_st attachsql_query_row_st;

#ifdef __cplusplus
}
#endif

