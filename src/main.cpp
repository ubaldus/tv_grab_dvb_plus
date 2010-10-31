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
#include "dvb_epg.h"
#include "freesat_epg.h"
#include "sky_epg.h"
#include "lookup.h"
#include "chanid.h"
#include "stats.h"
#include "log.h"
#include "freesat_test.h" // only required during test mode
#include "tuner.h"

char *ProgName;

int adapter = 0;
int demuxno = 0;

int format = DATA_FORMAT_DVB;
int sky_country;

int timeout = 10;
int days = 0; // 0=as much as possible, 1=1 day, 2=2 days,...
int day_offset = 0; // 0=today, 1=tomorrow,...
int time_offset = 0;

time_t start_of_period; // only print programmes that fall between this...
time_t end_of_period; // ...and this

bool ignore_bad_dates = true;
bool ignore_updates = true;
bool use_shortxmlids = false;
bool use_chanidents = false;
bool print_stats = false;

char demux[32] = "/dev/dvb/adapter0/demux0";
char conf[1024] = "/usr/share/xmltv/tv_grab_dvb_plus";

const char * defaultxmltvidformat = "%s.dvb.guide";
const char * defaultfreesatxmltvidformat = "%s.dvb.guide";
const char * defaultskyxmltvidformat = "%s.%p.dvb.guide";
char * xmltvidformat = NULL;

#define TUNECONF "tune.conf"
char* tuneconf = NULL;
int frequency = -1;

cFilter * filters = NULL;

#define SECONDS_PER_DAY 86400
#define DAYS_PER_YEAR 365

/*
 * Print usage information
 */
static void usage() {
	fprintf(
			stderr,
			"usage: %s [-h] [-c] [-D] [-d] [-I] [-k] [-p] [-q] [-S] [-u] [-x] [-a adapter] [-C file] [-f format] [-F freq] [-h hours] [-i file] [-n days] [-O days] [-o file] [-s sharedir] [-t timeout]\n\n"
				"\t-h (--help)               output this help text\n"
				"\t-c (--chanids)            use channel identifiers from file 'chanidents'\n"
				"\t                          (default sidnumber.dvb.guide)\n"
				"\t-D (--description)        output a description which identified the grabber\n"
				"\t-I (--invalid-dates)      output invalid dates (default is to ignore them)\n"
				"\t-k (--capabilities)       output a list of all the capabilities that this grabber supports\n"
				"\t-p (--preferredmethod)    output how the grabber prefers to be called\n"
				"\t-q (--quiet)              supress all progress information. only output error messages\n"
				"\t                          (same as -d error)\n"
				"\t-S (--stats)              output runtime statistics (default false)\n"
				"\t-x (--short-xml)          output short xml ids (default false)\n"
				"\t-X (--xmltvidformat)      specify xmltvid format as format string, specifiers:\n"
				"\t                            %%s - sid\n"
				"\t                            %%c - channel number\n"
				"\t                            %%n - channel name\n"
				"\t                            %%p - provider name\n"
				"\t-u (--updated-info)       output updated info (will result in repeated information) (default false)\n"
				"\t-a (--adapter) adapter#   change the adapter number (default 0)\n"
				"\t-F (--freq)    freq       frequency to tune in Hz (default don't tune)\n"
				"\t-C (--tuneconf) file      file to load for tuning details, from w_scan or dvbtools (default sharedir/tune.conf\n"
				"\t-d (--debug)   level      output debug infoi (none|trace|debug|warning|error) (default error)\n"
				"\t-f (--format)  format     format of incoming data (dvb|freesat|skyXX|mhw1|mhw2) (default dvb)\n"
				"\t                          (XX can be AU, IT or UK - case insensitive)\n"
				"\t-H (--hours)   hours      offset time offset in hours from -12 to 12 (default 0)\n"
				"\t-i (--input)   file       read from file/device instead of %s\n"
				"\t-n (--days)    days       supply data for this number of days (defaults 0 means as much as possible)\n"
				"\t-O (--offset)  days       supply data for today plus this number of days (default 0 means today)\n"
				"\t-o (--output)  file       write output to file (default stdout)\n"
				"\t-s (--share)   sharedir   specify the location of the metatdata for the grabber\n"
				"\t                          (default %s)\n"
				"\t-t (--timeout) timeout    stop after timeout seconds of no new data (default 10)\n"
				"\n", ProgName, demux, conf);
	exit(1);
}

/*
 * Print description
 */
static void description() {
	fprintf(stdout, "EIT data from DVB, Freesat, Sky or MediaHighway stream\n");
	exit(0);
}

/*
 * Print capabilities
 */
