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
#include "net.h"
#include "pack.h"
#include "pack_macros.h"
#ifdef HAVE_ZLIB
# include <zlib.h>
#endif

#ifdef HAVE_OPENSSL
# include <openssl/ssl.h>
# include <openssl/err.h>
#endif

#ifdef HAVE_OPENSSL
void ascore_ssl_run(ascon_st *con)
{
  int r;

  if (!SSL_is_init_finished(con->ssl.ssl))
  {
    asdebug("SSL handshake in progress");
    r= SSL_connect(con->ssl.ssl);
    if (r < 0)
    {
      ascore_ssl_handle_error(con, r);
    }
    ascore_ssl_data_check(con);
  }
  else
  {
    asdebug("Attempting SSL buffer read");
    r= SSL_read(con->ssl.ssl, con->ssl.ssl_read_buffer, sizeof(con->ssl.ssl_read_buffer));
    if (r < 0)
    {
      ascore_ssl_handle_error(con, r);
    }
    asdebug("Read %d encrypted bytes", r);
    ascore_ssl_data_check(con);
  }
}

void ascore_ssl_data_check(ascon_st *con)
{
  int bytes_read= 0;
  uv_buf_t send_buffer[1];

  if (con->ssl.write_buffer)
  {
    size_t write_buffer_len= ascore_buffer_unread_data(con->ssl.write_buffer);
    if (write_buffer_len > 0)
    {
      int r= SSL_write(con->ssl.ssl, con->ssl.write_buffer->buffer_read_ptr, (int)write_buffer_len);
      if (r < 0)
      {
        int error= SSL_get_error(con->ssl.ssl, r);
        if (error != SSL_ERROR_WANT_READ)
        {
          con->local_errcode= ASRET_NET_SSL_ERROR;
          asdebug("SSL write fail: %d", error);
          con->command_status= ASCORE_COMMAND_STATUS_SEND_FAILED;
          con->next_packet_queue_used= 0;
          con->status= ASCORE_CON_STATUS_SSL_ERROR;
          attachsql_snprintf(con->errmsg, ASCORE_ERROR_BUFFER_SIZE - 1, "SSL write fail: %s", ERR_reason_error_string(ERR_get_error()));
          con->errmsg[ASCORE_ERROR_BUFFER_SIZE - 1]= '\0';
        }
      }
      else
      {
        asdebug("%d bytes written to SSL", r);
        con->ssl.write_buffer->buffer_read_ptr+= r;
      }
    }
  }

  while((bytes_read= BIO_read(con->ssl.write_bio, con->ssl.ssl_write_buffer, sizeof(con->ssl.ssl_write_buffer))) > 0)
  {
    asdebug("%d bytes sent from SSL to net", bytes_read);
    send_buffer[0].base= con->ssl.ssl_write_buffer;
    send_buffer[0].len= bytes_read;
    uv_write_t *req= new (std::nothrow) uv_write_t;
	int ret = uv_write(req, con->uv_objects.stream, send_buffer, 1, on_write);
    if (ret != 0)
    {
      con->local_errcode= ASRET_NET_WRITE_ERROR;
      asdebug("Write fail: %s", uv_err_name(ret));
      con->command_status= ASCORE_COMMAND_STATUS_SEND_FAILED;
      con->next_packet_queue_used= 0;
    }
  }
}

void ascore_ssl_handle_error(ascon_st *con, int result)
{
  int error= SSL_get_error(con->ssl.ssl, result);
  if (error == SSL_ERROR_WANT_READ)
  {
    ascore_ssl_data_check(con);
  }
  else
  {
    con->local_errcode= ASRET_NET_SSL_ERROR;
    unsigned long errcode= ERR_get_error();
    asdebug("SSL fail: %d, %s", error, ERR_error_string(errcode, NULL));
    con->status= ASCORE_CON_STATUS_SSL_ERROR;
    con->command_status= ASCORE_COMMAND_STATUS_SEND_FAILED;
    con->next_packet_queue_used= 0;
    attachsql_snprintf(con->errmsg, ASCORE_ERROR_BUFFER_SIZE - 1, "SSL read/write fail: %s", ERR_reason_error_string(errcode));
    con->errmsg[ASCORE_ERROR_BUFFER_SIZE - 1]= '\0';
  }
}

int ascore_ssl_buffer_write(ascon_st *con, uv_buf_t *buf, int buf_len)
{
  size_t required_size= 0;
  int current_buf;
  for (current_buf= 0; current_buf < buf_len; current_buf++)
  {
    required_size+= buf[current_buf].len;
  }
  if (con->ssl.write_buffer == NULL)
  {
    asdebug("Creating SSL write buffer");
    con->ssl.write_buffer= ascore_buffer_create();
  }
  if (ascore_buffer_get_available(con->ssl.write_buffer) < required_size)
  {
    asdebug("Enlarging SSL write buffer");
    ascore_buffer_increase(con->ssl.write_buffer);
  }
  for (current_buf= 0; current_buf < buf_len; current_buf++)
  {
    memcpy(con->ssl.write_buffer->buffer_write_ptr, buf[current_buf].base, buf[current_buf].len);
    ascore_buffer_move_write_ptr(con->ssl.write_buffer, buf[current_buf].len);
  }
  ascore_ssl_run(con);
  return 0;
}

