/*
 * tst_invalidread.c - regression tests for libpskc memory bug
 * Copyright (C) 2014-2020 David Woodhouse
 * Copyright (C) 2020-2024 Simon Josefsson
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
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#include <config.h>

#include <pskc/pskc.h>

#include <stdio.h>
#include <string.h>

static const char *testkey = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
  "<KeyContainer xmlns=\"urn:ietf:params:xml:ns:keyprov:pskc\" "
  "Version=\"1.0\" Id=\"exampleID1\">"
  "<KeyPackage><Key Id=\"12345678\" "
  "Algorithm=\"urn:ietf:params:xml:ns:keyprov:pskc:hotp\">"
  "<Data>"
  "<Secret>"
  "<PlainValue>YoTOTvAMztz6JhLQGaoUqhS1f/C0+Jr0B7xIfA0=</PlainValue>"
  "</Secret>"
  "<Counter>"
  "<PlainValue>35</PlainValue>"
  "</Counter>" "</Data>" "</Key>" "</KeyPackage>" "</KeyContainer>";

int
main (void)
{
  pskc_t *container;
  pskc_key_t *key;
  char *buf;
  size_t s;

  if (pskc_global_init () || pskc_init (&container) ||
      pskc_parse_from_memory (container, strlen (testkey), testkey) ||
      !(key = pskc_get_keypackage (container, 0)))
    {
      fprintf (stderr, "init error\n");
      return 1;
    }
  printf ("Counter: %lu\n", (unsigned long)
	  pskc_get_key_data_counter (key, NULL));

  pskc_set_key_data_counter (key, pskc_get_key_data_counter (key, NULL) + 1);
  pskc_output (container, PSKC_OUTPUT_XML, &buf, &s);
  printf ("output: %.*s\n", (int) s, buf);
  pskc_free (buf);
  pskc_build_xml (container, &buf, &s);
  printf ("New XML: '%.*s'\n", (int) s, buf);
  pskc_free (buf);
  pskc_build_xml (container, &buf, &s);
  printf ("XML again: '%.*s'\n", (int) s, buf);
  pskc_free (buf);
  pskc_done (container);
  pskc_global_done ();

  return 0;
}