static void capabilities() {
	fprintf(stdout, "baseline\n");
	fprintf(stdout, "share\n");
	fprintf(stdout, "preferredmethod\n");
	exit(0);
}

/*
 * Print preferred calling method
 */
static void preferredmethod() {
	fprintf(stdout, "allatonce\n");
	exit(0);
}

/*
 * Parse command line arguments
 */
static int do_options(int arg_count, char **arg_strings) {
	static const struct option Long_Options[] = {
			{"adapter", 1, 0, 'a'},
			{"chanids", 0, 0, 'c'},
			{"description", 0, 0, 'D'},
			{"debug", 1, 0, 'd'},
			{"freq", 1, 0, 'F'},
			{"format", 1, 0, 'f'},
			{"hours", 1, 0, 'H'},
			{"help", 0, 0, 'h'},
			{"invalid-dates", 0, 0, 'I'},
			{"input", 1, 0, 'i'},
			{"capabilities", 0, 0, 'k'},
			{"days", 1, 0, 'n'},
			{"offset", 1, 0, 'O'},
			{"output", 1, 0, 'o'},
			{"preferredmethod", 0, 0, 'p'},
			{"quiet", 0, 0, 'q'},
			{"stats", 0, 0, 'S'},
			{"sharedir", 1, 0, 's'},
			{"timeout", 1, 0, 't'},
			{"tunefile", 1, 0, 'C'},
			{"updated-info", 0, 0, 'u'},
			{"short-xml", 0, 0, 'x'},
			{"xmltvidformat", 1, 0, 'X'},
			{0, 0, 0, 0}
	};
    int Option_Index = 0;
	int fd;

	xmltvidformat = strdup(defaultxmltvidformat);

	while (1) {
		int c = getopt_long(arg_count, arg_strings,
				"a:C:cDd:F:f:H:hIi:kn:O:o:pqSs:Tt:uxX:", Long_Options, &Option_Index);
		if (c == EOF)
			break;
		switch (c) {
		case 'a':
			adapter = atoi(optarg);
			sprintf(demux, "/dev/dvb/adapter%d/demux%d", adapter, demuxno);
			break;
		case 'C':
			if (strchr(optarg, '/') != NULL)
				asprintf(&tuneconf, "%s", optarg);
			else
				asprintf(&tuneconf, "%s/%s", conf, optarg);
			break;
		case 'c':
			use_chanidents = true;
			break;
		case 'D':
			description();
			break;
		case 'd':
			log_level(optarg);
			break;
		case 'F':
			frequency = atoi(optarg);
			break;
		case 'f':
			if (strcasecmp(optarg, "dvb") == 0) {
				format = DATA_FORMAT_DVB;
			} else if (strcasecmp(optarg, "freesat") == 0) {
				format = DATA_FORMAT_FREESAT;
			} else if (strcasecmp(optarg, "skyau") == 0) {
				format = DATA_FORMAT_SKY_AU;
			} else if (strcasecmp(optarg, "skyit") == 0) {
				format = DATA_FORMAT_SKY_IT;
			} else if (strcasecmp(optarg, "skyuk") == 0) {
				format = DATA_FORMAT_SKY_UK;
			} else if (strcasecmp(optarg, "mhw1") == 0) {
				format = DATA_FORMAT_MHW_1;
			} else if (strcasecmp(optarg, "mhw2") == 0) {
				format = DATA_FORMAT_MHW_2;
			} else {
				log_message(ERROR, "invalid format \"%s\"", optarg);
				usage();
			}
			break;
		case 'H':
			time_offset = atoi(optarg);
			if ((time_offset < -12) || (time_offset > 12)) {
				log_message(ERROR, "invalid time offset");
				usage();
			}
			break;
		case 'h':
		case '?':
			usage();
			break;
		case 'I':
			ignore_bad_dates = false;
			break;
		case 'i':
			sprintf(demux, "%s", optarg);
			break;
		case 'k':
			capabilities();
			break;
		case 'n':
			days = atoi(optarg);
			if (days < 0) {
				log_message(ERROR, "invalid number of days");
				usage();
			}
			break;
		case 'O':
			day_offset = atoi(optarg);
			if (day_offset < 0) {
				log_message(ERROR, "invalid day offset");
				usage();
			}
			break;
		case 'o':
			if ((fd = open(optarg, O_CREAT | O_TRUNC | O_WRONLY, 0666)) < 0) {
				log_message(ERROR, "can't write to file %s", optarg);
				usage();
			}
			dup2(fd, STDOUT_FILENO);
			close(fd);
			break;
		case 'p':
			preferredmethod();
			break;
		case 'q':
			log_level((char*)"ERROR");
			break;
		case 'S':
			print_stats = true;
			break;
		case 's':
			strcpy(conf, optarg);
			break;
		case 'T': // undocumented
			freesat_test();
			_exit(1);
			break;
		case 't':
			timeout = atoi(optarg);
			if (0 == timeout) {
				log_message(ERROR, "invalid timeout value");
				usage();
			}
			break;
		case 'u':
			ignore_updates = false;
			break;
		case 'x':
			use_shortxmlids = true;
			break;
		case 'X':
			asprintf(&xmltvidformat, "%s", optarg);
			break;
		case 0:
		default:
			log_message(ERROR, "unknown getopt error - returned code %02x", c);
			_exit(1);
		}
	}
	return 0;
}

