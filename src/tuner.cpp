

#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/dvb/dmx.h>
#include <linux/dvb/version.h>
#include <linux/dvb/frontend.h>
#include "tuner.h"
#include "log.h"

static struct dvb_frontend_info fe_info; // = {.type = -1};

static LIST_HEAD(new_transponders);
static struct transponder *current_tp;

//static int this_rotor_pos = -1;			// 20090320: DVB-S/S2, current rotor position
//static struct lnb_types_st this_lnb;		// 20090320: DVB-S/S2, LNB type, initialized in main to 'UNIVERSAL'

struct w_scan_flags flags = {
	0,		// readback value w_scan version {YYYYMMDD}
	FE_OFDM,	// frontend type
	//ATSC_VSB,	// default for ATSC scan
	0,		// need 2nd generation frontend
	0, //DE,		// country index or sat index
	1,		// tuning speed {1 = fast, 2 = medium, 3 = slow}
	0,		// filter timeout {0 = default, 1 = long} 
	1,		// get_other_nits, atm always
	1,		// add_frequencies, atm always
	1,		// dump_provider, dump also provider name
	6,		// VDR version number, VDR-1.6.x
	0,		// 0 = qam auto, 1 = search qams
	1,		// scan encrypted channels = yes
	-1,		// rotor position, unused
	0x0302,		// assuming DVB API version 3.2
};

/* According to the DVB standards, the combination of network_id and
 * transport_stream_id should be unique, but in real life the satellite
 * operators and broadcasters don't care enough to coordinate
 * the numbering. Thus we identify TPs by frequency (scan handles only
 * one satellite at a time). Further complication: Different NITs on
 * one satellite sometimes list the same TP with slightly different
 * frequencies, so we have to search within some bandwidth.
 */
struct transponder *alloc_transponder(uint32_t frequency)
{
	struct transponder *tp = (struct transponder*)calloc(1, sizeof(*tp));

	tp->param.frequency = frequency;
	tp->updated_by_nit = 0;
	INIT_LIST_HEAD(&tp->list);
	INIT_LIST_HEAD(&tp->services);
	list_add_tail(&tp->list, &new_transponders);
	return tp;
}

void print_transponder(char * dest, struct transponder * t) {
#if 0
	memset(dest, 0, sizeof(dest));
	switch (t->type) {
		case FE_OFDM:
			sprintf(dest, "%-8s f = %6d kHz I%sB%sC%sD%sT%sG%sY%s",
				xine_modulation_name(t->param.u.ofdm.constellation),
				t->param.frequency/1000,
				vdr_inversion_name(t->param.inversion),
				vdr_bandwidth_name(t->param.u.ofdm.bandwidth),
				vdr_fec_name(t->param.u.ofdm.code_rate_HP),
				vdr_fec_name(t->param.u.ofdm.code_rate_LP),
				vdr_transmission_mode_name(t->param.u.ofdm.transmission_mode),
				vdr_guard_name(t->param.u.ofdm.guard_interval),
				vdr_hierarchy_name(t->param.u.ofdm.hierarchy_information));
			break;
		case FE_ATSC:
			sprintf(dest, "%-8s f=%d kHz",
				atsc_mod_to_txt(t->param.u.vsb.modulation),
				t->param.frequency/1000);
			break;
		case FE_QAM:
			sprintf(dest, "%-8s f = %d kHz S%dC%s",
				xine_modulation_name(t->param.u.qam.modulation),
				t->param.frequency/1000,
				t->param.u.qam.symbol_rate/1000,
				vdr_fec_name(t->param.u.qam.fec_inner));
			break;
		case FE_QPSK:
			sprintf(dest, "%-2s f = %d kHz %s SR = %5d %4s 0,%s %5s",
				qpsk_delivery_system_to_txt(t->param.u.qpsk.modulation_system),
				t->param.frequency/1000,
				qpsk_pol_to_txt(t->param.u.qpsk.polarization),
				t->param.u.qpsk.symbol_rate/1000,
				qpsk_fec_to_txt(t->param.u.qpsk.fec_inner),
				qpsk_rolloff_to_txt(t->param.u.qpsk.rolloff),
				qpsk_mod_to_txt(t->param.u.qpsk.modulation_type));
		
	break;
		default:
			warning("unimplemented frontend type %d\n", t->type);
		}
#endif
}

/*
 * actual tuning stuff
 *
 */
static int mem_is_zero (const void *mem, unsigned int size)
{
	const char *p = (const char*)mem;
	unsigned long i;

	for (i=0; i<size; i++) {
		if (p[i] != 0x00)
			return 0;
	}

	return 1;
}

