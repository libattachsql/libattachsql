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

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <inttypes.h>
#ifdef _WIN32

typedef uint16_t in_port_t;

#else
# include <arpa/inet.h>
#endif

/* These four need to go first, in this order */
#include <libattachsql-2.0/visibility.h>
#include <libattachsql-2.0/constants.h>
#include <libattachsql-2.0/structs.h>
#include <libattachsql-2.0/error.h>

#include <libattachsql-2.0/connect.h>
#include <libattachsql-2.0/pool.h>
#include <libattachsql-2.0/query.h>
#include <libattachsql-2.0/statement.h>
#include <libattachsql-2.0/utility.h>
