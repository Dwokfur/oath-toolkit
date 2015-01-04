/*
 * usersfile.c - implementation of UsersFile based HOTP validation
 * Copyright (C) 2009-2016 Simon Josefsson
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
#undef GNULIB_POSIXCHECK	/* too many complaints for now */

#include "oath.h"

#include <stdio.h>		/* For snprintf, getline. */
#include <stdlib.h>		/* For free. */
#include <unistd.h>		/* For ssize_t. */
#include <fcntl.h>		/* For fcntl. */
#include <errno.h>		/* For errno. */
#include <sys/stat.h>		/* For S_IRUSR, S_IWUSR. */

#define IF_ERROR_GOTO(test, rc_value, goto_label) if (test) { rc = rc_value; goto goto_label; }

static const char *whitespace = " \t\r\n";
#define TIME_FORMAT_STRING "%Y-%m-%dT%H:%M:%SL"
#define TIME_BUFFER_SIZE 30
#define BUFFER_SIZE 1024

/*
 * parse_usersfile :
 * @str: string with token type to parse
 * @digits: output variable holding the length of OTP (6, 7 or 8)
 * @totpstepsize: output variable holding the interval (in second) of TOTP
 *
 * internal fonction
 *
 * Returns: 0 on success, -1 on error
 **/
static int
parse_type (const char *str, unsigned int *digits, unsigned int *totpstepsize)
{
  *totpstepsize = 0;
  if (strcmp (str, "HOTP/E/6") == 0
      || strcmp (str, "HOTP/E") == 0 || strcmp (str, "HOTP") == 0)
    *digits = 6;
  else if (strcmp (str, "HOTP/E/7") == 0)
    *digits = 7;
  else if (strcmp (str, "HOTP/E/8") == 0)
    *digits = 8;
  else if (strncmp (str, "HOTP/T30", 8) == 0)
    {
      *totpstepsize = 30;
      if (strcmp (str, "HOTP/T30") == 0 || strcmp (str, "HOTP/T30/6") == 0)
	*digits = 6;
      else if (strcmp (str, "HOTP/T30/7") == 0)
	*digits = 7;
      else if (strcmp (str, "HOTP/T30/8") == 0)
	*digits = 8;
      else
	return -1;
    }
  else if (strncmp (str, "HOTP/T60", 8) == 0)
    {
      *totpstepsize = 60;
      if (strcmp (str, "HOTP/T60") == 0 || strcmp (str, "HOTP/T60/6") == 0)
	*digits = 6;
      else if (strcmp (str, "HOTP/T60/7") == 0)
	*digits = 7;
      else if (strcmp (str, "HOTP/T60/8") == 0)
	*digits = 8;
      else
	return -1;
    }
  else
    return -1;

  return 0;
}

/* compute the timestamp */
static int
compute_timestamp (char timestamp[])
{
  struct tm now;
  time_t t;
  size_t l;

  if (time (&t) == (time_t) - 1)
    return OATH_TIME_ERROR;

  if (localtime_r (&t, &now) == NULL)
    return OATH_TIME_ERROR;

  l = strftime (timestamp, TIME_BUFFER_SIZE, TIME_FORMAT_STRING, &now);
  if (l != 20)
    return OATH_TIME_ERROR;

  return OATH_OK;
}


/*
 * parse_usersfile :
 * @username: string with name of user
 * @otp: string with one-time password to authenticate
 * @window: how many past/future OTPs to search
 * @passwd: string with password, or NULL if password checking is disabled
 * @usersfile_fd: input file descriptor for usersfile
 * @parameter_last_otp_timestamp: output variable holding last successful authentication timestamp
 * @old_log_start: output variable holding the file position of the beginning of the "log" of the validation line
 * @old_log_end: output variable holding the file position of the end of the "log" of the validation line
 *
 * internal fonction
 *
 * By "log" (old_log_start/end) I mean the end of line that should contain a moving factor/search position, an otp and a timestamp.
 * The "log" contain all the fields from the 5th ; it can also be empty if the line contain only 4 fields.
 *
 * Returns: %OATH_OK on success, negative value on error (see oath.h)
 **/

