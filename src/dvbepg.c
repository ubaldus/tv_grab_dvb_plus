/*
 * dvbepg.c
 *
 * Parse DVB or Freesat EPG data and generate an xmltv file.
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

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>

#include "sitables.h"
#include "dvbepg.h"
#include "dvbinfo.h"
#include "dvbtext.h"
#include "lookup.h"
#include "crc32.h"
#include "chanid.h"
#include "stats.h"
#include "log.h"

extern char *ProgName;

extern int timeout;
extern int time_offset;
extern bool ignore_bad_dates;
extern bool ignore_updates;

static int packet_count = 0;
static int programme_count = 0;
static int update_count = 0;
static int crcerr_count = 0;
static int invalid_date_count = 0;

typedef struct chninfo {
    struct chninfo *next;
    int sid;
    int eid;
    int ver;
} chninfo_t;

static struct chninfo *channels;

/*
 * Print progress indicator
 */
static void status()
{
    log_raw_message(DEBUG,
		    "\rStatus: %d pkts, %d prgms, %d updates, %d invalid, %d CRC err",
		    packet_count, programme_count, update_count, invalid_date_count, crcerr_count);
}

/* Parse 0x4D Short Event Descriptor */
enum ER { TITLE, SUB_TITLE };
static void parseEventDescription(void *data, enum ER round)
{
    assert(GetDescriptorTag(data) == 0x4D);
    struct descr_short_event *evtdesc = CastShortEventDescriptor(data);
    char evt[256];
    char dsc[256];

    int evtlen = evtdesc->event_name_length;
    if (round == TITLE) {
	if (!evtlen)
	    return;
	strncpy(evt, (char *) &evtdesc->data, evtlen);
	evt[evtlen] = '\0';
	printf("\t<title lang=\"%s\">%s</title>\n", lookup_language(&evtdesc->lang_code1), convert_text(evt));
	return;
    }

    if (round == SUB_TITLE) {
	int dsclen = evtdesc->data[evtlen];
	strncpy(dsc, (char *) &evtdesc->data[evtlen + 1], dsclen);
	dsc[dsclen] = '\0';

	if (*dsc) {
	    char *d = convert_text(dsc);
	    if (d && *d)
		printf("\t<sub-title lang=\"%s\">%s</sub-title>\n", lookup_language(&evtdesc->lang_code1), d);
	}
    }
}

/* Parse 0x4E Extended Event Descriptor */
static void parseLongEventDescription(void *data)
{
    assert(GetDescriptorTag(data) == 0x4E);
    struct descr_extended_event *levt = CastExtendedEventDescriptor(data);
    char dsc[256];
    bool non_empty = (levt->descriptor_number || levt->last_descriptor_number || levt->length_of_items || levt->data[0]);

    if (non_empty && levt->descriptor_number == 0)
	printf("\t<desc lang=\"%s\">", lookup_language(&levt->lang_code1));

    void *p = &levt->data;
    void *data_end = CastExtendedEventDescriptor(data) + DESCR_GEN_LEN + GetDescriptorLength(data);
    while (p < (void *) levt->data + levt->length_of_items) {
	struct item_extended_event *name = p;
	int name_len = name->item_description_length;
	assert(p + ITEM_EXTENDED_EVENT_LEN + name_len < data_end);
	strncpy(dsc, (char *) &name->data, name_len);
	dsc[name_len] = '\0';
	printf("%s: ", convert_text(dsc));

	p += ITEM_EXTENDED_EVENT_LEN + name_len;

	struct item_extended_event *value = p;
	int value_len = value->item_description_length;
	assert(p + ITEM_EXTENDED_EVENT_LEN + value_len < data_end);
	strncpy(dsc, (char *) &value->data, value_len);
	dsc[value_len] = '\0';
	printf("%s; ", convert_text(dsc));

	p += ITEM_EXTENDED_EVENT_LEN + value_len;
    }
    struct item_extended_event *text = p;
    int len = text->item_description_length;
    if (non_empty && len) {
	strncpy(dsc, (char *) &text->data, len);
	dsc[len] = '\0';
	printf("%s", convert_text(dsc));
    }
    //printf("/%d/%d/%s", levt->descriptor_number, levt->last_descriptor_number, convert_text(dsc));
    if (non_empty && levt->descriptor_number == levt->last_descriptor_number)
	printf("</desc>\n");
}

/* Parse 0x50 Component Descriptor.  {{{
   video is a flag, 1=> output the video information, 0=> output the
   audio information.  seen is a pointer to a counter to ensure we
   only output the first one of each (XMLTV can't cope with more than
   one) */
