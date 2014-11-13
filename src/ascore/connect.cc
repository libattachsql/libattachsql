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

#include "config.h"
#include "common.h"
#include "connect.h"
#include "sha1.h"
#include "net.h"
#include <errno.h>
#include <string.h>
#ifdef HAVE_OPENSSL
# include <openssl/ssl.h>
#endif

ascon_st *ascore_con_create(const char *host, in_port_t port, const char *user, const char *pass, const char *schema)
{
  ascon_st *con;

  con = new (std::nothrow) ascon_st;
  if (con == NULL)
  {
    return NULL;
  }

  con->host= host;
  con->port= port;
  if ((user == NULL) or (strlen(user) > ASCORE_MAX_USER_SIZE))
  {
    con->local_errcode= ASRET_USER_TOO_LONG;
    con->status= ASCORE_CON_STATUS_PARAMETER_ERROR;
    return con;
  }
  con->user= user;
  // We don't really care how long pass is since we itterate though it during
  // SHA1 passes.  Needs to be nul terminated.  NULL is also acceptable.
  con->pass= pass;
  if ((schema == NULL) or (strlen(schema) > ASCORE_MAX_SCHEMA_SIZE))
  {
    con->local_errcode= ASRET_SCHEMA_TOO_LONG;
    con->status= ASCORE_CON_STATUS_PARAMETER_ERROR;
    return con;
  }
  con->schema= schema;

  return con;
}

void ascore_con_destroy(ascon_st *con)
{
  if (con == NULL)
  {
    return;
  }

  if (con->read_buffer != NULL)
  {
    ascore_buffer_free(con->read_buffer);
  }

  if (con->read_buffer_compress != NULL)
  {
    ascore_buffer_free(con->read_buffer_compress);
  }

  if (con->uncompressed_buffer != NULL)
  {
    free(con->uncompressed_buffer);
  }

  if (con->compressed_buffer != NULL)
  {
    free(con->compressed_buffer);
  }

  if (con->next_packet_queue != NULL)
  {
    free(con->next_packet_queue);
  }

#ifdef HAVE_OPENSSL
  if (con->ssl.bio_buffer != NULL)
  {
    free(con->ssl.bio_buffer);
  }
  if (con->ssl.write_buffer != NULL)
  {
    ascore_buffer_free(con->ssl.write_buffer);
  }
  /* This frees BIOs as well */
  if (con->ssl.ssl != NULL)
  {
    SSL_free(con->ssl.ssl);
  }
  if (con->ssl.context != NULL)
  {
    SSL_CTX_free(con->ssl.context);
  }
#endif
  // On a net error we already closed the connection
  if ((con->uv_objects.stream != NULL) and (con->status != ASCORE_CON_STATUS_NET_ERROR))
  {
    uv_check_stop(&con->uv_objects.check);
    uv_close((uv_handle_t*)con->uv_objects.stream, NULL);
    if (not con->in_group)
    {
      uv_run(con->uv_objects.loop, UV_RUN_DEFAULT);
    }
  }
  if (not con->in_group)
  {
    if (con->uv_objects.loop != NULL)
    {
      uv_loop_delete(con->uv_objects.loop);
    }
    delete con;
  }
}

void on_resolved(uv_getaddrinfo_t *resolver, int status, struct addrinfo *res)
{
  ascon_st *con= (ascon_st *)resolver->data;

  asdebug("Resolver callback");
  if (status != 0)
  {
    asdebug("DNS lookup failure: %s", uv_err_name(uv_last_error(resolver->loop)));
    con->status= ASCORE_CON_STATUS_CONNECT_FAILED;
    con->local_errcode= ASRET_DNS_ERROR;
    snprintf(con->errmsg, ASCORE_ERROR_BUFFER_SIZE, "DNS lookup failure: %s", uv_err_name(uv_last_error(resolver->loop)));
    return;
  }
  char addr[17] = {'\0'};

  uv_ip4_name((struct sockaddr_in*) res->ai_addr, addr, 16);
  asdebug("DNS lookup success: %s", addr);
  uv_tcp_init(resolver->loop, &con->uv_objects.socket.tcp);
  con->uv_objects.socket.tcp.data= con;
  con->uv_objects.connect_req.data= (void*) &con->uv_objects.socket.tcp;
  uv_tcp_connect(&con->uv_objects.connect_req, &con->uv_objects.socket.tcp, *(struct sockaddr_in*) res->ai_addr, on_connect);

  uv_freeaddrinfo(res);
}