#endif

void ascore_send_data(ascon_st *con, char *data, size_t length)
{
  uv_buf_t send_buffer[2];

#ifdef HAVE_OPENSSL
  if (con->ssl.enabled && !con->ssl.handshake_done)
  {
    con->ssl.read_bio= BIO_new(BIO_s_mem());
    con->ssl.write_bio= BIO_new(BIO_s_mem());
    SSL_set_bio(con->ssl.ssl, con->ssl.read_bio, con->ssl.write_bio);
    SSL_set_connect_state(con->ssl.ssl);
    SSL_do_handshake(con->ssl.ssl);
    ascore_ssl_run(con);
    con->ssl.handshake_done= true;
  }
#endif

  asdebug("Sending %zd bytes to server", length);
  ascore_pack_int3(con->packet_header, length);
  con->packet_number++;
  con->packet_header[3]= con->packet_number;

#ifdef HAVE_ZLIB
  /* Can't use con->options.compression because at this point we haven't even connected so the flag isn't set */
  if ((con->client_capabilities & ASCORE_CAPABILITY_COMPRESS) && (con->status != ASCORE_CON_STATUS_CONNECTING))
  {
    ascore_send_compressed_packet(con, data, length, 0);
    return;
  }
#endif

  send_buffer[0].base= con->packet_header;
  send_buffer[0].len= 4;
  send_buffer[1].base= data;
  send_buffer[1].len= length;
  asdebug_hex(data, length);
  con->command_status= ASCORE_COMMAND_STATUS_READ_RESPONSE;

  int r;
#ifdef HAVE_OPENSSL
  if (con->ssl.handshake_done)
  {
    r= ascore_ssl_buffer_write(con, send_buffer, 2);
  }
  else
#endif
  {
    uv_write_t *req= new (std::nothrow) uv_write_t;
    r= uv_write(req, con->uv_objects.stream, send_buffer, 2, on_write);
  }
  if (r != 0)
  {
      con->local_errcode= ASRET_NET_WRITE_ERROR;
      asdebug("Write fail: %s", uv_err_name(r));
      con->command_status= ASCORE_COMMAND_STATUS_SEND_FAILED;
      con->next_packet_queue_used= 0;
  }
}