enum CR { LANGUAGE, VIDEO, AUDIO, SUBTITLES };
static void parseComponentDescription(void *data, enum CR round, int *seen)
{
    assert(GetDescriptorTag(data) == 0x50);
    struct descr_component *dc = CastComponentDescriptor(data);
    char buf[256];

    int len = dc->descriptor_length;
    strncpy(buf, (char *) &dc->data, len);
    buf[len] = '\0';

    switch (dc->stream_content) {
    case 0x01:			// Video Info
	if (round == VIDEO && !*seen) {
	    //if ((dc->component_type-1)&0x08) //HD TV
	    //if ((dc->component_type-1)&0x04) //30Hz else 25
	    printf("\t<video>\n");
	    printf("\t\t<aspect>%s</aspect>\n", lookup_aspect((dc->component_type - 1) & 0x03));
	    printf("\t</video>\n");
	    (*seen)++;
	}
	break;
    case 0x02:			// Audio Info
	if (round == AUDIO && !*seen) {
	    printf("\t<audio>\n");
	    printf("\t\t<stereo>%s</stereo>\n", lookup_audio(dc->component_type));
	    printf("\t</audio>\n");
	    (*seen)++;
	}
	if (round == LANGUAGE) {
	    if (!*seen) {
		printf("\t<language>%s</language>\n", lookup_language(&dc->lang_code1));
	    } else {
		//printf("\t<!--language>%s</language-->\n", lookup_language(&dc->lang_code1));
	    }
	    (*seen)++;
	}
	break;
	//case 0x03: // Teletext Info
	//if (round == SUBTITLES) {
	// FIXME: is there a suitable XMLTV output for this?
	// if ((dc->component_type)&0x10) //subtitles
	// if ((dc->component_type)&0x20) //subtitles for hard of hearing
	//printf("\t<subtitles type=\"teletext\">\n");
	//printf("\t\t<language>%s</language>\n", lookup_language(&dc->lang_code1));
	//printf("\t</subtitles>\n");
	//}
	//break;
	// case 0x04: // AC3 info
    }
}

static inline void set_bit(int *bf, int b)
{
    int i = b / 8 / sizeof(int);
    int s = b % (8 * sizeof(int));
    bf[i] |= (1 << s);
}

static inline bool get_bit(int *bf, int b)
{
    int i = b / 8 / sizeof(int);
    int s = b % (8 * sizeof(int));
    return bf[i] & (1 << s);
}

/* Parse 0x54 Content Descriptor */
static void parseContentDescription(void *data)
{
    assert(GetDescriptorTag(data) == 0x54);
    struct descr_content *dc = CastContentDescriptor(data);
    int once[256 / 8 / sizeof(int)] = { 0, };
    void *p;
    for (p = &dc->data; p < data + dc->descriptor_length; p += NIBBLE_CONTENT_LEN) {
	struct nibble_content *nc = p;
	int c1 = (nc->content_nibble_level_1 << 4) + nc->content_nibble_level_2;
#ifdef CATEGORY_UNKNOWN
	int c2 = (nc->user_nibble_1 << 4) + nc->user_nibble_2;
#endif
	if (c1 > 0 && !get_bit(once, c1)) {
	    set_bit(once, c1);
	    char *c = lookup_description(c1);
	    if (c)
		if (c[0])
		    printf("\t<category>%s</category>\n", c);
#ifdef CATEGORY_UNKNOWN
		else
		    printf("\t<!--category>%s %02X %02X</category-->\n", c + 1, c1, c2);
	    else
		printf("\t<!--category>%02X %02X</category-->\n", c1, c2);
#endif
	}
	// This is weird in the uk, they use user but not content, and almost the same values
    }
}

/* Parse 0x55 Rating Descriptor */
static void parseRatingDescription(void *data)
{
    assert(GetDescriptorTag(data) == 0x55);
    struct descr_parental_rating *pr = CastParentalRatingDescriptor(data);
    void *p;
    for (p = &pr->data; p < data + pr->descriptor_length; p += PARENTAL_RATING_ITEM_LEN) {
	struct parental_rating_item *pr = p;
	switch (pr->rating) {
	case 0x00:		/*undefined */
	    break;
	case 0x01 ... 0x0F:
	    printf("\t<rating system=\"dvb\">\n");
	    printf("\t\t<value>%d</value>\n", pr->rating + 3);
	    printf("\t</rating>\n");
	    break;
	case 0x10 ... 0xFF:	/*broadcaster defined */
	    break;
	}
    }
}

/* Parse 0x5F Private Data Specifier */
static int parsePrivateDataSpecifier(void *data)
{
    assert(GetDescriptorTag(data) == 0x5F);
    return GetPrivateDataSpecifier(data);
}

static void parseUnknown(void *data)
{
    struct descr_component *dc = data;
    char buf[256];

    int len = GetDescriptorLength(data);
    strncpy(buf, (char *) &dc->data, len);
    buf[len] = '\0';

    //printf("\t<!-- unknown id=\"%x\" len=\"%d\" value=\"%s\" -->\n", GetDescriptorTag(data), len, buf);
}

