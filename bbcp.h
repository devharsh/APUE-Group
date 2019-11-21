#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* 1 MB buffer size for copying */
#ifndef BUF_SIZE
#define BUF_SIZE 1048576 
#endif

/* limit for a file name */
#ifndef BUF_LIMIT
#define BUF_LIMIT 512
#endif
