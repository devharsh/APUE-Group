#include <sys/stat.h>

#include <dirent.h>
#include <errno.h>
#include <fts.h>
#include <magic.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int fts_options = FTS_COMFOLLOW | FTS_NOCHDIR | FTS_PHYSICAL;
int has_index = 0;

char *pp[2];
char* listing = NULL;

FTS *ftsp;
FTSENT *ent;

int 
name_compare(const FTSENT ** first, const FTSENT ** second); 

void
directory_indexing(char* path);

const char*
get_mime_type(char* path);
