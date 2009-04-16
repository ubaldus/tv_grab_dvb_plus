#ifndef __DVBTEXT_H__
#define __DVBTEXT_H__

/*
 * dvbtext.h
 *
 * Routines to decode text, optionally huffman decode it and then escape
 * XML entities.
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

extern char *convert_text(const char *s);
extern char *xmlify(const char *s);
extern char *iso6937_encoding;

#ifdef __cplusplus
}
#endif

#endif
