/*
 * freesat_huffman.c
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
#include <stdlib.h> 
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "freesat_huffman.h"
#include "freesat_tables.h"
#include "stats.h"
#include "log.h"

//#define NEW_TABLE_FORMAT 1
#ifdef NEW_TABLE_FORMAT

#define TABLE_1 "freesat.t1"
#define TABLE_2 "freesat.t2"

extern char conf[1024];

int freesat_decode_error = 0;  /* If set an error has occurred during decoding */

static int loaded = 0;
static char result[5];

static char *escape_char(u_char c)
{
    u_char b;
    char *r = result;

    switch (c) {
    case '\'':
        *r++ = '\\';
        *r++ = '\'';
        break;
    case '"':
        *r++ = '\\';
        *r++ = '"';
        break;
    case '\\':
        *r++ = '\\';
        *r++ = '\\';
        break;
    case 0x00 ... 0x1F:
    case 0x7F:
    case 0x80 ... 0xFF:
        *r++ = '0';
        *r++ = 'x';
        b = (c >> 4) & 0x0F;
        *r++ = (char)(b > 9 ? b + 0x37 : b  + 0x30);
        b = c & 0x0F;
        *r++ = (char)(b > 9 ? b + 0x37 : b  + 0x30);
        break;
    default:
        *r++ = c;
        break;
    }
    *r = '\0';
    return result;
}

