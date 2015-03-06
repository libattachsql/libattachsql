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

#include <libattachsql-2.0/attachsql.h>

/* These two need to go first, in this order */
#include "constants.h"
#include "structs.h"
#include "return.h"
#include "pack_macros.h"
#include "pack.h"
#include "net.h"
#include "connect.h"
#include "command.h"
#include "buffer.h"
#include "statement.h"
