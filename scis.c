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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <scis.h>
#include <symtab.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

char *output_file = "sci.out";
int wide_exports = 0;

#ifdef DEBUG
#  define DEBUG_LEXING
#  define DEBUG_OUTPUT
#endif

#define MAX_SCRIPT_SIZE 0xffff

#define REF_TYPE_ABSOLUTE_AUTORELOC 1
#define REF_TYPE_ABSOLUTE 0
#define REF_TYPE_RELATIVE 0x100 /* Anded onto it */

#define SECT_OBJECT 1
#define SECT_CODE 2
#define SECT_SYNONYMS 3
#define SECT_SAID 4
#define SECT_STRINGS 5
#define SECT_CLASS 6
#define SECT_EXPORTS 7
#define SECT_RELOCATION 8
#define SECT_PRELOAD_TEXT 9
#define SECT_LOCALS 10

struct {
	char *name, value;
} sections[] = {
	{"object", SECT_OBJECT},
	{"code", SECT_CODE},
	{"synonyms", SECT_SYNONYMS},
	{"said", SECT_SAID},
	{"strings", SECT_STRINGS},
	{"class", SECT_CLASS},
	{"exports", SECT_EXPORTS},
	{"preload-text", SECT_PRELOAD_TEXT},
	{"locals", SECT_LOCALS},
	{NULL, 0}
};

static unsigned char script[MAX_SCRIPT_SIZE];
static int script_pos = 0;
static int section_start;
static int section_type, last_section_type;

int dump_result = 0;

#define IN_CODE (section_type == SECT_CODE)

static int errors = 0;
static int arg_sizes[5];
static int args_count;
static int args_pos;
static int op_begin, op_size;

void
wr_bulk(unsigned char *c, int len)
{
	if (len + script_pos >= MAX_SCRIPT_SIZE)
		report_error(1, "Maximum script length exceeded!\n");

	if (!section_type) {
		report_error(0, "Can only specify data inside section\n");
		return;
	}
#ifdef DEBUG_OUTPUT
 {
	 int j;
	 fprintf(stderr, "[DBG-OUT] %04x: Writing %d:", script_pos, len);
	 for (j = 0; j < len && j < 15; j++)
		 fprintf(stderr, " %02x", c[j]);
	 if (len >= 15)
		 fprintf(stderr, "...");
	 fprintf(stderr, "\n");
 }
#endif

	memcpy(script + script_pos, c, len);
	script_pos += len;
}

void
wr_byte(int b)
{
	unsigned char v[1];
	v[0] = b & 0xff;
	wr_bulk(v, 1);
}

void
wr_word(int b)
{
	unsigned char v[2];
	v[0] = b & 0xff;
	v[1] = (b >> 8) & 0xff;
	wr_bulk(v, 2);
}