int get_api_version(int frontend_fd, struct w_scan_flags * flags) {

	struct dtv_property p[1]; // = {{.cmd = DTV_API_VERSION }};
	struct dtv_properties cmdseq; // = {.num = 1, .props = p};

	p[0].cmd = DTV_API_VERSION;
	cmdseq.num = 1;
	cmdseq.props = p;

	/* expected to fail with old drivers,
	 * therefore no warning to user. 20090324 -wk
	 */
	if (ioctl(frontend_fd, FE_GET_PROPERTY, &cmdseq))
		return -1;

	flags->api_version = p[0].u.data;
	return 0;
}

/* might be removed later - intermediate solution. */
static int copy_fe_params_new_to_old(struct dvb_frontend_parameters * dest,
	    	   struct extended_dvb_frontend_parameters * source) {
	dest->frequency				= source->frequency;
	dest->inversion				= source->inversion;
	dest->u.qpsk.symbol_rate		= source->u.qpsk.symbol_rate;
	dest->u.qpsk.fec_inner			= source->u.qpsk.fec_inner;
	dest->u.qam.symbol_rate			= source->u.qam.symbol_rate;
	dest->u.qam.fec_inner			= source->u.qam.fec_inner;
	dest->u.qam.modulation			= source->u.qam.modulation;
	dest->u.vsb.modulation			= source->u.vsb.modulation;
	dest->u.ofdm.bandwidth			= source->u.ofdm.bandwidth;
	dest->u.ofdm.code_rate_HP		= source->u.ofdm.code_rate_HP;
	dest->u.ofdm.code_rate_LP		= source->u.ofdm.code_rate_LP;
	dest->u.ofdm.constellation		= source->u.ofdm.constellation;
	dest->u.ofdm.transmission_mode		= source->u.ofdm.transmission_mode;
	dest->u.ofdm.guard_interval		= source->u.ofdm.guard_interval;
	dest->u.ofdm.hierarchy_information	= source->u.ofdm.hierarchy_information;
	return 0;
}

const char * frontend_type_to_text (fe_type_t frontend_type) {
	switch(frontend_type) {
		case FE_QAM:  return "DVB-C";
		case FE_QPSK: return "DVB-S";
		case FE_OFDM: return "DVB-T";
		case FE_ATSC: return "ATSC";
		default: return "UNKNOWN";
		}
}

uint32_t bandwidth_Hz (fe_bandwidth_t bandwidth) {
	switch (bandwidth) {
		case BANDWIDTH_8_MHZ: return 8000000;
		case BANDWIDTH_7_MHZ: return 7000000;
		case BANDWIDTH_6_MHZ: return 6000000;
		#ifdef BANDWIDTH_5_MHZ
		case BANDWIDTH_5_MHZ: return 5000000;
		#endif
		default: return 0;
		}
}

fe_delivery_system_t atsc_del_sys(fe_modulation_t modulation) {
	switch (modulation) {
		case VSB_8:
		case VSB_16:
			return SYS_ATSC;
		default:;
			return SYS_DVBC_ANNEX_B;
		}
}