ascore_con_status_t ascore_con_poll(ascon_st *con)
{
  //asdebug("Connection poll");
  if (con == NULL)
  {
    return ASCORE_CON_STATUS_PARAMETER_ERROR;
  }

  if ((con->status == ASCORE_CON_STATUS_NOT_CONNECTED) or (con->status == ASCORE_CON_STATUS_CONNECT_FAILED) or (con->status == ASCORE_CON_STATUS_IDLE) or (con->status == ASCORE_CON_STATUS_SSL_ERROR))
  {
    return con->status;
  }
#ifdef HAVE_OPENSSL
  if (con->ssl.handshake_done)
  {
    ascore_ssl_run(con);
  }
#endif
  ascore_run_uv_loop(con);

  return con->status;
}

void ascore_check_for_data_cb(uv_check_t *handle, int status)
{
  (void) status;
  asdebug("Check called");
  struct ascon_st *con= (struct ascon_st*)handle->data;
  ascore_con_process_packets(con);
}

ascore_con_status_t ascore_connect(ascon_st *con)
{
  int ret;

  if (con == NULL)
  {
    return ASCORE_CON_STATUS_PARAMETER_ERROR;
  }

  con->uv_objects.hints.ai_family = PF_INET;
  con->uv_objects.hints.ai_socktype = SOCK_STREAM;
  con->uv_objects.hints.ai_protocol = IPPROTO_TCP;
  con->uv_objects.hints.ai_flags = 0;

  if (con->status != ASCORE_CON_STATUS_NOT_CONNECTED)
  {
    return con->status;
  }

  if (not con->in_group)
  {
    con->uv_objects.loop= uv_loop_new();
  }
  if (con->uv_objects.loop == NULL)
  {
    asdebug("Loop initalize failure");
    con->local_errcode= ASRET_OUT_OF_MEMORY_ERROR;
    snprintf(con->errmsg, ASCORE_ERROR_BUFFER_SIZE, "Loop initialization failure, either out of memory or out of file descripitors (usually the latter)");
    con->status= ASCORE_CON_STATUS_CONNECT_FAILED;
    return con->status;
  }

  snprintf(con->str_port, 6, "%d", con->port);
  // If port is 0 and no explicit option set then assume we mean UDS
  // instead of TCP
  if (con->options.protocol == ASCORE_CON_PROTOCOL_UNKNOWN)
  {
    if (con->port == 0)
    {
      con->options.protocol= ASCORE_CON_PROTOCOL_UDS;
    }
    else
    {
      con->options.protocol= ASCORE_CON_PROTOCOL_TCP;
    }
  }
  switch(con->options.protocol)
  {
    case ASCORE_CON_PROTOCOL_TCP:
      asdebug("TCP connection");
      asdebug("Async DNS lookup: %s", con->host);
      con->uv_objects.resolver.data= con;
      ret= uv_getaddrinfo(con->uv_objects.loop, &con->uv_objects.resolver, on_resolved, con->host, con->str_port, &con->uv_objects.hints);
      if (ret)
      {
        asdebug("DNS lookup fail: %s", uv_err_name(uv_last_error(con->uv_objects.loop)));
        con->local_errcode= ASRET_DNS_ERROR;
        snprintf(con->errmsg, ASCORE_ERROR_BUFFER_SIZE, "DNS lookup failure: %s", uv_err_name(uv_last_error(con->uv_objects.loop)));
        con->status= ASCORE_CON_STATUS_CONNECT_FAILED;
        return con->status;
      }
      con->status= ASCORE_CON_STATUS_CONNECTING;
      ascore_run_uv_loop(con);
      break;
    case ASCORE_CON_PROTOCOL_UDS:
      asdebug("UDS connection");
      uv_pipe_init(con->uv_objects.loop, &con->uv_objects.socket.uds, 1);
      con->uv_objects.socket.uds.data= con;
      con->uv_objects.connect_req.data= (void*) &con->uv_objects.socket.uds;
      con->status= ASCORE_CON_STATUS_CONNECTING;
      uv_pipe_connect(&con->uv_objects.connect_req, &con->uv_objects.socket.uds, con->host, on_connect);
      ascore_run_uv_loop(con);
      break;
    case ASCORE_CON_PROTOCOL_UNKNOWN:
      asdebug("Unknown protocol, this shouldn't happen");
      con->status= ASCORE_CON_STATUS_CONNECT_FAILED;
  }

  return con->status;
}


