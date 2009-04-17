/*
 * chanid.c
 *
 * Convert an id into an xmltvid.
 * How this is done depends on whether this is DVb/Freesat or Sky.
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
#include <stdio.h>
#include <stdbool.h>

#include "constants.h"
#include "stats.h"

struct stats_table *stats;

/*
 * lookup an entry in the stats table
 * return it
 * create a new entry if not found
 */
struct stats_table *stats_entry(char *n) {
	struct stats_table *s;
	struct stats_table *t;

	s = stats;
	while (s != NULL) {
		if (strcmp(s->name, n) == 0) {
			return s;
		}
		s = s->next;
	}
	/*
	 * if stat is not found, create a new one
	 * add to the start of the list
	 * it is easier than adding it at the end
	 */
	t = (struct stats_table *)malloc(sizeof(struct stats_table));
	t->name = strdup(n);
	t->value = 0;
	t->next = stats;
	stats = t;
	return t;
}

void incr_stat(char *n) {
	struct stats_table *s;

	s = stats_entry(n);
	s->value += 1;
}

void decr_stat(char *n) {
	struct stats_table *s;

	s = stats_entry(n);
	s->value -= 1;
}

void set_stat(char *n, int v) {
	struct stats_table *s;

	s = stats_entry(n);
	s->value = v;
}

int get_stat(char *n) {
	struct stats_table *s;

	s = stats_entry(n);
	return s->value;
}
