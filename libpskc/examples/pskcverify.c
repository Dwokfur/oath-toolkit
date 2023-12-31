#include <stdio.h>
#include <pskc/pskc.h>

/*
 * $ cc -o pskcverify pskcverify.c $(pkg-config --cflags --libs libpskc)
 * $ ./pskcverify signed.xml pskc-root-crt.pem
 * OK
 * $
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
  int rc, valid_sig;

  if (argc != 3)
    {
      printf ("Usage: %s <PSKCFILE> <X509CERT>\n", argv[0]);
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

  rc = pskc_verify_x509crt (container, argv[2], &valid_sig);
  PSKC_CHECK_RC;
  puts (valid_sig ? "OK" : "FAIL");

  pskc_done (container);
  pskc_global_done ();
}
