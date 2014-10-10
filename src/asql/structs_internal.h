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
#define ATTACHSQL_STMT_CHAR_BUFFER_SIZE 40

struct attachsql_stmt_row_st
{
  char *data;
  size_t length;
  ascore_column_type_t type;

  attachsql_stmt_row_st() :
    data(NULL),
    length(0),
    type(ASCORE_COLUMN_TYPE_NULL)
  { }
};

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
  attachsql_stmt_row_st *stmt_row;
  char *stmt_null_bitmap;
  uint16_t stmt_null_bitmap_length;
  char stmt_tmp_buffer[ATTACHSQL_STMT_CHAR_BUFFER_SIZE];

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
    stmt(NULL),
    stmt_row(NULL),
    stmt_null_bitmap(NULL),
    stmt_null_bitmap_length(0)
  {
    stmt_tmp_buffer[0]= '\0';
  }
};

struct attachsql_error_t
{
  int code;
  char msg[ATTACHSQL_MESSAGE_SIZE];
  char sqlstate[ATTACHSQL_SQLSTATE_SIZE];
  attachsql_error_level_t level;

  attachsql_error_t() :
    code(0),
    level(ATTACHSQL_ERROR_LEVEL_NOTICE)
  {
    msg[0]= '\0';
    sqlstate[0]= '\0';
  }
};

struct attachsql_group_t
{
  attachsql_connect_t **connections;
  size_t connection_count;
  uv_loop_t *loop;

  attachsql_group_t() :
    connections(NULL),
    connection_count(0),
    loop(NULL)
  { }

};

#ifdef __cplusplus
}
#endif

