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