void on_connect(uv_connect_t *req, int status)
{
  ascon_st *con= (ascon_st*)req->handle->data;
  asdebug("Connect event callback");
  if (status != 0)
  {
    asdebug("Connect fail: %s", uv_err_name(uv_last_error(req->handle->loop)));
    con->local_errcode= ASRET_CONNECT_ERROR;
    con->status= ASCORE_CON_STATUS_CONNECT_FAILED;
    snprintf(con->errmsg, ASCORE_ERROR_BUFFER_SIZE, "Connection failed: %s", uv_err_name(uv_last_error(req->handle->loop)));
    return;
  }
  asdebug("Connection succeeded!");
  ascore_packet_queue_push(con, ASCORE_PACKET_TYPE_HANDSHAKE);
  // maybe move the set con->stream to connect function
  con->uv_objects.stream= (uv_stream_t*)req->data;
  uv_check_init(con->uv_objects.loop, &con->uv_objects.check);
  con->uv_objects.check.data= con;
  uv_check_start(&con->uv_objects.check, ascore_check_for_data_cb);
  uv_read_start((uv_stream_t*)req->data, on_alloc, ascore_read_data_cb);
}

uv_buf_t on_alloc(uv_handle_t *client, size_t suggested_size)
{
  size_t buffer_free;
  uv_buf_t buf;
  ascon_st *con= (ascon_st*) client->data;

#ifdef HAVE_OPENSSL
  if (con->ssl.handshake_done && (con->ssl.bio_buffer_size < suggested_size))
  {
    asdebug("Increasing SSL read buffer to %zd", suggested_size);
    char *realloc_buffer= (char*)realloc(con->ssl.bio_buffer, suggested_size);
    if (realloc_buffer)
    {
      con->ssl.bio_buffer= realloc_buffer;
      con->ssl.bio_buffer_size= suggested_size;
    }
    else
    {
      con->local_errcode= ASRET_OUT_OF_MEMORY_ERROR;
      asdebug("SSL buffer realloc failure");
      con->command_status= ASCORE_COMMAND_STATUS_SEND_FAILED;
      con->next_packet_queue_used= 0;
    }
  }
#endif

  if (not con->options.compression)
  {
    asdebug("%zd bytes requested for read buffer", suggested_size);

    if (con->read_buffer == NULL)
    {
      asdebug("Creating read buffer");
      con->read_buffer= ascore_buffer_create();
    }
    buffer_free= ascore_buffer_get_available(con->read_buffer);
    if (buffer_free < suggested_size)
    {
      asdebug("Enlarging buffer, free: %zd, requested: %zd", buffer_free, suggested_size);
      ascore_buffer_increase(con->read_buffer);
      buffer_free= ascore_buffer_get_available(con->read_buffer);
    }
    buf.base= con->read_buffer->buffer_write_ptr;
    buf.len= buffer_free;
  }
  else
  {
    asdebug("%zd bytes requested for compressed read buffer", suggested_size);

    if (con->read_buffer_compress == NULL)
    {
      asdebug("Creating compressed read buffer");
      con->read_buffer_compress= ascore_buffer_create();
    }
    buffer_free= ascore_buffer_get_available(con->read_buffer_compress);
    if (buffer_free < suggested_size)
    {
      asdebug("Enlarging compress buffer, free: %zd, requested: %zd", buffer_free, suggested_size);
      ascore_buffer_increase(con->read_buffer_compress);
      buffer_free= ascore_buffer_get_available(con->read_buffer_compress);
    }
    buf.base= con->read_buffer_compress->buffer_write_ptr;
    buf.len= buffer_free;
  }

#ifdef HAVE_OPENSSL
  if (con->ssl.handshake_done)
  {
    buf.base= con->ssl.bio_buffer;
    buf.len= con->ssl.bio_buffer_size;
  }
#endif

  return buf;
}

