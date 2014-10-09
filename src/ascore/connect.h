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

ascon_st *ascore_con_create(const char *host, in_port_t port, const char *user, const char *pass, const char *schema);

ascore_con_status_t ascore_connect(ascon_st *con);

ascore_con_status_t ascore_con_poll(ascon_st *con);

void ascore_con_destroy(ascon_st *con);

void on_resolved(uv_getaddrinfo_t *resolver, int status, struct addrinfo *res);

void on_connect(uv_connect_t *req, int status);

void ascore_packet_read_handshake(ascon_st *con);

uv_buf_t on_alloc(uv_handle_t *client, size_t suggested_size);

asret_t scramble_password(ascon_st *con, unsigned char *buffer);

void ascore_handshake_response(ascon_st *con);

void ascore_library_init(void);

#ifdef HAVE_OPENSSL
bool ascore_con_set_ssl(ascon_st *con, const char *key, const char *cert, const char *ca, const char *capath, const char *cipher, bool verify);

int ascore_ssl_buffer_write(ascon_st *con, uv_buf_t *buf, int buf_len);
#endif

void ascore_check_for_data_cb(uv_check_t *handle, int status);

#ifdef __cplusplus
}
#endif


