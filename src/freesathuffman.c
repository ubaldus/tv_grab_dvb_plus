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