void ascore_packet_read_handshake(ascon_st *con)
{
  asdebug("Connect handshake packet");
  buffer_st *buffer= con->read_buffer;

  // Rejection error before handshake
  if ((unsigned char)buffer->buffer_read_ptr[0] == 0xff)
  {
    ascore_packet_read_response(con);
  }

  // Protocol version
  if (buffer->buffer_read_ptr[0] != 10)
  {
    // Note that 255 is a special immediate auth fail case
    asdebug("Bad protocol version");
    con->local_errcode= ASRET_BAD_PROTOCOL;
    snprintf(con->errmsg, ASCORE_ERROR_BUFFER_SIZE, "Incompatible protocol version");
    return;
  }

  // Server version (null-terminated string)
  buffer->buffer_read_ptr++;
  strncpy(con->server_version, buffer->buffer_read_ptr, ASCORE_MAX_SERVER_VERSION_LEN);
  con->server_version[ASCORE_MAX_SERVER_VERSION_LEN - 1]= '\0';
  buffer->buffer_read_ptr+= strlen(con->server_version) + 1;

  // Thread ID
  con->thread_id= ascore_unpack_int4(buffer->buffer_read_ptr);
  buffer->buffer_read_ptr+= 4;

  // Scramble buffer and 1 byte filler
  memcpy(con->scramble_buffer, buffer->buffer_read_ptr, 8);
  buffer->buffer_read_ptr+= 9;

  // Server capabilities
  con->server_capabilities= (ascore_capabilities_t)ascore_unpack_int2(buffer->buffer_read_ptr);
  buffer->buffer_read_ptr+= 2;
  // Check MySQL 4.1 protocol capability is on, we won't support old auth
  if (not (con->server_capabilities & ASCORE_CAPABILITY_PROTOCOL_41))
  {
    asdebug("MySQL <4.1 Auth not supported");
    con->local_errcode= ASRET_NO_OLD_AUTH;
    snprintf(con->errmsg, ASCORE_ERROR_BUFFER_SIZE, "MySQL 4.1 protocol and higher required");
  }

  con->charset= buffer->buffer_read_ptr[0];
  buffer->buffer_read_ptr++;

  con->server_status= ascore_unpack_int2(buffer->buffer_read_ptr);
  // 13 byte filler and unrequired scramble length (until auth plugins)
  buffer->buffer_read_ptr+= 15;

  memcpy(con->scramble_buffer + 8, buffer->buffer_read_ptr, 12);
  // '\0' scramble terminator
  buffer->buffer_read_ptr+= 13;

  // MySQL 5.5 onwards has more password plugin stuff here, ignore for now
  ascore_packet_read_end(con);

  // Create response packet
  ascore_handshake_response(con);
}

