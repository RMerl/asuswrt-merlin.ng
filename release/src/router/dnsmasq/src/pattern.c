/* dnsmasq is Copyright (c) 2000-2021 Simon Kelley

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 dated June, 1991, or
   (at your option) version 3 dated 29 June, 2007.
 
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
     
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "dnsmasq.h"

#ifdef HAVE_CONNTRACK

#define LOG(...) \
  do { \
    my_syslog(LOG_DEBUG, __VA_ARGS__); \
  } while (0)

#define ASSERT(condition) \
  do { \
    if (!(condition)) \
      my_syslog(LOG_ERR, _("[pattern.c:%d] Assertion failure: %s"), __LINE__, #condition); \
  } while (0)

/**
 * Determines whether a given string value matches against a glob pattern
 * which may contain zero-or-more-character wildcards denoted by '*'.
 *
 * Based on "Glob Matching Can Be Simple And Fast Too" by Russ Cox,
 * See https://research.swtch.com/glob
 *
 * @param      value                A string value.
 * @param      num_value_bytes      The number of bytes of the string value.
 * @param      pattern              A glob pattern.
 * @param      num_pattern_bytes    The number of bytes of the glob pattern.
 *
 * @return 1                        If the provided value matches against the glob pattern.
 * @return 0                        Otherwise.
 */
static int is_string_matching_glob_pattern(
  const char *value,
  size_t num_value_bytes,
  const char *pattern,
  size_t num_pattern_bytes)
{
  ASSERT(value);
  ASSERT(pattern);
  
  size_t value_index = 0;
  size_t next_value_index = 0;
  size_t pattern_index = 0;
  size_t next_pattern_index = 0;
  while (value_index < num_value_bytes || pattern_index < num_pattern_bytes)
    {
      if (pattern_index < num_pattern_bytes)
	{
	  char pattern_character = pattern[pattern_index];
	  if ('a' <= pattern_character && pattern_character <= 'z')
	    pattern_character -= 'a' - 'A';
	  if (pattern_character == '*')
	    {
	      /* zero-or-more-character wildcard */
	      /* Try to match at value_index, otherwise restart at value_index + 1 next. */
	      next_pattern_index = pattern_index;
	      pattern_index++;
	      if (value_index < num_value_bytes)
		next_value_index = value_index + 1;
	      else
		next_value_index = 0;
	      continue;
	    }
	  else
	    {
	      /* ordinary character */
	      if (value_index < num_value_bytes)
	        {
		  char value_character = value[value_index];
		  if ('a' <= value_character && value_character <= 'z')
		    value_character -= 'a' - 'A';
		  if (value_character == pattern_character)
		    {
		      pattern_index++;
		      value_index++;
		      continue;
		    }
		}
	    }
	}
      if (next_value_index)
	{
	  pattern_index = next_pattern_index;
	  value_index = next_value_index;
	  continue;
	}
      return 0;
    }
  return 1;
}

/**
 * Determines whether a given string value represents a valid DNS name.
 *
 * - DNS names must adhere to RFC 1123: 1 to 253 characters in length, consisting of a sequence of labels
 *   delimited by dots ("."). Each label must be 1 to 63 characters in length, contain only
 *   ASCII letters ("a"-"Z"), digits ("0"-"9"), or hyphens ("-") and must not start or end with a hyphen.
 *
 * - A valid name must be fully qualified, i.e., consist of at least two labels.
 *   The final label must not be fully numeric, and must not be the "local" pseudo-TLD.
 *
 * - Examples:
 *   Valid: "example.com"
 *   Invalid: "ipcamera", "ipcamera.local", "8.8.8.8"
 *
 * @param      value                A string value.
 *
 * @return 1                        If the provided string value is a valid DNS name.
 * @return 0                        Otherwise.
 */
