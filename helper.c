#include "helper.h"

char*
get_time_now() {
	time_t now;
	time(&now);
	return ctime(&now);
}

const char*
get_mime_type(char* path) {
	magic_t magic;
	magic = magic_open(MAGIC_MIME_TYPE);
	magic_load(magic, NULL);
	return magic_file(magic, path);
}

int 
name_compare(const FTSENT ** first, const FTSENT ** second) {
        return (strcmp((*first)->fts_name, (*second)->fts_name));
}
