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

#include <yatl/lite.h>
#include "version.h"
#include <libattachsql-1.0/attachsql.h>

int main(int argc, char *argv[])
{
  (void) argc;
  (void) argv;
  attachsql_connect_t *con;
  attachsql_error_st *error;
  const char *data= "SHOW PROCESSLIST";

  con= attachsql_connect_create("localhost", 3306, "bad_user", "test", "", NULL);
  error= attachsql_query(con, strlen(data), data, 0, NULL);
  SKIP_IF_((error->code == 2002), "Error not NULL");
  ASSERT_EQ_(1045, error->code, "Error code is wrong");
  ASSERT_STREQL_("28000", error->sqlstate, 5, "SQLSTATE is wrong");
  ASSERT_STREQ_("Access denied for user 'bad_user'@'localhost' (using password: YES)", error->msg, "Message is wrong");

  attachsql_query_close(con);
}