int is_valid_dns_name(const char *value)
{
  ASSERT(value);
  
  size_t num_bytes = 0;
  size_t num_labels = 0;
  const char *label = NULL;
  int is_label_numeric = 1;
  for (const char *c = value;; c++)
    {
      if (*c &&
	  *c != '-' && *c != '.' &&
	  (*c < '0' || *c > '9') &&
	  (*c < 'A' || *c > 'Z') &&
	  (*c < 'a' || *c > 'z'))
	{
	  LOG(_("Invalid DNS name: Invalid character %c."), *c);
	  return 0;
	}
      if (*c)
	num_bytes++;
      if (!label)
	{
	  if (!*c || *c == '.')
	    {
	      LOG(_("Invalid DNS name: Empty label."));
	      return 0;
	    }
	  if (*c == '-')
	    {
	      LOG(_("Invalid DNS name: Label starts with hyphen."));
	      return 0;
	    }
	  label = c;
	}
      if (*c && *c != '.')
	{
	  if (*c < '0' || *c > '9')
	    is_label_numeric = 0;
	}
      else
	{
	  if (c[-1] == '-')
	    {
	      LOG(_("Invalid DNS name: Label ends with hyphen."));
	      return 0;
	    }
	  size_t num_label_bytes = (size_t) (c - label);
	  if (num_label_bytes > 63)
	    {
	      LOG(_("Invalid DNS name: Label is too long (%zu)."), num_label_bytes);
	      return 0;
	    }
	  num_labels++;
	  if (!*c)
	    {
	      if (num_labels < 2)
		{
		  LOG(_("Invalid DNS name: Not enough labels (%zu)."), num_labels);
		  return 0;
		}
	      if (is_label_numeric)
		{
		  LOG(_("Invalid DNS name: Final label is fully numeric."));
		  return 0;
		}
	      if (num_label_bytes == 5 &&
		  (label[0] == 'l' || label[0] == 'L') &&
		  (label[1] == 'o' || label[1] == 'O') &&
		  (label[2] == 'c' || label[2] == 'C') &&
		  (label[3] == 'a' || label[3] == 'A') &&
		  (label[4] == 'l' || label[4] == 'L'))
		{
		  LOG(_("Invalid DNS name: \"local\" pseudo-TLD."));
		  return 0;
		}
	      if (num_bytes < 1 || num_bytes > 253)
		{
		  LOG(_("DNS name has invalid length (%zu)."), num_bytes);
		  return 0;
		}
	      return 1;
	    }
	  label = NULL;
	  is_label_numeric = 1;
	}
    }
}

/**
 * Determines whether a given string value represents a valid DNS name pattern.
 *
 * - DNS names must adhere to RFC 1123: 1 to 253 characters in length, consisting of a sequence of labels
 *   delimited by dots ("."). Each label must be 1 to 63 characters in length, contain only
 *   ASCII letters ("a"-"Z"), digits ("0"-"9"), or hyphens ("-") and must not start or end with a hyphen.
 *
 * - Patterns follow the syntax of DNS names, but additionally allow the wildcard character "*" to be used up to
 *   twice per label to match 0 or more characters within that label. Note that the wildcard never matches a dot
 *   (e.g., "*.example.com" matches "api.example.com" but not "api.us.example.com").
 *
 * - A valid name or pattern must be fully qualified, i.e., consist of at least two labels.
 *   The final label must not be fully numeric, and must not be the "local" pseudo-TLD.
 *   A pattern must end with at least two literal (non-wildcard) labels.
 *
 * - Examples:
 *   Valid: "example.com", "*.example.com", "video*.example.com", "api*.*.example.com", "*-prod-*.example.com"
 *   Invalid: "ipcamera", "ipcamera.local", "*", "*.com", "8.8.8.8"
 *
 * @param      value                A string value.
 *
 * @return 1                        If the provided string value is a valid DNS name pattern.
 * @return 0                        Otherwise.
 */
