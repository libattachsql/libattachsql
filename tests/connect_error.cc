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
#include <libattachsql2/attachsql.h>

int main(int argc, char *argv[])
{
  (void) argc;
  (void) argv;
  attachsql_connect_t *con;
  attachsql_error_t *error= NULL;
  attachsql_return_t aret= ATTACHSQL_RETURN_NONE;
  const char *data= "SHOW PROCESSLIST";

  con= attachsql_connect_create("localhost", 3306, "bad_user", "test", "", NULL);
  attachsql_query(con, strlen(data), data, 0, NULL, &error);
  while (aret != ATTACHSQL_RETURN_ERROR)
  {
    aret= attachsql_connect_poll(con, &error);
  }
  SKIP_IF_((attachsql_error_code(error) == 2002), "Error not NULL");
  ASSERT_EQ_(1045, attachsql_error_code(error), "Error code is wrong");
  ASSERT_STREQL_("28000", attachsql_error_sqlstate(error), 5, "SQLSTATE is wrong");
  ASSERT_STREQ_("Access denied for user 'bad_user'@'localhost' (using password: YES)", attachsql_error_message(error), "Message is wrong");
  attachsql_error_free(error);
  attachsql_query_close(con);
  attachsql_connect_destroy(con);
}
