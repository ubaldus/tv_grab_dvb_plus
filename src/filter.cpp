/*
 * filter.cpp
 *
 * Routines to set up DVB PID filters.
 *
 * Only used by the DVB./Freesat code, but should be used by the Sky code
 * as well.
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

#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/dvb/frontend.h>
#include <linux/dvb/dmx.h>

#include "constants.h"
#include "filter.h"
#include "log.h"

struct filter_t filters[MAX_FILTERS];

int no_filters = 0;

extern char *ProgName;

void add_filter(unsigned short int pid, unsigned char tid, unsigned char mask)
{
    if (no_filters >= MAX_FILTERS) {
	log_message(ERROR,
		"numbers of filters is greater than %i, can't add filter pid=0x%04x tid=0x%02x mask=0x%02x",
		MAX_FILTERS, pid, tid, mask);
	return;
    }
    filters[no_filters].pid = pid;
    filters[no_filters].tid = tid;
    filters[no_filters].mask = mask;
    no_filters++;
}

int start_filters(int fd)
{
    struct dmx_sct_filter_params sctFilterParams;
    int id;

    log_message(DEBUG, "starting %d filter(s)\n", no_filters);
    memset(&sctFilterParams, 0, sizeof(sctFilterParams));
    sctFilterParams.timeout = 0;
    sctFilterParams.flags = DMX_IMMEDIATE_START;
    for (id = 0; id < no_filters; id++) {
        sctFilterParams.pid = filters[id].pid;
	sctFilterParams.filter.filter[0] = filters[id].tid;
	sctFilterParams.filter.mask[0] = filters[id].mask;
	if (ioctl(fd, DMX_SET_FILTER, &sctFilterParams) < 0) {
	    log_message(ERROR,
		    "can't start filter pid=0x%04x tid=0x%02x mask=0x%02x\n",
		    filters[id].pid, filters[id].tid, filters[id].mask);
            return FAILURE;
	}
    }
    return SUCCESS;
}