#ifdef HAVE_ZLIB
void ascore_send_compressed_packet(ascon_st *con, char *data, size_t length, uint8_t command)
{
  uv_buf_t send_buffer[2];
  char *realloc_buffer;
  size_t required_uncompressed;
  size_t new_size;
  size_t compressed_length;

  asdebug("Packet compress");
  required_uncompressed= length + 4;  // Data plus packet header
  if (command)
  {
    required_uncompressed+= 1 + con->write_buffer_extra;
    con->compressed_packet_number= 0;
  }
  if (con->uncompressed_buffer_len < required_uncompressed)
  {
    /* Enlarge to a multiple of ASCORE_WRITE_BUFFER_SIZE */
    size_t rounding= required_uncompressed % ASCORE_WRITE_BUFFER_SIZE;
    new_size= required_uncompressed + ASCORE_WRITE_BUFFER_SIZE - rounding;
    asdebug("Enlarging uncompressed buffer to %zu bytes", new_size);
    realloc_buffer= (char*)realloc(con->uncompressed_buffer, new_size);
    if (realloc_buffer == NULL)
    {
      con->local_errcode= ASRET_OUT_OF_MEMORY_ERROR;
      asdebug("Uncompressed buffer realloc failure");
      con->command_status= ASCORE_COMMAND_STATUS_SEND_FAILED;
      con->next_packet_queue_used= 0;
      return;
    }
    con->uncompressed_buffer= realloc_buffer;
    con->uncompressed_buffer_len= new_size;
  }
  memcpy(con->uncompressed_buffer, con->packet_header, 4);
  if (command)
  {
    con->uncompressed_buffer[4]= (char)command;
    if (con->write_buffer_extra)
    {
      memcpy(&con->uncompressed_buffer[5], &con->write_buffer[1], con->write_buffer_extra);
    }
    memcpy(&con->uncompressed_buffer[5 + con->write_buffer_extra], data, length);
    con->write_buffer_extra= 0;
  }
  else
  {
    memcpy(&con->uncompressed_buffer[4], data, length);
  }

  if (length > ASCORE_MINIMUM_COMPRESS_SIZE)
  {
    size_t required_compressed;
    /* compress the packet */
    asdebug("Compressing packet");
    required_compressed= (size_t)compressBound((uLong) required_uncompressed);
    if (con->compressed_buffer_len < required_compressed)
    {
      /* Enlarge to a multiple of ASCORE_WRITE_BUFFER_SIZE */
      size_t rounding= required_compressed % ASCORE_WRITE_BUFFER_SIZE;
      new_size= required_compressed + ASCORE_WRITE_BUFFER_SIZE - rounding;
      asdebug("Enlarging compressed buffer to %zu bytes", new_size);
      realloc_buffer= (char*)realloc(con->compressed_buffer, new_size);
      if (realloc_buffer == NULL)
      {
        con->local_errcode= ASRET_OUT_OF_MEMORY_ERROR;
        asdebug("Compressed buffer realloc failure");
        con->command_status= ASCORE_COMMAND_STATUS_SEND_FAILED;
        con->next_packet_queue_used= 0;
        return;
      }
      con->compressed_buffer= realloc_buffer;
      con->compressed_buffer_len= new_size;
    }
    compressed_length= con->compressed_buffer_len;
    int res= compress((Bytef*)con->compressed_buffer, (uLongf*)&compressed_length, (Bytef*)con->uncompressed_buffer, (uLong)required_uncompressed);
    if ((res != Z_OK) || (con->uncompressed_buffer_len < compressed_length))
    {
      con->local_errcode= ASRET_COMPRESSION_FAILURE;
      asdebug("Compression failure");
      con->command_status= ASCORE_COMMAND_STATUS_SEND_FAILED;
      con->next_packet_queue_used= 0;
      return;
    }
    ascore_pack_int3(con->compressed_packet_header, compressed_length);
    con->compressed_packet_header[3]= con->compressed_packet_number;
    ascore_pack_int3(con->compressed_packet_header+4, required_uncompressed);
    send_buffer[0].base= con->compressed_packet_header;
    send_buffer[0].len= 7;
    send_buffer[1].base= con->compressed_buffer;
    send_buffer[1].len= compressed_length;
    asdebug("Old size: %zu, new size: %zu", required_uncompressed, compressed_length);
  }
  else
  {
    ascore_pack_int3(con->compressed_packet_header, required_uncompressed);
    con->compressed_packet_header[3]= con->compressed_packet_number;
    ascore_pack_int3(con->compressed_packet_header+4, 0);
    asdebug_hex(con->compressed_packet_header, 7);
    send_buffer[0].base= con->compressed_packet_header;
    send_buffer[0].len= 7;
    send_buffer[1].base= con->uncompressed_buffer;
    send_buffer[1].len= required_uncompressed;
    asdebug("Packet too small to compress, sending %zu bytes", required_uncompressed);
    asdebug_hex(con->uncompressed_buffer, required_uncompressed);
  }
  asdebug_hex(data, length);
  con->command_status= ASCORE_COMMAND_STATUS_READ_RESPONSE;

  int r;
#ifdef HAVE_OPENSSL
  if (con->ssl.handshake_done)
  {
    r= ascore_ssl_buffer_write(con, send_buffer, 2);
  }
  else
#endif
  {
    uv_write_t *req= new (std::nothrow) uv_write_t;
    r= uv_write(req, con->uv_objects.stream, send_buffer, 2, on_write);
  }
  if (r != 0)
  {
    con->local_errcode= ASRET_NET_WRITE_ERROR;
    asdebug("Write fail: %s", uv_err_name(r));
    con->command_status= ASCORE_COMMAND_STATUS_SEND_FAILED;
    con->next_packet_queue_used= 0;
  }
}
#endif

void on_write(uv_write_t *req, int status)
{
  ascon_st *con= (ascon_st*)req->handle->data;
  asdebug("Write callback, status: %d", status);

  if (ascore_packet_queue_peek(con) == ASCORE_PACKET_TYPE_NONE)
  {
    con->status= ASCORE_CON_STATUS_IDLE;
    con->command_status= ASCORE_COMMAND_STATUS_EOF;
  }
  if (status != 0)
  {
    con->local_errcode= ASRET_NET_WRITE_ERROR;
    asdebug("Write fail: %s", uv_err_name(status));
    con->command_status= ASCORE_COMMAND_STATUS_SEND_FAILED;
    con->next_packet_queue_used= 0;
    con->status= ASCORE_CON_STATUS_NET_ERROR;
    attachsql_snprintf(con->errmsg, ASCORE_ERROR_BUFFER_SIZE, "Net write failure: %s", uv_err_name(status));
    uv_check_stop(&con->uv_objects.check);
    uv_close((uv_handle_t*)con->uv_objects.stream, NULL);
  }
  delete req;
}

void ascore_read_data_cb(uv_stream_t* tcp, ssize_t read_size, const uv_buf_t* buf)
{
  (void) buf;
  struct ascon_st *con= (struct ascon_st*)tcp->data;

  if (read_size < 0)
  {
    con->local_errcode= ASRET_NET_READ_ERROR;
    asdebug("Read fail: %s", uv_err_name(read_size));
    con->command_status= ASCORE_COMMAND_STATUS_READ_FAILED;
    con->next_packet_queue_used= 0;
    con->status= ASCORE_CON_STATUS_NET_ERROR;
    attachsql_snprintf(con->errmsg, ASCORE_ERROR_BUFFER_SIZE, "Net read failure: %s", uv_err_name(read_size));
    uv_check_stop(&con->uv_objects.check);
    uv_close((uv_handle_t*)con->uv_objects.stream, NULL);
    return;
  }

#ifdef HAVE_OPENSSL
  if (con->ssl.handshake_done)
  {
    asdebug("Got encrypted data, %zd bytes", read_size);
    BIO_write(con->ssl.read_bio, buf->base, (int)read_size);
    buffer_st *buffer= NULL;
    if (con->options.compression)
    {
      buffer= con->read_buffer_compress;
    }
    else
    {
      buffer= con->read_buffer;
    }
    size_t available_buffer= ascore_buffer_get_available(buffer);
    int r= SSL_read(con->ssl.ssl, buffer->buffer_write_ptr, (int)available_buffer);
    if (r < 0)
    {
      ascore_ssl_handle_error(con, r);
    }
    else if (r > 0)
    {
      asdebug("Got unencrypted data, %d bytes", r);
      ascore_buffer_move_write_ptr(buffer, r);
    }
    return;
  }
#endif

  asdebug("Got data, %zd bytes", read_size);
  if (con->options.compression)
  {
    ascore_buffer_move_write_ptr(con->read_buffer_compress, read_size);
  }
  else
  {
    ascore_buffer_move_write_ptr(con->read_buffer, read_size);
  }
  ascore_con_process_packets(con);
}

