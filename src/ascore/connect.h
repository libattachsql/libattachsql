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

#include "return.h"
#include "buffer.h"
#include "common.h"
#include <sys/types.h>
#include <stdint.h>
#ifndef _WIN32
# include <sys/socket.h>
#endif

#ifdef __cplusplus
#include <cstddef>
extern "C" {
#endif

attachsql_connect_t *attachsql_con_create(const char *host, in_port_t port, const char *user, const char *pass, const char *schema);

attachsql_con_status_t attachsql_do_connect(attachsql_connect_t *con);

attachsql_con_status_t attachsql_con_poll(attachsql_connect_t *con);

void attachsql_con_destroy(attachsql_connect_t *con);

void on_resolved(uv_getaddrinfo_t *resolver, int status, struct addrinfo *res);

void on_connect(uv_connect_t *req, int status);

void attachsql_packet_read_handshake(attachsql_connect_t *con);

uv_buf_t on_alloc(uv_handle_t *client, size_t suggested_size);

asret_t scramble_password(attachsql_connect_t *con, unsigned char *buffer);

void attachsql_handshake_response(attachsql_connect_t *con);

#ifdef HAVE_OPENSSL
bool attachsql_con_set_ssl(attachsql_connect_t *con, const char *key, const char *cert, const char *ca, const char *capath, const char *cipher, bool verify);

int attachsql_ssl_buffer_write(attachsql_connect_t *con, uv_buf_t *buf, int buf_len);
#endif

void attachsql_check_for_data_cb(uv_check_t *handle, int status);

#ifdef __cplusplus
}
#endif


