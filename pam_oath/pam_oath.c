/*
 * pam_oath.c - a PAM module for OATH one-time passwords
 * Copyright (C) 2009-2026 Simon Josefsson
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

#include <config.h>

#include "oath.h"
#include "pam_modutil.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <libgen.h>
#include <ctype.h>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <syslog.h>

/* Libtool defines PIC for shared objects */
#ifndef PIC
# define PAM_STATIC
#endif

/* These #defines must be present according to PAM documentation. */
#define PAM_SM_AUTH
#define PAM_SM_ACCOUNT
#define PAM_SM_SESSION
#define PAM_SM_PASSWORD

#ifdef HAVE_SECURITY_PAM_APPL_H
# include <security/pam_appl.h>
#endif
#ifdef HAVE_SECURITY_PAM_MODULES_H
# include <security/pam_modules.h>
#endif
#ifdef HAVE_SECURITY_PAM_EXT_H
# include <security/pam_ext.h>
#endif

#define D(fmt, ...) do {						\
    printf ("[%s:%s(%d)] " fmt "\n",					\
            __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__);		\
  } while (0)

static void
_pam_oath_syslog (pam_handle_t *pamh, const char *file,
		  const char *func, int line, const char *fmt, ...)
{
  char buf[4096];
  va_list ap;

  va_start (ap, fmt);
  vsnprintf (buf, sizeof (buf), fmt, ap);
  va_end (ap);
  pam_syslog (pamh, LOG_DEBUG, "[%s:%s(%d)] %s", file, func, line, buf);
}

