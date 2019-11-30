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

void
directory_indexing(char* path) {
	pp[0] = path;
	pp[1] = NULL;

	ftsp = fts_open(pp, fts_options, &name_compare);
	if (ftsp == NULL) {
		perror("fts_open");
		exit(1);
	}

	while(1) {
		ent = fts_read(ftsp);
		if (ent == NULL) {
       	        	if (errno == 0) {
               	        	break;
                       	} else {
                       		perror("fts_read");
                       		exit(1);
                       	}
                }

		if (ent->fts_info == FTS_D || ent->fts_info == FTS_F) {
			if (ent->fts_level == 1) {
				if (listing == NULL) {
					listing = ent->fts_name;
				} else {
					strncat(listing, ent->fts_name, strlen(ent->fts_name));
				}
				strncat(listing, "\n", 1);

				if (strcmp(ent->fts_name, "index.html") == 0) {
					has_index = 1;
					break;
				}
			}	
		}
	}

	if (has_index) {
		/* respond with index.html */
		/* call fileCopy from bbcp */
	} else if (listing != NULL) {
		printf("%s", listing);
	}

	if (fts_close(ftsp) == -1) {
		perror("fts_close");
		exit(1);
	}	
}
