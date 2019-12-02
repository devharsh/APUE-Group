#include "bbcp.h"

int
fileCopy(struct response* res, struct server_information server_info, char* source) {
	int inputFD;
	char filebuf[BUF_SIZE];
	struct stat buf;
	ssize_t number;

	if (access(source, R_OK) != 0) {
		fprintf(stderr, "%s is not readable (access denied)\n", source);
		generate_error_response(res, server_info, 500, "Internal Server Error");
		return 1;
	}
	
	if (stat(source, &buf) != 0) {
		fprintf(stderr, "%s is not readable (access denied)\n", source);
		generate_error_response(res, server_info, 500, "Internal Server Error");
		return 1;
	}

	if (!S_ISREG(buf.st_mode)) {
		fprintf(stderr, "Not a valid file: %s\n", source);
		generate_error_response(res, server_info, 500, "Internal Server Error");
		return 1;
	}
	
	inputFD = open(source, O_RDONLY);

	if (inputFD == -1) {
		fprintf(stderr, "Error opening file: %s\n", source);
		generate_error_response(res, server_info, 500, "Internal Server Error");
		return 1;
	}

	while ((number = read(inputFD, filebuf, BUF_SIZE)) > 0) {
		if(data == NULL) {
			data = filebuf;
		} else {	
			strncat(data, filebuf, BUF_SIZE);
		}
	}

	if (number == -1) {
		fprintf(stderr, "Error writing file\n");
		generate_error_response(res, server_info, 500, "Internal Server Error");
		return 1;
	}

	if (close(inputFD) == -1) {
		fprintf(stderr, "Error closing file:%s\n", source);
		generate_error_response(res, server_info, 500, "Internal Server Error");
		return 1;
	}

	if (data != NULL) {
		res->status = 200;
		res->content_length = strlen(data);
		res->content_type = get_mime_type(source);
		res->data = data;
		res->server = server_info.server_name;
	} else {
		fprintf(stderr, "%s is not readable (access denied)\n", source);
		generate_error_response(res, server_info, 500, "Internal Server Error");
		return 1;
	}

	return 0;
}
