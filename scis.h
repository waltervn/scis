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

/* SCI assembler header file */


#ifndef SCIS_H
#define SCIS_H
#include <stdio.h>
#include <stdlib.h>

#ifdef DEBUG
#define DEBUG_OUTPUT
#define DEBUG_LEXING
#endif

/* Callbacks for the lexer */

/* Debug information */
extern int line_nr;
extern char *file_name;

/* Options */
typedef struct {
	char *script_filename;
	char *heap_filename;
	int wide_exports;
	int wide_calls;
	int absolute_lofs;
	int dump_results;
	int big_endian;
	int resource_fork;
} options_t;

/* Generator */
typedef struct {
	int
	(*init) (const options_t *options);
	/* Initializes the generator
	** Returns: 1 on success, 0 otherwise
	*/

	void
	(*deinit) ();
	/* Deinitializes the generator
	*/

	void
	(*handle_section) (char *section);
	/* Handles a section specification
	** Parameters: (char *) section: The specification to handle
	*/

	void
	(*define_label) (char *ident);
	/* Defines a new local label
	** Parameters: (char *) ident: Name of the label
	*/

	void
	(*handle_label) (char *label);
	/* Handles a label dereferenciation
	** Parameters: (char *) label: Name of the label
	*/

	void
	(*handle_identifier) (char *ident);
	/* Handles an identifier
	** Parameters: (char *) ident: The symbolic identifier to handle
	** Right now this may be any of the following:
	**  - vm op
	*/

	void
	(*handle_numeric) (int num, int word);
	/* Handles a numeric value
	** Parameters: (int) num: The number to handle
	**             (int) word: Whether to interpret this as a 16 bit value
	*/

	void
	(*end_file) ();
	/* Handles the end of the assembly file
	*/

	void
	(*end_line) ();
	/* Handles the end of an assembly line
	*/

	void
	(*handle_string) (char *string);
	/* Handles an embedded string
	** Parameters: (char *) string: The string to handle
	*/

	void
	(*handle_comma) ();
	/* Handles the occurence of a comma
	*/

	void
	(*handle_said_fragment) (char c);
	/* Handles the occurence of a single character associated with a Said() fragment
	** Parameters: (char c): One of '#', '&', '(', ')', '[', ']', '<', '>', ',', and '!'; the latter
	**			 represents a terminator.
	*/
} generator_t;

void
report_error(int critical, char *fmt, ...);
/* Reports an error situation
** Parameters: (int) critical: non-zero iff assembly should be aborted
**             (char *) fmt ...: Text to display
*/

void
use_absolute_lofs();
/* Generate absolute lofs instructions
*/

void
use_wide_calls();
/* Generate calls with word-size frame size
*/

/*==========---------------=========*/
/* Internal assembler functionality */
/*==========---------------=========*/

void
init_scis();
/* Initializes everything */

void
finish_scis(char *dest_file);

#define ARGSIZE_BYTE 0
#define ARGSIZE_VARIABLE 1
#define ARGSIZE_WORD 2
#define ARGSIZE_RELATIVE 16 /* ORred on top of the others */

int
decode_vm_op(char *name, int *opcode, int *arg_sizes, int *arg_count);
/* Returns nonzero on failure */

extern generator_t *gen;

void report_error_internally(char * _file_name, int _line_nr, int critical, char *fmt, ...);
void report_error(int critical, char *fmt, ...);
int errors_found();
void set_filename(const char *f);

#endif
