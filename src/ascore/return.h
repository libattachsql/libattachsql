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

enum asret_t {
  ASRET_OK,
  ASRET_PARAMETER_ERROR,
  ASRET_CONNECTING,
  ASRET_DNS_ERROR,
  ASRET_CONNECT_ERROR,
  ASRET_OUT_OF_MEMORY_ERROR,
  ASRET_NO_SCRAMBLE,
  ASRET_USER_TOO_LONG,
  ASRET_SCHEMA_TOO_LONG,
  ASRET_NET_READ_ERROR,
  ASRET_NET_WRITE_ERROR,
  ASRET_PACKET_OUT_OF_SEQUENCE,
  ASRET_BAD_PROTOCOL,
  ASRET_NO_OLD_AUTH,
  ASRET_BAD_SCRAMBLE,
  ASRET_COMPRESSION_FAILURE
};

#ifdef __cplusplus
}
#endif
