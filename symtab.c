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

#include <stdio.h>
#include <stdlib.h>
#include <symtab.h>
#include <string.h>

/* OPTIMIZE THIS! */

char *xstrdup(char *s)
{
	int sl = strlen(s) + 1;
	char *m = malloc(sl);
	memcpy(m, s, sl);
	return m;
}

typedef struct {
	char *name;
	int value;
	char *file;
	int line;
	int type;
} symtab_entry_t;

struct symtab_struct {
	int dup;
	int entries_used;
	int table_size;
	symtab_entry_t *table;
};

struct symtab_struct *
new_symtab(int dup)
{
	struct symtab_struct *st = malloc(sizeof(struct symtab_struct));
	st->dup = dup;
	st->table_size = 64;
	st->entries_used = 0;
	st->table = malloc(sizeof(symtab_entry_t) * st->table_size);

	return st;
}

void
free_symtab(struct symtab_struct *symtab)
{
	int i;

	for (i = 0; i < symtab->entries_used; i++) {
		symtab_entry_t *e = (symtab->table) + i;

		free(e->name);
	}

	free(symtab->table);
	free(symtab);
}

int
add_symbol(struct symtab_struct *symtab, char *symbol, int value, char *file, int line, int type)
{
	symtab_entry_t *e;

	if (!symtab->dup)
		if (!read_symbol(symtab, symbol, NULL, NULL, NULL, NULL))
			return 1; /* Already defined */

	if (symtab->entries_used + 1 == symtab->table_size) {
		symtab->table_size <<= 1;
		symtab->table = realloc(symtab->table,
					sizeof(symtab_entry_t) * symtab->table_size);
	}

	e = (symtab->table) + (symtab->entries_used)++;

	e->name = xstrdup(symbol);
	e->file = file;
	e->line = line;
	e->value = value;
	e->type = type;
	return 0;
}


int
read_symbol(struct symtab_struct *symtab, char *symbol, int *value, char **file, int *line, int *type)
{
	int i;

	for (i = 0; i < symtab->entries_used; i++)
		if (!strcmp(symtab->table[i].name, symbol)) {
			symtab_entry_t *e = (symtab->table) + i;
			if (value)
				*value = e->value;
			if (file)
				*file = e->file;
			if (line)
				*line = e->line;
			if (type)
				*type = e->type;
			return 0;
		}

	return 1; /* failure */
}

int
pop_symbol(struct symtab_struct *symtab, char **symbol, int *value, char **file, int *line, int *type)
{
	if (symtab->entries_used <= 0)
		return 1;
	else {
		symtab_entry_t *e = (symtab->table) + --(symtab->entries_used);
		if (symbol)
			*symbol = e->name;
		else free(e->name);

		if (value)
			*value = e->value;
		if (file)
			*file = e->file;
		if (line)
			*line = e->line;
		if (type)
			*type = e->type;
		return 0;
	}
}
