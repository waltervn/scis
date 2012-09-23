/***************************************************************************
 scis.h Copyright (C) 2002 Christoph Reichenbach


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
extern char *output_file;
extern int errors;

/* Options */
typedef struct {
	char *script_filename;
	char *heap_filename;
	int wide_exports;
	int absolute_lofs;
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
	(*finish_op) ();
	/* Handles the termination of an operation specification, if open
	** Does not result in an error if no operation is open
	*/

	void
	(*end_file) ();
	/* Handles the end of the assembly file
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

#endif
