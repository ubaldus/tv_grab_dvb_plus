#ifndef __LOOKUP_H__
#define __LOOKUP_H__

union lookup_key {
	int i;
	char c[4];
};
struct lookup_table {
	union lookup_key u;
	char *desc;
};

extern char *lookup(const struct lookup_table *l, int id);
extern int load_lookup(struct lookup_table **l, const char *file);

extern const struct lookup_table description_table[];
extern const struct lookup_table aspect_table[];
extern const struct lookup_table audio_table[];

extern const struct lookup_table languageid_table[];

#endif
