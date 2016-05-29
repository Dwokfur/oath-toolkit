/*
 * pam_oath.c - a PAM module for OATH one-time passwords
 * Copyright (C) 2009-2016 Simon Josefsson
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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>
#include <sys/wait.h>

/* Libtool defines PIC for shared objects */
#ifndef PIC
#define PAM_STATIC
#endif

/* These #defines must be present according to PAM documentation. */
#define PAM_SM_AUTH
#define PAM_SM_ACCOUNT
#define PAM_SM_SESSION
#define PAM_SM_PASSWORD

#ifdef HAVE_SECURITY_PAM_APPL_H
#include <security/pam_appl.h>
#endif
#ifdef HAVE_SECURITY_PAM_MODULES_H
#include <security/pam_modules.h>
#endif
#include <security/pam_modutil.h>

#define D(x) do {							\
    printf ("[%s:%s(%d)] ", __FILE__, __FUNCTION__, __LINE__);		\
    printf x;								\
    printf ("\n");							\
  } while (0)
#define DBG(x) if (cfg.debug) { D(x); }

#ifndef PAM_EXTERN
#ifdef PAM_STATIC
#define PAM_EXTERN static
#else
#define PAM_EXTERN extern
#endif
#endif

#define MIN_OTP_LEN 6
#define MAX_OTP_LEN 8

struct cfg
{
  int debug;
  int alwaysok;
  int try_first_pass;
  int use_first_pass;
  char *usersfile;
  unsigned digits;
  unsigned window;
};

static void
parse_cfg (int flags, int argc, const char **argv, struct cfg *cfg)
{
  int i;

  cfg->debug = 0;
  cfg->alwaysok = 0;
  cfg->try_first_pass = 0;
  cfg->use_first_pass = 0;
  cfg->usersfile = NULL;
  cfg->digits = -1;
  cfg->window = 5;

  for (i = 0; i < argc; i++)
    {
      if (strcmp (argv[i], "debug") == 0)
	cfg->debug = 1;
      if (strcmp (argv[i], "alwaysok") == 0)
	cfg->alwaysok = 1;
      if (strcmp (argv[i], "try_first_pass") == 0)
	cfg->try_first_pass = 1;
      if (strcmp (argv[i], "use_first_pass") == 0)
	cfg->use_first_pass = 1;
      if (strncmp (argv[i], "usersfile=", 10) == 0)
	cfg->usersfile = (char *) argv[i] + 10;
      if (strncmp (argv[i], "digits=", 7) == 0)
	cfg->digits = atoi (argv[i] + 7);
      if (strncmp (argv[i], "window=", 7) == 0)
	cfg->window = atoi (argv[i] + 7);
    }

  if (cfg->digits != 6 && cfg->digits != 7 && cfg->digits != 8)
    {
      if (cfg->digits != -1)
	D (("only 6, 7, and 8 OTP lengths are supported: invalid value %d",
	    cfg->digits));
      cfg->digits = 0;
    }

  if (cfg->debug)
    {
      D (("called."));
      D (("flags %d argc %d", flags, argc));
      for (i = 0; i < argc; i++)
	D (("argv[%d]=%s", i, argv[i]));
      D (("debug=%d", cfg->debug));
      D (("alwaysok=%d", cfg->alwaysok));
      D (("try_first_pass=%d", cfg->try_first_pass));
      D (("use_first_pass=%d", cfg->use_first_pass));
      D (("usersfile=%s", cfg->usersfile ? cfg->usersfile : "(null)"));
      D (("digits=%d", cfg->digits));
      D (("window=%d", cfg->window));
    }
}

