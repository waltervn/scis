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

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "symtab.h"
#include "scis.h"
#include "resource.h"

static char *script_file, *heap_file;
static int dump_results = 0;

#define MAX_SCRIPT_SIZE 65535
#define MAX_HEAP_SIZE 65535
#define REF_TYPE_SCRIPT 0
#define REF_TYPE_HEAP 1
#define REF_TYPE_RELATIVE 0x100 /* Anded onto it */

typedef enum {
	SECT_EXPORTS = 1,
	SECT_DICT,
	SECT_CODE,
	SECT_LOCALS,
	SECT_OBJECT,
	SECT_STRINGS
} section_t;

typedef enum {
	RES_SCRIPT,
	RES_HEAP
} res_type_t;

static int cur_section = 0;
static int prev_script_sect = 0, prev_heap_sect = 0;
static int section_start;
static res_type_t cur_res;
static res_t *script, *heap;
struct symtab_struct *symbol_defs, *symbol_usages, *symbol_reloc_script, *symbol_reloc_heap;
static int arg_sizes[5];
static int args_count;
static int args_pos;
static int op_begin, op_size;
static int resource_fork;

static struct {
	char *name;
	section_t sec;
	res_type_t res;
} sections[] = {
	{"exports", SECT_EXPORTS, RES_SCRIPT},
	{"dict", SECT_DICT, RES_SCRIPT},
	{"code", SECT_CODE, RES_SCRIPT},
	{"locals", SECT_LOCALS, RES_HEAP},
	{"object", SECT_OBJECT, RES_HEAP},
	{"strings", SECT_STRINGS, RES_HEAP},
	{NULL, 0}
};

static int
init(const options_t *options)
{
	script_file = options->script_filename;
	heap_file = options->heap_filename;
	dump_results = options->dump_results;
	resource_fork = options->resource_fork;
	symbol_defs = new_symtab(0);
	symbol_usages = new_symtab(1);
	symbol_reloc_script = new_symtab(1);
	symbol_reloc_heap = new_symtab(1);

	use_absolute_lofs();
	if (options->wide_calls)
		use_wide_calls();

	script = res_alloc(0x82, MAX_SCRIPT_SIZE, options->big_endian);
	if (!script)
		report_error(1, "Failed to allocate script\n");

	res_write_word(script, 0xffff); /* Pointer to relocation block */
	res_write_word(script, 0); /* Unknown */
	res_write_word(script, 0); /* Unknown */
	res_write_word(script, 0); /* Number of exports */

	heap = res_alloc(0x91, MAX_HEAP_SIZE, options->big_endian);
	if (!heap)
		report_error(1, "Failed to allocate heap\n");

	res_write_word(heap, 0xffff); /* Pointer to relocation block */
	res_write_word(heap, 0); /* Number of locals */

	cur_res = RES_SCRIPT;

	return 1;
}

static void
verify_objects()
{
	/* SCI2 does not use -propDict- and -methDict- when reading in object
	** dictionaries. Instead, it reads them sequentially from the script.
	** Here we check that the dictionaries are indeed in the proper order.
	** Currently we also enforce this for SCI1.1.
	*/

	int dictpos = 8 + res_read_word(script, 6) * 2;
	int objpos = 4 + res_read_word(heap, 2) * 2;

	while (res_read_word(heap, objpos) == 0x1234) {
		int size = res_read_word(heap, objpos + 2);
		int propdict = res_read_word(heap, objpos + 4);
		int methdict = res_read_word(heap, objpos + 6);
		int species = res_read_word(heap, objpos + 10);
		int isclass = res_read_word(heap, objpos + 14) & 0x8000;
		int methods;

		if (isclass && species == 0xffff)
			report_error_internally(__FILE__, __LINE__, 0, "Class flag set, but -script- not specified for object at heap offset %04x\n", objpos);

		if (!isclass && species != 0xffff)
			report_error_internally(__FILE__, __LINE__, 0, "Class flag not set, but -script- specified for object at heap offset %04x\n", objpos);

		if (dictpos != propdict) {
			report_error_internally(__FILE__, __LINE__, 0, "Non-sequential -propDict- detected for object at heap offset %04x\n", objpos);
			dictpos = propdict;
		}

		if (isclass)
			dictpos += size * 2;

		if (dictpos != methdict) {
			report_error_internally(__FILE__, __LINE__, 0, "Non-sequential -methDict- detected for object at heap offset %04x\n", objpos);
			dictpos = methdict;
		}

		methods = res_read_word(script, dictpos);
		dictpos += 2 + (methods * 4);

		objpos += size * 2;
	}
}

