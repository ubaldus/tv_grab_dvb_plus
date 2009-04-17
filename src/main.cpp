/*
 * main.c
 *
 * tv_grab_dvb_plus main routine
 *
 * Parse DVB, Freesat, Sky AU/IT/UK and MediaHighway EPG data
 * and generate an xmltv file
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

#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
#include <signal.h>
#include <poll.h>
#include <sys/ioctl.h>

#include <linux/dvb/dmx.h>
#include <linux/dvb/frontend.h>

#include "constants.h"
#include "filter.h"
#include "dvbepg.h"
#include "freesatepg.h"
#include "loadepg.h"
#include "lookup.h"
#include "chanid.h"
#include "log.h"

char *ProgName;

int adapter = 0;
int demuxno = 0;
int vdrmode = 0;
bool useshortxmlids = false;

int format = DATA_FORMAT_DVB;

int timeout = 10;
int time_offset = 0;
bool ignore_bad_dates = true;
bool ignore_updates = true;
bool use_chanidents = false;
bool print_stats = false;

char demux[32] = "/dev/dvb/adapter0/demux0";
char conf[1024] = "./conf";


/*
 * Print usage information
 */
static void usage()
{
    fprintf(stderr,
	    "usage: %s [-h] [-C] [-d] [-I] [-S] [-s] [-u] [-v] [-a adapter] [-c dir] [-f format] [-i file] [-O offset] [-o file] [-t timeout]\n\n"
	    "\t-h (--help)             output this help text\n"
	    "\t-C (--chanids)          use channel identifiers from file 'chanidents'\n"
	    "\t                        (default sidnumber.dvb.guide)\n"
	    "\t-I (--invalid-dates)    output invalid dates (default is to ignore them)\n"
	    "\t-S (--stats)            output runtime statistics (default false)\n"
	    "\t-s (--short-xml)        output short xml ids (default false)\n"
	    "\t-u (--updated-info)     output updated info (will result in repeated information) (default false)\n"
	    "\t-v (--vdr)              vdr output mode (default false)\n"
	    "\t-a (--adapter) adapter# change the adapter number (default 0)\n"
	    "\t-c (--conf)    dir      change the config directory (default ./conf)\n"
	    "\t-d (--debug)   level    output debug infoi (none|trace|debug|warning|error) (default error)\n"
	    "\t-d (--demux)   demux#   change the demux number (default 0)\n"
	    "\t-f (--format)  format   format of incoming data (dvb|freesat|sky|mhw1|mhw2) (default dvb)\n"
	    "\t-i (--input)   file     read from file/device instead of %s\n"
	    "\t-O (--offset)  offset   time offset in hours from -12 to 12 (default 0)\n"
	    "\t-o (--output)  file     write output to file (default stdout)\n"
	    "\t-t (--timeout) timeout  stop after timeout seconds of no new data (default 10)\n"
	    "\n", ProgName, demux);
    _exit(1);
}

/* 
 * Parse command line arguments
 */