PAM_EXTERN int
pam_sm_authenticate (pam_handle_t * pamh,
		     int flags, int argc, const char **argv)
{
  int retval, rc, child, fds[2];
  const char *user = NULL;
  const char *password = NULL;
  char otp[MAX_OTP_LEN + 1];
  int password_len = 0;
  struct pam_conv *conv;
  struct pam_message *pmsg[1], msg[1];
  struct pam_response *resp;
  int nargs = 1;
  struct cfg cfg;
  char *query_prompt = NULL;
  char *onlypasswd = NULL;

  parse_cfg (flags, argc, argv, &cfg);

  // create a pipe for the password
  if (pipe(fds) != 0)
    {
      DBG (("could not make pipe"));
      goto done;
    }

  retval = pam_get_user (pamh, &user, NULL);
  if (retval != PAM_SUCCESS)
    {
      DBG (("get user returned error: %s", pam_strerror (pamh, retval)));
      goto done;
    }
  DBG (("get user returned: %s", user));

  if (cfg.try_first_pass || cfg.use_first_pass)
    {
      retval = pam_get_item (pamh, PAM_AUTHTOK, (const void **) &password);
      if (retval != PAM_SUCCESS)
	{
	  DBG (("get password returned error: %s",
		pam_strerror (pamh, retval)));
	  goto done;
	}
      DBG (("get password returned: %s", password));
    }

  if (cfg.use_first_pass && password == NULL)
    {
      DBG (("use_first_pass set and no password, giving up"));
      retval = PAM_AUTH_ERR;
      goto done;
    }

  if (password == NULL)
    {
      retval = pam_get_item (pamh, PAM_CONV, (const void **) &conv);
      if (retval != PAM_SUCCESS)
	{
	  DBG (("get conv returned error: %s", pam_strerror (pamh, retval)));
	  goto done;
	}

      pmsg[0] = &msg[0];
      {
	const char *query_template = "One-time password (OATH) for `%s': ";
	size_t len = strlen (query_template) + strlen (user);
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

	msg[0].msg = query_prompt;
      }
      msg[0].msg_style = PAM_PROMPT_ECHO_OFF;
      resp = NULL;

      retval = conv->conv (nargs, (const struct pam_message **) pmsg,
			   &resp, conv->appdata_ptr);

      free (query_prompt);
      query_prompt = NULL;

      if (retval != PAM_SUCCESS)
	{
	  DBG (("conv returned error: %s", pam_strerror (pamh, retval)));
	  goto done;
	}

      DBG (("conv returned: %s", resp->resp));

      password = resp->resp;
    }

  if (password)
    password_len = strlen (password);
  else
    {
      DBG (("Could not read password"));
      retval = PAM_AUTH_ERR;
      goto done;
    }

  if (password_len < MIN_OTP_LEN)
    {
      DBG (("OTP too short: %s", password));
      retval = PAM_AUTH_ERR;
      goto done;
    }
  else if (cfg.digits != 0 && password_len < cfg.digits)
    {
      DBG (("OTP shorter than digits=%d: %s", cfg.digits, password));
      retval = PAM_AUTH_ERR;
      goto done;
    }
  else if (cfg.digits == 0 && password_len > MAX_OTP_LEN)
    {
      DBG (("OTP too long (and no digits=): %s", password));
      retval = PAM_AUTH_ERR;
      goto done;
    }
  else if (cfg.digits != 0 && password_len > cfg.digits)
    {
      onlypasswd = strdup (password);
      if (!onlypasswd)
        {
          retval = PAM_BUF_ERR;
          goto done;
        }

      /* user entered their system password followed by generated OTP? */

      onlypasswd[password_len - cfg.digits] = '\0';

      DBG (("Password: %s", onlypasswd));

      memcpy (otp, password + password_len - cfg.digits, cfg.digits);
      otp[cfg.digits] = '\0';

      retval = pam_set_item (pamh, PAM_AUTHTOK, onlypasswd);
      if (retval != PAM_SUCCESS)
        {
          DBG (("set_item returned error: %s", pam_strerror (pamh, retval)));
          goto done;
        }
    }
  else
    {
      strcpy (otp, password);
      password = NULL;
      // empty passwords never match
      onlypasswd = strdup ("");
      if (!onlypasswd)
        {
          retval = PAM_BUF_ERR;
          goto done;
        }
    }

  DBG (("OTP: %s", otp ? otp : "(null)"));

  /* fork */
  child = fork();
  if (child == 0)
    {
      static char *envp[] = { NULL };
      char cfg_window[255];
      char * const args[] = {
              "oath_usersfile",
              cfg.usersfile,
              (char*)user,
              otp,
              cfg_window,
              cfg.debug ? "1" : "0",
              NULL };

      if (snprintf(cfg_window, 255, "%u", cfg.window) >= 255)
        {
          retval = PAM_AUTH_ERR;
          DBG(("window representation in over 255 characters"));
          goto done;
        }

      close(fds[1]);

      // reopen stdin as pipe
      if (dup2(fds[0], STDIN_FILENO) != STDIN_FILENO)
        {
          retval = PAM_AUTH_ERR;
          DBG(("dup2 of stdin failed"));
          goto done;
        }

      /* must set the real uid to 0 so the helper will not error
       * out if pam is called from setuid binary (su, sudo...) */
      if (geteuid() == 0 && setuid(0) == -1)
        {
          DBG(("setuid failed"));
          retval = PAM_AUTH_ERR;
          goto done;
        }

      /* exec binary helper */
      DBG(("calling : %s %s %s %s %s %s", OATH_USERSFILE,
                                       args[1], args[2],
                                       args[3], args[4], args[5]));
      execve(OATH_USERSFILE, args, envp);

      /* should not get here: exit with error */
      DBG(("helper binary is not available"));
      retval = PAM_AUTH_ERR;
      goto done;
    }
  else if (child > 0)
    {
      /* send the password to the child */
      int len_left = strlen(onlypasswd) + 1;
      int len_written = 0;
      rc = write(fds[1], onlypasswd + len_written, len_left);
      while (rc != len_left)
        {
          if (rc == -1)
            {
              DBG(("unable to write the passwd to the pipe"));
              retval = PAM_AUTH_ERR;
              goto done;
            }
          len_left -= rc;
          len_written += rc;
          rc = write(fds[1], onlypasswd + len_written, len_left);
        }

      close(fds[0]);
      close(fds[1]);

      /* wait for helper to complete: */
      while ((rc = waitpid(child, &retval, 0)) < 0 && errno == EINTR);

      if (rc<0)
        {
          DBG (("oath_usersfile waitpid returned %d: %m", rc));
          retval = PAM_AUTH_ERR;
          goto done;
        }
      else if (!WIFEXITED(retval))
        {
          DBG (("oath_usersfile abnormal exit: %d", retval));
          retval = PAM_AUTH_ERR;
          goto done;
        }
      else
        {
          rc = WEXITSTATUS(retval);
        }
    }
  else
    {
      DBG(("fork failed"));
      retval = PAM_AUTH_ERR;
      goto done;
    }

  if (rc != OATH_OK)
    {
      DBG (("One-time password not authorized to login as user '%s'", user));
      retval = PAM_AUTH_ERR;
      goto done;
    }

  retval = PAM_SUCCESS;

done:
  free (query_prompt);
  free (onlypasswd);
  if (cfg.alwaysok && retval != PAM_SUCCESS)
    {
      DBG (("alwaysok needed (otherwise return with %d)", retval));
      retval = PAM_SUCCESS;
    }
  DBG (("done. [%s]", pam_strerror (pamh, retval)));

  return retval;
}

PAM_EXTERN int
pam_sm_setcred (pam_handle_t * pamh, int flags, int argc, const char **argv)
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
