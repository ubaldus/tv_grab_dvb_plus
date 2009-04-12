#ifndef __LOOKUP_H__
#define __LOOKUP_H__

#ifdef __cplusplus
extern "C" {
#endif

union lookup_key {
	int i;
	char *c;
};
struct lookup_table {
	union lookup_key u;
	char *desc;
};

extern char *lookup(const struct lookup_table *l, int id);
extern char *slookup(const struct lookup_table *l, char *id);
extern int load_lookup(struct lookup_table **l, const char *file);

extern const struct lookup_table description_table[];
extern const struct lookup_table aspect_table[];
extern const struct lookup_table audio_table[];

extern const struct lookup_table languageid_table[];

#ifdef __cplusplus
}
#endif 

#endif