static int set_frontend(int frontend_fd, struct transponder * t) {
#ifdef USE_LNB
	uint8_t switch_to_high_band = 0;
#endif
	uint32_t intermediate_freq = 0;
	int sequence_len = 0;
	struct dvb_frontend_parameters p;
	struct dtv_property cmds[12];
	struct dtv_properties cmdseq = {0, cmds};

	//info("(time: %.2d:%.2d) ", run_time() / 60, run_time() % 60);

	switch(t->type) {
		case FE_QPSK:

			if (t->param.u.qpsk.modulation_system == SYS_DVBS2) {
				if (!(fe_info.caps & FE_CAN_2G_MODULATION) ||
				     (flags.api_version < 0x0500)) {
					log_message(DEBUG, "\t%d: skipped (no driver support)", t->param.frequency/1000);
					return -2;
					}
				}

#ifdef USE_LNB
			if (this_lnb.high_val) {
				if (this_lnb.switch_val) { // voltage controlled switch
					switch_to_high_band = 0;

					if (t->param.frequency >= this_lnb.switch_val)
						switch_to_high_band++;

					setup_switch (frontend_fd, committed_switch,
						t->param.u.qpsk.polarization == POLARIZATION_VERTICAL ? 0 : 1,
						switch_to_high_band, uncommitted_switch);

					usleep(50000);

					if (switch_to_high_band)
						intermediate_freq = abs(t->param.frequency - this_lnb.high_val);
					else
						intermediate_freq = abs(t->param.frequency - this_lnb.low_val);
					}
				else { // C-Band Multipoint LNB
					if (t->param.u.qpsk.polarization == POLARIZATION_VERTICAL)
						intermediate_freq = abs(t->param.frequency - this_lnb.low_val);
					else
						intermediate_freq = abs(t->param.frequency - this_lnb.high_val);
					}
				}
			else // Monopoint LNB w/o switch
				intermediate_freq = abs(t->param.frequency - this_lnb.low_val);

			if ((intermediate_freq < 950000) || (intermediate_freq > 2150000)) {
				log_message (DEBUG, "\t%d: skipped (invalid frequency)", intermediate_freq);
				return -2;
				}

			if (sat_list[this_channellist].rotor_position > -1) { // rotate DiSEqC rotor to correct orbital position
				/*
				if (t->param.u.qpsk.orbital_position)
					rotor_pos = rotor_nn(t->param.u.qpsk.orbital_position, t->param.u.qpsk.west_east_flag);
				 */
				if (rotate_rotor(frontend_fd, &this_rotor_pos,
				    sat_list[this_channellist].rotor_position,
				    t->param.u.qpsk.polarization == POLARIZATION_VERTICAL ? 0 : 1,
				    switch_to_high_band))
					log_message(ERROR, "Error rotating rotor");
				}
#endif

			break;
		default:;
		}

	if (mem_is_zero (&t->param, sizeof(struct extended_dvb_frontend_parameters)))
		return -1;

	switch (flags.api_version) {
		case 0x0302:
			log_message(DEBUG, "%s: using DVB API 3.2", __FUNCTION__);
			copy_fe_params_new_to_old(&p, &t->param);
			switch (t->type) {
				case FE_QPSK:
					p.frequency = intermediate_freq;
					break;
				default:;
				}
			if (ioctl(frontend_fd, FE_SET_FRONTEND, &p) == -1) {
				log_message(ERROR, "Setting frontend parameters failed (API v3.2)");
				return -1;
				}
			break;
		case 0x0500 ... 0x5FF:
			log_message(DEBUG, "%s: using DVB API %x.%x",
                          __FUNCTION__,
                         flags.api_version >> 8,
                         flags.api_version & 0xFF);

			/* some 'shortcut' here :-)) --wk 20090324 */
			#define set_cmd_sequence(_cmd, _data)	cmds[sequence_len].cmd = _cmd; \
								cmds[sequence_len].u.data = _data; \
								cmdseq.num = ++sequence_len

			set_cmd_sequence(DTV_CLEAR, DTV_UNDEFINED);
			switch (t->type) {
				case FE_QPSK:
					set_cmd_sequence(DTV_DELIVERY_SYSTEM, 	t->param.u.qpsk.modulation_system);
					set_cmd_sequence(DTV_FREQUENCY, 	intermediate_freq);
					set_cmd_sequence(DTV_INVERSION, 	t->param.inversion);
					set_cmd_sequence(DTV_MODULATION, 	t->param.u.qpsk.modulation_type);
					set_cmd_sequence(DTV_SYMBOL_RATE, 	t->param.u.qpsk.symbol_rate);
					set_cmd_sequence(DTV_INNER_FEC, 	t->param.u.qpsk.fec_inner);
					set_cmd_sequence(DTV_PILOT, 		t->param.u.qpsk.pilot);
					set_cmd_sequence(DTV_ROLLOFF, 		t->param.u.qpsk.rolloff);
					break;
				case FE_QAM:
					set_cmd_sequence(DTV_DELIVERY_SYSTEM, 	SYS_DVBC_ANNEX_AC);
					set_cmd_sequence(DTV_FREQUENCY, 	t->param.frequency);
					set_cmd_sequence(DTV_INVERSION, 	t->param.inversion);
					set_cmd_sequence(DTV_MODULATION, 	t->param.u.qam.modulation);
					set_cmd_sequence(DTV_SYMBOL_RATE, 	t->param.u.qam.symbol_rate);
					set_cmd_sequence(DTV_INNER_FEC, 	t->param.u.qam.fec_inner);
					break;
				case FE_OFDM:
					set_cmd_sequence(DTV_DELIVERY_SYSTEM, 	SYS_DVBT);
					set_cmd_sequence(DTV_FREQUENCY, 	t->param.frequency);
					set_cmd_sequence(DTV_INVERSION, 	t->param.inversion);
					set_cmd_sequence(DTV_BANDWIDTH_HZ, 	bandwidth_Hz(t->param.u.ofdm.bandwidth));
					set_cmd_sequence(DTV_CODE_RATE_HP, 	t->param.u.ofdm.code_rate_HP);
					set_cmd_sequence(DTV_CODE_RATE_LP, 	t->param.u.ofdm.code_rate_LP);
					set_cmd_sequence(DTV_MODULATION, 	t->param.u.ofdm.constellation);
					set_cmd_sequence(DTV_TRANSMISSION_MODE, t->param.u.ofdm.transmission_mode);
					set_cmd_sequence(DTV_GUARD_INTERVAL, 	t->param.u.ofdm.guard_interval);
					set_cmd_sequence(DTV_HIERARCHY, 	t->param.u.ofdm.hierarchy_information);
					break;
				case FE_ATSC:
					set_cmd_sequence(DTV_DELIVERY_SYSTEM, 	atsc_del_sys(t->param.u.vsb.modulation));
					set_cmd_sequence(DTV_FREQUENCY, 	t->param.frequency);
					set_cmd_sequence(DTV_INVERSION, 	t->param.inversion);
					set_cmd_sequence(DTV_MODULATION, 	t->param.u.vsb.modulation);
					break;
				default:
					log_message(ERROR, "Unhandled type %d", t->type);
					exit(1);
				}
			set_cmd_sequence(DTV_TUNE, DTV_UNDEFINED);
						
			if (ioctl(frontend_fd, FE_SET_PROPERTY, &cmdseq) < 0) {
				log_message(ERROR, "Setting frontend parameters failed (API v5.x)");
				return -1;
				}
			break;
		default:
			log_message(ERROR, "unsupported DVB API Version %x.%x",
				flags.api_version >> 8,
				flags.api_version & 0xFF);
			exit(1);
		}
	return 0;
}