/* Parse Descriptor
 * Tags should be output in this order:

'title', 'sub-title', 'desc', 'credits', 'date', 'category', 'language',
'orig-language', 'length', 'icon', 'url', 'country', 'episode-num',
'video', 'audio', 'previously-shown', 'premiere', 'last-chance',
'new', 'subtitles', 'rating', 'star-rating'
*/
static void parseDescription(void *data, size_t len)
{
    int round, pds = 0;

    for (round = 0; round < 8; round++) {
	int seen = 0;		// no title/language/video/audio/subtitles seen in this round
	void *p;
	for (p = data; p < data + len; p += DESCR_GEN_LEN + GetDescriptorLength(p)) {
	    struct descr_gen *desc = p;
	    switch (GetDescriptorTag(desc)) {
	    case 0:
		break;
	    case 0x4D:		//short evt desc, [title] [sub-title]
		// there can be multiple language versions of these
		if (round == 0) {
		    parseEventDescription(desc, TITLE);
		} else if (round == 1)
		    parseEventDescription(desc, SUB_TITLE);
		break;
	    case 0x4E:		//long evt descriptor [desc]
		if (round == 2)
		    parseLongEventDescription(desc);
		break;
	    case 0x50:		//component desc [language] [video] [audio] [subtitles]
		if (round == 4)
		    parseComponentDescription(desc, LANGUAGE, &seen);
		else if (round == 5)
		    parseComponentDescription(desc, VIDEO, &seen);
		else if (round == 6)
		    parseComponentDescription(desc, AUDIO, &seen);
		else if (round == 7)
		    parseComponentDescription(desc, SUBTITLES, &seen);
		break;
	    case 0x53:		// CA Identifier Descriptor
		break;
	    case 0x54:		// content desc [category]
		if (round == 3)
		    parseContentDescription(desc);
		break;
	    case 0x55:		// Parental Rating Descriptor [rating]
		if (round == 7)
		    parseRatingDescription(desc);
		break;
	    case 0x5f:		// Private Data Specifier
		pds = parsePrivateDataSpecifier(desc);
		break;
	    case 0x64:		// Data broadcast desc - Text Desc for Data components
		break;
	    case 0x69:		// Programm Identification Label
		break;
	    case 0x81:		// TODO ???
		if (pds == 5)	// ARD_ZDF_ORF
		    break;
	    case 0x82:		// VPS (ARD, ZDF, ORF)
		if (pds == 5)	// ARD_ZDF_ORF
		    // TODO: <programme @vps-start="???">
		    break;
	    case 0x4F:		// Time Shifted Event
	    case 0x52:		// Stream Identifier Descriptor
	    case 0x5E:		// Multi Lingual Component Descriptor
	    case 0x83:		// Logical Channel Descriptor (some kind of news-ticker on ARD-MHP-Data?)
	    case 0x84:		// Preferred Name List Descriptor
	    case 0x85:		// Preferred Name Identifier Descriptor
	    case 0x86:		// Eacem Stream Identifier Descriptor
	    default:
		if (round == 0) {
		    parseUnknown(desc);
		}
	    }
	}
    }
}

/* Check that program has at least a title as is required by xmltv.dtd */
static bool validateDescription(void *data, size_t len)
{
    void *p;
    for (p = data; p < data + len; p += DESCR_GEN_LEN + GetDescriptorLength(p)) {
	struct descr_gen *desc = p;
	if (GetDescriptorTag(desc) == 0x4D) {
	    struct descr_short_event *evtdesc = p;
	    // make sure that title isn't empty
	    if (evtdesc->event_name_length)
		return true;
	}
    }
    return false;
}

/* Use the routine specified in ETSI EN 300 468 V1.4.1, {{{
 * "Specification for Service Information in Digital Video Broadcasting"
 * to convert from Modified Julian Date to Year, Month, Day. */
static void parseMJD(long int mjd, struct tm *t)
{
    int year = (int) ((mjd - 15078.2) / 365.25);
    int month = (int) ((mjd - 14956.1 - (int) (year * 365.25)) / 30.6001);
    int day = mjd - 14956 - (int) (year * 365.25) - (int) (month * 30.6001);
    int i = (month == 14 || month == 15) ? 1 : 0;
    year += i;
    month = month - 2 - i * 12;

    t->tm_mday = day;
    t->tm_mon = month;
    t->tm_year = year;
    t->tm_isdst = -1;
    t->tm_wday = t->tm_yday = 0;
}

