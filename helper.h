#include <sys/stat.h>

#include <dirent.h>
#include <errno.h>
#include <fts.h>
#include <magic.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

FTS *ftsp;
FTSENT *ent;

int 
name_compare(const FTSENT ** first, const FTSENT ** second); 

char*
get_time_now();

const char*
get_mime_type(char* path);
