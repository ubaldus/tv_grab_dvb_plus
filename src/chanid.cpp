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

/*
 * to get asprintf definition
 */
//#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "constants.h"
#include "lookup.h"
#include "log.h"

extern bool use_chanidents;
extern bool use_shortxmlids;
extern char * xmltvidformat;

struct str_lookup_table *channelid_table;

int load_channel_table(char *chanidfile) {
	if (load_lookup(&channelid_table, chanidfile)) {
		return FAILURE;
	}
	return SUCCESS;
}

/*
 * Lookup channel id
 * For DVB and Freesat, chanid is the ServiceID
 * If the key is not found then return a manufactured channel id
 */
const char *dvbxmltvid(int chanid) {
	char *id;
	const char *c;

	if (use_chanidents && channelid_table) {
		asprintf(&id, "%d", chanid);
		c = slookup(channelid_table, id);
		free(id);
		if (c) {
			return c;
		} else {
			return NULL;
		}
	}
	asprintf(&id, "%d.dvb.guide", chanid);
	return id;
}

/*
 * Lookup channel id (xmltvid) based on a string lookup
 * For Sky, the string is "sid,skynumber" which is unique
 * If the key is not found then return a manufactured channel id
 */
const char *formattedxmltvid(int channelnumber, int sid, char *shortname,
		char *providername) {
	char *chanid;
	char *returnstring;
	const char *c;

	if (shortname != NULL && use_shortxmlids) {
		asprintf(&returnstring, "%d.%s.%s.dvb.guide", sid, shortname,
				providername);
	} else {
		asprintf(&chanid, "%d.%d", channelnumber, sid);
		if (use_chanidents && channelid_table) {
			c = slookup(channelid_table, chanid);
			if (c == NULL)
			{
				free(chanid);
				asprintf(&chanid, "%d", sid);
				c = slookup(channelid_table, chanid);
			}
			if (c) {
				log_message(
						TRACE,
						"found match (%s) for channelnumber=%d sid=%d shortname=\"%s\" providername=\"%s\"",
						c, channelnumber, sid, shortname, providername);
				free(chanid);
				return c;
			} else {
				log_message(
						TRACE,
						"did not find match for channelnumber=%d sid=%d shortname=\"%s\" providername=\"%s\"",
						channelnumber, sid, shortname, providername);
				free(chanid);
				return NULL;
			}
		}
		int maxlen = 10+10 + strlen(xmltvidformat) + strlen(providername);
		if (shortname != NULL)
			maxlen += strlen(shortname);
		//asprintf(&returnstring, "%s.%s.dvb.guide", chanid, providername);
		returnstring = (char*)calloc(1, maxlen+10);
		char *p = xmltvidformat;
		char *d = returnstring;
		char c;
		while ((c=*p++))
		{
			switch (c)
			{
				case '%':
					switch ((c=*p++))
					{
						case '%': *d++ = c; break;
						case 's': d += sprintf(d, "%d", sid); break;
						case 'c': d += sprintf(d, "%d", channelnumber); break;
						case 'n': if (shortname) { d += sprintf(d, "%s", shortname); } break;
						case 'p': if (providername) { d += sprintf(d, "%s", providername); } break;
						default: 
							log_message(ERROR, "invalid xmltv format specifier '%c'", c);
							break;
					}
					break;
				default:
					*d++ = c; break;
					break;
			}
		}
		free(chanid);
	}
	return returnstring;
}
