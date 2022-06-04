# This package provides logging functions to print to the console and debug files.
# Logging functions are provided for various "levels" (error, debug, verbose).
#
# $ Copyright Broadcom Corporation $
#
# <<Broadcom-WL-IPTag/Proprietary:>>


# Namespace.
package Log;

use strict;
use warnings;
use File::Basename

# Module version number.
our $VERSION = '1.00';

# For exporting functions.
use base 'Exporter';

# Exported functions.
our @EXPORT =
qw (
	log_debug
	log_debug_raw
	log_error
	log_print
	log_verbose
	log_enable_debug
	log_enable_verbose
	log_set_debug_fd
	log_get_debug_fd
);

# Debug file handle.
my $g_debug_fd = "nil";

# Default logging levels.
my $g_log_debug_enabled   = 1;
my $g_log_verbose_enabled = 0;


# Utility functions to print to the console and debug files.
#
# Params: func_name:   IN  Function name of caller to print.
#         log_console: IN  Log to console if 1.
#         log_file:    IN  Log to file if 1.
#         log_func:    IN  Log function name if 1.
#         log_script:  IN  Log script name if 1.
#         prefix:      IN  Prefix string to add to log.
#         print_args:  IN  Var args to print.
#
# Returns: Nothing.
sub log_common {
	my ($func_name)   = shift;
	my ($log_console) = shift;
	my ($log_file)    = shift;
	my ($log_func)    = shift;
	my ($log_script)  = shift;
	my ($prefix)      = shift;
	my @print_args    = @_;

	# Log to console.
	if ($log_console) {
		# Prefix the log with the name of this script.
		if ($log_script) {
			printf("%s:%s", basename($0), $prefix);
		}
		printf(@print_args);
	}

	# Log to debug file.
	if (($log_file) && ($g_debug_fd ne "nil")) {
		# Log caller function name.
		if ($log_func) {
			printf $g_debug_fd ("%s:%s ", $func_name, $prefix);
		}
		printf $g_debug_fd (@print_args);
	}
}

# Params:  print_args: IN  Var args to print.
sub log_verbose {
	if ($g_log_verbose_enabled) {
		my @print_args = @_;
		log_common(func_name_caller(), 0, 1, 1, 0, "", @print_args);
	}
}


# Params:  print_args: IN  Var args to print.
sub log_debug {
	my @print_args = @_;

	if ($g_log_debug_enabled) {
		# Print only to log file.
		log_common(func_name_caller(), 0, 1, 1, 0, "", @print_args);
	}
}

# Params:  print_args: IN  Var args to print.
sub log_debug_raw {
	my @print_args = @_;

	if ($g_log_debug_enabled) {
		# Print only to log file. No function name prefix.
		log_common(func_name_caller(), 0, 1, 0, 0, "", @print_args);
	}
}

# Params:  print_args: IN  Var args to print.
sub log_error {
	my @print_args = @_;

	# Print to both console and log file.
	log_common(func_name_caller(), 1, 1, 1, 1, "ERROR: ", @print_args);
}

# Params:  print_args: IN  Var args to print.
sub log_print {
	my @print_args = @_;

	# Print to both console and log file.
	log_common(func_name_caller(), 1, 1, 1, 0, "", @print_args);
}


# Utility functions to enable/disable logging levels (debug, verbose, etc).
#
# Params: enable: IN  Enable(1)/disable(0).
#
# Returns: Nothing.
sub log_enable_debug {
	my ($enable) = @_;
	$g_log_debug_enabled = $enable;
}
sub log_enable_verbose {
	my ($enable) = @_;
	$g_log_verbose_enabled = $enable;
}

# Set debug log file descriptor.
#
# Params: fd: IN  Debug log file descriptor.
#
# Returns: Nothing.
sub log_set_debug_fd {
	my ($fd) = @_;
	$g_debug_fd = $fd;
}

# Get debug log file descriptor.
#
# Params: None
#
# Returns: Debug log file descriptor.
sub log_get_debug_fd {
       return ($g_debug_fd);
}


# Utility function that returns the name of the caller function.
#
# Params:  None.
#
# Returns: Name of the caller function.
sub func_name_caller {
	my $caller = (caller(2))[3];
	if (!defined($caller)) {
		$caller = "main";
	}
	return $caller;
}


# Module must end with true value.
1;