static void
deinit()
{
	free_symtab(symbol_defs);
	free_symtab(symbol_usages);
	free_symtab(symbol_reloc_script);
	free_symtab(symbol_reloc_heap);
	int errors = errors_found();

	if (dump_results) {
		printf("%s:\n", script_file);
		res_dump(script);
		printf("\n%s:\n", heap_file);
		res_dump(heap);
	}

	if (errors)
		fprintf(stderr, "Encountered %d errors: No output was written.\n", errors);
	else if (resource_fork) {
		/* Append four zero bytes to indicate uncompressed resource */
		res_write_word(script, 0);
		res_write_word(script, 0);
		res_write_word(heap, 0);
		res_write_word(heap, 0);
		res_save_resource_fork(script, script_file);
		res_save_resource_fork(heap, heap_file);
	} else {
		res_save(script, script_file);
		res_save(heap, heap_file);
	}

	res_free(script);
	res_free(heap);
}

static void
write_relocation_table(res_t *res, struct symtab_struct *reloc)
{
	int location;
	int reloc_count = 0;

	res_modify_word(res, res_get_pos(res), 0); /* Pointer to relocation table */
	res_write_word(res, 0xffff); /* Number of relocations */

	while (!pop_symbol(reloc, NULL, &location, 0, 0, 0)) {
		++reloc_count;
		res_write_word(res, location);
	}

	res_modify_word(res, reloc_count, res_get_pos(res) - (reloc_count + 1) * 2);
}

static void
dereference_symbols()
{
	char *name;
	int location;
	char *ref_file;
	int ref_line, value, reftype;

	while (!pop_symbol(symbol_usages, &name, &location, &ref_file, &ref_line, &reftype)) {
		int def_res;
		if (read_symbol(symbol_defs, name, &value, NULL, NULL, &def_res))
			report_error_internally(ref_file, ref_line, 0,
						"Undefined label '%s' used\n",
						name);
		else {
			if (reftype & REF_TYPE_RELATIVE) {
				res_modify_word(script, value - location - (reftype & 0xff), location);
			} else if (reftype == REF_TYPE_SCRIPT) {
				if (def_res == RES_HEAP)
					add_symbol(symbol_reloc_script, name, location, NULL, 0, 0);
				res_modify_word(script, value, location);
			} else {
				if (def_res == RES_HEAP)
					add_symbol(symbol_reloc_heap, name, location, NULL, 0, 0);
				res_modify_word(heap, value, location);
			}
		}
	}
}

static int
handle_nextop(int num)
{
	if (args_pos < args_count) {
		if (arg_sizes[args_pos])
			res_write_word(script, num);
		else
			res_write_byte(script, num);

		return arg_sizes[args_pos++];
	} else
		report_error(0, "Too many arguments to operation\n");

	return -1;
}

static res_t *
get_current_res()
{
	if (cur_res == RES_SCRIPT)
		return script;
	else
		return heap;
}

static void
finish_op()
{
#ifdef DEBUG_LEXING
	fprintf(stderr, "[DBG] Possible end-of-operation encountered at %04x\n", script_pos);
#endif
	if (cur_section == SECT_CODE && args_count != args_pos)
		report_error(0, "Incorrect number of arguments to VM op: Expected %d, got %d\n",
			     args_count, args_pos);
}

static void
end_section()
{
	res_t *res = get_current_res();
	int pos = res_get_pos(res);

	finish_op();

	if (cur_section == SECT_EXPORTS)
		res_modify_word(res, (pos - 8) >> 1, 6);
	else if (cur_section == SECT_LOCALS)
		res_modify_word(res, (pos - 4) >> 1, 2);

	if (pos & 1)
		res_write_byte(res, 0); /* Pad for word alignment */
}

