#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/dvb/frontend.h>
#include <linux/dvb/dmx.h>

#include "constants.h"
#include "filter.h"

struct filter_t filters[MAX_FILTERS];

int no_filters = 0;

extern char *ProgName;

void add_filter(unsigned short int pid, unsigned char tid, unsigned char mask)
{
    if (no_filters >= MAX_FILTERS) {
	fprintf(stderr,
		"%s: error, numbers of filters is greater than %i, can't add filter pid=0x%04x tid=0x%02x mask=0x%02x",
		ProgName, MAX_FILTERS, pid, tid, mask);
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

    fprintf(stderr, "%s: starting %d filter(s)\n", ProgName, no_filters);
    memset(&sctFilterParams, 0, sizeof(sctFilterParams));
    sctFilterParams.timeout = 0;
    sctFilterParams.flags = DMX_IMMEDIATE_START;
    for (id = 0; id < no_filters; id++) {
        sctFilterParams.pid = filters[id].pid;
	sctFilterParams.filter.filter[0] = filters[id].tid;
	sctFilterParams.filter.mask[0] = filters[id].mask;
	if (ioctl(fd, DMX_SET_FILTER, &sctFilterParams) < 0) {
	    fprintf(stderr,
		    "%s: error, can't start filter pid=0x%04x tid=0x%02x mask=0x%02x\n",
		    ProgName, filters[id].pid, filters[id].tid, filters[id].mask);
            return FAILURE;
	}
    }
    return SUCCESS;
}
