/* scis - SCI assembler
 *
 * scis is the legal property of its developers, whose names are listed
 * in the AUTHORS file.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include "scis.h"
#include "resource.h"
#include "lexer.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_LIBMCPP
	#include <mcpp_lib.h>
#elif defined(RTCPP) && defined(HAVE_FORK) && defined(HAVE_PIPE) \
      && defined(HAVE_DUP2) && defined(HAVE__EXIT)
	#define EXEC_CPP
#endif

extern generator_t generator_sci0;
extern generator_t generator_sci11;

static int errors = 0;
static int filedes[2];
static char **include_paths = NULL;
static int nr_include_paths = 0;
static char **defines = NULL;
static int nr_defines = 0;
generator_t *gen = NULL;

void
print_disclaimer()
{
	printf("scis - SCI assembler, version " VERSION "\n"
	       "\nCopyright (C) 2002-2012 by the authors listed in the AUTHORS file\n"
	       "This program is Free Software, licensed under the terms of the GNU\n"
	       "General Public License (GPL) v3 or any later version (at your choice).\n"
	       "It is provided WITHOUT WARRANTY of any kind, either expressed or\n"
	       "implied. For further licensing details, please refer to the file\n"
	       "COPYING, which should have come with your version of the program,\n"
	       "or contact the Free Software Foundation (http://www.fsf.org) in\n"
	       "order to request a copy.\n");
}

void
print_usage()
{
	printf("Usage: scis [OPTIONS] [FILE]\n"
	       "Translates SCI assembly code to bytecode\n\n"
	       "Options:\n"
	       "\t-h:\tUsage help\n"
	       "\t-v:\tPrint version\n"
	       "\t-I <d>:\tAdd directory <d> to header search path list\n"
	       "\t-D <m>:\tDefine macro <m>\n"
	       "\t-d:\tDump binary result to stdout\n"
	       "\t-g <n>:\tSCI generation to target\n"
	       "\t\t1: late SCI0 and early SCI1 (default)\n"
	       "\t\t2: late SCI1\n"
	       "\t\t3: SCI1.1\n"
	       "\t\t4: SCI2\n"
	       "\t-o <f>:\tWrite script to <f> (defaults to 'sci.scr')\n"
	       "\t-H <f>:\tWrite heap to <f> (defaults to 'sci.hep')\n"
	       );
}

void
report_error_internally(char * _file_name, int _line_nr, int critical, char *fmt, ...)
{
	va_list args;

	fprintf(stderr, "Error in %s, L%d : ", _file_name, _line_nr);
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);

	++errors;

	if (critical) {
		fprintf(stderr, "Critical error. Aborting...\n");
		exit(1);
	}
}


void
report_error(int critical, char *fmt, ...)
{
	va_list args;

	fprintf(stderr, "Error in %s, L%d : ", file_name, line_nr);
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);

	++errors;

	if (critical) {
		fprintf(stderr, "Critical error. Aborting...\n");
		exit(1);
	}
}

int
errors_found()
{
	return errors;
}

void
set_filename(const char *f)
{
	if (file_name)
		free(file_name);

	file_name = malloc(strlen(f) + 1);
	strcpy(file_name, f);
}

char **
build_exec_args(const char *cmd, int *size)
{
	/* Add two for filename and NULL */
	int extra_entries = 2 + nr_include_paths + nr_defines;
	char **cpp_args = malloc(sizeof(char *) * extra_entries);
	char *arg;
	int i = 0, j;
	char *cppcmd = malloc(strlen(cmd) + 1);
	strcpy(cppcmd, cmd);

	for (arg = strtok(cppcmd, " "); arg; arg = strtok(NULL, " ")) {
		cpp_args = realloc(cpp_args, sizeof(char *) * (i + 1 + extra_entries));
		cpp_args[i] = malloc(strlen(arg) + 1);
		strcpy(cpp_args[i++], arg);
	}
	for (j = 0; j < nr_include_paths; j++) {
		cpp_args[i] = malloc(strlen(include_paths[j]) + 3);
		strcpy(cpp_args[i], "-I");
		strcat(cpp_args[i++], include_paths[j]);
	}
	for (j = 0; j < nr_defines; j++) {
		cpp_args[i] = malloc(strlen(defines[j]) + 3);
		strcpy(cpp_args[i], "-D");
		strcat(cpp_args[i++], defines[j]);
	}
	cpp_args[i] = malloc(strlen(file_name) + 1);
	strcpy(cpp_args[i++], file_name);
	cpp_args[i] = NULL;
	if (size)
		*size = i;
	free(cppcmd);
	return cpp_args;
}

