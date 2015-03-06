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

enum attachsql_ret_t {
  ATTACHSQL_RET_OK,
  ATTACHSQL_RET_PARAMETER_ERROR,
  ATTACHSQL_RET_CONNECTING,
  ATTACHSQL_RET_DNS_ERROR,
  ATTACHSQL_RET_CONNECT_ERROR,
  ATTACHSQL_RET_OUT_OF_MEMORY_ERROR,
  ATTACHSQL_RET_NO_SCRAMBLE,
  ATTACHSQL_RET_USER_TOO_LONG,
  ATTACHSQL_RET_SCHEMA_TOO_LONG,
  ATTACHSQL_RET_NET_READ_ERROR,
  ATTACHSQL_RET_NET_SSL_ERROR,
  ATTACHSQL_RET_NET_WRITE_ERROR,
  ATTACHSQL_RET_PACKET_OUT_OF_SEQUENCE,
  ATTACHSQL_RET_BAD_PROTOCOL,
  ATTACHSQL_RET_NO_OLD_AUTH,
  ATTACHSQL_RET_BAD_SCRAMBLE,
  ATTACHSQL_RET_COMPRESSION_FAILURE,
  ATTACHSQL_RET_BAD_STMT_PARAMETER
};

#ifdef __cplusplus
}
#endif