static int
parse_usersfile (const char *username,
		 const char *otp,
		 const size_t window,
		 const char *passwd,
		 FILE * usersfile_fd,
		 time_t * parameter_last_otp_timestamp,
		 off_t * old_log_start,
		 off_t * old_log_end, char *new_log_buffer)
{
  int rc = OATH_OK;

  // to record the fact we read a line matching the username
  int matching_user_line = 0;
  // to record the fact we read a line matching the username and the passwd
  int matching_user_and_passwd_line = 0;

  char *line_buffer = NULL;
  size_t line_buffer_size = 0;
  ssize_t line_size = 0;

  // for each line of userfile
  while ((line_size =
	  getline (&line_buffer, &line_buffer_size, usersfile_fd)) != -1)
    {
      // used internally by strtok_r in order to maintain context
      // between successive calls that parse the same string.
      char *saveptr;

      // read the first token/element of the line
      char *p = strtok_r (line_buffer, whitespace, &saveptr);

      if (p == NULL || *p == '#')
	// blank line or comment
	continue;

      // read token type
      unsigned int digits, totpstepsize;
      if (parse_type (p, &digits, &totpstepsize) != 0)
	{
	  // wrong formated token type
	  rc = OATH_WRONG_TOKEN_TYPE;
	  break;
	}

      // read username
      p = strtok_r (NULL, whitespace, &saveptr);
      if (p == NULL)
	{
	  // there is no username in the current line of usersfile
	  rc = OATH_NO_USERNAME;
	  break;
	}

      if (strcmp (p, username) != 0)
	// username doesn't match
	continue;

      // we record the fact we read a line matching the username
      matching_user_line = 1;

      // read password
      p = strtok_r (NULL, whitespace, &saveptr);

      if (passwd)
	{
	  // password checking is enabled

	  if (p == NULL)
	    {
	      // there is no password in the usersfile
	      rc = OATH_NO_PASSWORD;
	      break;
	    }

	  if (strcmp (p, "-") == 0)
	    {
	      if (*passwd != '\0')
		// the user supply a non empty password but
		// there is no password ("-") in the current usersfile line
		rc = OATH_BAD_PASSWORD;
	    }
	  else if (strcmp (p, "+") == 0)
	    {
	      /* Externally verified. */
	    }
	  else if (strcmp (p, passwd) != 0)
	    {
	      // the password supply by the user doesn't match the one in the
	      // current usersfile line
	      rc = OATH_BAD_PASSWORD;
	    }

	  if (rc == OATH_BAD_PASSWORD)
	    {
	      // the user supply a non empty password but there is no password ("-") in the current usersfile line
	      // or
	      // the password supply by the user doesn't match the one in the current usersfile line

	      // we continue because a user can have multiple password
	      rc = OATH_OK;
	      continue;
	    }

	  // we record the fact we read a line matching the username and the passwd
	  matching_user_and_passwd_line = 1;
	}

      // read secret key
      p = strtok_r (NULL, whitespace, &saveptr);
      if (p == NULL)
	{
	  // there is no secret key in the current line of usersfile
	  rc = OATH_NO_SECRET;
	  break;
	}

      // convert the secret key in binary format
      char secret[32];
      size_t secret_length = sizeof (secret);
      rc = oath_hex2bin (p, secret, &secret_length);
      if (rc != OATH_OK)
	// the secret key can't be converted the binary format
	break;

      // record the size of the "log" of the current line
      long old_log_size = line_size - strlen (p) - (p - line_buffer);

      // read the (optional) "log"
      unsigned long long last_moving_factor = 0;
      int last_totp_position = 0;
      char *last_otp = NULL;
      time_t last_otp_timestamp = (time_t) - 1;

      p = strtok_r (NULL, whitespace, &saveptr);
      if (p)
	{
	  // the current line contain a non empty "log"

	  if (totpstepsize == 0)
	    {
	      // token type algorithm is HOTP

	      // convert the string to unsigned long long
	      char *endptr;
	      last_moving_factor = strtoull (p, &endptr, 10);
	      if (endptr && *endptr != '\0')
		{
		  // the moving factor can't be converted to unsigned long long
		  rc = OATH_INVALID_COUNTER;
		  break;
		}
	    }
	  else
	    {
	      // token type algorithm is TOTP

	      // convert the string to int
	      char *endptr;
	      last_totp_position = strtol (p, &endptr, 10);
	      if (endptr && *endptr != '\0')
		{
		  // the search position can't be converted to long
		  rc = OATH_INVALID_COUNTER;
		  break;
		}
	    }

	  // read last OTP
	  last_otp = strtok_r (NULL, whitespace, &saveptr);
	  if (!last_otp)
	    {
	      rc = OATH_INVALID_LAST_OTP;
	      break;
	    }

	  // Read last OTP timestamp
	  p = strtok_r (NULL, whitespace, &saveptr);
	  if (!p)
	    {
	      rc = OATH_INVALID_TIMESTAMP;
	      break;
	    }

	  // we convert it to tm
	  struct tm tm;
	  char *ts;
	  ts = strptime (p, TIME_FORMAT_STRING, &tm);
	  if (ts == NULL || *ts != '\0')
	    {
	      // the timestamp is bad formatted
	      rc = OATH_INVALID_TIMESTAMP;
	      break;
	    }
	  tm.tm_isdst = -1;

	  // convert the tm into time_t and
	  // store it in last_otp_timestamp
	  last_otp_timestamp = mktime (&tm);
	  if (last_otp_timestamp == (time_t) - 1)
	    {
	      // the tm is wrong
	      rc = OATH_INVALID_TIMESTAMP;
	      break;
	    }

	  if (parameter_last_otp_timestamp)
	    // if the function caller want to record
	    // the last OTP timestamp
	    *parameter_last_otp_timestamp = last_otp_timestamp;
	}

      if (last_otp && strcmp (last_otp, otp) == 0)
	{
	  // the OTP supply by the user is the
	  // same that is stored in usersfile
	  rc = OATH_REPLAYED_OTP;
	  break;
	}

      int new_totp_position = 0;
      if (totpstepsize == 0)
	{
	  // token type algorithm is HOTP

	  // check if the suppied OTP is valid
	  rc = oath_hotp_validate (secret, secret_length,
				   last_moving_factor, window, otp);
	}
      else if (last_otp)
	{
	  // token type algorithm is TOTP
	  // and the current line contain a "log"

	  // check if the suppied OTP is valid
	  rc = oath_totp_validate2 (secret, secret_length,
				    time (NULL), totpstepsize, 0, window,
				    &new_totp_position, otp);

	  if (rc >= OATH_OK)
	    {
	      // the supplied OTP is valide
	      // but it may have been already be played
	      // since it's valide for a periode of time in which a new
	      // OTP could have been played :
	      //
	      //                OTP1 → OTP2 → OTP1(replay)
	      // OTP1 validity   |--------------------------|
	      //
	      // in that case OTP1(replay) should be rejected

	      unsigned long long new_totp_time_step_number,
		last_totp_time_step_number;
	      last_totp_time_step_number =
		(last_otp_timestamp / totpstepsize) + last_totp_position;
	      new_totp_time_step_number =
		(time (NULL) / totpstepsize) + new_totp_position;

	      if (last_totp_time_step_number >= new_totp_time_step_number)
		{
		  // last recorded otp is newer than the one supplied by the user
		  rc = OATH_REPLAYED_OTP;
		  break;
		}
	    }
	}
      else
	{
	  // token type algorithm is TOTP
	  // but the current line doesn't contain a "log"
	  // it's the first OTP the user supply

	  // check if the suppied OTP is valid
	  rc = oath_totp_validate (secret, secret_length,
				   time (NULL), totpstepsize, 0, window, otp);
	}

      if (rc == OATH_INVALID_OTP)
	{
	  // the supplied otp doesn't match the current line

	  // we continue because we can have multiple secret key for the
	  // same pair <user,passwd>
	  rc = OATH_OK;
	  continue;
	}

      if (rc < 0)
	// there were an error other than OATH_INVALID_OTP
	break;

      // OTP is valide

      // compute the new moving factor
      unsigned long long new_moving_factor = last_moving_factor + rc;

      // compute the current timestamp
      char timestamp_buffer[TIME_BUFFER_SIZE];
      rc = compute_timestamp (timestamp_buffer);

      if (rc != OATH_OK)
	// timestamp can't be generated
	break;

      // record the new "log" in new_log_buffer
      if (totpstepsize == 0)
	{
	  // token type algorithm is HOTP

	  rc = snprintf (new_log_buffer, BUFFER_SIZE, "\t%llu\t%s\t%s\n",
			 new_moving_factor, otp, timestamp_buffer);
	}
      else
	{
	  // token type algorithm is TOTP

	  rc = snprintf (new_log_buffer, BUFFER_SIZE, "\t%d\t%s\t%s\n",
			 new_totp_position, otp, timestamp_buffer);
	}

      if (rc < 0)
	{
	  rc = OATH_PRINTF_ERROR;
	  break;
	}

      // save the file position of the start and end
      // of the old "log"
      *old_log_end = ftello (usersfile_fd);
      *old_log_start = *old_log_end - old_log_size;

      if (*old_log_end == -1)
	{
	  rc = OATH_FILE_TELL_ERROR;
	  break;
	}

      free (line_buffer);
      return OATH_OK;
    }

  free (line_buffer);

  if (rc != OATH_OK)
    return rc;

  // the usersfile was parse entirely without error
  // but no matching OTP have been found

  if (matching_user_and_passwd_line)
    // there were line(s) matchine username and password
    return OATH_INVALID_OTP;
  else if (matching_user_line)
    // there were line(s) matchine username (but not password)
    return OATH_BAD_PASSWORD;

  // there were no line matchine the username
  return OATH_UNKNOWN_USER;
}