int is_valid_dns_name_pattern(const char *value)
{
  ASSERT(value);
  
  size_t num_bytes = 0;
  size_t num_labels = 0;
  const char *label = NULL;
  int is_label_numeric = 1;
  size_t num_wildcards = 0;
  int previous_label_has_wildcard = 1;
  for (const char *c = value;; c++)
    {
      if (*c &&
	  *c != '*' && /* Wildcard. */
	  *c != '-' && *c != '.' &&
	  (*c < '0' || *c > '9') &&
	  (*c < 'A' || *c > 'Z') &&
	  (*c < 'a' || *c > 'z'))
	{
	  LOG(_("Invalid DNS name pattern: Invalid character %c."), *c);
	  return 0;
	}
      if (*c && *c != '*')
	num_bytes++;
      if (!label)
	{
	  if (!*c || *c == '.')
	    {
	      LOG(_("Invalid DNS name pattern: Empty label."));
	      return 0;
	    }
	  if (*c == '-')
	    {
	      LOG(_("Invalid DNS name pattern: Label starts with hyphen."));
	      return 0;
	    }
	  label = c;
	}
      if (*c && *c != '.')
	{
	  if (*c < '0' || *c > '9')
	    is_label_numeric = 0;
	  if (*c == '*')
	    {
	      if (num_wildcards >= 2)
		{
		  LOG(_("Invalid DNS name pattern: Wildcard character used more than twice per label."));
		  return 0;
		}
	      num_wildcards++;
	    }
	}
      else
	{
	  if (c[-1] == '-')
	    {
	      LOG(_("Invalid DNS name pattern: Label ends with hyphen."));
	      return 0;
	    }
	  size_t num_label_bytes = (size_t) (c - label) - num_wildcards;
	  if (num_label_bytes > 63)
	    {
	      LOG(_("Invalid DNS name pattern: Label is too long (%zu)."), num_label_bytes);
	      return 0;
	    }
	  num_labels++;
	  if (!*c)
	    {
	      if (num_labels < 2)
		{
		  LOG(_("Invalid DNS name pattern: Not enough labels (%zu)."), num_labels);
		  return 0;
		}
	      if (num_wildcards != 0 || previous_label_has_wildcard)
		{
		  LOG(_("Invalid DNS name pattern: Wildcard within final two labels."));
		  return 0;
		}
	      if (is_label_numeric)
		{
		  LOG(_("Invalid DNS name pattern: Final label is fully numeric."));
		  return 0;
		}
	      if (num_label_bytes == 5 &&
		  (label[0] == 'l' || label[0] == 'L') &&
		  (label[1] == 'o' || label[1] == 'O') &&
		  (label[2] == 'c' || label[2] == 'C') &&
		  (label[3] == 'a' || label[3] == 'A') &&
		  (label[4] == 'l' || label[4] == 'L'))
		{
		  LOG(_("Invalid DNS name pattern: \"local\" pseudo-TLD."));
		  return 0;
		}
	      if (num_bytes < 1 || num_bytes > 253)
		{
		  LOG(_("DNS name pattern has invalid length after removing wildcards (%zu)."), num_bytes);
		  return 0;
		}
	      return 1;
	    }
	    label = NULL;
	    is_label_numeric = 1;
	    previous_label_has_wildcard = num_wildcards != 0;
	    num_wildcards = 0;
	  }
    }
}

/**
 * Determines whether a given DNS name matches against a DNS name pattern.
 *
 * @param      name                 A valid DNS name.
 * @param      pattern              A valid DNS name pattern.
 *
 * @return 1                        If the provided DNS name matches against the DNS name pattern.
 * @return 0                        Otherwise.
 */
int is_dns_name_matching_pattern(const char *name, const char *pattern)
{
  ASSERT(name);
  ASSERT(is_valid_dns_name(name));
  ASSERT(pattern);
  ASSERT(is_valid_dns_name_pattern(pattern));
  
  const char *n = name;
  const char *p = pattern;
  
  do {
    const char *name_label = n;
    while (*n && *n != '.')
      n++;
    const char *pattern_label = p;
    while (*p && *p != '.')
      p++;
    if (!is_string_matching_glob_pattern(
        name_label, (size_t) (n - name_label),
        pattern_label, (size_t) (p - pattern_label)))
      break;
    if (*n)
      n++;
    if (*p)
      p++;
  } while (*n && *p);
  
  return !*n && !*p;
}

#endif
