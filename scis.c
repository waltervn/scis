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

extern int yylex();
extern FILE *yyin;

extern generator_t generator_sci0;
extern generator_t generator_sci11;

static int errors = 0;
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
	printf("scis [-h] [-v] [-d] [-g generation] [-o scrfile] [-H hepfile] <source.s>\n"
	       "Options:\n"
	       "\t-h:\tUsage help\n"
	       "\t-v:\tPrint version\n"
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

	while ((c = getopt(argc, argv, "dhvg:H:o:")) > -1) {

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

		default:
			fprintf(stderr, "Unrecognized option '%c': Internal error!\n", c);
			exit(1);
		}
	}

	file_name = "-";

	if (optind < argc) {
		if (optind + 1 < argc) {
			printf("Only one file at a time may be assembled\n");
			print_usage();
			exit(1);
		}
		file_name = argv[optind];
		
	} else {
		print_usage();
		exit(1);
	}

	if (strcmp(file_name, "-")) {
		/* not stdin */
		yyin = fopen(file_name, "r");
		if (!yyin) {
			perror(file_name);
			exit(1);
		}
	}

	gen->init(&options);
	yylex();
	gen->deinit();

	return 0;
}

