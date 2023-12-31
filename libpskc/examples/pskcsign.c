#include <stdio.h>
#include <pskc/pskc.h>

/*
 * $ cc -o pskcsign pskcsign.c $(pkg-config --cflags --libs libpskc)
 * $ ./pskcsign pskc-hotp.xml pskc-ee-key.pem pskc-ee-crt.pem > signed.xml
 */

#define PSKC_CHECK_RC					   \
  if (rc != PSKC_OK) {					   \
    printf ("%s (%d): %s\n", pskc_strerror_name (rc),	   \
	    rc, pskc_strerror (rc));			   \
    return 1;						   \
  }

int
main (int argc, const char *argv[])
{
  char buffer[4096];
  FILE *fh;
  size_t len;
  pskc_t *container;
  char *out;
  int rc;

  if (argc != 4)
    {
      printf ("Usage: %s <PSKCFILE> <X509KEY> <X509CERT>\n", argv[0]);
      return 1;
    }
  fh = fopen (argv[1], "r");
  if (!fh)
    {
      perror ("fopen");
      return 1;
    }
  len = fread (buffer, 1, sizeof (buffer), fh);
  fclose (fh);

  rc = pskc_global_init ();
  PSKC_CHECK_RC;

  rc = pskc_init (&container);
  PSKC_CHECK_RC;
  rc = pskc_parse_from_memory (container, len, buffer);
  PSKC_CHECK_RC;

  rc = pskc_sign_x509 (container, argv[2], argv[3]);
  PSKC_CHECK_RC;

  rc = pskc_output (container, PSKC_OUTPUT_XML, &out, &len);
  PSKC_CHECK_RC;
  fwrite (out, 1, len, stdout);
  pskc_free (out);

  pskc_done (container);
  pskc_global_done ();

  return 0;
}