#ifdef HAVE_ZLIB
bool ascore_con_decompress_read_buffer(ascon_st *con)
{
  size_t data_size;
  size_t buffer_free;
  uint32_t compressed_packet_size;
  uint32_t uncompressed_packet_size;

  /* compress packet header is 7 bytes */
  data_size= ascore_buffer_unread_data(con->read_buffer_compress);
  if (data_size < 7)
  {
    return false;
  }
  // First 3 bytes are packet size
  compressed_packet_size= ascore_unpack_int3(con->read_buffer_compress->buffer_read_ptr);
  //con->packet_size= packet_len;

  if ((compressed_packet_size + 7) > data_size)
  {
    asdebug("Don't have whole compressed packet, expected %u bytes, got %zu", compressed_packet_size, data_size - 7);
    return false;
  }

  con->compressed_packet_number++;

  // Fourth byte is packet number
  asdebug("Got compressed packet %d, expected %d", con->read_buffer_compress->buffer_read_ptr[3], con->compressed_packet_number);

  if (con->compressed_packet_number != con->read_buffer_compress->buffer_read_ptr[3])
  {
    asdebug("Compressed packet out of sequence!");
    con->local_errcode= ASRET_PACKET_OUT_OF_SEQUENCE;
    con->command_status= ASCORE_COMMAND_STATUS_READ_FAILED;
    con->next_packet_queue_used= 0;
    return true;
  }
  uncompressed_packet_size= ascore_unpack_int3(con->read_buffer_compress->buffer_read_ptr+4);
  asdebug("%u bytes requested for read buffer", uncompressed_packet_size);
  con->read_buffer_compress->buffer_read_ptr+= 7;

  if (con->read_buffer == NULL)
  {
    asdebug("Creating read buffer");
    con->read_buffer= ascore_buffer_create();
  }
  buffer_free= ascore_buffer_get_available(con->read_buffer);
  if (buffer_free < uncompressed_packet_size)
  {
    asdebug("Enlarging buffer, free: %zu, requested: %u", buffer_free, uncompressed_packet_size);
    ascore_buffer_increase(con->read_buffer);
    buffer_free= ascore_buffer_get_available(con->read_buffer);
  }
  con->read_buffer_compress->packet_end_ptr= con->read_buffer_compress->buffer_read_ptr + compressed_packet_size;

  if (!uncompressed_packet_size)
  {
    asdebug("Compression packet with no compression");
    memcpy(con->read_buffer->buffer_write_ptr, con->read_buffer_compress->buffer_read_ptr, compressed_packet_size);
    con->read_buffer_compress->buffer_read_ptr+= compressed_packet_size;
    ascore_buffer_move_write_ptr(con->read_buffer, compressed_packet_size);
    return true;
  }
  else
  {
    asdebug("Decompressing %u bytes into %u bytes", compressed_packet_size, uncompressed_packet_size);
    int res= uncompress((Bytef*)con->read_buffer->buffer_write_ptr, (uLongf*)&buffer_free, (Bytef*)con->read_buffer_compress->buffer_read_ptr, (uLong)compressed_packet_size);
    if (res != Z_OK)
    {
      asdebug("Decompression error: %d", res);
      con->local_errcode= ASRET_COMPRESSION_FAILURE;
      con->command_status= ASCORE_COMMAND_STATUS_READ_FAILED;
      con->next_packet_queue_used= 0;
      return true;
    }
    con->read_buffer_compress->buffer_read_ptr+= compressed_packet_size;
    ascore_buffer_move_write_ptr(con->read_buffer, buffer_free);
    return true;
  }
}
#endif

