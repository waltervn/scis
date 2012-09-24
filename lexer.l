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

%{

#include <scis.h>
#include <ctype.h>
#include <string.h>

int line_nr = 1;
char *file_name = NULL;

static int have_comma = 0;

%}

DIGIT [0-9]
HEXDIGIT [0-9ABCDEFabcdef]
PRENUMTOKEN ({DIGIT}+)|(${HEXDIGIT}+)|(0[xX]{HEXDIGIT}+)
NUMTOKEN {PRENUMTOKEN}|{PRENUMTOKEN}[uU]
IDENTCHARFIRST [[:alpha:]_\-\+\*\?]
IDENTCHARNEXT  [[:alnum:]_\-\+\*\?]
IDENTIFIER {IDENTCHARFIRST}{IDENTCHARNEXT}*
STRING "\"".*[^\\]"\""

%%


\.{IDENTIFIER} {
	gen->finish_op();
	gen->handle_section(yytext + 1);
}


{IDENTIFIER}[[:blank:]]*":" {
	char *c = yytext + 1;

	while (!isblank(*c) && *c != ':')
		++c;

	*c = 0; /* Terminate */

	gen->finish_op();
	gen->define_label(yytext);
}

{IDENTIFIER} {
	gen->finish_op();
	gen->handle_identifier(yytext);
}

{NUMTOKEN} {
	int num;
	int word = 1;
	char *cp;
	int sl = strlen(yytext);

	if (toupper(yytext[sl-1]) == 'U') {
		word = 0;
		yytext[sl-1] = 0; /* Remove */
	}

	if (*yytext == '$')
		num = strtol(yytext + 1, &cp, 16);
	else if (yytext[1] == 'x' || yytext[1] == 'X')
		num = strtol(yytext + 2, &cp, 16);
	else
		num = strtol(yytext, &cp, 10);

	if (*cp)
		report_error(0, "Invalid numeric '%s'\n", yytext);

	gen->handle_numeric(num, word);
}

\&{IDENTIFIER} {
	gen->handle_label(yytext + 1);
}

{STRING} {
	int sl = strlen(yytext);
	char *c = yytext;
	c[sl-1] = 0; /* Remove last '"' */

	while (*c) {
		if (*c == '\\') {
			if (c[1] == '0') {
				int count = 2;
				int n = 0;
				while (c[count] >= '0'
				       && c[count] < '8') {
					n <<= 3;
					n += c[count] - '0';
					++count;
				}

				--count;
				memmove(c, c+count, sl--);
				sl -= count;
				*c = n;

			} else {
				memmove(c, c+1, sl--);
				switch (*c) {
				case 'n':
					*c = '\n';
					break;

				case 'b':
					*c = '\b';
					break;

				case 't':
					*c = '\t';
					break;

				case 'r':
					*c = '\r';
					break;

				default:
					break;
				}
			}
		}
		--sl;
		++c;
	}

	gen->handle_string(yytext + 1);
}

"," {
	gen->handle_comma();
	if (have_comma)
		report_error(0, "Duplicate comma");
	have_comma = 1;
}

[\&#\[\]\(\)\/\<\>!] {
	gen->handle_said_fragment(*yytext);
}

";".*$ /* Ignore comments */

\n ++line_nr;

[[:blank:]]+ /* Eat whitespace */


<<EOF>> {
	gen->finish_op();
	gen->end_file();
	yy_delete_buffer( YY_CURRENT_BUFFER );
	yyterminate();
}


. report_error(0, "Syntax error: Unexpected character '%c'\n", yytext[0]);

%%

int
yywrap()
{
        return 1; /* no further input */
}