static void dump_tables()
{
    FILE *fp;
    int i;
    int j;
    int k;
    int indx;
    int line;

    if ((fp = fopen("freesat_tables.new", "w")) == NULL ) {
        log_message(ERROR, "could not open new table dump file");
	return;
    }

    log_message(INFO, "writing huffman tables");
    fprintf(fp, "/*\n");
    fprintf(fp, " * freesat_tables.cpp\n");
    fprintf(fp, " *\n");
    fprintf(fp, " * Huffman tables for Freesat.\n");
    fprintf(fp, " *\n");
    fprintf(fp, " * This program is free software: you can redistribute it and/or modify\n");
    fprintf(fp, " * it under the terms of the GNU General Public License as published by\n");
    fprintf(fp, " * the Free Software Foundation, either version 3 of the License, or\n");
    fprintf(fp, " * (at your option) any later version.\n");
    fprintf(fp, " *\n");
    fprintf(fp, " * This program is distributed in the hope that it will be useful,\n");
    fprintf(fp, " * but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
    fprintf(fp, " * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n");
    fprintf(fp, " * GNU General Public License for more details.\n");
    fprintf(fp, " *\n");
    fprintf(fp, " * You should have received a copy of the GNU General Public License\n");
    fprintf(fp, " * along with this program.  If not, see <http://www.gnu.org/licenses/>.\n");
    fprintf(fp, " */\n");
    fprintf(fp, "\n");
    fprintf(fp, "#include \"freesat_tables.h\"\n");

    /*
     * fsat_table_1 and fsat_table_2
     */
    for (i = 0; i < 2; i++) {
        line =0;
        fprintf(fp, "\n");
        fprintf(fp, "struct fsattab fsat_table_%1d[] = {\n", i + 1);
        for (j = 0; j < 127; j++) {//was 255
            fprintf(fp, "    /* %4d                             */\n", table_size[i][j]);
            for (k = 0; k < table_size[i][j]; k++) {
                fprintf(fp, "    { 0x%08x, %2i, %3d}, /* %4d '%s' */\n", tables[i][j][k].value, tables[i][j][k].bits, tables[i][j][k].next, line++, escape_char(tables[i][j][k].next));
            }
        }
        j = 127;
        fprintf(fp, "    /* %4d                             */\n", table_size[i][j]);
        for (k = 0; k < table_size[i][j] - 1; k++) {
            fprintf(fp, "    { 0x%08x, %2i, %3d}, /* %4d '%s' */\n", tables[i][j][k].value, tables[i][j][k].bits, tables[i][j][k].next, line++, escape_char(tables[i][j][k].next));
        }
        k = table_size[i][j] - 1;
        if (k >= 0) {
            fprintf(fp, "    { 0x%08x, %2i, %3d}  /* %4d '%s' */\n", tables[i][j][k].value, tables[i][j][k].bits, tables[i][j][k].next, line++, escape_char(tables[i][j][k].next));
        }
        fprintf(fp, "};\n");

        /*
         * fsat_index_1 and fsat_index_2
         */
        indx = 0;
        fprintf(fp, "\n");
        fprintf(fp, "unsigned fsat_index_%1d[] = {\n", i + 1);
        for (j = 0 ; j < 128; j++) {
            fprintf(fp, "        %4d, /* %3d */\n", indx, j);
            indx += table_size[i][j];
        }
        j = 128;
        fprintf(fp, "        %4d  /* %3d */\n", indx, j);
        fprintf(fp, "};\n");
    }

    log_message(INFO, "finished writing huffman tables");
    fclose(fp);
}

/** \brief Convert a textual character description into a value
 *
 *  \param str - Encoded (in someway) string
 *
 *  \return Raw character
 */
static unsigned char resolve_char(char *str)
{
    int val;
    if ( strcmp(str,"ESCAPE") == 0 ) {
        return ESCAPE;
    } else if ( strcmp(str,"STOP") == 0 ) {
        return STOP;
    } else if ( strcmp(str,"START") == 0 ) {
        return START;
    } else if ( sscanf(str,"0x%02x", &val) == 1 ) {
        return val;
    }
    return str[0];
}

/** \brief Decode a binary string into a value
 *
 *  \param binary - Binary string to decode
 *
 *  \return Decoded value
 */
static unsigned long decode_binary(char *binary)
{
    unsigned long mask = 0x80000000;
    unsigned long maskval = 0;
    unsigned long val = 0;
    size_t i;

    for ( i = 0; i < strlen(binary); i++ ) {
        if ( binary[i] == '1' ) {
            val |= mask;
        }
        maskval |= mask;
        mask >>= 1;
    }
    return val;
}

/** \brief Load an individual freesat data file
 *
 *  \param tableid   - Table id that should be loaded
 *  \param filename  - Filename to load
 */
static void load_file(int tableid, char *filename)
{
    char     buf[1024];
    char    *from, *to, *binary; 
    FILE    *fp;

    tableid--;

    if ( ( fp = fopen(filename,"r") ) != NULL ) {
        log_message(TRACE, "loading table %d filename \"%s\"",tableid + 1, filename);

        while ( fgets(buf,sizeof(buf),fp) != NULL ) {
            from = binary = to = NULL;
            int elems = sscanf(buf,"%a[^:]:%a[^:]:%a[^:]:", &from, &binary, &to);
            if ( elems == 3 ) {
                int bin_len = strlen(binary);
                int from_char = resolve_char(from);
                char to_char = resolve_char(to);
                unsigned long bin = decode_binary(binary);
                int i = table_size[tableid][from_char]++;

                tables[tableid][from_char] = (struct fsattab *)realloc(tables[tableid][from_char], (i+1) * sizeof(tables[tableid][from_char][0]));
                tables[tableid][from_char][i].value = bin;
                tables[tableid][from_char][i].next = to_char;
                tables[tableid][from_char][i].bits = bin_len;
                free(from);
                free(to);
                free(binary);
            }
        }
    } else {
        log_message(ERROR, "cannot load \"%s\" for table %d", filename, tableid + 1);
    }
}

static void freesat_table_load()
{
    char *f1;
    char *f2;
    int i;


    if ( loaded == 0 ) {
        loaded = 1;

        /* Reset all the tables */
        for ( i = 0 ; i < 256; i++ ) {
            if ( tables[0][i] != NULL ) {
                free(tables[0][i]);
            }
            if ( tables[1][i] ) {
                free(tables[1][i]);
            }
            tables[0][i] = NULL;
            tables[1][i] = NULL;
            table_size[0][i] = 0;
            table_size[1][i] = 0;
        }


        /* And load the files up */
        asprintf(&f1, "%s/%s", conf, TABLE_1);
	log_message(TRACE, "f1=\"%s\"", f1);
        load_file(1, f1);
        free(f1);

        asprintf(&f2, "%s/%s", conf, TABLE_2);
	log_message(TRACE, "f2=\"%s\"", f2);
        load_file(2, f2);
        free(f2);
        dump_tables();
    }
}

/** \brief Decode an EPG string as necessary
 *
 *  \param src - Possibly encoded string
 *  \param size - Size of the buffer
 *
 *  \retval NULL - Can't decode
 *  \return A decoded string
 */
unsigned char *freesat_huffman_to_string(u_char *src, uint size)
{ 
    int tableid;

    freesat_decode_error = 0;

    if (src[0] == 0x1f && (src[1] == 1 || src[1] == 2)) {
        int    uncompressed_len = 30;
        u_char * uncompressed = (u_char *)calloc(1,uncompressed_len + 1);
        unsigned value = 0, byte = 2, bit = 0; 
        int p = 0; 
        int lastch = START; 

        tableid = src[1] - 1;
        while (byte < 6 && byte < size) {
            value |= src[byte] << ((5-byte) * 8); 
            byte++; 
        } 
  
        freesat_table_load();   /* Load the tables as necessary */

        do {
            int found = 0; 
            unsigned bitShift = 0; 
            if (lastch == ESCAPE) {
                char nextCh = (value >> 24) & 0xff; 
                found = 1; 
                // Encoded in the next 8 bits. 
                // Terminated by the first ASCII character. 
                bitShift = 8; 
                if ((nextCh & 0x80) == 0) 
                    lastch = nextCh; 
                if (p >= uncompressed_len) {
                    uncompressed_len += 10;
                    uncompressed = (u_char *)realloc(uncompressed, uncompressed_len + 1);
                }
                uncompressed[p++] = nextCh; 
                uncompressed[p] = 0;
            } else {
                int j;
                for ( j = 0; j < table_size[tableid][lastch]; j++) {
                    unsigned mask = 0, maskbit = 0x80000000; 
                    short kk;
                    for ( kk = 0; kk < tables[tableid][lastch][j].bits; kk++) {
                        mask |= maskbit; 
                        maskbit >>= 1; 
                    } 
                    if ((value & mask) == tables[tableid][lastch][j].value) {
                        char nextCh = tables[tableid][lastch][j].next; 
                        bitShift = tables[tableid][lastch][j].bits; 
                        if (nextCh != STOP && nextCh != ESCAPE) {
                            if (p >= uncompressed_len) {
                                uncompressed_len += 10;
                                uncompressed = (u_char *)realloc(uncompressed, uncompressed_len + 1);
                            }
                            uncompressed[p++] = nextCh; 
                            uncompressed[p] = 0;
                        } 
                        found = 1;
                        lastch = nextCh; 
                        break; 
                    } 
                } 
            } 
            if (found) {
                // Shift up by the number of bits. 
                unsigned b;
                for ( b = 0; b < bitShift; b++) 
                { 
                    value = (value << 1) & 0xfffffffe; 
                    if (byte < size) 
                        value |= (src[byte] >> (7-bit)) & 1; 
                    if (bit == 7) 
                    { 
                        bit = 0; 
                        byte++; 
                    } 
                    else bit++; 
                } 
            } else {
                char  temp[1020];
                size_t   tlen = 0;

                tlen = snprintf(temp, sizeof(temp), "[%02x][%02x][%02x][%02x]",(value >> 24 ) & 0xff, (value >> 16 ) & 0xff, (value >> 8) & 0xff, value &0xff);
                do {
                    // Shift up by the number of bits. 
                    unsigned b;
                    for ( b = 0; b < 8; b++) {
                        value = (value << 1) & 0xfffffffe; 
                        if (byte < size) 
                            value |= (src[byte] >> (7-bit)) & 1; 
                        if (bit == 7) {
                            bit = 0; 
                            byte++; 
                        } 
                        else bit++; 
                    } 
                    tlen += snprintf(temp+tlen, sizeof(temp) - tlen,"[%02x]", value & 0xff);
                } while ( tlen < sizeof(temp) - 6 && byte <  size);
                
                uncompressed_len += tlen;
                uncompressed = (u_char *)realloc(uncompressed, uncompressed_len + 1);
                freesat_decode_error = 1;
                strcpy((char *)uncompressed + p, temp);
                log_message(ERROR, "missing table %d entry \"%s\"",tableid + 1, uncompressed);
                // Entry missing in table. 
                return uncompressed; 
            } 
        } while (lastch != STOP && value != 0); 
        return uncompressed;
    } 
    return NULL; 
} 

#else

void dump_compressed(u_char *src, uint size) {
    char temp[1024];
    size_t tlen;
    unsigned int value;
    unsigned b;
    unsigned int byte;
    unsigned int bit;

    value = 0;
    byte = 0;	// real data starts at 2, but want whole buffer for test data; so value is 0
    bit = 0;
    tlen = 0;
    do {
        // Shift up by the number of bits. 
        for ( b = 0; b < 8; b++) {
	    value = (value << 1) & 0xfffffffe; 
	    if (byte < size) 
	        value |= (src[byte] >> (7-bit)) & 1; 
	    if (bit == 7) {
	        bit = 0; 
	        byte++; 
	    } 
	    else bit++; 
        } 
        tlen += snprintf(temp+tlen, sizeof(temp) - tlen,"%02x, ", value & 0xff);
    } while ( tlen < sizeof(temp) - 6 && byte <  size);

    log_message(ERROR, "table %d len =%d uncompressed=\"%s\"", src[1], size, temp);
}

unsigned char *freesat_huffman_to_string(u_char *src, uint size)
{
    struct fsattab *fsat_table;
    unsigned int *fsat_index;
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
    char temp[1020];
    size_t tlen;


    /*
     * variable for debugging
     */
    //uint i;
    //int alloc;
    //int incr;
    //float percentage_incrementing;
    //int uc;
    //int co;
    //float ratio;
    //float average_length;
    //float length_ratio;
    //int length;
    int allocated = 0;
    int num_incr;

    //dump_compressed(src, size);
    //if (is_logging(TRACE)) {
	//log_message(TRACE, "freesat text length=%d", size);
	//for (i = 0; i < size; i++) {
	    //log_message(TRACE, "freesat text string=\"%2.2X\"", src[i]);
	//}
    //}
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
        if (src[1] == 1) {
	    fsat_table = fsat_table_1;
	    fsat_index = fsat_index_1;
	} else {
	    fsat_table = fsat_table_2;
	    fsat_index = fsat_index_2;
	}
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
		//if (src[1] == 2)
		//    indx |= 0x80;
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
                tlen = snprintf(temp, sizeof(temp), "[%02x][%02x][%02x][%02x]",(value >> 24 ) & 0xff, (value >> 16 ) & 0xff, (value >> 8) & 0xff, value &0xff);
                do {
                    // Shift up by the number of bits. 
                    unsigned b;
                    for ( b = 0; b < 8; b++) {
                        value = (value << 1) & 0xfffffffe; 
                        if (byte < size) 
                            value |= (src[byte] >> (7-bit)) & 1; 
                        if (bit == 7) {
                            bit = 0; 
                            byte++; 
                        } 
                        else bit++; 
                    } 
                    tlen += snprintf(temp+tlen, sizeof(temp) - tlen,"[%02x]", value & 0xff);
                } while ( tlen < sizeof(temp) - 6 && byte <  size);
                
                alloc_size += tlen;
                uncompressed = (u_char *)realloc(uncompressed, alloc_size + 1);
                strcpy((char *)uncompressed + p, temp);
		log_message(ERROR,
			    "entry missing in huffman table %d for Freesat value=%x0 uncompressed=\"%s\"", src[1], value, uncompressed);
                return uncompressed; 
	    }
	} while (lastch != STOP && byte < size + 4);

	uncompressed[p++] = '\0';
	//log_message(TRACE, "freesat text=\"%s\"", uncompressed);
	//if (is_logging(TRACE)) {
	    //length = strlen((char *) uncompressed);
	    //length_ratio = (float) length / (float) size;
	    //uc = get_stat("freesathuffman.uncompressed");
	    //co = get_stat("freesathuffman.compressed");
	    //ratio = (float) uc / (float) co;
	    //alloc = get_stat("freesathuffman.alloc");
	    //incr = get_stat("freesathuffman.increment");
	    //percentage_incrementing =
		//((float) incr / (float) alloc) * 100.0;
	    //average_length = (float) uc / (float) alloc;
	    ////log_message(TRACE,
			//"freesat huffman size=%d len=%d allocated=%d lr=%f num_incr=%d co=%d uc=%d ratio=%f avglen=%f alloc=%d incr=%d percincr=%f",
			//size, length, allocated, length_ratio, num_incr,
			//co, uc, ratio, average_length, alloc, incr,
			//percentage_incrementing);
	//}
	return uncompressed;
    } else {
	uncompressed[0] = '\0';
	log_message(WARNING, "empty freesat text");
	return uncompressed;
    }
}

#endif