static void
handle_section(char *section)
{
	int i = 0, section_type, section_res, prev_section, allowed = 1;
#ifdef DEBUG_LEXING
	fprintf(stderr, "[DBG] Section '%s' encountered at %04x\n", section, script_pos);
#endif
	end_section();

	section_type = 0;
	while (section_type == 0 && sections[i].name) {
		if (!strcasecmp(sections[i].name, section)) {
			section_type = sections[i].sec;
			section_res = sections[i].res;
		}
		++i;
	}

	if (section_type == 0)
		report_error(1, "Unknown kind of section '%s'\n", section);

	if (cur_section != 0) {
		if (cur_res == RES_SCRIPT)
			prev_script_sect = cur_section;
		else
			prev_heap_sect = cur_section;
	}

	cur_section = section_type;
	cur_res = section_res;
	prev_section = (cur_res == RES_SCRIPT ? prev_script_sect : prev_heap_sect);

	if (cur_section == SECT_EXPORTS || cur_section == SECT_LOCALS) {
		/* Exports and locals must come first */
		allowed = prev_section == 0;
	} else if (prev_section == SECT_STRINGS || prev_section == SECT_CODE) {
		/* Nothing can follow strings or code */
		allowed = 0;
	}

	if (!allowed) {
		report_error(1, "Section '%s' not allowed after section '%s'\n",
		             section, sections[prev_section - 1].name);
	}

	if (cur_section == SECT_STRINGS) {
		/* Terminate the object list first */
		res_write_word(heap, 0);
	}

	section_start = res_get_pos(get_current_res());
}

static void
handle_said_fragment(char c)
{
}

static void
handle_comma()
{
}

static void
define_label(char *label)
{
#ifdef DEBUG_LEXING
	fprintf(stderr, "[DBG] Label '%s' defined at %04x\n", label, script_pos);
#endif
	finish_op();
	if (add_symbol(symbol_defs, label, res_get_pos(get_current_res()), file_name, line_nr, cur_res)) {
		char *def_file;
		int def_line;

		read_symbol(symbol_defs, label, NULL, &def_file, &def_line, NULL);
		report_error(0, "Label '%s' redefined (previously defined in %s, L%d)\n",
			     label, def_file, def_line);
	}
}

static void
handle_label(char *label)
{
	int refmode = REF_TYPE_SCRIPT;
	res_t *r = get_current_res();
	int pos = res_get_pos(r);
	int referencing_pos = pos;
#ifdef DEBUG_LEXING
	fprintf(stderr, "[DBG] Label '%s' dereferenced at %04x\n", label, script_pos);
#endif
	if (cur_res == RES_HEAP) {
		refmode = REF_TYPE_HEAP;
		res_write_word(r, 0xffff);
	} else {
		if (cur_section == SECT_CODE) {
			int argsize = handle_nextop(0);
			if (argsize == ARGSIZE_BYTE) {
				report_error(0, "Cannot use label for byte-sized argument\n");
				return;
			}
			if (argsize & ARGSIZE_RELATIVE)
				refmode = REF_TYPE_RELATIVE | (op_size - (referencing_pos - op_begin));
		} else {
			res_write_word(r, 0xffff);
		}
	}

	add_symbol(symbol_usages, label, referencing_pos, file_name, line_nr, refmode);
}

static void
handle_identifier(char *ident)
{
	int op;

#ifdef DEBUG_LEXING
	fprintf(stderr, "[DBG] Identifier '%s' encountered at %04x\n", ident, script_pos);
#endif
	finish_op();
	if (cur_section != SECT_CODE)
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

		/* &rest */
		if (op == 0x58)
			++op;

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

	if (cur_section == SECT_CODE) {
		handle_nextop(num);
	} else {
		if (word)
			res_write_word(get_current_res(), num);
		else
			res_write_byte(get_current_res(), num);
	}
}

static void
end_file()
{
	end_section();

	/* If we haven't written any strings, we still need to terminate the object list */
	if (cur_section != SECT_STRINGS && prev_heap_sect != SECT_STRINGS)
		res_write_word(heap, 0);

	dereference_symbols();
	write_relocation_table(script, symbol_reloc_script);
	write_relocation_table(heap, symbol_reloc_heap);
	verify_objects();
}

static void
end_line()
{
}

static void
handle_string(char *string)
{
#ifdef DEBUG_LEXING
	fprintf(stderr, "[DBG] String '%s' encountered at %04x\n", string, script_pos);
#endif
	if (cur_section != SECT_STRINGS)
		report_error(0, "String encountered outside of string section\n");
	else
		res_write_bulk(heap, (unsigned char *) string, strlen(string) + 1);
}

generator_t generator_sci11 = {
	init,
	deinit,
	handle_section,
	define_label,
	handle_label,
	handle_identifier,
	handle_numeric,
	end_file,
	end_line,
	handle_string,
	handle_comma,
	handle_said_fragment,
};
