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

struct attachsql_connect_t
{
  ascon_st *core_con;
  attachsql_callback_fn *callback_fn;
  void *callback_context;
  char *query_buffer;
  bool in_query;
  attachsql_query_column_st *columns;
  attachsql_query_row_st *row;

  attachsql_connect_t():
    core_con(NULL),
    callback_fn(NULL),
    callback_context(NULL),
    query_buffer(NULL),
    in_query(false),
    columns(NULL),
    row(NULL)
  { }
};

#ifdef __cplusplus
}
#endif