bool ascore_con_process_packets(ascon_st *con)
{
  uint32_t packet_len;
  size_t data_size;

  if (!con->options.compression && (con->read_buffer == NULL))
  {
    return false;
  }
#ifdef HAVE_ZLIB
  if (con->options.compression)
  {
    if (con->read_buffer_compress == NULL)
    {
      return false;
    }
    if (!ascore_con_decompress_read_buffer(con) && (con->read_buffer == NULL))
    {
      return false;
    }
  }
#endif

  ascore_packet_type_t next_packet_type;
  while ((next_packet_type= ascore_packet_queue_peek(con)) != ASCORE_PACKET_TYPE_NONE)
  {
    // Packet header is 4 bytes, if we don't have that, then we don't have enough
    data_size= ascore_buffer_unread_data(con->read_buffer);
    if (data_size < 4)
    {
      asdebug("Read less than 4 bytes (%zu bytes), waiting for more", data_size);
      return false;
    }

    // First 3 bytes are packet size
    packet_len= ascore_unpack_int3(con->read_buffer->buffer_read_ptr);
    con->packet_size= packet_len;

    if ((packet_len + 4) > data_size)
    {
      asdebug("Don't have whole packet, expected %u bytes, got %zu", packet_len, data_size - 4);
      return false;
    }

    // If initial read handshake packet_number is 0, so don't increment
    if (next_packet_type != ASCORE_PACKET_TYPE_HANDSHAKE)
    {
      con->packet_number++;
    }
    ascore_packet_queue_pop(con);
    // Fourth byte is packet number
    asdebug("Got packet %d, expected %d", con->read_buffer->buffer_read_ptr[3], con->packet_number);
    if (con->packet_number != con->read_buffer->buffer_read_ptr[3])
    {
      asdebug("Packet out of sequence!");
      con->local_errcode= ASRET_PACKET_OUT_OF_SEQUENCE;
      con->command_status= ASCORE_COMMAND_STATUS_READ_FAILED;
      con->next_packet_queue_used= 0;
      return true;
    }
    con->read_buffer->buffer_read_ptr+= 4;
    con->read_buffer->packet_end_ptr= con->read_buffer->buffer_read_ptr + packet_len;
    asdebug_hex(con->read_buffer->buffer_read_ptr, packet_len);
    switch(next_packet_type)
    {
      case ASCORE_PACKET_TYPE_NONE:
        // Shoudn't happen
        return true;
      case ASCORE_PACKET_TYPE_HANDSHAKE:
        ascore_packet_read_handshake(con);
        break;
      case ASCORE_PACKET_TYPE_HANDSHAKE_SSL:
        ascore_handshake_response(con);
        break;
      case ASCORE_PACKET_TYPE_RESPONSE:
        ascore_packet_read_response(con);
        break;
      case ASCORE_PACKET_TYPE_PREPARE_RESPONSE:
        ascore_packet_read_prepare_response(con);
        break;
      case ASCORE_PACKET_TYPE_PREPARE_PARAMETER:
        ascore_packet_read_prepare_parameter(con);
        break;
      case ASCORE_PACKET_TYPE_PREPARE_COLUMN:
        ascore_packet_read_prepare_column(con);
        break;
      case ASCORE_PACKET_TYPE_COLUMN:
        ascore_packet_read_column(con);
        break;
      case ASCORE_PACKET_TYPE_STMT_ROW:
      case ASCORE_PACKET_TYPE_ROW:
        ascore_packet_read_row(con);
        return true;
        break;
    }
  }

  return false;
}

void ascore_packet_read_row(ascon_st *con)
{
  // If we hit an EOF instead
  if ((unsigned char)con->read_buffer->buffer_read_ptr[0] == 0xfe)
  {
    con->result.row_data= NULL;
    con->result.row_length= 0;
    ascore_packet_read_response(con);
    return;
  }
  asdebug("Row read");
  con->result.row_data= con->read_buffer->buffer_read_ptr;
  con->result.row_length= con->packet_size;
  con->command_status= ASCORE_COMMAND_STATUS_ROW_IN_BUFFER;
  con->status= ASCORE_CON_STATUS_IDLE;
}

void ascore_packet_read_end(ascon_st *con)
{
  asdebug("Packet end");

  ascore_buffer_packet_read_end(con->read_buffer);
}

void ascore_packet_read_prepare_response(ascon_st *con)
{
  asdebug("Prepare response packet");
  buffer_st *buffer= con->read_buffer;

  if (buffer->buffer_read_ptr[0] != 0x00)
  {
    /* Stmt error packets are the same as normal ones */
    ascore_packet_read_response(con);
  }
  else
  {
    uint32_t data_read= 0;
    asdebug("Got PREPARE_OK packet");
    buffer->buffer_read_ptr++;
    data_read++;
    con->stmt->id= ascore_unpack_int4(buffer->buffer_read_ptr);
    buffer->buffer_read_ptr+= 4;
    data_read+= 4;
    con->stmt->column_count= ascore_unpack_int2(buffer->buffer_read_ptr);
    buffer->buffer_read_ptr+= 2;
    data_read+= 2;
    con->stmt->param_count= ascore_unpack_int2(buffer->buffer_read_ptr);
    /* one byte filler */
    buffer->buffer_read_ptr+= 3;
    data_read+= 3;
    con->warning_count= ascore_unpack_int2(buffer->buffer_read_ptr);
    buffer->buffer_read_ptr+= 2;
    data_read+= 2;
    if (con->stmt->param_count)
    {
      con->stmt->params= new (std::nothrow) column_t[con->stmt->param_count];
      con->stmt->param_data= new (std::nothrow) ascore_stmt_param_st[con->stmt->param_count];
    }
    if (con->stmt->param_count > 0)
    {
      ascore_packet_queue_push(con, ASCORE_PACKET_TYPE_PREPARE_PARAMETER);
      con->command_status= ASCORE_COMMAND_STATUS_READ_STMT_PARAM;
    }
    else if (con->stmt->column_count > 0)
    {
      ascore_packet_queue_push(con, ASCORE_PACKET_TYPE_PREPARE_COLUMN);
      con->command_status= ASCORE_COMMAND_STATUS_READ_STMT_COLUMN;
    }
    else
    {
      con->command_status= ASCORE_COMMAND_STATUS_EOF;
      con->status= ASCORE_CON_STATUS_IDLE;
      ascore_packet_read_end(con);
    }
    buffer->buffer_read_ptr+= (con->packet_size - data_read);
    con->stmt->state= ASCORE_STMT_STATE_PREPARED;
    ascore_packet_read_end(con);
  }
}

