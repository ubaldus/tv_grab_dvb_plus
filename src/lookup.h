#ifndef __LOOKUP_H__
#define __LOOKUP_H__

/*
 * lookup.h
 *
 * Routines to lookup a key in a name value pair and return the value
 * and also provide a routine to load the table from a file.
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

#ifdef __cplusplus
extern "C" {
#endif

union lookup_key {
	int i;
	char *c;
};
struct lookup_table {
	union lookup_key u;
	char *desc;
};

extern char *lookup(const struct lookup_table *l, int id);
extern char *slookup(const struct lookup_table *l, char *id);
extern int load_lookup(struct lookup_table **l, const char *file);

#ifdef __cplusplus
}
#endif 

#endif
