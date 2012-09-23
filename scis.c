/***************************************************************************
 scis.c Copyright (C) 2002--2006 Christoph Reichenbach


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

#include <stdarg.h>
#include "scis.h"

int errors = 0;

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