static int do_options(int arg_count, char **arg_strings)
{
    static const struct option Long_Options[] = {
	{"adapter", 1, 0, 'a'},
	{"chanids", 0, 0, 'C'},
	{"conf", 1, 0, 'c'},
	{"debug", 1, 0, 'd'},
	{"format", 1, 0, 'f'},
	{"help", 0, 0, 'h'},
	{"invalid-dates", 0, 0, 'I'},
	{"input", 1, 0, 'i'},
	{"offset", 1, 0, 'O'},
	{"output", 1, 0, 'o'},
	{"short-xml", 0, 0, 's'},
	{"stats", 0, 0, 'S'},
	{"timeout", 1, 0, 't'},
	{"updated-info", 0, 0, 'u'},
	{"vdr", 0, 0, 'v'},
	{0, 0, 0, 0}
    };
    int Option_Index = 0;
    int fd;
    char *f;

    while (1) {
	int c = getopt_long(arg_count, arg_strings, "a:Cc:d:f:hIi:O:o:Sst:uv",
			    Long_Options, &Option_Index);
	if (c == EOF)
	    break;
	switch (c) {
	case 'a':
	    adapter = atoi(optarg);
	    sprintf(demux, "/dev/dvb/adapter%d/demux%d", adapter, demuxno);
	    break;
	case 'C':
	    use_chanidents = true;
	    break;
	case 'c':
	    strcpy(conf, optarg);
	    break;
	case 'D':
	    ignore_bad_dates = false;
	    break;
	case 'd':
	    log_level(optarg);
	    break;
	case 'f':
	    f = optarg;
	    if (strcasecmp(f, "dvb") == 0) {
		format = DATA_FORMAT_DVB;
	    } else if (strcasecmp(f, "freesat") == 0) {
		format = DATA_FORMAT_FREESAT;
	    } else if (strcasecmp(f, "sky") == 0) {
		format = DATA_FORMAT_SKYBOX;
	    } else if (strcasecmp(f, "mhw1") == 0) {
		format = DATA_FORMAT_MHW_1;
	    } else if (strcasecmp(f, "mhw2") == 0) {
		format = DATA_FORMAT_MHW_2;
	    } else {
		fprintf(stderr, "%s: invalid format \"%s\"\n", ProgName,
			f);
		usage();
	    }
	    break;
	case 'h':
	case '?':
	    usage();
	    break;
	case 'i':
	    sprintf(demux, "%s", optarg);
	    break;
	case 'O':
	    time_offset = atoi(optarg);
	    if ((time_offset < -12) || (time_offset > 12)) {
		fprintf(stderr, "%s: invalid time offset\n", ProgName);
		usage();
	    }
	    break;
	case 'o':
	    if ((fd =
		 open(optarg, O_CREAT | O_TRUNC | O_WRONLY, 0666)) < 0) {
		fprintf(stderr, "%s: can't write file %s\n", ProgName,
			optarg);
		usage();
	    }
	    dup2(fd, STDOUT_FILENO);
	    close(fd);
	    break;
	case 'S':
	    print_stats = true;
	    break;
	case 's':
	    useshortxmlids = true;
	    break;
	case 't':
	    timeout = atoi(optarg);
	    if (0 == timeout) {
		fprintf(stderr, "%s: invalid timeout value\n", ProgName);
		usage();
	    }
	    break;
	case 'u':
	    ignore_updates = false;
	    break;
	case 'v':
	    vdrmode = 1;
	    break;
	case 0:
	default:
	    fprintf(stderr,
		    "%s: unknown getopt error - returned code %02x\n",
		    ProgName, c);
	    _exit(1);
	}
    }
    return 0;
}

/* 
 * XML header
 */
static void header()
{
    if (!vdrmode) {
	printf("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	printf("<!DOCTYPE tv SYSTEM \"xmltv.dtd\">\n");
	printf("<tv generator-info-name=\"tv_grab_dvb_plus\">\n");
    }
}

/* 
 * XML footer
 */
static void footer()
{
    if (!vdrmode) {
	printf("</tv>\n");
    }
}

/* 
 * Exit hook: close xml tags
 */
static void finish_up(int)
{
    log_message(DEBUG, "\n");
    footer();
    exit(0);
}

/* 
 * Setup demuxer or open file as STDIN
 */