void set_start_and_end_times(void) {
	char date_buffer[256];
	time_t now;
	struct tm *tmCurrent;
	time_t midnight_today;

	now = time(NULL);
	tmCurrent = gmtime(&now);
	tmCurrent->tm_hour = 0;
	tmCurrent->tm_min = 0;
	tmCurrent->tm_sec = 0;
	midnight_today = mktime(tmCurrent);
	start_of_period = midnight_today + (day_offset * SECONDS_PER_DAY);
	if (days != 0) {
		end_of_period = start_of_period + (days * SECONDS_PER_DAY);
	} else {
		end_of_period = start_of_period + (DAYS_PER_YEAR * SECONDS_PER_DAY); // should be long enough!
	}
	strftime(date_buffer, sizeof(date_buffer), "\"%Y%m%d%H%M%S %z\"",
			localtime(&midnight_today));
	log_message(DEBUG, " midnight today=%s", date_buffer);
	strftime(date_buffer, sizeof(date_buffer), "\"%Y%m%d%H%M%S %z\"",
			localtime(&start_of_period));
	log_message(DEBUG, "start of period=%s", date_buffer);
	strftime(date_buffer, sizeof(date_buffer), "\"%Y%m%d%H%M%S %z\"",
			localtime(&end_of_period));
	log_message(DEBUG, "  end of period=%s", date_buffer);
}

/*
 * XML header
 */
static void header() {
	printf("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	printf("<!DOCTYPE tv SYSTEM \"xmltv.dtd\">\n");
	printf("<tv generator-info-name=\"tv_grab_dvb_plus\">\n");
}

/*
 * XML footer
 */
static void footer() {
	printf("</tv>\n");
}

/*
 * Exit hook: close xml tags
 */
static void finish_up(int) {
	if (filters)
		delete filters;
	footer();
	exit(0);
}

/*
 * Setup demuxer or open file as STDIN
 */
