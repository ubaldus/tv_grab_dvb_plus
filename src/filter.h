#ifndef __FILTER_H__
#define __FILTER_H__

struct filter_t {
    unsigned short int pid;
    unsigned char tid;
    unsigned char mask;
};

extern void add_filter(unsigned short int pid, unsigned char tid, unsigned char mask);
extern int start_filters(int fd);

#endif
