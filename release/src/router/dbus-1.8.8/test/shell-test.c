#include <config.h>
#include <stdio.h>
#include <stdlib.h>

#include <dbus/dbus-internals.h>
#include <dbus/dbus-list.h>
#include <dbus/dbus-memory.h>
#include <dbus/dbus-shell.h>
#include <dbus/dbus-string.h>
#include <dbus/dbus-sysdeps.h>

static dbus_bool_t
test_command_line (const char *arg1, ...)
{
  int i, original_argc, shell_argc;
  char **shell_argv;
  char **original_argv;
  char *command_line, *tmp;
  DBusString str;
  DBusList *list = NULL, *node;
  va_list var_args;
  DBusError error;

  va_start (var_args, arg1);
  _dbus_list_append (&list, (char *)arg1);
  do
    {
      tmp = va_arg (var_args, char *);
      if (!tmp)
        break;
      _dbus_list_append (&list, tmp);
    } while (tmp);
  va_end (var_args);

  original_argc = _dbus_list_get_length (&list);
  original_argv = dbus_new (char *, original_argc);
  _dbus_string_init (&str);
  for (i = 0, node = _dbus_list_get_first_link (&list); i < original_argc && node;
       i++, node = _dbus_list_get_next_link (&list, node))
    {
      original_argv[i] = node->data;
      if (i > 0)
        _dbus_string_append_byte (&str, ' ');
      _dbus_string_append (&str, original_argv[i]);
    }
  
  _dbus_list_clear (&list);
  command_line = _dbus_string_get_data (&str);
  printf ("\n\nTesting command line '%s'\n", command_line);

  dbus_error_init (&error);
  if (!_dbus_shell_parse_argv (command_line, &shell_argc, &shell_argv, &error))
    {
      fprintf (stderr, "Error parsing command line: %s\n", error.message ? error.message : "");
      return FALSE;
    }
  else
    {
      if (shell_argc != original_argc)
        {
          printf ("Number of arguments returned (%d) don't match original (%d)\n",
                  shell_argc, original_argc);
          return FALSE;
        } 
      printf ("Number of arguments: %d\n", shell_argc);
      for (i = 0; i < shell_argc; i++)
        {
          char *unquoted;
          
          unquoted = _dbus_shell_unquote (original_argv[i]);
          if (strcmp (unquoted ? unquoted : "",
                      shell_argv[i] ? shell_argv[i] : ""))
            {
              printf ("Position %d, returned argument (%s) does not match original (%s)\n",
                      i, shell_argv[i], unquoted);
              dbus_free (unquoted);
              return FALSE;
            }
          dbus_free (unquoted);
          if (shell_argv[i])
            printf ("Argument %d = %s\n", i, shell_argv[i]);
        }
      
      dbus_free_string_array (shell_argv);
    }
  
  _dbus_string_free (&str);
  
  return TRUE;
}

int
main (int argc, char **argv)
{
  if (!test_command_line ("command", "-s", "--force-shutdown", "\"a string\"", "123", NULL)
      || !test_command_line ("command", "-s", NULL)
      || !test_command_line ("/opt/gnome/bin/service-start", NULL)
      || !test_command_line ("grep", "-l", "-r", "-i", "'whatever'", "files*.c", NULL)
      || !test_command_line ("/home/boston/johnp/devel-local/dbus/test/test-segfault", NULL)
      || !test_command_line ("ls", "-l", "-a", "--colors", _dbus_get_tmpdir(), NULL)
      || !test_command_line ("rsync-to-server", NULL)
      || !test_command_line ("test-segfault", "--no-segfault", NULL)
      || !test_command_line ("evolution", "mailto:pepe@cuco.com", NULL)
      || !test_command_line ("run", "\"a \n multiline\"", NULL)
      || test_command_line ("ls", "\"a wrong string'", NULL) /* invalid command line */ )
    return -1;
  
  return 0;
}