static int openInput(int format) {
	int fd_epg;
	struct stat stat_buf;

	if (!strcmp(demux, "-"))
		return 0; // Read from STDIN, which is open al

	if ((fd_epg = open(demux, O_RDWR)) < 0) {
		perror("fd_epg DEVICE: ");
		return -1;
	}

	if (fstat(fd_epg, &stat_buf) < 0) {
		close(fd_epg);
		perror("fd_epg DEVICE: ");
		return -1;
	}
	close(fd_epg);
	if (S_ISCHR(stat_buf.st_mode)) {
		filters = new cFilter(demux);
		switch (format) {
		case DATA_FORMAT_DVB:
			log_message(TRACE, "set up DVB filter");
			filters->AddFilter(0x14, 0x70, 0xfc); // TOT && TDT
			filters->AddFilter(0x11, 0x42, 0xff); // SDT
			filters->AddFilter(DVB_EIT_PID, 0x00, 0x00);
			break;
		case DATA_FORMAT_FREESAT:
			log_message(TRACE, "set up Freesat filter");
			filters->AddFilter(0x14, 0x70, 0xfc); // TOT && TDT
			filters->AddFilter(0x11, 0x42, 0xff); // SDT
			filters->AddFilter(FREESAT_EIT_PID, 0x00, 0x00);
			break;
		case DATA_FORMAT_SKY_AU:
		case DATA_FORMAT_SKY_IT:
		case DATA_FORMAT_SKY_UK:
			log_message(TRACE, "set up Sky filter");
			filters->AddFilter(0x14, 0x70, 0xfc); // TOT && TDT
			filters->AddFilter(0x11, 0x4a, 0xff);
			filters->AddFilter(0x30, 0xa0, 0xfc);
			filters->AddFilter(0x31, 0xa0, 0xfc);
			filters->AddFilter(0x32, 0xa0, 0xfc);
			filters->AddFilter(0x33, 0xa0, 0xfc);
			filters->AddFilter(0x34, 0xa0, 0xfc);
			filters->AddFilter(0x35, 0xa0, 0xfc);
			filters->AddFilter(0x36, 0xa0, 0xfc);
			filters->AddFilter(0x37, 0xa0, 0xfc);
			filters->AddFilter(0x40, 0xa8, 0xfc);
			filters->AddFilter(0x41, 0xa8, 0xfc);
			filters->AddFilter(0x42, 0xa8, 0xfc);
			filters->AddFilter(0x43, 0xa8, 0xfc);
			filters->AddFilter(0x44, 0xa8, 0xfc);
			filters->AddFilter(0x45, 0xa8, 0xfc);
			filters->AddFilter(0x46, 0xa8, 0xfc);
			filters->AddFilter(0x47, 0xa8, 0xfc);
			break;
		case DATA_FORMAT_MHW_1:
			log_message(TRACE, "set up MediaHighway 1 filter");
			filters->AddFilter(0x14, 0x70, 0xfc); // TOT && TDT
			filters->AddFilter(0xd2, 0x90, 0xff);
			filters->AddFilter(0xd3, 0x90, 0xff);
			filters->AddFilter(0xd3, 0x91, 0xff);
			filters->AddFilter(0xd3, 0x92, 0xff);
			break;
		case DATA_FORMAT_MHW_2:
			log_message(TRACE, "set up MediaHighway 2 filter");
			filters->AddFilter(0x14, 0x70, 0xfc); // TOT && TDT
			filters->AddFilter(0x231, 0xc8, 0xff);
			filters->AddFilter(0x234, 0xe6, 0xff);
			filters->AddFilter(0x236, 0x96, 0xff);
			break;
		default:
			log_message(ERROR, "did not set up any filter!");
			break;
		}

		signal(SIGALRM, finish_up);
		alarm(timeout);
	} else {
		// disable alarm timeout for normal files
		timeout = 0;
	}

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
	set_start_and_end_times();

	/* Load lookup tables. */
	if (use_chanidents) {
		switch (format) {
		case DATA_FORMAT_DVB:
			asprintf(&chanidfile, "%s/%s.dvb", conf, CHANIDENTS);
			break;
		case DATA_FORMAT_FREESAT:
			asprintf(&chanidfile, "%s/%s.freesat", conf, CHANIDENTS);
			break;
		case DATA_FORMAT_SKY_AU:
			asprintf(&chanidfile, "%s/%s.skyau", conf, CHANIDENTS);
			break;
		case DATA_FORMAT_SKY_IT:
			asprintf(&chanidfile, "%s/%s.skyit", conf, CHANIDENTS);
			break;
		case DATA_FORMAT_SKY_UK:
			asprintf(&chanidfile, "%s/%s.skyuk", conf, CHANIDENTS);
			break;
		case DATA_FORMAT_MHW_1:
			asprintf(&chanidfile, "%s/%s.mhw1", conf, CHANIDENTS);
			break;
		case DATA_FORMAT_MHW_2:
			asprintf(&chanidfile, "%s/%s.mhw2", conf, CHANIDENTS);
			break;
		default:
			log_message(ERROR, "no format specified");
			asprintf(&chanidfile, "%s/%s", conf, CHANIDENTS);
			break;
		}
		if (use_chanidents && !load_channel_table(chanidfile)) {
			log_message(WARNING, "error loading %s, continuing", chanidfile);
		}
	}

	if (frequency > 0)
	{
		if (tuneconf == NULL)
			asprintf(&tuneconf, "%s/%s", conf, TUNECONF);
		if (!tune(frequency, adapter, 0))
		{
			log_message(ERROR, "failed to tune to %s, aborting", frequency);
			free(tuneconf);
			exit(2);
		}
		free(tuneconf);
	}

	header();
	switch (format) {
	case DATA_FORMAT_DVB:
		if (xmltvidformat == NULL)
			xmltvidformat = strdup(defaultxmltvidformat);
		if (openInput(format) != 0) {
			log_message(ERROR, "unable to get event data from multiplex");
			exit(1);
		}
		readEventTables(format, filters);
		writeChannels(format);
		break;
	case DATA_FORMAT_FREESAT:
		if (xmltvidformat == NULL)
			xmltvidformat = strdup(defaultfreesatxmltvidformat);
		if (openInput(format) != 0) {
			log_message(ERROR, "unable to get event data from multiplex");
			exit(1);
		}
		readEventTables(format, filters);
		writeChannels(format);
		break;
	case DATA_FORMAT_SKY_AU:
	case DATA_FORMAT_SKY_IT:
	case DATA_FORMAT_SKY_UK:
		if (xmltvidformat == NULL)
			xmltvidformat = strdup(defaultskyxmltvidformat);
		EPGGrabber epgGrabber;
		epgGrabber.Grab();
		break;
	}
	if (filters)
		delete filters;
	footer();
	return 0;
}
