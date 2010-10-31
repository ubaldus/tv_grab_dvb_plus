#ifndef __TUNER_H__
#define __TUNER_H__

#include <stdio.h>
#include <errno.h>
#include <time.h>
#include "extended_frontend.h"
#include <sys/types.h>
#include <stdint.h>
#include "list.h"


/******************************************************************************
 * internal definitions.
 *****************************************************************************/

struct w_scan_flags {
	uint32_t	version;
	fe_type_t	fe_type;
	//uint8_t		atsc_type;
	uint8_t		need_2g_fe;
	uint32_t	list_id;
	uint8_t		tuning_timeout;
	uint8_t		filter_timeout;
	uint8_t		get_other_nits;
	uint8_t		add_frequencies;
	uint8_t		dump_provider;
	uint8_t		vdr_version;
	uint8_t		qam_no_auto;
	uint8_t		ca_select;
	int		rotor_position;
	uint16_t	api_version;
};

#define AUDIO_CHAN_MAX (32)
#define CA_SYSTEM_ID_MAX (16)

struct transponder_ids {
	int network_id;
	int original_network_id;		/* onid patch by Hartmut Birr */
	int transport_stream_id; 
};

struct transponder {
	struct list_head list;
	struct list_head services;
	struct transponder_ids pids;
	enum fe_type type;
	struct extended_dvb_frontend_parameters param;
	unsigned int scan_done:1;
	unsigned int last_tuning_failed:1;
	unsigned int other_frequency_flag:1;	/* DVB-T */
	int n_other_f;
	int updated_by_nit;
	uint32_t *other_f;			/* DVB-T frequency-list descriptor */
	char *network_name;
};


struct transponder *alloc_transponder(uint32_t frequency);

/* write transponder data to dest. no memory allocating,
 * so dest has to be big enough - think about before use!
 */
void print_transponder(char * dest, struct transponder * t);


#endif