void ascore_packet_read_response(ascon_st *con)
{
  asdebug("Response packet");
  uint8_t bytes;
  uint32_t data_read= 0;
  buffer_st *buffer= con->read_buffer;

  if (buffer->buffer_read_ptr[0] == 0x00)
  {
    // This is an OK packet
    asdebug("Got OK packet");
    buffer->buffer_read_ptr++;
    data_read++;
    con->affected_rows= ascore_unpack_length(buffer->buffer_read_ptr, &bytes, NULL);
    buffer->buffer_read_ptr+= bytes;
    data_read+= bytes;
    con->insert_id= ascore_unpack_length(buffer->buffer_read_ptr, &bytes, NULL);
    buffer->buffer_read_ptr+= bytes;
    data_read+= bytes;
    con->server_status= ascore_unpack_int2(buffer->buffer_read_ptr);
    buffer->buffer_read_ptr+= 2;
    data_read+= 2;
    con->warning_count= ascore_unpack_int2(buffer->buffer_read_ptr);
    buffer->buffer_read_ptr+= 2;
    data_read+= 2;
    attachsql_snprintf(con->server_message, ASCORE_MAX_MESSAGE_LEN, "%.*s", (con->packet_size - data_read), buffer->buffer_read_ptr);
    con->server_message[ASCORE_MAX_MESSAGE_LEN - 1]= '\0';
    buffer->buffer_read_ptr+= (con->packet_size - data_read);
    if (con->status == ASCORE_CON_STATUS_CONNECTING)
    {
      con->command_status= ASCORE_COMMAND_STATUS_CONNECTED;
      if (con->client_capabilities & ASCORE_CAPABILITY_COMPRESS)
      {
        con->options.compression= true;
      }
    }
    else
    {
      con->command_status= ASCORE_COMMAND_STATUS_EOF;
    }
    con->status= ASCORE_CON_STATUS_IDLE;
    ascore_packet_read_end(con);
  }
  else if ((unsigned char)buffer->buffer_read_ptr[0] == 0xff)
  {
    // This is an Error packet
    asdebug("Got Error packet");
    buffer->buffer_read_ptr++;
    data_read++;
    con->server_errno= ascore_unpack_int2(buffer->buffer_read_ptr);
    // Also skip the SQLSTATE marker, always a '#'
    buffer->buffer_read_ptr+= 3;
    data_read+= 3;
    memcpy(con->sqlstate, buffer->buffer_read_ptr, 5);
    buffer->buffer_read_ptr+= 5;
    data_read+= 5;
    attachsql_snprintf(con->server_message, ASCORE_MAX_MESSAGE_LEN, "%.*s", (con->packet_size - data_read), buffer->buffer_read_ptr);
    con->server_message[ASCORE_MAX_MESSAGE_LEN - 1]= '\0';
    buffer->buffer_read_ptr+= (con->packet_size - data_read);
    if (con->command_status == ASCORE_COMMAND_STATUS_READ_RESPONSE)
    {
      con->status= ASCORE_CON_STATUS_CONNECT_FAILED;
    }
    else
    {
      con->status= ASCORE_CON_STATUS_IDLE;
    }
    con->next_packet_queue_used= 0;
    con->command_status= ASCORE_COMMAND_STATUS_NONE;
    ascore_packet_read_end(con);
  }
  else if ((unsigned char)buffer->buffer_read_ptr[0] == 0xfe)
  {
    // This is an EOF packet
    asdebug("Got EOF packet");
    buffer->buffer_read_ptr++;
    con->warning_count= ascore_unpack_int2(buffer->buffer_read_ptr);
    buffer->buffer_read_ptr+= 2;
    con->server_status= ascore_unpack_int2(buffer->buffer_read_ptr);
    buffer->buffer_read_ptr+= 2;
    if (con->command_status == ASCORE_COMMAND_STATUS_READ_COLUMN)
    {
      con->command_status= ASCORE_COMMAND_STATUS_READ_ROW;
      ascore_packet_queue_push(con, ASCORE_PACKET_TYPE_ROW);
      ascore_buffer_packet_read_end(con->read_buffer);
    }
    else if (con->command_status == ASCORE_COMMAND_STATUS_READ_STMT_PARAM)
    {
      if (con->stmt->column_count > 0)
      {
        con->command_status= ASCORE_COMMAND_STATUS_READ_STMT_COLUMN;
        ascore_packet_queue_push(con, ASCORE_PACKET_TYPE_PREPARE_COLUMN);
        ascore_buffer_packet_read_end(con->read_buffer);
      }
      else
      {
        con->command_status= ASCORE_COMMAND_STATUS_EOF;
        con->status= ASCORE_CON_STATUS_IDLE;
        ascore_packet_read_end(con);
      }
    }
    else
    {
      con->command_status= ASCORE_COMMAND_STATUS_EOF;
      con->status= ASCORE_CON_STATUS_IDLE;
      ascore_packet_read_end(con);
    }
  }
  else
  {
    // This is a result packet
    asdebug("Got result packet");
    con->result.column_count= (uint16_t)ascore_unpack_length(buffer->buffer_read_ptr, &bytes, NULL);
    buffer->buffer_read_ptr+= bytes;
    con->result.columns= new (std::nothrow) column_t[con->result.column_count];
    ascore_buffer_packet_read_end(con->read_buffer);
    ascore_packet_queue_push(con, ASCORE_PACKET_TYPE_COLUMN);
    con->command_status= ASCORE_COMMAND_STATUS_READ_COLUMN;
  }
}