void
wr_word_to(int b, int dest)
{
	unsigned char v[2];
	v[0] = b & 0xff;
	v[1] = (b >> 8) & 0xff;
	memcpy(script + dest, v, 2);
#ifdef DEBUG_OUTPUT
	fprintf(stderr, "[DBG-OUT] %04x: Modifying to %04x\n", dest, b);
#endif
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



/* ------------------------------ */
struct symtab_struct *symbol_defs, *symbol_usages, *symbol_relocation;

void
init_scis()
{
	script_pos = 0;
	section_start = -1;
	section_type = 0;
	symbol_defs = new_symtab(0);
	symbol_usages = new_symtab(1);
	symbol_relocation = new_symtab(1);
}

void
finish_scis(char *dest_file)
{
	free_symtab(symbol_defs);
	free_symtab(symbol_usages);
	free_symtab(symbol_relocation);

	if (errors)
		fprintf(stderr, "Encountered %d errors: No output was written.\n", errors);
	else {
		int fd = creat(dest_file, 0644);
		if (!fd) {
			perror ("While creating output file");
			fprintf(stderr, "File was: '%s'\n", dest_file);
		} else {
			unsigned char desc[2] = {0x82, 0x00};
			int written;

			written = write(fd, desc, 2);
			if (written >= 2)
				written = write(fd, script, script_pos);
			if (written < script_pos) {
				perror ("While writing to output file");
				fprintf(stderr, "File was: '%s'\n", dest_file);
			}
		}
		close(fd);

		if (dump_result) {
			int i;
			for (i = 0; i < script_pos; i++) {
				if (!(i & 0xf))
					printf("[%04x] ", i);

				printf(" %02x", script[i]);

				if ((i & 0xf) == 0xf)
					printf("\n");
			}

			printf("\n");
		}
	}
}

void
dereference_symbols()
{
	char *name;
	int location;
	char *ref_file;
	int ref_line, value, reftype;

	while (!pop_symbol(symbol_usages, &name, &location, &ref_file, &ref_line, &reftype)) {
		if (read_symbol(symbol_defs, name, &value, NULL, NULL, NULL))
			report_error_internally(ref_file, ref_line, 0,
						"Undefined label '%s' used\n",
						name);
		else switch (reftype) {

		case REF_TYPE_ABSOLUTE:
			add_symbol(symbol_relocation, name, location, NULL, 0, 0);
			/* Fall through */
		case REF_TYPE_ABSOLUTE_AUTORELOC:
			wr_word_to(value, location);
			break;

		default: 
			if (reftype & REF_TYPE_RELATIVE) {
				wr_word_to(value - location - (reftype & 0xff), location);
			} else
			report_error(1, "Internal error in %s, L%d: Reftype %d\n",
				     __FILE__, __LINE__, reftype);
		}
	}
}


static void
end_section()
{
	if (section_start > -1)
		wr_word_to(script_pos - section_start, section_start + 2);
	if (last_section_type == SECT_EXPORTS)
		wr_word_to((script_pos - section_start - 6) >> (wide_exports ? 2 : 1), section_start + 4);
}

static void
handle_section(char *section)
{
	int i = 0;
#ifdef DEBUG_LEXING
	fprintf(stderr, "[DBG] Section '%s' encountered at %04x\n", section, script_pos);
#endif
	if (script_pos & 1)
		wr_byte(0); /* Align */

	last_section_type = section_type;
	section_type = -1;
	while (section_type == -1 && sections[i].name) {
		if (!strcasecmp(sections[i].name, section))
			section_type = sections[i].value;
		++i;
	}

	if (section_type == -1)
		report_error(1, "Unknown kind of section '%s'\n", section);

	if (script_pos & 1)
		wr_byte(0); /* Pad for word alignment */

	end_section();

	section_start = script_pos;
	wr_word(section_type);
	wr_word(0);

	if (section_type == SECT_EXPORTS)
		wr_word(0xffff);

	args_count = 0;
}

#define SAID_FRAGMENTS_NR 11

struct {
	char token;
	int representation;
} said_fragments[SAID_FRAGMENTS_NR] = {
	{',', 0xf0},
	{'&', 0xf1},
	{'/', 0xf2},
	{'(', 0xf3},
	{')', 0xf4},
	{'[', 0xf5},
	{']', 0xf6},
	{'#', 0xf7},
	{'<', 0xf8},
	{'>', 0xf9},
	{'!', 0xff}
};

static void
handle_said_fragment(char c)
{
	int i;

	if (section_type != SECT_SAID) {
		if (c == '&')
			report_error(0, "Name reference '&' used without name (or misplaced Said spec?)\n");
		else
			report_error(0, "Said spec fragment '%c' encountered outside of .said block\n", c);

		return;
	}

	for (i = 0; i < SAID_FRAGMENTS_NR; i++)
		if (said_fragments[i].token == c) {
			wr_byte(said_fragments[i].representation);
			return;
		}

	report_error(1, "Internal error: handle_said_fragment used on unsupported '%c'\n", c);
}

static void
handle_comma()
{
	if (section_type == SECT_SAID)
		handle_said_fragment(',');
}

static void
define_label(char *label)
{
#ifdef DEBUG_LEXING
	fprintf(stderr, "[DBG] Label '%s' defined at %04x\n", label, script_pos);
#endif
	if (add_symbol(symbol_defs, label, script_pos, file_name, line_nr, 0)) {
		char *def_file;
		int def_line;

		read_symbol(symbol_defs, label, NULL, &def_file, &def_line, NULL);
		report_error(0, "Label '%s' redefined (previously defined in %s, L%d)\n",
			     def_file, def_line);
	}
}


static int /* nonzero if relative */
handle_nextop(int num)
{
	if (args_pos < args_count) {
		if (arg_sizes[args_pos])
			wr_word(num);
		else
			wr_byte(num);

		return arg_sizes[args_pos++] & ARGSIZE_RELATIVE;
	} else
		report_error(0, "Too many arguments to operation\n");

	return 0;
}

static inline int
getInt16(unsigned char *d)
{
	return (*d | (d[1] << 8));
}

static void
handle_label(char *label)
{
	int refmode = REF_TYPE_ABSOLUTE;
/*	int pos = script_pos;*/
	int referencing_pos = script_pos;
#ifdef DEBUG_LEXING
	fprintf(stderr, "[DBG] Label '%s' dereferenced at %04x\n", label, script_pos);
#endif
	if (IN_CODE) {
		if (handle_nextop(0))
			refmode = REF_TYPE_RELATIVE | (op_size - (referencing_pos - op_begin));
	} else if (section_type == SECT_EXPORTS)
		refmode = REF_TYPE_ABSOLUTE_AUTORELOC;
	else if (section_type == SECT_CLASS || section_type == SECT_OBJECT) {
		if (script_pos > section_start + 4 + 8
		    && script_pos > (section_start + 4 + 8 +
				     getInt16(script + section_start + 4 + 4)))
			/* funcsel area? */
			refmode = REF_TYPE_ABSOLUTE_AUTORELOC;
	}

	if (!IN_CODE)
		wr_word(0xffff);

	if (section_type == SECT_EXPORTS && wide_exports)
		wr_word(0);

	add_symbol(symbol_usages, label, referencing_pos, file_name, line_nr, refmode);
}

static void
handle_identifier(char *ident)
{
	int op;

#ifdef DEBUG_LEXING
	fprintf(stderr, "[DBG] Identifier '%s' encountered at %04x\n", ident, script_pos);
#endif
	if (!IN_CODE)
		report_error(0, "Cannot handle identifiers/opcodes outside of code section\n");
	else {
		int i;
		op_begin = script_pos;
		if (decode_vm_op(ident, &op, (int *)arg_sizes, &args_count))
			report_error(0, "Unknown SCI operation '%s'\n", ident);

		op_size = 1;
		for (i = 0; i < args_count; i++) {
			++op_size;
			if (arg_sizes[i])
				++op_size;
		}
		wr_byte(op);
		args_pos = 0;
	}
}


static void
handle_numeric(int num, int word)
{
#ifdef DEBUG_LEXING
	fprintf(stderr, "[DBG] Numeric '%d' (%s) encountered at %04x\n", num,
		word? "word":"byte", script_pos);
#endif
	if (section_type == SECT_SAID) {
		/* Write out as big-endian */
		wr_byte((num >> 8) & 0xff);
		wr_byte(num & 0xff);
		return;
	}

	if (IN_CODE) {
		handle_nextop(num);
	} else {
		if (word)
			wr_word(num);
		else
			wr_byte(num);
	}
}

static void
finish_op()
{
#ifdef DEBUG_LEXING
	fprintf(stderr, "[DBG] Possible end-of-operation encountered at %04x\n", script_pos);
#endif
	if (IN_CODE && args_count != args_pos)
		report_error(0, "Incorrect number of arguments to VM op: Expected %d, got %d\n",
			     args_count, args_pos);
}

static void
end_file()
{
	int reloc_count = 0;
	int count_pos = 0;
	int location;
#ifdef DEBUG_LEXING
	fprintf(stderr, "[DBG] End-of-file encountered at %04x\n", script_pos);
#endif
	end_section();

	dereference_symbols();

	/* Create relocation table, if neccessary */
	while (!pop_symbol(symbol_relocation, NULL, &location, 0, 0, 0)) {
		if (!reloc_count) {
			wr_word(SECT_RELOCATION);
			count_pos = script_pos;
			wr_word(0);
			wr_word(0); /* Number of entries */
			if (wide_exports)
				wr_word(0);
		}
		++reloc_count;
		wr_word(location);
	}
	if (reloc_count) {
		/* Size */
		wr_word_to(4 + (wide_exports ? 4 : 2) + reloc_count * 2, count_pos);
		/* # of relocated things */
		wr_word_to(reloc_count, count_pos + 2);
	}

	wr_word(0);
	wr_word(0);
}

static void
handle_string(char *string)
{
#ifdef DEBUG_LEXING
	fprintf(stderr, "[DBG] String '%s' encountered at %04x\n", string, script_pos);
#endif
	if (section_type != SECT_STRINGS)
		report_error(0, "String encountered outside of string section\n");
	else
		wr_bulk((unsigned char *) string, strlen(string) + 1);
}

generator_t generator_sci0 = {
	handle_section,
	define_label,
	handle_label,
	handle_identifier,
	handle_numeric,
	finish_op,
	end_file,
	handle_string,
	handle_comma,
	handle_said_fragment,
};
