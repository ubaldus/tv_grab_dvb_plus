#ifndef __FREESAT_TABLES_H__
#define __FREESAT_TABLES_H__

/*
 * freesat_tables.h
 *
 * Huffman tables structures for Freesat.
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

struct fsattab {
    unsigned int value;
    short bits;
    char next;
};

#define START   '\0'
#define STOP    '\0'
#define ESCAPE  '\1'

extern struct fsattab fsat_table_1[];
extern struct fsattab fsat_table_2[];
extern unsigned fsat_index_1[];
extern unsigned fsat_index_2[];

#endif