void ascore_packet_read_prepare_parameter(ascon_st *con)
{
  asdebug("Prepare parameter packet read");
  column_t *column;

  column= &con->stmt->params[con->stmt->current_param];
  ascore_packet_get_column(con, column);
  con->stmt->current_param++;
  if (con->stmt->current_param == con->stmt->param_count)
  {
    ascore_packet_queue_push(con, ASCORE_PACKET_TYPE_RESPONSE);
  }
  else
  {
    ascore_packet_queue_push(con, ASCORE_PACKET_TYPE_PREPARE_PARAMETER);
  }
}

void ascore_packet_read_prepare_column(ascon_st *con)
{
  /* Skipping these packets as they are useless */
  asdebug("Prepare column packet callback (skipped)");
  ascore_buffer_packet_read_end(con->read_buffer);
  con->stmt->current_column++;
  if (con->stmt->current_column == con->stmt->column_count)
  {
    ascore_packet_queue_push(con, ASCORE_PACKET_TYPE_RESPONSE);
  }
  else
  {
    ascore_packet_queue_push(con, ASCORE_PACKET_TYPE_PREPARE_COLUMN);
  }
}

void ascore_packet_read_column(ascon_st *con)
{
  asdebug("Column packet callback");
  column_t *column;

  column= &con->result.columns[con->result.current_column];
  ascore_packet_get_column(con, column);
  con->result.current_column++;
  if (con->result.current_column == con->result.column_count)
  {
    ascore_packet_queue_push(con, ASCORE_PACKET_TYPE_RESPONSE);
  }
  else
  {
    ascore_packet_queue_push(con, ASCORE_PACKET_TYPE_COLUMN);
  }
}

