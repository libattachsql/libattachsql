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

#include "common.h"
#include "buffer.h"
#include <sys/types.h>
#include <stdint.h>
#ifdef _WIN32

typedef uint16_t in_port_t;

#else
# include <sys/socket.h>
# include <arpa/inet.h>
#endif

#include <uv.h>

#ifdef __cplusplus
#include <cstddef>
extern "C" {
#endif

struct column_t
{
  char schema[ASCORE_MAX_SCHEMA_SIZE];
  char table[ASCORE_MAX_TABLE_SIZE];
  char origin_table[ASCORE_MAX_TABLE_SIZE];
  char column[ASCORE_MAX_COLUMN_SIZE];
  char origin_column[ASCORE_MAX_COLUMN_SIZE];
  uint16_t charset;
  uint32_t length;
  ascore_column_type_t type;
  ascore_column_flags_t flags;
  uint8_t decimals;
  char default_value[ASCORE_MAX_DEFAULT_VALUE_SIZE];
  size_t default_size;
};

struct result_t
{
  uint16_t column_count;
  uint64_t extra;
  column_t *columns;
  uint16_t current_column;
  char *row_data;
  size_t row_length;

  result_t() :
    column_count(0),
    extra(0),
    columns(NULL),
    current_column(0),
    row_data(NULL),
    row_length(0)
  { }
};

struct ascon_st
{
  const char *host;
  in_port_t port;
  char str_port[6];
  const char *user;
  const char *pass;
  const char *schema;
  struct options_t
  {
    bool polling;
    ascore_con_protocol_t protocol;

    options_t() :
      polling(false),
      protocol(ASCORE_CON_PROTOCOL_UNKNOWN)
    { }
  } options;

  ascore_con_status_t status;
  char errmsg[ASCORE_ERROR_BUFFER_SIZE];
  asret_t local_errcode;
  buffer_st *read_buffer;
  char write_buffer[ASCORE_WRITE_BUFFER_SIZE];
  uint8_t packet_number;
  uint32_t thread_id;
  char server_version[ASCORE_MAX_SERVER_VERSION_LEN];
  unsigned char scramble_buffer[20];
  char packet_header[4];
  ascore_capabilities_t server_capabilities;
  int client_capabilities;
  uint32_t packet_size;
  uint64_t affected_rows;
  uint64_t insert_id;
  uint16_t server_status;
  uint16_t warning_count;
  uint16_t server_errno;
  char server_message[ASCORE_MAX_MESSAGE_LEN];
  char sqlstate[ASCORE_SQLSTATE_SIZE];
  uint8_t charset;
  struct result_t result;
  ascore_command_status_t command_status;
  ascore_packet_type_t next_packet_type;
  struct uv_objects_t
  {
    uv_loop_t *loop;
    struct addrinfo hints;
    uv_getaddrinfo_t resolver;
    uv_write_t write_req;
    uv_connect_t connect_req;
    uv_stream_t *stream;
    union socket_t
    {
      uv_tcp_t tcp;
      uv_pipe_t uds;
    } socket;

    uv_objects_t() :
      loop(NULL),
      stream(NULL)
    { }
  } uv_objects;

  ascon_st() :
    host(NULL),
    port(0),
    user(NULL),
    pass(NULL),
    schema(NULL),
    status(ASCORE_CON_STATUS_NOT_CONNECTED),
    local_errcode(ASRET_OK),
    read_buffer(NULL),
    packet_number(0),
    thread_id(0),
    server_capabilities(ASCORE_CAPABILITY_NONE),
    client_capabilities(0),
    packet_size(0),
    affected_rows(0),
    insert_id(0),
    server_status(0),
    warning_count(0),
    server_errno(0),
    charset(0),
    command_status(ASCORE_COMMAND_STATUS_NONE),
    next_packet_type(ASCORE_PACKET_TYPE_NONE)
  {
    errmsg[0]= '\0';
    server_version[0]= '\0';
    scramble_buffer[0]= '\0';
    packet_header[0]= '\0';
    server_message[0]= '\0';
    sqlstate[0]= '\0';
    write_buffer[0]= '\0';
  }
};

#ifdef __cplusplus
}
#endif
