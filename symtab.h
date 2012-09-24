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

#ifndef SYMTAB_H
#define SYMTAB_H

struct symtab_struct;

struct symtab_struct *
new_symtab(int dup);
/* dup: May have duplicate symbols */

void
free_symtab(struct symtab_struct *symtab);

int
add_symbol(struct symtab_struct *symtab, char *symbol, int value, char *file,
	   int line, int type);
/* Returns 0 on success, or 1 if the symbol was already defined */

int
read_symbol(struct symtab_struct *symtab, char *symbol, int *value, char **file,
	    int *line, int *type);
/* Returns 0 on success */

int
pop_symbol(struct symtab_struct *symtab, char **symbol, int *value, char **file,
	   int *line, int *type);
/* Removes the symbol that was last added; '*symbol' is malloc'd! */

#endif /* !SYMTAB_H */