void ascore_packet_get_column(ascon_st *con, column_t *column)
{
  uint8_t bytes;
  uint64_t str_len;
  size_t str_read;
  buffer_st *buffer= con->read_buffer;

  // Skip catalog since no MySQL version actually uses this yet
  str_len= ascore_unpack_length(buffer->buffer_read_ptr, &bytes, NULL);
  buffer->buffer_read_ptr+= bytes;
  buffer->buffer_read_ptr+= str_len;

  // Schema
  str_len= ascore_unpack_length(buffer->buffer_read_ptr, &bytes, NULL);
  buffer->buffer_read_ptr+= bytes;
  if (str_len >= ASCORE_MAX_SCHEMA_SIZE)
  {
    str_read= ASCORE_MAX_SCHEMA_SIZE - 1;
  }
  else
  {
    str_read= (size_t)str_len;
  }
  if (str_read > 0)
  {
    memcpy(column->schema, buffer->buffer_read_ptr, str_read);
  }
  column->schema[str_read]= '\0';
  buffer->buffer_read_ptr+= str_len;

  // Table
  str_len= ascore_unpack_length(buffer->buffer_read_ptr, &bytes, NULL);
  buffer->buffer_read_ptr+= bytes;
  if (str_len >= ASCORE_MAX_TABLE_SIZE)
  {
    str_read= ASCORE_MAX_TABLE_SIZE -1;
  }
  else
  {
    str_read= (size_t)str_len;
  }
  if (str_read > 0)
  {
    memcpy(column->table, buffer->buffer_read_ptr, str_read);
  }
  column->table[str_read]= '\0';
  buffer->buffer_read_ptr+= str_len;

  // Origin table
  str_len= ascore_unpack_length(buffer->buffer_read_ptr, &bytes, NULL);
  buffer->buffer_read_ptr+= bytes;
  if (str_len >= ASCORE_MAX_TABLE_SIZE)
  {
    str_read= ASCORE_MAX_TABLE_SIZE -1;
  }
  else
  {
    str_read= (size_t)str_len;
  }
  if (str_read > 0)
  {
    memcpy(column->origin_table, buffer->buffer_read_ptr, str_read);
  }
  column->origin_table[str_read]= '\0';
  buffer->buffer_read_ptr+= str_len;

  // Column
  str_len= ascore_unpack_length(buffer->buffer_read_ptr, &bytes, NULL);
  buffer->buffer_read_ptr+= bytes;
  if (str_len >= ASCORE_MAX_COLUMN_SIZE)
  {
    str_read= ASCORE_MAX_COLUMN_SIZE -1;
  }
  else
  {
    str_read= (size_t)str_len;
  }
  if (str_read > 0)
  {
    memcpy(column->column, buffer->buffer_read_ptr, str_read);
  }
  column->column[str_read]= '\0';
  buffer->buffer_read_ptr+= str_len;

  // Origin column
  str_len= ascore_unpack_length(buffer->buffer_read_ptr, &bytes, NULL);
  buffer->buffer_read_ptr+= bytes;
  if (str_len >= ASCORE_MAX_COLUMN_SIZE)
  {
    str_read= ASCORE_MAX_COLUMN_SIZE -1;
  }
  else
  {
    str_read= (size_t)str_len;
  }
  if (str_read > 0)
  {
    memcpy(column->origin_column, buffer->buffer_read_ptr, str_read);
  }
  column->origin_column[str_read]= '\0';
  buffer->buffer_read_ptr+= str_len;

  // Padding
  buffer->buffer_read_ptr++;

  // Charset
  column->charset= ascore_unpack_int2(buffer->buffer_read_ptr);
  buffer->buffer_read_ptr+= 2;

  // Length
  column->length= ascore_unpack_int4(buffer->buffer_read_ptr);
  buffer->buffer_read_ptr+= 4;

  // Type
  /* direct char -> enum seems to cause signed -> unsigned conversion issues */
  uint8_t type= (uint8_t)buffer->buffer_read_ptr[0];
  column->type= (ascore_column_type_t)type;
  buffer->buffer_read_ptr++;

  // Flags
  column->flags= (ascore_column_flags_t)ascore_unpack_int2(buffer->buffer_read_ptr);
  buffer->buffer_read_ptr+= 2;

  // Decimals
  column->decimals= buffer->buffer_read_ptr[0];
  buffer->buffer_read_ptr++;

  // Padding
  buffer->buffer_read_ptr+= 2;

  // Default value
  str_len= ascore_unpack_length(buffer->buffer_read_ptr, &bytes, NULL);
  buffer->buffer_read_ptr+= bytes;
  if (str_len >= ASCORE_MAX_DEFAULT_VALUE_SIZE)
  {
    str_read= ASCORE_MAX_DEFAULT_VALUE_SIZE -1;
  }
  else
  {
    str_read= (size_t)str_len;
  }
  if (str_read > 0)
  {
    memcpy(column->default_value, buffer->buffer_read_ptr, str_read);
  }
  column->default_size= str_read;
  buffer->buffer_read_ptr+= str_len;
  asdebug("Got column %s.%s.%s", column->schema, column->table, column->column);
  ascore_buffer_packet_read_end(con->read_buffer);
}

void ascore_run_uv_loop(ascon_st *con)
{
  if (!con->in_group)
  {
    if (con->options.semi_block)
    {
      uv_run(con->uv_objects.loop, UV_RUN_ONCE);
    }
    else
    {
      uv_run(con->uv_objects.loop, UV_RUN_NOWAIT);
    }
  }
}

bool ascore_packet_queue_push(ascon_st *con, ascore_packet_type_t packet_type)
{
  asdebug("Push packet type: %d, count: %zu", packet_type, con->next_packet_queue_used+1);
  ascore_packet_type_t *new_queue= NULL;
  if (con->next_packet_queue_size == 0)
  {
    con->next_packet_queue= (ascore_packet_type_t*)malloc(sizeof(ascore_packet_type_t) * ASCORE_DEFAULT_PACKET_QUEUE_SIZE);
    con->next_packet_queue_size= ASCORE_DEFAULT_PACKET_QUEUE_SIZE;
    if (con->next_packet_queue == NULL)
    {
      return false;
    }
  }

  if (con->next_packet_queue_used >= con->next_packet_queue_size)
  {
    new_queue= (ascore_packet_type_t*)realloc(con->next_packet_queue, con->next_packet_queue_size * 2);
    if (new_queue == NULL)
    {
      return false;
    }
    con->next_packet_queue= new_queue;
    con->next_packet_queue_size= con->next_packet_queue_size * 2;
  }
  con->next_packet_queue[con->next_packet_queue_used]= packet_type;
  con->next_packet_queue_used++;
  return true;
}

ascore_packet_type_t ascore_packet_queue_pop(ascon_st *con)
{
  if (con->next_packet_queue_used == 0)
  {
    return ASCORE_PACKET_TYPE_NONE;
  }
  con->next_packet_queue_used--;
  asdebug("Pop packet type: %d, count: %zu", con->next_packet_queue[con->next_packet_queue_used], con->next_packet_queue_used);
  return con->next_packet_queue[con->next_packet_queue_used];
}

ascore_packet_type_t ascore_packet_queue_peek(ascon_st *con)
{
  asdebug("Peek packet type");
  if (con->next_packet_queue_used == 0)
  {
    return ASCORE_PACKET_TYPE_NONE;
  }
  return con->next_packet_queue[con->next_packet_queue_used-1];
}
