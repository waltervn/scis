/***************************************************************************
 sci0.c Copyright (C) 2002--2006 Christoph Reichenbach


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

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "symtab.h"
#include "scis.h"
#include "resource.h"

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

static res_t *script;
static int section_start;
static int section_type, last_section_type;

#define IN_CODE (section_type == SECT_CODE)

static int arg_sizes[5];
static int args_count;
static int args_pos;
static int op_begin, op_size;
static char *dest_file;
static int wide_exports;

/* ------------------------------ */
struct symtab_struct *symbol_defs, *symbol_usages, *symbol_relocation;

static int
init(const options_t *options)
{
	dest_file = options->script_filename;
	wide_exports = options->wide_exports;

	if (options->absolute_lofs)
		use_absolute_lofs();

	section_start = -1;
	section_type = 0;
	symbol_defs = new_symtab(0);
	symbol_usages = new_symtab(1);
	symbol_relocation = new_symtab(1);
	script = res_alloc(0x82, MAX_SCRIPT_SIZE);
	if (!script)
		report_error(1, "Failed to allocate script\n");
	return 1;
}

static void
deinit()
{
	free_symtab(symbol_defs);
	free_symtab(symbol_usages);
	free_symtab(symbol_relocation);
	res_save(script, dest_file);
}

static void
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
			res_modify_word(script, value, location);
			break;

		default: 
			if (reftype & REF_TYPE_RELATIVE) {
				res_modify_word(script, value - location - (reftype & 0xff), location);
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
		res_modify_word(script, res_get_pos(script) - section_start, section_start + 2);
	if (last_section_type == SECT_EXPORTS)
		res_modify_word(script, (res_get_pos(script) - section_start - 6) >> (wide_exports ? 2 : 1), section_start + 4);
}

static void
handle_section(char *section)
{
	int i = 0;
#ifdef DEBUG_LEXING
	fprintf(stderr, "[DBG] Section '%s' encountered at %04x\n", section, script_pos);
#endif
	if (res_get_pos(script) & 1)
		res_write_byte(script, 0); /* Align */

	last_section_type = section_type;
	section_type = -1;
	while (section_type == -1 && sections[i].name) {
		if (!strcasecmp(sections[i].name, section))
			section_type = sections[i].value;
		++i;
	}

	if (section_type == -1)
		report_error(1, "Unknown kind of section '%s'\n", section);

	if (res_get_pos(script) & 1)
		res_write_byte(script, 0); /* Pad for word alignment */

	end_section();

	section_start = res_get_pos(script);
	res_write_word(script, section_type);
	res_write_word(script, 0);

	if (section_type == SECT_EXPORTS)
		res_write_word(script, 0xffff);

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
			res_write_byte(script, said_fragments[i].representation);
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
	if (add_symbol(symbol_defs, label, res_get_pos(script), file_name, line_nr, 0)) {
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
			res_write_word(script, num);
		else
			res_write_byte(script, num);

		return arg_sizes[args_pos++] & ARGSIZE_RELATIVE;
	} else
		report_error(0, "Too many arguments to operation\n");

	return 0;
}

static void
handle_label(char *label)
{
	int refmode = REF_TYPE_ABSOLUTE;
	int pos = res_get_pos(script);
	int referencing_pos = pos;
#ifdef DEBUG_LEXING
	fprintf(stderr, "[DBG] Label '%s' dereferenced at %04x\n", label, script_pos);
#endif
	if (IN_CODE) {
		if (handle_nextop(0))
			refmode = REF_TYPE_RELATIVE | (op_size - (referencing_pos - op_begin));
	} else if (section_type == SECT_EXPORTS)
		refmode = REF_TYPE_ABSOLUTE_AUTORELOC;
	else if (section_type == SECT_CLASS || section_type == SECT_OBJECT) {
		if (pos > section_start + 4 + 8
		    && pos > (section_start + 4 + 8 +
				     res_read_word(script, section_start + 4 + 4)))
			/* funcsel area? */
			refmode = REF_TYPE_ABSOLUTE_AUTORELOC;
	}

	if (!IN_CODE)
		res_write_word(script, 0xffff);

	if (section_type == SECT_EXPORTS && wide_exports)
		res_write_word(script, 0);

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
		op_begin = res_get_pos(script);
		if (decode_vm_op(ident, &op, (int *)arg_sizes, &args_count))
			report_error(0, "Unknown SCI operation '%s'\n", ident);

		op_size = 1;
		for (i = 0; i < args_count; i++) {
			++op_size;
			if (arg_sizes[i])
				++op_size;
		}
		res_write_byte(script, op);
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
		res_write_byte(script, (num >> 8) & 0xff);
		res_write_byte(script, num & 0xff);
		return;
	}

	if (IN_CODE) {
		handle_nextop(num);
	} else {
		if (word)
			res_write_word(script, num);
		else
			res_write_byte(script, num);
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
			res_write_word(script, SECT_RELOCATION);
			count_pos = res_get_pos(script);
			res_write_word(script, 0);
			res_write_word(script, 0); /* Number of entries */
			if (wide_exports)
				res_write_word(script, 0);
		}
		++reloc_count;
		res_write_word(script, location);
	}
	if (reloc_count) {
		/* Size */
		res_modify_word(script, 4 + (wide_exports ? 4 : 2) + reloc_count * 2, count_pos);
		/* # of relocated things */
		res_modify_word(script, reloc_count, count_pos + 2);
	}

	res_write_word(script, 0);
	res_write_word(script, 0);
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
		res_write_bulk(script, (unsigned char *) string, strlen(string) + 1);
}

generator_t generator_sci0 = {
	init,
	deinit,
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
