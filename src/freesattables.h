#ifndef __FREESAT_TABLES_H__
#define __FREESAT_TABLES_H__

struct fsattab {
    unsigned int value;
    short bits;
    char next;
};

#define START   '\0'
#define STOP    '\0'
#define ESCAPE  '\1'

extern struct fsattab fsat_table[];
extern unsigned fsat_index[];

#endif
