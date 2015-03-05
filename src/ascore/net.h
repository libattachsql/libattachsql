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

#ifdef __cplusplus
extern "C" {
#endif

void attachsql_send_data(attachsql_connect_t *con, char *data, size_t length);

void on_write(uv_write_t *req, int status);

void attachsql_read_data_cb(uv_stream_t* tcp, ssize_t read_size, const uv_buf_t buf);

bool attachsql_con_process_packets(attachsql_connect_t *con);

void attachsql_packet_read_end(attachsql_connect_t *con);

void attachsql_packet_read_response(attachsql_connect_t *con);

void attachsql_packet_read_prepare_response(attachsql_connect_t *con);

void attachsql_packet_read_prepare_parameter(attachsql_connect_t *con);

void attachsql_packet_read_prepare_column(attachsql_connect_t *con);

void attachsql_packet_get_column(attachsql_connect_t *con, column_t *column);

void attachsql_packet_read_column(attachsql_connect_t *con);

void attachsql_packet_stmt_read_row(attachsql_connect_t *con);

void attachsql_packet_read_row(attachsql_connect_t *con);

void attachsql_run_uv_loop(attachsql_connect_t *con);

bool attachsql_packet_queue_push(attachsql_connect_t *con, attachsql_packet_type_t packet_type);

attachsql_packet_type_t attachsql_packet_queue_pop(attachsql_connect_t *con);

attachsql_packet_type_t attachsql_packet_queue_peek(attachsql_connect_t *con);

#ifdef HAVE_ZLIB
void attachsql_send_compressed_packet(attachsql_connect_t *con, char *data, size_t length, uint8_t command);
#endif

#ifdef HAVE_OPENSSL
void attachsql_ssl_run(attachsql_connect_t *con);

void attachsql_ssl_data_check(attachsql_connect_t *con);

void attachsql_ssl_handle_error(attachsql_connect_t *con, int result);
#endif

#ifdef __cplusplus
}
#endif