/**
 * oath_authenticate_usersfile:
 * @usersfile: string with user credential filename, in UsersFile format
 * @username: string with name of user
 * @otp: string with one-time password to authenticate
 * @window: how many past/future OTPs to search
 * @passwd: string with password, or NULL to disable password checking
 * @last_otp_timestamp: output variable holding last successful authentication timestamp, or NULL to not record this value
 *
 * Authenticate user named @username with the one-time password @otp
 * and (optional) password @passwd.  Credentials are read (and
 * updated) from a text file named @usersfile.
 *
 * Note that for TOTP the usersfile will only record the last OTP and
 * use that to make sure more recent OTPs have not been seen yet when
 * validating a new OTP.  That logics relies on using the same search
 * window for the same user.
 *
 * Returns: On successful validation, %OATH_OK is returned.  If the
 *   supplied @otp is the same as the last successfully authenticated
 *   one-time password, %OATH_REPLAYED_OTP is returned and the
 *   timestamp of the last authentication is returned in @last_otp_timestamp.
 *   If the one-time password is not found in the indicated search
 *   window, %OATH_INVALID_OTP is returned.  Otherwise, an error code
 *   is returned.
 **/
int
oath_authenticate_usersfile (const char *usersfile,
			     const char *username,
			     const char *otp,
			     size_t window,
			     const char *passwd, time_t * last_otp_timestamp)
{
  int rc = OATH_OK;
  // temporary variable to check various syscall output for error
  int syscall_output = 0;

  // open usersfile
  FILE *usersfile_fd;
  usersfile_fd = fopen (usersfile, "r+");
  IF_ERROR_GOTO (usersfile_fd == NULL, OATH_FILE_OPEN_ERROR, end);

  {				// this context was created to avoid goto crosses variable declaration

    // put a read lock on usersfile
    struct flock lock;
    memset (&lock, 0, sizeof (lock));
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_type = F_RDLCK;
    syscall_output = fcntl (fileno (usersfile_fd), F_SETLKW, &lock);
    IF_ERROR_GOTO (syscall_output == -1, OATH_FILE_LOCK_ERROR, close_end);

    // parse usersfile and check if the supplied otp is valide
    off_t old_log_start, old_log_end;
    char new_log_buffer[BUFFER_SIZE];
    rc = parse_usersfile (username, otp, window, passwd, usersfile_fd,
			  last_otp_timestamp, &old_log_start, &old_log_end,
			  new_log_buffer);
    if (rc != OATH_OK)
      // supplied otp is not valide or
      // the were error during usersfile the parsing
      goto close_end;

    // the otp is valide
    // now we have to write the new "log" in usersfile

    // put a write lock on usersfile
    lock.l_type = F_WRLCK;
    syscall_output = fcntl (fileno (usersfile_fd), F_SETLKW, &lock);
    IF_ERROR_GOTO (syscall_output == -1, OATH_FILE_LOCK_ERROR, close_end);

    size_t new_log_len = strlen (new_log_buffer);
    if ((size_t) (old_log_end - old_log_start) == new_log_len)
      {
	// the new and old "log" have the same size
	// we write directly the new log in the usersfile inplace
	// of the old "log"

	// go the the start position of the old "log"
	syscall_output = fseeko (usersfile_fd, old_log_start, SEEK_SET);
	IF_ERROR_GOTO (syscall_output == -1, OATH_FILE_SEEK_ERROR, close_end);

	// write the new "log"
	fwrite (new_log_buffer, sizeof (char), strlen (new_log_buffer),
		usersfile_fd);
	IF_ERROR_GOTO (ferror (usersfile_fd), OATH_FILE_WRITE_ERROR,
		       close_end);
      }
    else
      {
	// the new and old "log" doesn't have the same size
	// we will load in memory the end of the usersfile from the end of the old "log" to the end of the file
	// write the new "log" from the starting point of the old "log"
	// and append the end of the usersfile previously loaded in memory

	// compute the amont of memory we will need
	syscall_output = fseeko (usersfile_fd, 0, SEEK_END);
	IF_ERROR_GOTO (syscall_output == -1, OATH_FILE_SEEK_ERROR, close_end);

	off_t usersfile_buffer_size = ftello (usersfile_fd) - old_log_end;
	IF_ERROR_GOTO (usersfile_buffer_size < 0, OATH_FILE_TELL_ERROR,
		       close_end);

	// allocate the dynamic memory
	char *usersfile_buffer =
	  malloc (usersfile_buffer_size * sizeof (char));
	IF_ERROR_GOTO (usersfile_buffer == NULL, OATH_MALLOC_ERROR,
		       close_end);

	// load usersfile from the end of the old "log" to the end of the file
	syscall_output = fseeko (usersfile_fd, old_log_end, SEEK_SET);
	IF_ERROR_GOTO (syscall_output == -1, OATH_FILE_SEEK_ERROR,
		       free_close_end);

	syscall_output =
	  fread (usersfile_buffer, sizeof (char), usersfile_buffer_size,
		 usersfile_fd);
	IF_ERROR_GOTO (syscall_output != usersfile_buffer_size,
		       OATH_FILE_READ_ERROR, free_close_end);

	// write the new log in the usersfile from the start of the old "log"
	syscall_output = fseeko (usersfile_fd, old_log_start, SEEK_SET);
	IF_ERROR_GOTO (syscall_output == -1, OATH_FILE_SEEK_ERROR,
		       free_close_end);

	fwrite (new_log_buffer, sizeof (char), strlen (new_log_buffer),
		usersfile_fd);
	IF_ERROR_GOTO (ferror (usersfile_fd), OATH_FILE_WRITE_ERROR,
		       free_close_end);

	// write the end of the usersfile previously loaded in memory
	fwrite (usersfile_buffer, sizeof (char), usersfile_buffer_size,
		usersfile_fd);
	IF_ERROR_GOTO (ferror (usersfile_fd), OATH_FILE_WRITE_ERROR,
		       free_close_end);

	// in case the old log was longer than the new one we truncate the end of the file
	syscall_output =
	  ftruncate (fileno (usersfile_fd), ftello (usersfile_fd));
	IF_ERROR_GOTO (syscall_output == -1, OATH_FILE_TRUNCATE_ERROR,
		       free_close_end);

      free_close_end:
	free (usersfile_buffer);
      }
  }

close_end:
  // close the usersfile and remove the lock
  fclose (usersfile_fd);
end:
  return rc;
}
