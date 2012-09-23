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

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "resource.h"
#include "scis.h"

static int dump_result = 0;

struct res {
	unsigned int pos;
	unsigned int max_size;
	unsigned char type;
	char *data;
};

res_t *
res_alloc(int type, int max_size)
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
	v[0] = b & 0xff;
	v[1] = (b >> 8) & 0xff;
	res_write_bulk(r, v, 2);
}

int
res_read_word(const res_t *r, unsigned int pos)
{
	if (pos + 2 > r->pos)
		report_error(1, "Reading beyond current resource size %d!\n", r->pos);

	return (r->data[pos] | (r->data[pos + 1] << 8));
}

void
res_modify_word(res_t *r, int b, unsigned int pos)
{
	unsigned char v[2];

	if (pos + 2 > r->pos)
		report_error(1, "Modifying word beyond current resource size %d!\n", r->pos);

	v[0] = b & 0xff;
	v[1] = (b >> 8) & 0xff;
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
	if (errors)
		fprintf(stderr, "Encountered %d errors: No output was written.\n", errors);
	else {
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
				perror ("While writing to output file");
				fprintf(stderr, "File was: '%s'\n", filename);
			}
		}
		close(fd);

		if (dump_result) {
			int i;
			for (i = 0; i < r->pos; i++) {
				if (!(i & 0xf))
					printf("[%04x] ", i);

				printf(" %02x", r->data[i]);

				if ((i & 0xf) == 0xf)
					printf("\n");
			}

			printf("\n");
		}
	}
}

void
res_set_dump_result(int b)
{
	dump_result = b;
}