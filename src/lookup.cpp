/*
 * lookup.c
 *
 * Lookup a key in a name value pair and return the value.
 * The key can either be an integer or a string.
 * Also provide a routine to load the table from a file.
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
#include <stdio.h>
#include <string.h>

#include "lookup.h"


char *lookup(const struct int_lookup_table *l, int id) {
	while ((l->i != -1) && (l->i != id)) {
		l++;
	}
	return l->desc;
}

char *slookup(const struct str_lookup_table *l, char *id) {
	while ((l->c != NULL) && strcmp(l->c, id)) {
		l++;
	}
	return l->desc;
}

/*
 * Read lookup_table from file into newly allocated table.
 * The table is a single allocation consisting of two parts:
 * first the array of structs, followed by a char-array of strings
 */
int load_lookup(struct str_lookup_table **l, const char *file) {
	char name[256];
	char value[256];
	int n = 1, size = sizeof(struct str_lookup_table);

	if (file == NULL) {
		return -1;
	}

	FILE *fd = fopen(file, "r");
	if (!fd) {
		return -1;
	}

	// 1st: determine size needed
	while (fscanf(fd, "%255s %255s", name, value) == 2) {
		n++;
		size += sizeof(struct str_lookup_table);
		size += strlen(name) + 1;
		size += strlen(value) + 1;
	}
	struct str_lookup_table *p = *l = (struct str_lookup_table *)malloc(size);
	if (p == NULL) {
		return -1;
	}

	// 2nd: read data
	rewind(fd);
	char *c = (char *) (p + n);
	char d[255];
	while (fscanf(fd, "%255s %255s", d, c) == 2) {
		p->desc = c;
		c += strlen(c) + 1;
		strcpy(c, d);
		p->c = c;
		c += strlen(d) + 1;
		p++;
	}
	p->c = NULL;
	p->desc = NULL;

	fclose(fd);
	return 0;
}
