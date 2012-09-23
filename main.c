/***************************************************************************
 main.c Copyright (C) 2002 Christoph Reichenbach


 This program may be modified and copied freely according to the terms of
 the GNU general public license (GPL), as long as the above copyright
 notice and the licensing information contained herein are preserved.

 Please refer to www.gnu.org for licensing details.

 This work is provided AS IS, without warranty of any kind, expressed or
 implied, including but not limited to the warranties of merchantibility,
 noninfringement, and fitness for a specific purpose. The author will not
 be held liable for any damage caused by this work or derivatives of it.

 By using this source code, you agree to the licensing terms as stated
 above.


 Please contact the maintainer for bug reports or inquiries.

 Current Maintainer:

    Christoph Reichenbach (CR) <jameson@linuxgames.com>

***************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "scis.h"
#include "resource.h"

extern int yylex();
extern FILE *yyin;

extern generator_t generator_sci0;

static int quiet = 0;
generator_t *gen = NULL;

void
print_disclaimer()
{
	printf("SCI Assembler, version " VERSION "\n"
	       "\nCopyright 2002--2012 Christoph Reichenbach, Walter van Niftrik\n"
	       "This program is Free Software, licensed under the terms of the GNU\n"
	       "General Public License (GPL) v2 or any later version (at your choice).\n"
	       "It is provided WITHOUT WARRANTY of any kind, either expressed or\n"
	       "implied. For further licensing details, please refer to the file\n"
	       "COPYING, which should have come with your version of the program,\n"
	       "or contact the Free Software Foundation (http://www.fsf.org) in\n"
	       "order to request a copy.\n");
}

void
print_usage()
{
	printf("scis [-q] [-h] [-v] [-d] [-g generation] [-o scrfile] [-H hepfile] <source.s>\n"
	       "Assembles <source.s>.\n"
	       "Other options:\n"
	       "\t-h:\tUsage help\n"
	       "\t-v:\tPrint version\n"
	       "\t-q:\tQuiet mode\n"
	       "\t-d:\tDump binary result to stdout\n"
	       "\t-g <n>:\tSCI generation to target\n"
	       "\t\t1: late SCI0 and early SCI1 (default)\n"
	       "\t\t2: late SCI1\n"
	       "\t-o <f>:\tWrite script to <f> (defaults to 'sci.scr')\n"
	       "\t-H <f>:\tWrite heap to <f> (defaults to 'sci.hep')\n"
	       );
}

int
main (int argc, char **argv)
{
	int c;
	options_t options;

	options.script_filename = "sci.scr";
	options.heap_filename = "sci.hep";
	options.wide_exports = 0;
	options.absolute_lofs = 0;
	gen = &generator_sci0;

	while ((c = getopt(argc, argv, "dqhvg:H:o:")) > -1) {

		switch (c) {

		case 'd':
			res_set_dump_result(1);
			break;

		case 'h':
			print_usage();
			exit(0);

		case 'v':
			printf("scis, version "VERSION"\n");
			print_disclaimer();
			exit(0);

		case 'q':
			quiet = 1;
			break;

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

	if (!quiet)
		print_disclaimer();

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

