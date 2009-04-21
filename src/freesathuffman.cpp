/*
 * freesathuffman.c
 *
 * Decode a Freesat huffman encoded buffer.
 * Once decoded the buffer can be used like a "standard" DVB buffer.
 *
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
#include <stdint.h>
#include <stdbool.h>
#include <malloc.h>
#include <sys/types.h>

#include "freesathuffman.h"
#include "freesattables.h"
#include "stats.h"
#include "log.h"

unsigned char *freesat_huffman_to_string(u_char *src, uint size)
{
    uint i;
    unsigned char *uncompressed;
    uint p;
    unsigned int value;
    unsigned int byte;
    unsigned int bit;
    char lastch;
    int found;
    unsigned int bitShift;
    char nextCh;
    unsigned int indx;
    unsigned int j;
    unsigned int mask;
    unsigned int maskbit;
    unsigned short kk;
    unsigned int b;
    unsigned int alloc_size;

    /*
     * variable for debugging
     */
    int alloc;
    int incr;
    float percentage_incrementing;
    int uc;
    int co;
    float ratio;
    float average_length;
    float length_ratio;
    int length;
    int allocated = 0;
    int num_incr;

    if (is_logging(TRACE)) {
	log_message(TRACE, "freesat text length=%d", size);
	for (i = 0; i < size; i++) {
	    log_message(TRACE, "freesat text string=\"%2.2X\"", src[i]);
	}
    }
    p = 0;
    /*
     * The routine allocates a buffer that is ALLOC_MULT times the size of
     * the input buffer and * if we run out of space added another ALLOC_INCR bytes.
     *
     * Use the output of the stats to see what the expansion ratio is for a sample stream
     * and adjust the multiplier accordingly. 
     * Create a stats object and output it in the xml file at the end of a run.
     *
     * The values if freesathuffman.h reflect doing this.
     */
    alloc_size = (size * ALLOC_MULT) + 1;
    if (is_logging(TRACE)) {
	allocated = alloc_size;
	num_incr = 0;
    }
    incr_stat("freesathuffman.alloc");
    add_to_stat("freesathuffman.compressed", size);
    add_to_stat("freesathuffman.uncompressed", alloc_size);

    uncompressed = (unsigned char *) malloc(alloc_size);
    if (src[1] == 1 || src[1] == 2) {
	value = 0;
	byte = 2;
	bit = 0;
	while (byte < 6 && byte < size) {
	    value |= src[byte] << ((5 - byte) * 8);
	    byte++;
	}
	lastch = START;

	do {
	    found = false;
	    bitShift = 0;
	    nextCh = STOP;
	    if (lastch == ESCAPE) {
		found = true;
		// Encoded in the next 8 bits.
		// Terminated by the first ASCII character.
		nextCh = (value >> 24) & 0xff;
		bitShift = 8;
		if ((nextCh & 0x80) == 0) {
		    if (nextCh < ' ')
			nextCh = STOP;
		    lastch = nextCh;
		}
	    } else {
		indx = (unsigned int) lastch;
		if (src[1] == 2)
		    indx |= 0x80;
		for (j = fsat_index[indx]; j < fsat_index[indx + 1]; j++) {
		    mask = 0;
		    maskbit = 0x80000000;
		    for (kk = 0; kk < fsat_table[j].bits; kk++) {
			mask |= maskbit;
			maskbit >>= 1;
		    }
		    if ((value & mask) == fsat_table[j].value) {
			nextCh = fsat_table[j].next;
			bitShift = fsat_table[j].bits;
			found = true;
			lastch = nextCh;
			break;
		    }
		}
	    }
	    if (found) {
		if (nextCh != STOP && nextCh != ESCAPE) {
		    if (p >= alloc_size) {
			log_message(TRACE,
				    "had to realloc string in freesat huffman decoding");
			alloc_size += ALLOC_INCR;
			if (is_logging(TRACE)) {
			    allocated += ALLOC_INCR;
			    num_incr++;
			}
			incr_stat("freesathuffman.increment");
			add_to_stat("freesathuffman.uncompressed",
				    ALLOC_INCR);
			uncompressed =
			    (unsigned char *) realloc(uncompressed,
						      alloc_size);
		    }
		    uncompressed[p++] = nextCh;
		}
		// Shift up by the number of bits.
		for (b = 0; b < bitShift; b++) {
		    value = (value << 1) & 0xfffffffe;
		    if (byte < size)
			value |= (src[byte] >> (7 - bit)) & 1;
		    if (bit == 7) {
			bit = 0;
			byte++;
		    } else
			bit++;
		}
	    } else {
		// Entry missing in table.
		if (p >= alloc_size) {
		    log_message(TRACE,
				"had to realloc string in freesat huffman decoding (missing entry)");
		    alloc_size += 4;
		    uncompressed =
			(unsigned char *) realloc(uncompressed,
						  alloc_size);
		}
		uncompressed[p++] = '.';
		uncompressed[p++] = '.';
		uncompressed[p++] = '.';
		uncompressed[p++] = '\0';
		log_message(ERROR,
			    "entry missing in huffman table for Freesat");
		return uncompressed;
	    }
	} while (lastch != STOP && byte < size + 4);

	uncompressed[p++] = '\0';
	log_message(TRACE, "freesat text=\"%s\"", uncompressed);
	if (is_logging(TRACE)) {
	    length = strlen((char *) uncompressed);
	    length_ratio = (float) length / (float) size;
	    uc = get_stat("freesathuffman.uncompressed");
	    co = get_stat("freesathuffman.compressed");
	    ratio = (float) uc / (float) co;
	    alloc = get_stat("freesathuffman.alloc");
	    incr = get_stat("freesathuffman.increment");
	    percentage_incrementing =
		((float) incr / (float) alloc) * 100.0;
	    average_length = (float) uc / (float) alloc;
	    log_message(TRACE,
			"freesat huffman size=%d len=%d allocated=%d lr=%f num_incr=%d co=%d uc=%d ratio=%f avglen=%f alloc=%d incr=%d percincr=%f",
			size, length, allocated, length_ratio, num_incr,
			co, uc, ratio, average_length, alloc, incr,
			percentage_incrementing);
	}
	return uncompressed;
    } else {
	uncompressed[0] = '\0';
	log_message(WARNING, "empty freesat text");
	return uncompressed;
    }
}
