#ifndef __LOG_H__
#define __LOG_H__

/*
 * log.h
 *
 * Routines for logging message to the console.
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

#define INFO    5
#define ERROR   4
#define WARNING 3
#define DEBUG   2
#define TRACE   1
#define NONE    0

extern void log_level(const char *l);
extern int is_logging(int l);
extern void log_message(int l, const char *format, ...);

#endif
