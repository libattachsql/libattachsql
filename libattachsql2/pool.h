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

ASQL_API
attachsql_pool_t *attachsql_pool_create(attachsql_error_t **error);

ASQL_API
void attachsql_pool_destroy(attachsql_pool_t *pool);

ASQL_API
void attachsql_pool_add_connection(attachsql_pool_t *pool, attachsql_connect_t *con, attachsql_error_t **error);

ASQL_API
void attachsql_pool_run(attachsql_pool_t *pool);

#ifdef __cplusplus
}
#endif

