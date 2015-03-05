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
#include "version.h"
#include "src/asql/common.h"

const char *attachsql_get_library_version(void)
{
  return LIBATTACHSQL_VERSION_STRING;
}

uint8_t attachsql_get_library_version_major(void)
{
  return (uint8_t)((LIBATTACHSQL_VERSION_HEX & 0xff000000) >> 24);
}

uint8_t attachsql_get_library_version_minor(void)
{
  return (uint8_t)((LIBATTACHSQL_VERSION_HEX & 0x00fff000) >> 12);
}

uint8_t attachsql_get_library_version_patch(void)
{
  return (uint8_t)(LIBATTACHSQL_VERSION_HEX & 0x00000fff);
}

void attachsql_library_init(void)
{
#ifdef HAVE_OPENSSL
  SSL_load_error_strings();
  SSL_library_init();
#endif
}
