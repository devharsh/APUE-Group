#include "network.h"

/**
 * Directory Indexing - listing the contents of the directory in alphanumeric
 *                      order. If directory contains index.html, contents of 
 *                      the file are copied to the response.
 * */
int 
traverse_files(struct request *req, struct response *res, struct server_information info)
{
    FTS     *ftsp;
    FTSENT  *ftsent;
    FTSENT  *children;
    int     options;
    char    **arguments;
    char    *contents;
    char    f_name[MAXNAMLEN];
    bool    index_file_found = false;
    char    *html;
    bool    err_flag = false;
    int     status = 500;
    char    *index_path;

    compar = &sortLexographical;
    options = FTS_PHYSICAL | FTS_NOCHDIR;

    arguments = malloc(1 * sizeof(char *));
    if (arguments == NULL) { 
        fprintf(stderr, "Memory error: %s\n", strerror(errno));
    }
    if (req->uri != NULL) {
        arguments[0] = req->uri;
    }

    if((contents = malloc(BUFFERSIZE)) == NULL) {
        fprintf(stderr, "Could not allocate memory: %s \n", strerror(errno));
		exit(1);
    }

    if((index_path = malloc(PATH_MAX)) == NULL) {
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
            if(ftsent->fts_level < 1) {
                if (ftsent->fts_info == FTS_ERR || 
                    ftsent->fts_info == FTS_DNR ||
                    ftsent->fts_info == FTS_NS) {
                        err_flag = true;
                        contents = generate_error_contents(ftsent->fts_errno);
                        break;
                    }
            } 
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

                if (sprintf(f_name, "\
                                    <tr>\n\
                                        <td>\n\
                                            <a href='%s%s'>%s</a>\n\
                                        </td>\n\
                                    </tr>\n", 
                                    children->fts_accpath, children->fts_name, children->fts_name) < 0) {
                    fprintf(stderr, "read error %s\n", children->fts_name);
                }
                
                strcat(contents, f_name);
            }
            children = children->fts_link;
        }
    }

    if(index_file_found) {
        /* TODO: handle code for  index.html */
        index_path = req->uri;
        strcat(index_path, "/index.html");
        (void) fileCopy(res, info, index_path);
    } else  {
        if(err_flag) {
            /* URI/Permissions are invalid - generating html for error */
            /* do something if necessary */
            status = 403;
        } else {
            /* URI/Permissions are valid - generating html for directory listing */
            status = 200;
            contents = prepare_listing_table(contents);
        }
        html = generate_html(contents);
        prepare_response_directorylisting(res, html, status, info);
    }
    
    (void) free(arguments);
    (void) free(contents);
    (void) free(index_path);

    return 0;
}

/**
 * This function generates table for Directory listing
 * */
char*   
prepare_listing_table(char* data){
    char *table;
    char *r_table;

    if((table = malloc(BUFFERSIZE)) == NULL) {
        fprintf(stderr, "Could not allocate memory: %s \n", strerror(errno));
		exit(1);
    }

    if (sprintf(table, "\
                        <table>\n\
                            <thead> \n\
                                <tr> \n\
                                    <th>Name</th> \n\
                                </tr> \n\
                            </thead> \n\
                            <tbody> \n\
                                %s \n\
                            </tbody> \n\
                        </table>\n", data) < 0) {
        fprintf(stderr, "read error %s\n", data);
    }
    
    if ((r_table = strdup(table)) == NULL) {
        fprintf(stderr, "Out of memory.\n");
        exit(EXIT_FAILURE);
    }

    (void) free(table);

    return r_table;
}


/**
 * This function generates error contents if any while 
 * performing directory indexing.
 * */
char*
generate_error_contents(int err_number) {
    char *content;
    char *r_content;

    if((content = malloc(BUFFERSIZE)) == NULL) {
        fprintf(stderr, "Could not allocate memory: %s \n", strerror(errno));
		exit(1);
    }

    switch (err_number)
    {
        case ENOENT:
            /* response code - 404 */
            if (sprintf(content, "<div> 404 Page not found error: %s </div>", strerror(err_number)) < 0) {
                fprintf(stderr, "read error %s\n", strerror(err_number));
            }
            break;
        case EACCES:
            /* response code - 401 */
            if (sprintf(content, "<div> 401 Forbidden request: %s </div>", strerror(err_number)) < 0) {
                fprintf(stderr, "read error %s\n", strerror(err_number));
            }
            break;
        default:
            break;
    }

    if ((r_content = strdup(content)) == NULL) {
        fprintf(stderr, "Out of memory.\n");
        exit(EXIT_FAILURE);
    }

    (void) free(content);

    return r_content;
}


/**
 * This is a comparator function in the FTS - to sort files lexogrraphically
 * */
int 
sortLexographical(const FTSENT **fileEntryPointer, const FTSENT **fileEntryPointerTwo) {
    const FTSENT *file1 = *fileEntryPointer;
    const FTSENT *file2 = *fileEntryPointerTwo;
    return strcmp(file1->fts_name, file2->fts_name);
}

/**
 * This function generates reponse object for all kind of senarios
 * */
void
prepare_response_directorylisting(struct response *res, char* html, int status, struct server_information info) {
    if (html != NULL) {
        res->data = html;
        res->content_type = "text/html";
        res->content_length = strlen(html);
        res->status = status;
        res->server = info.server_name;
    } else {
        /* error response */
    }
}
