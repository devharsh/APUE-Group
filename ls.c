#include "network.h"

/**
 * Directory Indexing - listing the contents of the directory in alphanumeric
 *                      order. If directory contains index.html, contents of 
 *                      the file are copied to the response.
 * */
int 
traverse_files(struct request *req)
{
    FTS     *ftsp;
    FTSENT  *ftsent;
    FTSENT  *children;
    int     options;
    char    **arguments;
    char    *files;
    char    f_name[MAXNAMLEN];
    bool    index_file_found = false;

    compar = &sortLexographical;
    options = FTS_PHYSICAL | FTS_NOCHDIR;

    arguments = malloc(1 * sizeof(char *));
    if (arguments == NULL) { 
        fprintf(stderr, "Memory error: %s\n", strerror(errno));
    }
    arguments[0] = req->uri;

    if((files = malloc(BUFFERSIZE)) == NULL) {
        fprintf(stderr, "Could not allocate memory: %s \n", strerror(errno));
		exit(1);
    }

    if ((ftsp = fts_open(arguments, options, compar)) == NULL) {
        fprintf(stderr, "FTS error : %s\n", strerror(errno));
    }

    while ((ftsent = fts_read(ftsp)) != NULL)
    {
        if (ftsent->fts_level > 1) {
            fts_set(ftsp, ftsent, FTS_SKIP);
            continue;
        }

        children = fts_children(ftsp, 0);

        if (children == NULL) {
            continue;
        }

        if (children->fts_level > 1) {
            continue;
        }

        while (children != NULL) { 
            if(strncmp(children->fts_name, ".", 1) != 0) {

                if(strcmp(children->fts_name, "index.html") == 0) {
                    index_file_found = true;
                    break;
                }

                if (sprintf(f_name, "<tr> <td> <a href='#'> %s </a> </td> </tr>\n", children->fts_name) < 0) {
                    fprintf(stderr, "read error %s\n", children->fts_name);
                }
                
                strcat(files, f_name);
            }
            children = children->fts_link;
        }
    }

    if(index_file_found) {
        /* handle code for  index.html */
    } else  {
        // printf("%s", files);
        generate_html(files);
        /*  generate response. */
    }
    
    (void) free(arguments);
    (void) free(files);

    return 0;
}


void 
generate_html(char* data) {
    char *html;

    if((html = malloc(BUFFERSIZE)) == NULL) {
        fprintf(stderr, "Could not allocate memory: %s \n", strerror(errno));
		exit(1);
    }

    if (sprintf(html, "<!DOCTYPE html> <html> <body> <table> <thead> <tr class='header'> <th> Name </th> </tr> </thead> <tbody> %s </tbody> </table> </body> </html>\n", data) < 0) {
        fprintf(stderr, "read error %s\n", data);
    }

    printf("%s", html);
    
    (void) free(html);
}


/**
 * This is a comparator function in the FTS - to sort files lexogrraphically
 * */
int sortLexographical(const FTSENT **fileEntryPointer, const FTSENT **fileEntryPointerTwo) {
    const FTSENT *file1 = *fileEntryPointer;
    const FTSENT *file2 = *fileEntryPointerTwo;
    return strcmp(file1->fts_name, file2->fts_name);
}