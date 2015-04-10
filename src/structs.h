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

#include "constants.h"
#include "buffer.h"
#include "return.h"
#include <sys/types.h>
#include <stdint.h>
#ifdef _WIN32

typedef uint16_t in_port_t;

#else
# include <sys/socket.h>
# include <arpa/inet.h>
#endif

#include <uv.h>
#include <libattachsql2/attachsql.h>

#ifdef HAVE_OPENSSL
# include <openssl/ssl.h>
#endif

#ifdef __cplusplus
#include <cstddef>
extern "C" {
#endif

typedef struct attachsql_connect_t attachsql_connect_t;
typedef struct attachsql_pool_t attachsql_pool_t;

#define ATTACHSQL_BUFFER_ROW_ALLOC_SIZE 100
#define ATTACHSQL_STMT_CHAR_BUFFER_SIZE 40

struct attachsql_stmt_row_st
{
  char *data;
  size_t length;
  attachsql_column_type_t type;

  attachsql_stmt_row_st() :
    data(NULL),
    length(0),
    type(ATTACHSQL_COLUMN_TYPE_NULL)
  { }
};

struct column_t
{
  char schema[ATTACHSQL_MAX_SCHEMA_SIZE];
  char table[ATTACHSQL_MAX_TABLE_SIZE];
  char origin_table[ATTACHSQL_MAX_TABLE_SIZE];
  char column[ATTACHSQL_MAX_COLUMN_SIZE];
  char origin_column[ATTACHSQL_MAX_COLUMN_SIZE];
  uint16_t charset;
  uint32_t length;
  attachsql_column_type_t type;
  attachsql_column_flags_t flags;
  uint8_t decimals;
  char default_value[ATTACHSQL_MAX_DEFAULT_VALUE_SIZE];
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

struct attachsql_datetime_st
{
  uint16_t year;
  uint8_t month;
  uint32_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  uint32_t microsecond;
  bool is_negative;

  attachsql_datetime_st ():
    year(0),
    month(0),
    day(0),
    hour(0),
    minute(0),
    second(0),
    microsecond(0),
    is_negative(false)
  { }
};

struct attachsql_stmt_param_st
{
  attachsql_column_type_t type;
  size_t length;
  bool is_long_data;
  bool is_unsigned;
  bool datetime_alloc;
  union data_t
  {
    uint8_t tinyint_data;
    uint16_t smallint_data;
    uint32_t int_data;
    uint64_t bigint_data;
    float float_data;
    double double_data;
    attachsql_datetime_st *datetime_data;
    char *string_data;
  } data;

  attachsql_stmt_param_st() :
    type(ATTACHSQL_COLUMN_TYPE_NULL),
    length(0),
    is_long_data(false),
    is_unsigned(false),
    datetime_alloc(false)
  { }
};

struct attachsql_stmt_st
{
  attachsql_connect_t *con;
  uint32_t id;
  uint16_t column_count;
  uint16_t current_column;
  uint16_t param_count;
  column_t *params;
  uint16_t current_param;
  attachsql_stmt_state_t state;
  char *exec_buffer;
  size_t exec_buffer_length;
  attachsql_stmt_param_st *param_data;
  bool new_bind;

  attachsql_stmt_st():
    con(NULL),
    id(0),
    column_count(0),
    current_column(0),
    param_count(0),
    params(NULL),
    current_param(0),
    state(ATTACHSQL_STMT_STATE_NONE),
    exec_buffer(NULL),
    exec_buffer_length(0),
    param_data(NULL),
    new_bind(true)
  { }
};

struct attachsql_connect_t
{
  const char *host;
  in_port_t port;
  char str_port[6];
  const char *user;
  const char *pass;
  const char *schema;
  struct options_t
  {
    bool compression;
    attachsql_con_protocol_t protocol;
    bool semi_block;

    options_t() :
      compression(false),
      protocol(ATTACHSQL_CON_PROTOCOL_UNKNOWN),
      semi_block(false)
    { }
  } options;

  attachsql_con_status_t status;
  char errmsg[ATTACHSQL_ERROR_BUFFER_SIZE];
  attachsql_ret_t local_errcode;
  buffer_st *read_buffer;
  buffer_st *read_buffer_compress;
  char write_buffer[ATTACHSQL_WRITE_BUFFER_SIZE];
  uint8_t write_buffer_extra; /* for extra bytes in packet header due to prepared statement */
  uint8_t packet_number;
  uint32_t thread_id;
  uint32_t connection_id; /* the pool ID */
  char server_version[ATTACHSQL_MAX_SERVER_VERSION_LEN];
  unsigned char scramble_buffer[20];
  char packet_header[4];
  attachsql_capabilities_t server_capabilities;
  int client_capabilities;
  uint32_t packet_size;
  uint64_t affected_rows;
  uint64_t insert_id;
  uint16_t server_status;
  uint16_t warning_count;
  uint16_t server_errno;
  char server_message[ATTACHSQL_MAX_MESSAGE_LEN];
  char sqlstate[ATTACHSQL_SQLSTATE_SIZE];
  uint8_t charset;
  struct result_t result;
  attachsql_command_status_t command_status;
  attachsql_packet_type_t *next_packet_queue;
  size_t next_packet_queue_size;
  size_t next_packet_queue_used;
  char *uncompressed_buffer;
  size_t uncompressed_buffer_len;
  char *compressed_buffer;
  size_t compressed_buffer_len;
  char compressed_packet_header[7];
  uint8_t compressed_packet_number;
  struct uv_objects_t
  {
    uv_loop_t *loop;
    uv_check_t check;
    struct addrinfo hints;
    uv_getaddrinfo_t resolver;
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
#ifdef HAVE_OPENSSL
  struct ssl_t
  {
    SSL *ssl;
    SSL_CTX *context;
    bool no_verify;
    bool enabled; // set to true after first handshake to signify send/receive should be encrypted.
    bool handshake_done;
    BIO* read_bio;
    BIO* write_bio;
    char ssl_read_buffer[1024*16];
    char ssl_write_buffer[1024*16];
    buffer_st *write_buffer;
    char* bio_buffer;
    size_t bio_buffer_size;

    ssl_t() :
      ssl(NULL),
      context(NULL),
      no_verify(false),
      enabled(false),
      handshake_done(false),
      read_bio(NULL),
      write_bio(NULL),
      write_buffer(NULL),
      bio_buffer(NULL),
      bio_buffer_size(0)
    {
      ssl_read_buffer[0]= '\0';
      ssl_write_buffer[0]= '\0';
    }
  } ssl;
#endif
  bool in_statement;
  attachsql_stmt_st *stmt;
  /* the following has been migrated during struct merge */
  attachsql_pool_t *pool;
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
  attachsql_stmt_row_st *stmt_row;
  char *stmt_null_bitmap;
  uint16_t stmt_null_bitmap_length;
  char stmt_tmp_buffer[ATTACHSQL_STMT_CHAR_BUFFER_SIZE];
  attachsql_events_t last_callback;

  attachsql_connect_t() :
    host(NULL),
    port(0),
    user(NULL),
    pass(NULL),
    schema(NULL),
    status(ATTACHSQL_CON_STATUS_NOT_CONNECTED),
    local_errcode(ATTACHSQL_RET_OK),
    read_buffer(NULL),
    read_buffer_compress(NULL),
    write_buffer_extra(0),
    packet_number(0),
    thread_id(0),
    connection_id(0),
    server_capabilities(ATTACHSQL_CAPABILITY_NONE),
    client_capabilities(0),
    packet_size(0),
    affected_rows(0),
    insert_id(0),
    server_status(0),
    warning_count(0),
    server_errno(0),
    charset(0),
    command_status(ATTACHSQL_COMMAND_STATUS_EOF),
    next_packet_queue(NULL),
    next_packet_queue_size(0),
    next_packet_queue_used(0),
    uncompressed_buffer(NULL),
    uncompressed_buffer_len(0),
    compressed_buffer(NULL),
    compressed_buffer_len(0),
    compressed_packet_number(0),
    in_statement(false),
    stmt(NULL),
    pool(NULL),
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
    stmt_row(NULL),
    stmt_null_bitmap(NULL),
    stmt_null_bitmap_length(0),
    last_callback(ATTACHSQL_EVENT_NONE)
  {
    str_port[0]= '\0';
    errmsg[0]= '\0';
    server_version[0]= '\0';
    scramble_buffer[0]= '\0';
    packet_header[0]= '\0';
    server_message[0]= '\0';
    sqlstate[0]= '\0';
    write_buffer[0]= '\0';
    compressed_packet_header[0]= '\0';
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

struct attachsql_pool_t
{
  attachsql_connect_t **connections;
  uint32_t connection_count;
  attachsql_callback_fn *callback_fn;
  void *callback_context;
  uv_loop_t *loop;

  attachsql_pool_t() :
    connections(NULL),
    connection_count(0),
    callback_fn(NULL),
    callback_context(NULL),
    loop(NULL)
  { }

};

#ifdef __cplusplus
}
#endif
