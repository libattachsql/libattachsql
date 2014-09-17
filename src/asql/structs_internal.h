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

#include "src/ascore/ascore.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ATTACHSQL_BUFFER_ROW_ALLOC_SIZE 100

struct attachsql_connect_t
{
  ascon_st *core_con;
  attachsql_callback_fn *callback_fn;
  void *callback_context;
  char *query_buffer;
  size_t query_buffer_length;
  bool query_buffer_alloc;
  bool query_buffer_statement;
  bool in_query;
  bool buffer_rows;
  attachsql_query_column_st *columns;
  attachsql_query_row_st *row;
  attachsql_query_row_st **row_buffer;
  uint64_t row_buffer_alloc_size;
  uint64_t row_buffer_count;
  uint64_t row_buffer_position;
  bool all_rows_buffered;
  ascore_stmt_st *stmt;

  attachsql_connect_t():
    core_con(NULL),
    callback_fn(NULL),
    callback_context(NULL),
    query_buffer(NULL),
    query_buffer_length(0),
    query_buffer_alloc(false),
    query_buffer_statement(false),
    in_query(false),
    buffer_rows(false),
    columns(NULL),
    row(NULL),
    row_buffer(NULL),
    row_buffer_alloc_size(0),
    row_buffer_count(0),
    row_buffer_position(0),
    all_rows_buffered(false),
    stmt(NULL)
  { }
};

#ifdef __cplusplus
}
#endif

