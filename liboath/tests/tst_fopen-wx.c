/*
 * tst_fopen-wx.c - helper code to trigger fopen(wx) bug CVE-2024-47191
 * Copyright (C) 2009-2025 Simon Josefsson
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see
 * <https://www.gnu.org/licenses/>.
 *
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "oath.h"

#include <stdio.h>
#include <stdlib.h>

int
main (int argc, char *argv[])
{
  const char *CREDS = argv[1];
  const char *USERNAME = argv[2];
  const char *OTP = argv[3];
  const char *PIN = argv[4];
  oath_rc rc;
  time_t last_otp;

  if (argc != 5)
    {
      printf ("Usage: %s USERSFILE USERNAME OTP PIN\n", argv[0]);
      printf ("Example:\n");
      printf ("rm -f cve.oath cve.oath.new cve.sshd-config cve.oath.lock\n");
      printf ("printf 'HOTP/E/8\\tsilver\\t4711\\t31323334353637383930"
	      "31323334353637383930313233343536373839303132\\n' > cve.oath\n");
      printf ("echo my-magic-cookie > cve.sshd-config\n");
      printf ("ln -s cve.sshd-config cve.oath.new\n");
      printf ("%s cve.oath silver 670691 4711\n", argv[0]);
      return EXIT_FAILURE;
    }

  printf ("Liboath fopen(wx) bug test for oath.h %s liboath.so %s\n",
	  OATH_VERSION, oath_check_version (NULL));

  rc = oath_init ();
  if (rc != OATH_OK)
    {
      fprintf (stderr, "FAIL: oath_init (%d): %s\n", rc,
	       oath_strerror_name (rc));
      return EXIT_FAILURE;
    }

  rc = oath_authenticate_usersfile (CREDS, USERNAME, OTP, 0, PIN, &last_otp);
  if (!(rc == OATH_FILE_CREATE_ERROR || rc == OATH_FILE_LOCK_ERROR))
    {
      if (rc == OATH_OK)
	fprintf (stderr, "FAIL: Liboath VULNERABLE to fopen(wx) bug.\n");
      else
	fprintf (stderr,
		 "FAIL: Broken setup? re-run printf/echo/ln setup. (%d/%s)\n",
		 rc, oath_strerror_name (rc));
      return EXIT_FAILURE;
    }

  printf ("PASS: Your liboath is NOT VULNERABLE to fopen(wx) bug.\n");

  rc = oath_done ();
  if (rc != OATH_OK)
    {
      fprintf (stderr, "FAIL: oath_done (%d): %s\n", rc,
	       oath_strerror_name (rc));
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
