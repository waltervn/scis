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

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "resource.h"
#include "scis.h"

struct res {
	unsigned int pos;
	unsigned int max_size;
	int type;
	unsigned char *data;
	int big_endian;
};

res_t *
res_alloc(int type, unsigned int max_size, int big_endian)
{
	res_t *res = malloc(sizeof(res_t));

	if (!res)
		return NULL;

	res->data = malloc(max_size);
	if (!res->data) {
		free(res);
		return NULL;
	}

	res->max_size = max_size;
	res->pos = 0;
	res->type = type;
	res->big_endian = big_endian;

	return res;
}

void
res_free(res_t *r)
{
	free(r->data);
	free(r);
}

void
res_write_bulk(res_t *r, const unsigned char *c, unsigned int len)
{
	if (len + r->pos > r->max_size)
		report_error(1, "Maximum resource length %d exceeded while writing %d bytes to offset %d!\n", r->max_size, len, r->pos);

#ifdef DEBUG_OUTPUT
	{
		int j;
		fprintf(stderr, "[DBG-OUT] %08x: Writing %d:", r->pos, len);
		for (j = 0; j < len && j < 15; j++)
			fprintf(stderr, " %02x", c[j]);
		if (len >= 15)
			fprintf(stderr, "...");
		fprintf(stderr, "\n");
	}
#endif

	memcpy(r->data + r->pos, c, len);
	r->pos += len;
}

void
res_write_byte(res_t *r, int b)
{
	unsigned char v[1];
	v[0] = b & 0xff;
	res_write_bulk(r, v, 1);
}

void
res_write_word(res_t *r, int b)
{
	unsigned char v[2];
	if (r->big_endian) {
		v[0] = (b >> 8) & 0xff;
		v[1] = b & 0xff;
	} else {
		v[0] = b & 0xff;
		v[1] = (b >> 8) & 0xff;
	}
	res_write_bulk(r, v, 2);
}

int
res_read_word(const res_t *r, unsigned int pos)
{
	if (pos + 2 > r->pos)
		report_error(1, "Reading beyond current resource size %d!\n", r->pos);

	if (r->big_endian)
		return ((r->data[pos] << 8) | r->data[pos + 1]);
	else
		return (r->data[pos] | (r->data[pos + 1] << 8));
}

void
res_modify_word(res_t *r, int b, unsigned int pos)
{
	unsigned char v[2];

	if (pos + 2 > r->pos)
		report_error(1, "Modifying word beyond current resource size %d!\n", r->pos);

	if (r->big_endian) {
		v[0] = (b >> 8) & 0xff;
		v[1] = b & 0xff;
	} else {
		v[0] = b & 0xff;
		v[1] = (b >> 8) & 0xff;
	}

	memcpy(r->data + pos, v, 2);
#ifdef DEBUG_OUTPUT
	fprintf(stderr, "[DBG-OUT] %04x: Modifying to %04x\n", pos, b);
#endif
}

unsigned int
res_get_pos(const res_t *r)
{
	return r->pos;
}

void
res_save(const res_t *r, const char *filename)
{
	int fd = creat(filename, 0644);
	if (!fd) {
		perror ("While creating output file");
		fprintf(stderr, "File was: '%s'\n", filename);
	} else {
		unsigned char desc[2] = {r->type, 0x00};

		int written;
		written = write(fd, desc, 2);
		if (written >= 2)
			written = write(fd, r->data, r->pos);
		if (written < r->pos) {
			perror("While writing to output file");
			fprintf(stderr, "File was: '%s'\n", filename);
		}
		close(fd);
	}
}

void
res_save_resource_fork(const res_t *r, const char *filename)
{
	FILE *f = fopen(filename, "w");
	if (!f) {
		perror("While creating output file");
		fprintf(stderr, "File was: '%s'\n", filename);
	} else {
		int i;
		const char *type_str;

		switch(r->type & 0x7f) {
		case 0x02:
			type_str = "SCR";
			break;
		case 0x11:
			type_str = "HEP";
			break;
		default:
			fprintf(stderr, "Unknown resource type %d encountered\n", r->type);
			fclose(f);
			return;			
		}
		fprintf(f, "data '%s ' (0, \"0.%s\") {\n", type_str, type_str);
		for (i = 0; i < r->pos; i++) {
			if (!(i & 0xf))
				fprintf(f, "        $\"%02x", r->data[i]);
			else
				fprintf(f, " %02x", r->data[i]);

			if ((i & 0xf) == 0xf && i != r->pos - 1)
				fprintf(f, "\"\n");
		}

		fprintf(f, "\"\n};\n");
		fclose(f);
	}
}

void
res_dump(const res_t *r)
{
	int i;
	for (i = 0; i < r->pos; i++) {
		if (!(i & 0xf))
			printf("[%04x] ", i);

		printf(" %02x", r->data[i]);

		if ((i & 0xf) == 0xf && i != r->pos - 1)
			printf("\n");
	}

	printf("\n");
}
