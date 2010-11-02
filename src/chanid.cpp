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
#include <ctype.h>

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
const char *formattedxmltvid(
		int channelnumber, 
		int channelid, 
		int sid, 
		int regionid, 
		char *shortname,
		char *name,
		char *providername)
{
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
		int maxlen = 11+11+11+11 + strlen(xmltvidformat);
		if (providername != NULL)
			maxlen += strlen(providername);
		if (shortname != NULL)
			maxlen += strlen(shortname);
		//asprintf(&returnstring, "%s.%s.dvb.guide", chanid, providername);
		returnstring = (char*)calloc(1, maxlen+10);
		char *p = xmltvidformat;
		char *d = returnstring;
		char c;
		const char * prefix;
		const char * postfix;
		while ((c=*p++))
		{
			switch (c)
			{
				case '%':
					prefix = "";
					postfix = "";
doagain:
					switch ((c=*p++))
					{
						case '%': *d++ = c; break;
						case '-':
							switch ((c=*p++))
							{
								case '.':
									postfix = ".";
									goto doagain;
							}
							break;
						case '.':
							prefix = ".";
							goto doagain;
							break;
						case 's': d += sprintf(d, "%s%d%s", prefix, sid, postfix); break;
						case 'c': d += sprintf(d, "%s%d%s", prefix, channelnumber, postfix); break;
						case 'i': d += sprintf(d, "%s%d%s", prefix, channelid, postfix); break;
						case 'r': d += sprintf(d, "%s%d%s", prefix, regionid, postfix); break;
						case 'R': if (regionid > 0) { d += sprintf(d, "%s%d%s", prefix, regionid, postfix); } break;
						case 'x': if (shortname) { d += sprintf(d, "%s%s%s", prefix, shortname, postfix); } break;
						case 'X': if (shortname) { 
									  for(const char*s=prefix;s && *s;) *d++ = tolower(*s++); 
									  for(const char*s=shortname;s && *s;) *d++ = tolower(*s++); 
									  for(const char*s=postfix;s && *s;) *d++ = tolower(*s++); 
								  } break;
						case 'n': if (name) { d += sprintf(d, "%s%s%s", prefix, name, postfix); } break;
						case 'N': if (name) { 
									  for(const char*s=prefix;s && *s;) *d++ = tolower(*s++); 
									  for(const char*s=name;s && *s;) if (isalnum(*s)) *d++ = tolower(*s++); else s++; 
									  for(const char*s=postfix;s && *s;) *d++ = tolower(*s++); 
								  } break;
						case 'p': if (providername) { d += sprintf(d, "%s%s%s", prefix, providername, postfix); } break;
						case 'P': if (providername) { 
									  for(const char*s=prefix;s && *s;) *d++ = tolower(*s++); 
									  for(const char*s=providername;s && *s;) *d++ = tolower(*s++); 
									  for(const char*s=postfix;s && *s;) *d++ = tolower(*s++); 
								  } break;
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
