/***************************************************************************
 symtab.h Copyright (C) 2002 Christoph Reichenbach


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