static int __tune_to_transponder (int frontend_fd, struct transponder *t, int v) {

	fe_status_t s;
	int i, res;

	if (t == NULL)
		return -3;
	current_tp = t;
	if (current_tp->network_name != NULL) {
		free(current_tp->network_name);
		current_tp->network_name = NULL;
		}

	if ((is_logging(DEBUG)) && (v > 0)) {
		char * buf = (char *) malloc(128); // paranoia, max = 52
		print_transponder(buf, t);
		log_message(DEBUG, "tune to: %s %s",
			buf, t->last_tuning_failed?" (no signal)\n":"\n");
		free(buf);
		}

	res = set_frontend(frontend_fd, t);

	if (res < 0)
		return res;

	for (i = 0; i < 5 * flags.tuning_timeout; i++) {
		usleep (200000);

		if (ioctl(frontend_fd, FE_READ_STATUS, &s) == -1) {
			log_message(ERROR, "FE_READ_STATUS failed");
			return -1;
			}

		if (v > 0)
			log_message(DEBUG, ">>> tuning status == 0x%02x", s);

		if (s & FE_HAS_LOCK) {
			t->last_tuning_failed = 0;
			return 0;
			}
		}

	if (v > 0)
		log_message(DEBUG, "----------no signal----------");

	t->last_tuning_failed = 1;

	return -1;
}

int tune_to_transponder (int frontend_fd, struct transponder *t) {

	int res;
	/* move TP from "new" to "scanned" list */
	//list_del_init(&t->list);
	//list_add_tail(&t->list, &scanned_transponders);
	t->scan_done = 1;

	if (t->type != fe_info.type) {
		/* ignore cable descriptors in sat NIT and vice versa */
		t->last_tuning_failed = 1;
		return -1;
	}

	res = __tune_to_transponder (frontend_fd, t, 1);
	switch (res) {
		case 0:		return 0;
		case -1:	return __tune_to_transponder (frontend_fd, t, 1);
		case -2:	return -2;
		default:	return -1;
		}
}

int check_frontend (int fd, int verbose) {
	fe_status_t status;
	ioctl(fd, FE_READ_STATUS, &status);
	if (verbose) {
		uint16_t snr, signal;
		uint32_t ber, uncorrected_blocks;

		ioctl(fd, FE_READ_SIGNAL_STRENGTH, &signal);
		ioctl(fd, FE_READ_SNR, &snr);
		ioctl(fd, FE_READ_BER, &ber);
		ioctl(fd, FE_READ_UNCORRECTED_BLOCKS, &uncorrected_blocks);
		log_message(DEBUG, "signal %04x | snr %04x | ber %08x | unc %08x | %s",
							signal, snr, ber, uncorrected_blocks, (status & FE_HAS_LOCK)?"FE_HAS_LOCK":"");
		}
	return (status & FE_HAS_LOCK);
}

void tuner_init()
{
	fe_info.type = (fe_type_t)-1;
}

