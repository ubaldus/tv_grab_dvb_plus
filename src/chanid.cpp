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

extern bool use_chanidents;

struct str_lookup_table *channelid_table;

int load_channel_table(char *chanidfile)
{
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
const char *dvbxmltvid(int chanid)
{
    char *id;

    if (use_chanidents && channelid_table) {
	asprintf(&id, "%d", chanid);
	const char *c = slookup(channelid_table, id);
	free(id);
	if (c)
	    return c;
    }
    asprintf(&id, "%d.dvb.guide", chanid);
    return id;
}

/*
 * Lookup channel id (xmltvid) based on a string lookup
 * For Sky, the string is "sid,skynumber" which is unique
 * If the key is not found then return a manufactured channel id
 */
const char *skyxmltvid(const char *chanid, const char *provider)
{
    char *returnstring;

    if (use_chanidents && channelid_table) {
	const char *c = slookup(channelid_table, chanid);
	if (c)
	    return c;
    }
    asprintf(&returnstring, "%s.%s.dvb.guide", chanid, provider);
    return returnstring;
}