/* Parse Event Information Table */
static void parseEIT(void *data, size_t len)
{
    struct eit *e = data;
    void *p;
    struct tm dvb_time;
    char date_strbuf[256];

    len -= 4;			//remove CRC

    // For each event listing
    for (p = &e->data; p < data + len; p += EIT_EVENT_LEN + GetEITDescriptorsLoopLength(p)) {
	struct eit_event *evt = p;
	struct chninfo *c;
	// find existing information?
	for (c = channels; c != NULL; c = c->next) {
	    // found it
	    if (c->sid == HILO(e->service_id)
		&& (c->eid == HILO(evt->event_id))) {
		if (c->ver <= e->version_number)	// seen it before or its older FIXME: wrap-around to 0
		    return;
		else {
		    c->ver = e->version_number;	// update outputted version
		    update_count++;
		    if (ignore_updates)
			return;
		    break;
		}
	    }
	}

	// its a new program
	if (c == NULL) {
	    chninfo_t *nc = malloc(sizeof(struct chninfo));
	    nc->sid = HILO(e->service_id);
	    nc->eid = HILO(evt->event_id);
	    nc->ver = e->version_number;
	    nc->next = channels;
	    channels = nc;
	}

	/* we have more data, refresh alarm */
	if (timeout)
	    alarm(timeout);

	// No program info at end! Just skip it
	if (GetEITDescriptorsLoopLength(evt) == 0)
	    return;

	parseMJD(HILO(evt->mjd), &dvb_time);

	dvb_time.tm_sec = BcdCharToInt(evt->start_time_s);
	dvb_time.tm_min = BcdCharToInt(evt->start_time_m);
	dvb_time.tm_hour = BcdCharToInt(evt->start_time_h) + time_offset;
	time_t start_time = timegm(&dvb_time);

	dvb_time.tm_sec += BcdCharToInt(evt->duration_s);
	dvb_time.tm_min += BcdCharToInt(evt->duration_m);
	dvb_time.tm_hour += BcdCharToInt(evt->duration_h);
	time_t stop_time = timegm(&dvb_time);

	time_t now;
	time(&now);
	// basic bad date check. if the program ends before this time yesterday, or two weeks from today, forget it.
	if ((difftime(stop_time, now) < -24 * 60 * 60)
	    || (difftime(now, stop_time) > 14 * 24 * 60 * 60)) {
	    invalid_date_count++;
	    if (ignore_bad_dates)
		return;
	}
	// a program must have a title that isn't empty
	if (!validateDescription(&evt->data, GetEITDescriptorsLoopLength(evt))) {
	    return;
	}

	programme_count++;

	printf("<programme channel=\"%s\" ", dvbxmltvid(GetServiceId(e)));
	strftime(date_strbuf, sizeof(date_strbuf), "start=\"%Y%m%d%H%M%S %z\"", localtime(&start_time));
	printf("%s ", date_strbuf);
	strftime(date_strbuf, sizeof(date_strbuf), "stop=\"%Y%m%d%H%M%S %z\"", localtime(&stop_time));
	printf("%s>\n ", date_strbuf);

	//printf("\t<EventID>%i</EventID>\n", HILO(evt->event_id));
	//printf("\t<RunningStatus>%i</RunningStatus>\n", evt->running_status);
	//1 Airing, 2 Starts in a few seconds, 3 Pausing, 4 About to air

	parseDescription(&evt->data, GetEITDescriptorsLoopLength(evt));
	printf("</programme>\n");
    }
}

/*
 * Read EIT segments from DVB-demuxer or file
 */
void readEventTables(int format)
{
    int r = 0;
    int n = 0;
    char buf[1 << 12], *bhead = buf;
    int compressed;
    int uncompressed;
    float ratio;

    /* The dvb demultiplexer simply outputs individual whole packets (good),
     * but reading captured data from a file needs re-chunking. (bad). */
    do {
	if (n < sizeof(struct si_tab))
	    goto read_more;
	struct si_tab *tab = (struct si_tab *) bhead;
	if (GetTableId(tab) == 0)
	    goto read_more;
	size_t l = sizeof(struct si_tab) + GetSectionLength(tab);
	if (n < l)
	    goto read_more;
	packet_count++;
	if (dvb_crc32((uint8_t *) bhead, l) != 0) {
	    /* data or length is wrong. skip bytewise. */
	    //l = 1; // FIXME
	    crcerr_count++;
	} else
	    parseEIT(bhead, l);
	status();
	/* remove packet */
	n -= l;
	bhead += l;
	continue;
      read_more:
	/* move remaining data to front of buffer */
	if (n > 0)
	    memmove(buf, bhead, n);
	/* fill with fresh data */
	r = read(STDIN_FILENO, buf + n, sizeof(buf) - n);
	bhead = buf;
	n += r;
    } while (r > 0);
    uncompressed = get_stat("freesathuffman.uncompressed");
    compressed = get_stat("freesathuffman.compressed");
    ratio = uncompressed / compressed;
    log_message(DEBUG, "freesat huffman average expansion ratio: %f", ratio);
}
