/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  DDM4
 *
 *  Copyright (C) 2012 Data Differential, http://datadifferential.com/
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *      * Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 *
 *      * Redistributions in binary form must reproduce the above
 *  copyright notice, this list of conditions and the following disclaimer
 *  in the documentation and/or other materials provided with the
 *  distribution.
 *
 *      * The names of its contributors may not be used to endorse or
 *  promote products derived from this software without specific prior
 *  written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "config.h"

#include <unistd.h>
#include <getopt.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
  int help_flag= 0;

  struct option long_options[] =
  {
    /* These options set a flag. */
    {"help", no_argument, &help_flag, 'h'},
    {0, 0, 0, 0}
  };

  while (1)
  {
    /* getopt_long stores the option index here. */
    int option_index= 0;

    int c= getopt_long(argc, argv, "h", long_options, &option_index);

    /* Detect the end of the options. */
    if (c == -1)
    {
      break;
    }

    switch (c)
    {
    case 'h':
      if (long_options[option_index].flag != 0)
      {
        break;
      }

      printf ("option %s", long_options[option_index].name);
      if (optarg)
      {
        printf (" with arg %s", optarg);
      }
      printf ("\n");
      break;

    case '?': // getopt_long already printed an error message.
      break;
    }
  }

  printf("%s %s", CC, CFLAGS);

  return 0;
}

