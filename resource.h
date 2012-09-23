/***************************************************************************
 resource.c Copyright (C) 2002--2006 Christoph Reichenbach


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

#ifndef RESOURCE_H
#define RESOURCE_H

struct res;
typedef struct res res_t;

res_t *
res_alloc(int type, int max_size);

void
res_free(res_t *r);

void
res_write_bulk(res_t *r, const unsigned char *c, unsigned int len);

void
res_write_byte(res_t *r, int b);

void
res_write_word(res_t *r, int b);

int
res_read_word(const res_t *r, unsigned int pos);

void
res_modify_word(res_t *r, int b, unsigned int pos);

unsigned int
res_get_pos(const res_t *r);

void
res_save(const res_t *r, const char *filename);

void
res_dump(const res_t *r);

#endif
