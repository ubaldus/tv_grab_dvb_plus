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

#include <stdint.h>
#include <stdbool.h>
#include <malloc.h>
#include <sys/types.h>

#include "freesathuffman.h"
#include "freesattables.h"

extern char *ProgName;

unsigned char *freesat_huffman_to_string(unsigned char *src, uint size)
{
    int i;
    unsigned char *result;
    unsigned char *uncompressed;
    int count;
    int p;
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

    //fprintf(stderr, "%s: freesat text length=%d\n", ProgName, size);
    //for (i = 0; i < size; i++) {
	    //fprintf(stderr, "%s: freesat text string=\"%2.2X\"\n", ProgName, src[i]);
    //}
    count = 0;
    p = 0;
    /*
     * FIXME!
     *
     * The routine allocates a buffer that is 3 times the size of
     * the input buffer and * if we run out of space added another 10 bytes.
     *
     * We should really see what the expansion ratio is for a sample stream
     * and adjust the multiplier accordingly. Maybe the thing to do is to
     * create a stats object and output it (on stderr or in xml file) at
     * the end of a run.
     */
    alloc_size = (size * 3) + 1;
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
		    if (p >= count) {
			//fprintf(stderr, "%s: had to realloc string in freesat huffman decoding\n", ProgName);
			/*
			 * FIXME!
			 *
			 * See above comment about buffer sizes
			 */
			alloc_size += 10;
			uncompressed = (unsigned char *) realloc(uncompressed, alloc_size);
		    }
		    uncompressed[p++] = nextCh;
		    count++;
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
		    if (p >= count) {
			//fprintf(stderr, "%s: had to realloc string in freesat huffman decoding (missing entry)\n", ProgName);
			alloc_size += 4;
			uncompressed = (unsigned char *) realloc(uncompressed, alloc_size);
		    }
		uncompressed[p++] = '.';
		uncompressed[p++] = '.';
		uncompressed[p++] = '.';
		uncompressed[p++] = '\0';
		//fprintf(stderr, "%s: entry missing in huffman table for Freesat\n", ProgName);
		return uncompressed;
	    }
	} while (lastch != STOP && byte < size + 4);

	uncompressed[p++] = '\0';
	//fprintf(stderr, "%s: freesat text=\"%s\"\n", ProgName, uncompressed);
	return uncompressed;
    } else {
	uncompressed[0] = '\0';
	//fprintf(stderr, "%s: empty freesat text=\n", ProgName);
	return uncompressed;
    }
}
