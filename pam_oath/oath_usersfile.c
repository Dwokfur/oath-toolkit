/*
 * pam_oath.c - a PAM module for OATH one-time passwords
 * Copyright (C) 2009-2015 Simon Josefsson
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 */

#include "oath.h"
#include <stdio.h>
#include <unistd.h>
#include <security/pam_appl.h>

#define D(x) do {							\
    printf ("[%s:%s(%d)] ", __FILE__, __FUNCTION__, __LINE__);		\
    printf x;								\
    printf ("\n");							\
  } while (0)
#define DBG(x) if (argv[5][0] == '1') { D(x); }

int main (int argc, char** argv)
{
  int rc;
  size_t window;
  char onlypasswd[PAM_MAX_RESP_SIZE];
  unsigned int len_read = 0;

  if (argc != 6)
    return OATH_USERFILE_BIN_ERROR;

  DBG (("usersfile: %s, username: %s, otp: %s, window: %s, debug: %s", argv[1], argv[2], argv[3], argv[4], argv[5]));

  // read the password from stdin
  rc = read(STDIN_FILENO, onlypasswd + len_read, PAM_MAX_RESP_SIZE);
  while (rc != 0)
    {
      if (rc == -1)
        {
          DBG(("unable to read the passwd to the pipe"));
          return OATH_USERFILE_BIN_ERROR;
        }
      len_read += rc;
      rc = read(STDIN_FILENO, onlypasswd + len_read, PAM_MAX_RESP_SIZE);
    }

  DBG (("onlypasswd: %s", onlypasswd));

  rc = oath_init ();
  if (rc != OATH_OK)
    {
      DBG(("oath_init failed"));
      goto done;
    }

  sscanf(argv[4], "%zu", &window);

  rc = oath_authenticate_usersfile (argv[1] /* usersfile */,
                                    argv[2] /* username */,
                                    argv[3] /* otp */,
                                    window,
                                    onlypasswd,
                                    NULL);

done:
  oath_done ();
  return rc;
}