#define DBG(pamh, fmt, ...) do {					\
    if (cfg.debug)							\
      D(fmt, ##__VA_ARGS__);						\
    if (cfg.debug_syslog)						\
      _pam_oath_syslog (pamh, __FILE__, __FUNCTION__, __LINE__,		\
			fmt, ##__VA_ARGS__);				\
  } while (0)

#ifndef PAM_EXTERN
# ifdef PAM_STATIC
#  define PAM_EXTERN static
# else
#  define PAM_EXTERN extern
# endif
#endif

#define MIN_OTP_LEN 6
#define MAX_OTP_LEN 8

struct cfg
{
  int debug;
  int debug_syslog;
  int alwaysok;
  int try_first_pass;
  int use_first_pass;
  int no_usersfile_okay;
  int ignore_userfile_password;
  char *usersfile;
  unsigned digits;
  unsigned window;
  int allow_replayed_otp_within;
};

static void
parse_cfg (int flags, int argc, const char **argv, struct cfg *cfg)
{
  int i;

  cfg->debug = 0;
  cfg->debug_syslog = 0;
  cfg->alwaysok = 0;
  cfg->try_first_pass = 0;
  cfg->use_first_pass = 0;
  cfg->no_usersfile_okay = 0;
  cfg->ignore_userfile_password = 0;
  cfg->usersfile = NULL;
  cfg->digits = -1;
  cfg->window = 5;
  cfg->allow_replayed_otp_within = 0;

  for (i = 0; i < argc; i++)
    {
      if (strcmp (argv[i], "debug") == 0)
	cfg->debug = 1;
      if (strcmp (argv[i], "debug_syslog") == 0)
	cfg->debug_syslog = 1;
      if (strcmp (argv[i], "alwaysok") == 0)
	cfg->alwaysok = 1;
      if (strcmp (argv[i], "try_first_pass") == 0)
	cfg->try_first_pass = 1;
      if (strcmp (argv[i], "use_first_pass") == 0)
	cfg->use_first_pass = 1;
      if (strcmp (argv[i], "no_usersfile_okay") == 0)
	cfg->no_usersfile_okay = 1;
      if (strcmp (argv[i], "ignore_userfile_password") == 0)
	cfg->ignore_userfile_password = 1;
      if (strncmp (argv[i], "usersfile=", 10) == 0)
	cfg->usersfile = (char *) argv[i] + 10;
      if (strncmp (argv[i], "digits=", 7) == 0)
	cfg->digits = atoi (argv[i] + 7);
      if (strncmp (argv[i], "window=", 7) == 0)
	cfg->window = atoi (argv[i] + 7);
      if (strncmp (argv[i], "allow_replayed_otp_within=", 26) == 0)
	{
	  char *endptr;
	  long v = strtol (argv[i] + 26, &endptr, 10);
	  if (*endptr == '\0' && v > 0)
	    cfg->allow_replayed_otp_within = (int) v;
	  else
	    cfg->allow_replayed_otp_within = 0;
	}
    }

  if (cfg->digits != 6 && cfg->digits != 7 && cfg->digits != 8)
    {
      if (cfg->digits != -1)
	D ("only 6, 7, and 8 OTP lengths are supported: invalid value %d",
	    cfg->digits);
      cfg->digits = 0;
    }

  if (cfg->debug)
    {
      D ("called.");
      D ("flags %d argc %d", flags, argc);
      for (i = 0; i < argc; i++)
	D ("argv[%d]=%s", i, argv[i]);
      D ("debug=%d", cfg->debug);
      D ("debug_syslog=%d", cfg->debug_syslog);
      D ("alwaysok=%d", cfg->alwaysok);
      D ("try_first_pass=%d", cfg->try_first_pass);
      D ("use_first_pass=%d", cfg->use_first_pass);
      D ("no_usersfile_okay=%d", cfg->no_usersfile_okay);
      D ("ignore_userfile_password=%d", cfg->ignore_userfile_password);
      D ("usersfile=%s", cfg->usersfile ? cfg->usersfile : "(null)");
      D ("digits=%d", cfg->digits);
      D ("window=%d", cfg->window);
      D ("allow_replayed_otp_within=%d", cfg->allow_replayed_otp_within);
    }
}

static int
parse_usersfile_str (pam_handle_t *pamh, const struct cfg *cfg,
		     const char *user, char **usersfile,
		     uid_t *uid, gid_t *gid)
{
  int retval = PAM_SUCCESS;
  size_t name_len = 0;
  size_t home_len = 0;
  size_t len = strlen (cfg->usersfile) + 1;
  char *str = NULL;
  char *u = NULL;
  struct passwd *pw = NULL;

  if (*usersfile)
    {
      return PAM_BUF_ERR;
    }

  if ((str = strstr (cfg->usersfile, "${HOME}")) == NULL
      && strstr (cfg->usersfile, "${USER}") == NULL)
    {
      *usersfile = strdup (cfg->usersfile);
      if (!*usersfile)
	return PAM_BUF_ERR;
      return PAM_SUCCESS;
    }

  pw = pam_modutil_getpwnam (pamh, user);
  if (!pw)
    {
      return PAM_USER_UNKNOWN;
    }

  if (str)
    {
      /* Tell caller to drop privs. */
      *uid = pw->pw_uid;
      *gid = pw->pw_gid;
    }

  /*
     Find occurrences of the placeholder fields to determine
     userfile buffer length
   */
  name_len = strlen (pw->pw_name);
  home_len = strlen (pw->pw_dir);
  str = cfg->usersfile;
  while ((str = strstr (str, "${USER}")))
    {
      len += name_len;
      len -= 7;
      str += 7;
    }
  str = cfg->usersfile;
  while ((str = strstr (str, "${HOME}")))
    {
      len += home_len;
      len -= 7;
      str += 7;
    }

  *usersfile = malloc (len);
  if (!(*usersfile))
    {
      return PAM_BUF_ERR;
    }
  memset (*usersfile, 0, len);

  str = cfg->usersfile;
  u = *usersfile;
  while (*str)
    {
      char *c = strchr (str, '$');
      if (c)
	{
	  /* Copy all preceding characters */
	  const size_t str_len = c - str;
	  memcpy (u, str, str_len);
	  u += str_len;
	  str += str_len;

	  const char *rpl_str = NULL;
	  size_t rpl_len = 0;

	  if (strncmp (str, "${USER}", 7) == 0)
	    {
	      rpl_str = pw->pw_name;
	      rpl_len = name_len;
	      str += 7;
	    }
	  else if (strncmp (str, "${HOME}", 7) == 0)
	    {
	      rpl_str = pw->pw_dir;
	      rpl_len = home_len;
	      str += 7;
	    }
	  else
	    {
	      rpl_str = "$";
	      rpl_len = 1;
	      str += 1;
	    }

	  memcpy (u, rpl_str, rpl_len);
	  u += rpl_len;
	}
      else
	{
	  size_t str_len = strlen (str);
	  memcpy (u, str, str_len);
	  u += str_len;
	  str += str_len;
	}
    }
done:
  if (retval != PAM_SUCCESS)
    {
      free (*usersfile);
      *usersfile = NULL;
    }
  return retval;
}

PAM_EXTERN int
pam_sm_authenticate (pam_handle_t *pamh,
		     int flags, int argc, const char **argv)
{
  int retval, rc;
  const char *user = NULL;
  const char *password = NULL;
  char *usersfile = NULL;
  char otp[MAX_OTP_LEN + 1];
  int password_len = 0;
  struct cfg cfg;
  char *query_prompt = NULL;
  char *onlypasswd;
  uid_t pamuid = 0, old_euid = geteuid ();
  gid_t pamgid = 0, old_egid = getegid ();

  if (pamh == NULL)
    return PAM_SYSTEM_ERR;

  /* this has to be first in this function to avoid that cfg contain
     uninitialized variables. */
  parse_cfg (flags, argc, argv, &cfg);

  onlypasswd = strdup ("");	/* empty passwords never match */
  if (!onlypasswd)
    {
      retval = PAM_BUF_ERR;
      goto done;
    }

  retval = pam_get_user (pamh, &user, NULL);
  if (retval != PAM_SUCCESS)
    {
      DBG (pamh, "get user returned error: %s", pam_strerror (pamh, retval));
      goto done;
    }
  DBG (pamh, "get user returned: %s", user);

  retval = parse_usersfile_str (pamh, &cfg, user, &usersfile,
				&pamuid, &pamgid);
  if (retval != PAM_SUCCESS)
    {
      DBG (pamh, "parse usersfile string returned error: %s",
	    pam_strerror (pamh, retval));
      goto done;
    }
  DBG (pamh, "usersfile is %s (id %d/%d)", usersfile, pamuid, pamgid);

  if (pamuid || pamgid)
    {
      if (pamgid && setegid (pamgid) != 0)
	{
	  DBG (pamh, "setegid failed: %d", errno);
	  retval = PAM_SERVICE_ERR;
	  goto done;
	}

      if (pamuid && seteuid (pamuid) != 0)
	{
	  DBG (pamh, "seteuid failed: %d", errno);
	  retval = PAM_SERVICE_ERR;
	  goto done;
	}

      DBG (pamh, "Successfully dropped effective id to %d/%d", pamuid, pamgid);
    }

  if (cfg.no_usersfile_okay)
    {
      char *ucopy, *base;
      ucopy = strdup (usersfile);
      base = dirname (ucopy);

      /* make sure that the base dir exists so we are sure that, for example,
         the user home directory is mounted. */
      rc = access (base, F_OK);
      free (ucopy);
      if (rc != 0)
	{
	  DBG (pamh, "Basepath of file cannot be accessed '%s'", usersfile);
	  retval = PAM_AUTH_ERR;
	  goto done;
	}

      if (access (usersfile, F_OK) != 0)
	{
	  DBG (pamh, "no_usersfile_okay set and no userfile was found, authenticating...");
	  retval = PAM_SUCCESS;
	  goto done;
	}
    }


  // quick check to skip unconfigured users before prompting for password
  {
    time_t last_otp;
    otp[0] = '\0';
    rc = oath_authenticate_usersfile (usersfile,
				      user,
				      otp, cfg.window,
				      cfg.ignore_userfile_password
				      ? "" : onlypasswd,
				      &last_otp);

    DBG (pamh, "pre-auth user check rc %d (%s: %s) last otp %s", rc,
	  oath_strerror_name (rc) ? oath_strerror_name (rc) : "UNKNOWN",
	  oath_strerror (rc), ctime (&last_otp));
    if (rc == OATH_UNKNOWN_USER)
      {
	retval = PAM_USER_UNKNOWN;
	goto done;
      }
  }

  {
    const char *query_template = "One-time password (OATH) for `%s': ";
    size_t len = strlen (query_template) + strlen (user) + 1;
    size_t wrote;

    query_prompt = malloc (len);
    if (!query_prompt)
      {
	retval = PAM_BUF_ERR;
	goto done;
      }

    wrote = snprintf (query_prompt, len, query_template, user);
    if (wrote < 0 || wrote >= len)
      {
	retval = PAM_BUF_ERR;
	goto done;
      }
  }

  /* pam_get_authtok() first returns any cached PAM_AUTHTOK (satisfying
     try_first_pass/use_first_pass semantics), and if not already set it
     retrieves the token via the PAM conversation function.  This correctly
     handles daemon clients such as dovecot passdb pam that supply the
     password through the conversation mechanism rather than pre-populating
     PAM_AUTHTOK.  */
  retval = pam_get_authtok (pamh, PAM_AUTHTOK, &password, query_prompt);
  if (retval != PAM_SUCCESS)
    {
      retval = pam_get_authtok (pamh, PAM_AUTHTOK, &password, query_prompt);
      if (retval != PAM_SUCCESS)
	  {
	    DBG (pamh, "pam_get_authtok returned error: %s",
  		     pam_strerror (pamh, retval));
  	  goto done;
  	}
      DBG (pamh, "pam_get_authtok returned: %s", password);
    }
  DBG (pamh, "pam_get_authtok returned: %s", password);

  if (cfg.use_first_pass && password == NULL)
    {
      DBG (pamh, "use_first_pass set and no password, giving up");
      retval = PAM_AUTH_ERR;
      goto done;
    }

  rc = oath_init ();
  if (rc != OATH_OK)
    {
      DBG (pamh, "oath_init() failed (%d)", rc);
      retval = PAM_AUTHINFO_UNAVAIL;
      goto done;
    }

  if (password)
    password_len = strlen (password);
  else
    {
      DBG (pamh, "Could not read password");
      retval = PAM_AUTH_ERR;
      goto done;
    }

  if (password_len < MIN_OTP_LEN)
    {
      DBG (pamh, "OTP too short: %s", password);
      retval = PAM_AUTH_ERR;
      goto done;
    }
  else if (cfg.digits != 0 && password_len < cfg.digits)
    {
      DBG (pamh, "OTP shorter than digits=%d: %s", cfg.digits, password);
      retval = PAM_AUTH_ERR;
      goto done;
    }
  else if (cfg.digits == 0 && password_len > MAX_OTP_LEN)
    {
      DBG (pamh, "OTP too long (and no digits=): %s", password);
      retval = PAM_AUTH_ERR;
      goto done;
    }
  else if (cfg.digits != 0 && password_len > cfg.digits)
    {
      free (onlypasswd);
      onlypasswd = strdup (password);
      if (!onlypasswd)
	{
	  retval = PAM_BUF_ERR;
	  goto done;
	}

      /* user entered their system password followed by generated OTP? */

      onlypasswd[password_len - cfg.digits] = '\0';

      DBG (pamh, "Password: %s ", onlypasswd);

      memcpy (otp, password + password_len - cfg.digits, cfg.digits);
      otp[cfg.digits] = '\0';

      retval = pam_set_item (pamh, PAM_AUTHTOK, onlypasswd);
      if (retval != PAM_SUCCESS)
	{
	  DBG (pamh, "set_item returned error: %s", pam_strerror (pamh, retval));
	  goto done;
	}
    }
  else
    {
      strcpy (otp, password);
      password = NULL;
    }

  DBG (pamh, "OTP: %s", otp);

  {
    time_t last_otp;

    rc = oath_authenticate_usersfile (usersfile,
				      user,
				      otp, cfg.window,
				      cfg.ignore_userfile_password
				      ? "" : onlypasswd,
				      &last_otp);
    DBG (pamh, "authenticate rc %d (%s: %s) last otp %s", rc,
	  oath_strerror_name (rc) ? oath_strerror_name (rc) : "UNKNOWN",
	  oath_strerror (rc), ctime (&last_otp));

    if (rc == OATH_REPLAYED_OTP && cfg.allow_replayed_otp_within > 0)
      {
	time_t now = time (NULL);
	time_t delta = now - last_otp;

	if (delta < 0)
	  {
	    DBG (pamh, "Replayed OTP window check skipped: last_otp is in the "
		  "future (delta=%ld seconds, possible clock skew)", (long) delta);
	  }
	else if (delta <= (time_t) cfg.allow_replayed_otp_within)
	  {
	    DBG (pamh, "Replayed OTP accepted within window "
		  "(allow_replayed_otp_within=%d, delta=%ld seconds)",
		  cfg.allow_replayed_otp_within, (long) delta);
	    rc = OATH_OK;
	  }
      }
  }

  if (rc != OATH_OK)
    {
      DBG (pamh, "One-time password not authorized to login as user '%s'", user);
      retval = PAM_AUTH_ERR;
      goto done;
    }

  retval = PAM_SUCCESS;

done:
  if (pamuid || pamgid)
    {
      if (pamgid && setegid (old_egid) != 0)
	{
	  DBG (pamh, "Restoring setegid failed: %d", errno);
	  retval = PAM_SERVICE_ERR;
	}
      if (pamuid && seteuid (old_euid) != 0)
	{
	  DBG (pamh, "Restoring seteuid failed: %d", errno);
	  retval = PAM_SERVICE_ERR;
	}

      DBG (pamh, "Successfully restored effective id to %d/%d",
	    old_euid, old_egid);
    }
  oath_done ();
  free (usersfile);
  free (query_prompt);
  free (onlypasswd);
  if (cfg.alwaysok && retval != PAM_SUCCESS)
    {
      DBG (pamh, "alwaysok needed (otherwise return with %d)", retval);
      retval = PAM_SUCCESS;
    }
  DBG (pamh, "done. [%s]", pam_strerror (pamh, retval));

  return retval;
}

PAM_EXTERN int
pam_sm_setcred (pam_handle_t *pamh, int flags, int argc, const char **argv)
{
  return PAM_SUCCESS;
}

#ifdef PAM_STATIC

struct pam_module _pam_oath_modstruct = {
  "pam_oath",
  pam_sm_authenticate,
  pam_sm_setcred,
  NULL,
  NULL,
  NULL,
  NULL
};

#endif
