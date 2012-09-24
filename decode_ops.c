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

#include <scis.h>
#include <strings.h>

#ifndef NULL
#  define NULL 0
#endif

typedef enum {
  Script_Invalid=-1,
  Script_None=0,
  Script_Byte,
  Script_Word,
  Script_Variable,
  Script_Relative,
  Script_End
} opcode_format;

opcode_format formats[128][4]={
  /*00*/
  {Script_None}, {Script_None}, {Script_None}, {Script_None},
  /*04*/
  {Script_None}, {Script_None}, {Script_None}, {Script_None},
  /*08*/
  {Script_None}, {Script_None}, {Script_None}, {Script_None},
  /*0C*/
  {Script_None}, {Script_None}, {Script_None}, {Script_None},
  /*10*/
  {Script_None}, {Script_None}, {Script_None}, {Script_None},
  /*14*/
  {Script_None}, {Script_None}, {Script_None}, {Script_Relative, 0},
  /*18*/
  {Script_Relative, 0}, {Script_Relative, 0}, {Script_Variable, 0}, {Script_None},
  /*1C*/
  {Script_Variable, 0}, {Script_None}, {Script_None}, {Script_Variable, 0},
  /*20*/
  {Script_Relative, Script_Byte, 0}, {Script_Variable, Script_Byte, 0}, {Script_Variable, Script_Byte, 0}, {Script_Variable, Script_Variable, Script_Byte, 0},
  /*24 (24=ret)*/
  {Script_End}, {Script_Byte, 0}, {Script_Invalid}, {Script_Invalid},
  /*28*/
  {Script_Variable, 0}, {Script_Invalid}, {Script_Byte, 0}, {Script_Variable, Script_Byte, 0},
  /*2C*/
  {Script_Variable, 0}, {Script_Variable, Script_Variable, 0}, {Script_None}, {Script_Invalid},
  /*30*/
  {Script_None}, {Script_Variable, 0}, {Script_Variable, 0}, {Script_Variable, 0},
  /*34*/
  {Script_Variable, 0}, {Script_Variable, 0}, {Script_Variable, 0}, {Script_Variable, 0},
  /*38*/
  {Script_Variable, 0}, {Script_Relative, 0}, {Script_Relative, 0}, {Script_None},
  /*3C*/
  {Script_None}, {Script_None}, {Script_None}, {Script_Invalid},
  /*40-4F*/
  {Script_Variable, 0}, {Script_Variable, 0}, {Script_Variable, 0}, {Script_Variable, 0},
  {Script_Variable, 0}, {Script_Variable, 0}, {Script_Variable, 0}, {Script_Variable, 0},
  {Script_Variable, 0}, {Script_Variable, 0}, {Script_Variable, 0}, {Script_Variable, 0},
  {Script_Variable, 0}, {Script_Variable, 0}, {Script_Variable, 0}, {Script_Variable, 0},
  /*50-5F*/
  {Script_Variable, 0}, {Script_Variable, 0}, {Script_Variable, 0}, {Script_Variable, 0},
  {Script_Variable, 0}, {Script_Variable, 0}, {Script_Variable, 0}, {Script_Variable, 0},
  {Script_Variable, 0}, {Script_Variable, 0}, {Script_Variable, 0}, {Script_Variable, 0},
  {Script_Variable, 0}, {Script_Variable, 0}, {Script_Variable, 0}, {Script_Variable, 0},
  /*60-6F*/
  {Script_Variable, 0}, {Script_Variable, 0}, {Script_Variable, 0}, {Script_Variable, 0},
  {Script_Variable, 0}, {Script_Variable, 0}, {Script_Variable, 0}, {Script_Variable, 0},
  {Script_Variable, 0}, {Script_Variable, 0}, {Script_Variable, 0}, {Script_Variable, 0},
  {Script_Variable, 0}, {Script_Variable, 0}, {Script_Variable, 0}, {Script_Variable, 0},
  /*70-7F*/
  {Script_Variable, 0}, {Script_Variable, 0}, {Script_Variable, 0}, {Script_Variable, 0},
  {Script_Variable, 0}, {Script_Variable, 0}, {Script_Variable, 0}, {Script_Variable, 0},
  {Script_Variable, 0}, {Script_Variable, 0}, {Script_Variable, 0}, {Script_Variable, 0},
  {Script_Variable, 0}, {Script_Variable, 0}, {Script_Variable, 0}, {Script_Variable, 0}
};

