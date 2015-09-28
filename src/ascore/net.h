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

void ascore_send_data(ascon_st *con, char *data, size_t length);

void on_write(uv_write_t *req, int status);

void ascore_read_data_cb(uv_stream_t* tcp, ssize_t read_size, const uv_buf_t* buf);

bool ascore_con_process_packets(ascon_st *con);

void ascore_packet_read_end(ascon_st *con);

void ascore_packet_read_response(ascon_st *con);

void ascore_packet_read_prepare_response(ascon_st *con);

void ascore_packet_read_prepare_parameter(ascon_st *con);

void ascore_packet_read_prepare_column(ascon_st *con);

void ascore_packet_get_column(ascon_st *con, column_t *column);

void ascore_packet_read_column(ascon_st *con);

void ascore_packet_stmt_read_row(ascon_st *con);

void ascore_packet_read_row(ascon_st *con);

void ascore_run_uv_loop(ascon_st *con);

bool ascore_packet_queue_push(ascon_st *con, ascore_packet_type_t packet_type);

ascore_packet_type_t ascore_packet_queue_pop(ascon_st *con);

ascore_packet_type_t ascore_packet_queue_peek(ascon_st *con);

#ifdef HAVE_ZLIB
void ascore_send_compressed_packet(ascon_st *con, char *data, size_t length, uint8_t command);
#endif

#ifdef HAVE_OPENSSL
void ascore_ssl_run(ascon_st *con);

void ascore_ssl_data_check(ascon_st *con);

void ascore_ssl_handle_error(ascon_st *con, int result);
#endif

#ifdef __cplusplus
}
#endif