char **
free_exec_args(char **args)
{
	int i = 0;
	char **arg;
	for (arg = args; *arg; ++arg)
		free(*arg);
	free(args);
}

#ifdef HAVE_LIBMCPP

void
setup_cpp()
{
	char *buf;
	int size;
	char **args = build_exec_args("mcpp -I-", &size);
	mcpp_use_mem_buffers(1);
	mcpp_lib_main(size, args);
	buf = mcpp_get_mem_buffer(ERR);
	if (buf)
		fprintf(stderr, "%s", buf);
	buf = mcpp_get_mem_buffer(OUT);
	free_exec_args(args);
	yy_scan_string(buf);
}

#elif defined(EXEC_CPP)

void
setup_cpp()
{
	pid_t pid;
	pipe(filedes);

	pid = fork();

	if (pid < 0) {
		fprintf(stderr, "Failed to fork()\n");
		exit(1);
	}

	if (!pid) {
		char **args = build_exec_args(RTCPP, NULL);

		close(filedes[0]);
		dup2(filedes[1], STDOUT_FILENO);
		close(filedes[1]);
	
		execvp(args[0], args);
		perror(args[0]);
		_exit(1);
	}

	/* Parent */
	close(filedes[1]);
	dup2(filedes[0], STDIN_FILENO);
	close(filedes[0]);
}

#else

void
setup_cpp()
{
	if (strcmp(file_name, "-")) {
		/* not stdin */
		yyin = fopen(file_name, "r");
		if (!yyin) {
			perror(file_name);
			exit(1);
		}
	}
}

#endif

int
main (int argc, char **argv)
{
	int c;
	options_t options;

	options.script_filename = "sci.scr";
	options.heap_filename = "sci.hep";
	options.wide_exports = 0;
	options.wide_calls = 0;
	options.absolute_lofs = 0;
	options.dump_results = 0;
	gen = &generator_sci0;

	while ((c = getopt(argc, argv, "dhvg:H:o:I:D:")) > -1) {

		switch (c) {

		case 'd':
			options.dump_results = 1;
			break;

		case 'h':
			print_usage();
			exit(0);

		case 'v':
			print_disclaimer();
			exit(0);

		case 'g':
			switch (atoi(optarg)) {
			case 1:
				options.wide_exports = 0;
				options.absolute_lofs = 0;
				gen = &generator_sci0;
				break;
			case 2:
				options.wide_exports = 1;
				options.absolute_lofs = 1;
				gen = &generator_sci0;
				break;
			case 4:
				options.wide_calls = 1;
				/* Fall-through */
			case 3:
				gen = &generator_sci11;
				break;
			default:
				fprintf(stderr, "Invalid SCI generation target specified\n");
				exit(1);
			}
			break;

		case 'o':
			options.script_filename = optarg;
			break;

		case 'H':
			options.heap_filename = optarg;
			break;

		case 'I':
			include_paths = realloc(include_paths, sizeof(char *) * (nr_include_paths + 1));
			include_paths[nr_include_paths++] = optarg;
			break;

		case 'D':
			defines = realloc(defines, sizeof(char *) * (nr_defines + 1));
			defines[nr_defines++] = optarg;
			break;

		default:
			fprintf(stderr, "Unrecognized option '%c': Internal error!\n", c);
			exit(1);
		}
	}

	set_filename("-");

	if (optind < argc) {
		if (optind + 1 < argc) {
			printf("Only one file at a time may be assembled\n");
			print_usage();
			exit(1);
		}
		set_filename(argv[optind]);
		
	} else {
		print_usage();
		exit(1);
	}

	setup_cpp();
	gen->init(&options);
	yylex();
	gen->deinit();
	if (yyin != stdin)
		fclose(yyin);
	free(file_name);
	if (include_paths)
		free(include_paths);
	if (defines)
		free(defines);
	yylex_destroy();

	return 0;
}