void ascore_handshake_response(ascon_st *con)
{
  unsigned char *buffer_ptr;
  uint32_t capabilities;

  asdebug("Sending handshake response");
  buffer_ptr= (unsigned char*)con->write_buffer;

  capabilities= con->server_capabilities & ASCORE_CAPABILITY_CLIENT;
  capabilities|= ASCORE_CAPABILITY_MULTI_RESULTS;
  capabilities|= con->client_capabilities;

#ifdef HAVE_OPENSSL
  if (con->ssl.ssl != NULL)
  {
    if (not (con->server_capabilities & ASCORE_CAPABILITY_SSL))
    {
      asdebug("SSL disabled on server");
      con->local_errcode= ASRET_NET_SSL_ERROR;
      snprintf(con->errmsg, ASCORE_ERROR_BUFFER_SIZE, "SSL auth not supported enabled on server");
      con->command_status= ASCORE_COMMAND_STATUS_SEND_FAILED;
      con->next_packet_queue_used= 0;
      con->status= ASCORE_CON_STATUS_CONNECT_FAILED;
      return;
    }
    capabilities |= ASCORE_CAPABILITY_SSL;
  }
#endif

  ascore_pack_int4(buffer_ptr, capabilities);
  buffer_ptr+= 4;

  // Set max packet size to our buffer size for now
  ascore_pack_int4(buffer_ptr, ASCORE_DEFAULT_BUFFER_SIZE);
  buffer_ptr+= 4;

  // Change this when we support charsets
  buffer_ptr[0]= 0;
  buffer_ptr++;

  // 0x00 padding for 23 bytes
  memset(buffer_ptr, 0, 23);
  buffer_ptr+= 23;

#ifdef HAVE_OPENSSL
  if ((con->ssl.ssl != NULL) and (ascore_packet_queue_peek(con) != ASCORE_PACKET_TYPE_HANDSHAKE_SSL))
  {
    /* for SSL we do a short handshake ending here in plain text,
     * no user/password sent.
     * The enablement of the SSL capability tells the server that OpenSSL
     * handshake happens next followed by a full MySQL handshake packet with
     * user/pass encrypted
     */
    ascore_send_data(con, con->write_buffer, (size_t)(buffer_ptr - (unsigned char*)con->write_buffer));
    ascore_packet_queue_push(con, ASCORE_PACKET_TYPE_HANDSHAKE_SSL);
    ascore_handshake_response(con);
    return;
  }

  if ((con->ssl.ssl != NULL) and (ascore_packet_queue_peek(con) == ASCORE_PACKET_TYPE_HANDSHAKE_SSL))
  {
    /* second entry into handshake for SSL, this response and all other send /
     * receives should be encrypted from now on
     */
    con->ssl.enabled= true;
    ascore_packet_queue_pop(con);
  }
#endif

  // User name
  memcpy(buffer_ptr, con->user, strlen(con->user));
  buffer_ptr+= strlen(con->user);
  buffer_ptr[0]= '\0';
  buffer_ptr++;

  // Password
  // TODO: add support for password plugins
  if (con->pass[0] != '\0')
  {
    asret_t ret;
    buffer_ptr[0]= SHA1_DIGEST_LENGTH; // probably should use char packing?
    buffer_ptr++;
    ret= scramble_password(con, (unsigned char*)buffer_ptr);
    if (ret != ASRET_OK)
    {
      asdebug("Scramble problem!");
      con->local_errcode= ASRET_BAD_SCRAMBLE;
      return;
    }
    buffer_ptr+= SHA1_DIGEST_LENGTH;
  }
  else
  {
    buffer_ptr[0]= '\0';
    buffer_ptr++;
  }

  if (con->schema != NULL)
  {
    memcpy(buffer_ptr, con->schema, strlen(con->schema));
    buffer_ptr+= strlen(con->schema);
  }
  buffer_ptr[0]= '\0';
  buffer_ptr++;
  ascore_send_data(con, con->write_buffer, (size_t)(buffer_ptr - (unsigned char*)con->write_buffer));
  ascore_packet_queue_push(con, ASCORE_PACKET_TYPE_RESPONSE);
}