char *ops[128] = {
	"bnot",
	"add",
	"sub",
	"mul",
	"div",
	"mod",
	"shr",
	"shl",
	"xor",
	"and",
	"or",
	"neg",
	"not",
	"eq?",
	"ne?",
	"gt?",
	"ge?",
	"lt?",
	"le?",
	"ugt?",
	"uge?",
	"ult?",
	"ule?",
	"bt",
	"bnt",
	"jmp",
	"ldi",
	"push",
	"pushi",
	"toss",
	"dup",
	"link",
	"call",
	"callk",
	"callb",
	"calle",
	"ret",
	"send",
	NULL,
	NULL,
	"class",
	NULL,
	"self",
	"super",
	"&rest",
	"lea",
	"selfID",
	NULL,
	"pprev",
	"pToa",
	"aTop",
	"pTos",
	"sTop",
	"ipToa",
	"dpToa",
	"ipTos",
	"dpTos",
	"lofsa",
	"lofss",
	"push0",
	"push1",
	"push2",
	"pushSelf",
	NULL,
	"lag",
	"lal",
	"lat",
	"lap",
	"lsg",
	"lsl",
	"lst",
	"lsp",
	"lagi",
	"lali",
	"lati",
	"lapi",
	"lsgi",
	"lsli",
	"lsti",
	"lspi",
	"sag",
	"sal",
	"sat",
	"sap",
	"ssg",
	"ssl",
	"sst",
	"ssp",
	"sagi",
	"sali",
	"sati",
	"sapi",
	"ssgi",
	"ssli",
	"ssti",
	"sspi",
	"+ag",
	"+al",
	"+at",
	"+ap",
	"+sg",
	"+sl",
	"+st",
	"+sp",
	"+agi",
	"+ali",
	"+ati",
	"+api",
	"+sgi",
	"+sli",
	"+sti",
	"+spi",
	"-ag",
	"-al",
	"-at",
	"-ap",
	"-sg",
	"-sl",
	"-st",
	"-sp",
	"-agi",
	"-ali",
	"-ati",
	"-api",
	"-sgi",
	"-sli",
	"-sti",
	"-spi"
};

void
use_absolute_lofs()
{
	formats[0x39][0] = Script_Word;
	formats[0x3a][0] = Script_Word;
}

int
decode_vm_op(char *name, int *opcode, int *arg_sizes, int *arg_count)
{
	int i, op, argc = 0;

	op = -1;

	for (i = 0; i < 128 && op < 0; i++)
		if (ops[i] && !strcasecmp(ops[i], name))
			op = i;

	if (op < 0)
		return 1;

	*opcode = op << 1;
	argc = 0;

	for (i = 0; i < 4; i++) {
		switch(formats[op][i]) {

		case Script_End:
		case Script_None: break;

		case Script_Byte:
			arg_sizes[argc++] = ARGSIZE_BYTE;
			break;

		case Script_Word:
			arg_sizes[argc++] = ARGSIZE_WORD;
			break;

		case Script_Variable:
			arg_sizes[argc++] = ARGSIZE_VARIABLE;
			break;

		case Script_Relative:
			arg_sizes[argc++] = ARGSIZE_WORD | ARGSIZE_RELATIVE;
			break;

		case Script_Invalid:
		default:
			report_error(1, "Internal error in %s, L%d: Unexpected invalid(%d)"
				     " argument\n",
				     __FILE__, __LINE__, formats[op][i]);
			break;

		}
	}

	*arg_count = argc;
	return 0;
}
