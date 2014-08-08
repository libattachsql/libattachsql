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
attachsql_connect_t *attachsql_connect_create(const char *host, in_port_t port, const char *user, const char *pass, const char *schema, attachsql_error_st **error);

ASQL_API
void attachsql_connect_destroy(attachsql_connect_t *con);

ASQL_API
void attachsql_connect_set_callback(attachsql_connect_t *con, attachsql_callback_fn *function, void *context);

ASQL_API
const char *attachsql_connect_get_server_version(attachsql_connect_t *con);

ASQL_API
uint32_t attachsql_connect_get_connection_id(attachsql_connect_t *con);

ASQL_API
attachsql_return_t attachsql_connect_poll(attachsql_connect_t *con, attachsql_error_st **error);

ASQL_API
attachsql_error_st *attachsql_connect(attachsql_connect_t *con);

ASQL_API
bool attachsql_connect_set_option(attachsql_connect_t *con, attachsql_options_t option, const void *arg);

#ifdef __cplusplus
}
#endif