asret_t scramble_password(ascon_st *con, unsigned char *buffer)
{
  SHA1_CTX ctx;
  unsigned char stage1[SHA1_DIGEST_LENGTH];
  unsigned char stage2[SHA1_DIGEST_LENGTH];
  uint8_t it;

  if (con->scramble_buffer[0] == '\0')
  {
    asdebug("No scramble supplied from server");
    return ASRET_NO_SCRAMBLE;
  }

  // Double hash the password
  SHA1Init(&ctx);
  SHA1Update(&ctx, (unsigned char*)con->pass, strlen(con->pass));
  SHA1Final(stage1, &ctx);
  SHA1Init(&ctx);
  SHA1Update(&ctx, stage1, SHA1_DIGEST_LENGTH);
  SHA1Final(stage2, &ctx);

  // Hash the scramble with the double hash
  SHA1Init(&ctx);
  SHA1Update(&ctx, con->scramble_buffer, SHA1_DIGEST_LENGTH);
  SHA1Update(&ctx, stage2, SHA1_DIGEST_LENGTH);
  SHA1Final(buffer, &ctx);

  // XOR the hash with the stage1 hash
  for (it= 0; it < SHA1_DIGEST_LENGTH; it++)
  {
    buffer[it]= buffer[it] ^ stage1[it];
  }

  return ASRET_OK;
}

void ascore_library_init(void)
{
#ifdef HAVE_OPENSSL
  SSL_load_error_strings();
  SSL_library_init();
#endif
}

#ifdef HAVE_OPENSSL
bool ascore_con_set_ssl(ascon_st *con, const char *key, const char *cert, const char *ca, const char *capath, const char *cipher, bool verify)
{
  con->ssl.context= SSL_CTX_new(TLSv1_client_method());

  if (cipher != NULL)
  {
    if (SSL_CTX_set_cipher_list(con->ssl.context, cipher) != 1)
    {
      strncpy(con->errmsg, "Error setting SSL cipher list", ASCORE_ERROR_BUFFER_SIZE - 1);
      con->errmsg[ASCORE_ERROR_BUFFER_SIZE - 1]= '\0';
      return false;
    }
  }

  if (SSL_CTX_load_verify_locations(con->ssl.context, ca, capath) != 1)
  {
    strncpy(con->errmsg, "Error loading the SSL certificate authority file", ASCORE_ERROR_BUFFER_SIZE -1);
    con->errmsg[ASCORE_ERROR_BUFFER_SIZE - 1]= '\0';
    return false;
  }

  if (cert != NULL)
  {
    if (SSL_CTX_use_certificate_file(con->ssl.context, cert, SSL_FILETYPE_PEM) != 1)
    {
      strncpy(con->errmsg, "Error loading the SSL certificate file", ASCORE_ERROR_BUFFER_SIZE - 1);
      con->errmsg[ASCORE_ERROR_BUFFER_SIZE - 1]= '\0';
      return false;
    }

    if (key == NULL)
    {
      key= cert;
    }

    if (SSL_CTX_use_PrivateKey_file(con->ssl.context, key, SSL_FILETYPE_PEM) != 1)
    {
      strncpy(con->errmsg, "Cannot load the SSL key file", ASCORE_ERROR_BUFFER_SIZE - 1);
      con->errmsg[ASCORE_ERROR_BUFFER_SIZE - 1]= '\0';
      return false;
    }

    if (SSL_CTX_check_private_key(con->ssl.context) != 1)
    {
      strncpy(con->errmsg, "Error validating the SSL private key", ASCORE_ERROR_BUFFER_SIZE - 1);
      con->errmsg[ASCORE_ERROR_BUFFER_SIZE - 1]= '\0';
      return false;
    }
  }

  if (verify)
  {
    SSL_CTX_set_verify(con->ssl.context, SSL_VERIFY_PEER, NULL);
  }
  con->ssl.ssl= SSL_new(con->ssl.context);
  /* force client handshake mode to allow SSL_write before handshake complete */
  SSL_set_connect_state(con->ssl.ssl);

  return true;
}
#endif
