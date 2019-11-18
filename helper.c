#include "helper.h"

int 
name_compare(const FTSENT ** first, const FTSENT ** second) {
        return (strcmp((*first)->fts_name, (*second)->fts_name));
}

void
directory_indexing(char* path) {
	char *pp[2];
	pp[0] = path;
	pp[1] = NULL;

	ftsp = fts_open(pp, fts_options, &name_compare);
	if(ftsp == NULL) {
		perror("fts_open");
		exit(1);
	}

	while(1) {
		FTSENT *ent = fts_read(ftsp);
		if(ent == NULL) {
       	        	if(errno == 0) {
               	        	break;
                       	} else {
                       		perror("fts_read");
                       		exit(1);
                       	}
                }

		if (ent->fts_info == FTS_D || ent->fts_info == FTS_F) {
			printf("%s\n", ent->fts_name);	
		}
	}

	if(fts_close(ftsp) == -1) {
		perror("fts_close");
		exit(1);
	}	
}