static int openInput(int format)
{
    int fd_epg, to;
    struct stat stat_buf;
    struct dmx_sct_filter_params sctFilterParams;
    struct pollfd ufd;

    if (!strcmp(demux, "-"))
	return 0;		// Read from STDIN, which is open al

    if ((fd_epg = open(demux, O_RDWR)) < 0) {
	perror("fd_epg DEVICE: ");
	return -1;
    }

    if (fstat(fd_epg, &stat_buf) < 0) {
	perror("fd_epg DEVICE: ");
	return -1;
    }
    if (S_ISCHR(stat_buf.st_mode)) {
	bool found = false;
	switch (format) {
	case DATA_FORMAT_DVB:
	    fprintf(stderr, "%s: set up DVB filter\n", ProgName);
	    add_filter(0x14, 0x70, 0xfc);	// TOT && TDT
	    add_filter(DVB_EIT_PID, 0x00, 0x00);
	    break;
	case DATA_FORMAT_FREESAT:
	    fprintf(stderr, "%s: set up Freesat filter\n", ProgName);
	    add_filter(0x14, 0x70, 0xfc);	// TOT && TDT
	    add_filter(FREESAT_EIT_PID, 0x00, 0x00);
	    break;
	case DATA_FORMAT_SKYBOX:
	    fprintf(stderr, "%s: set up Sky UK filter\n", ProgName);
	    add_filter(0x14, 0x70, 0xfc);	// TOT && TDT
	    add_filter(0x11, 0x4a, 0xff);
	    add_filter(0x30, 0xa0, 0xfc);
	    add_filter(0x31, 0xa0, 0xfc);
	    add_filter(0x32, 0xa0, 0xfc);
	    add_filter(0x33, 0xa0, 0xfc);
	    add_filter(0x34, 0xa0, 0xfc);
	    add_filter(0x35, 0xa0, 0xfc);
	    add_filter(0x36, 0xa0, 0xfc);
	    add_filter(0x37, 0xa0, 0xfc);
	    add_filter(0x40, 0xa8, 0xfc);
	    add_filter(0x41, 0xa8, 0xfc);
	    add_filter(0x42, 0xa8, 0xfc);
	    add_filter(0x43, 0xa8, 0xfc);
	    add_filter(0x44, 0xa8, 0xfc);
	    add_filter(0x45, 0xa8, 0xfc);
	    add_filter(0x46, 0xa8, 0xfc);
	    add_filter(0x47, 0xa8, 0xfc);
	    break;
	case DATA_FORMAT_MHW_1:
	    fprintf(stderr, "%s: set up MediaHighway 1 filter\n",
		    ProgName);
	    add_filter(0x14, 0x70, 0xfc);	// TOT && TDT
	    add_filter(0xd2, 0x90, 0xff);
	    add_filter(0xd3, 0x90, 0xff);
	    add_filter(0xd3, 0x91, 0xff);
	    add_filter(0xd3, 0x92, 0xff);
	    break;
	case DATA_FORMAT_MHW_2:
	    fprintf(stderr, "%s: set up MediaHighway 2 filter\n",
		    ProgName);
	    add_filter(0x14, 0x70, 0xfc);	// TOT && TDT
	    add_filter(0x231, 0xc8, 0xff);
	    add_filter(0x234, 0xe6, 0xff);
	    add_filter(0x236, 0x96, 0xff);
	    break;
	default:
	    fprintf(stderr, "%s: set up no filter!\n", ProgName);
	    break;
	}
	if (!start_filters(fd_epg)) {
	    close(fd_epg);
	    return -1;
	}

	for (to = timeout; to > 0; to--) {
	    int res;

	    ufd.fd = fd_epg;
	    ufd.events = POLLIN;
	    res = poll(&ufd, 1, 1000);
	    if (0 == res) {
		fprintf(stderr, ".");
		fflush(stderr);
		continue;
	    }
	    if (1 == res) {
		found = true;
		break;
	    }
	    fprintf(stderr, "%s: error polling for data\n", ProgName);
	    close(fd_epg);
	    return -1;
	}
	fprintf(stdout, "\n");
	if (!found) {
	    fprintf(stderr, "%s: timeout - try tuning to a multiplex?\n",
		    ProgName);
	    close(fd_epg);
	    return -1;
	}

	signal(SIGALRM, finish_up);
	alarm(timeout);
    } else {
	// disable alarm timeout for normal files
	timeout = 0;
    }

    dup2(fd_epg, STDIN_FILENO);
    close(fd_epg);

    return 0;
}

/* 
 * Main function
 */
int main(int argc, char **argv) {
    char *chanidfile;

    /* Remove path from command */
    ProgName = strrchr(argv[0], '/');
    if (ProgName == NULL)
	ProgName = argv[0];
    else
	ProgName++;
    /* Process command line arguments */
    do_options(argc, argv);

    /* Load lookup tables. */
    if (use_chanidents) {
	switch (format) {
	case DATA_FORMAT_DVB:
	    asprintf(&chanidfile, "%s/%s.dvb", conf, CHANIDENTS);
	    break;
	case DATA_FORMAT_FREESAT:
	    asprintf(&chanidfile, "%s/%s.freesat", conf, CHANIDENTS);
	    break;
	case DATA_FORMAT_SKYBOX:
	    asprintf(&chanidfile, "%s/%s.sky", conf, CHANIDENTS);
	    break;
	case DATA_FORMAT_MHW_1:
	    asprintf(&chanidfile, "%s/%s.mhw1", conf, CHANIDENTS);
	    break;
	case DATA_FORMAT_MHW_2:
	    asprintf(&chanidfile, "%s/%s.mhw2", conf, CHANIDENTS);
	    break;
	default:
	    fprintf(stderr, "%s: no format specified!\n", ProgName);
	    asprintf(&chanidfile, "%s/%s", conf, CHANIDENTS);
	    break;
	}
        if (use_chanidents && !load_channel_table(chanidfile)) {
	    fprintf(stderr, "error loading %s, continuing.\n", chanidfile);
	}
    }

    header();
    switch (format) {
    case DATA_FORMAT_DVB:
	if (openInput(format) != 0) {
	    fprintf(stderr,
		    "%s: unable to get event data from multiplex.\n",
		    ProgName);
	    exit(1);
	}
	readEventTables(format);
	break;
    case DATA_FORMAT_FREESAT:
	if (openInput(format) != 0) {
	    fprintf(stderr,
		    "%s: unable to get event data from multiplex.\n",
		    ProgName);
	    exit(1);
	}
	readEventTables(format);
	break;
    case DATA_FORMAT_SKYBOX:
	EPGGrabber epgGrabber;
	epgGrabber.Grab();
	break;
    }
    footer();

    return 0;
}
