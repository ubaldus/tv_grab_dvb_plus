#ifndef __STATS_H__
#define __STATS_H__

/*
 * stats.h
 *
 * Routines for handling runtime stats.
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

extern bool print_stats;

struct stats_table {
	char *name;
	int value;
	struct stats_table *next;
};

extern void incr_stat(char *n);
extern void decr_stat(char *n);
extern void add_to_stat(char *n, int i);
extern void set_stat(char *n, int v);
extern int get_stat(char *n);


#ifdef __cplusplus
}
#endif 

#endif
