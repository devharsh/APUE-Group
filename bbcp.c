#include "bbcp.h"

int
fileCopy(char* source, char* destination) {
	int inputFD, outputFD;
	char filebuf[BUF_SIZE];
	char target_path[BUF_LIMIT];
	struct stat buf;
	ssize_t number, len;

	if (strcmp(source, destination) == 0) {
		fprintf(stderr, "both files are same (no action)\n");
		exit(1);
	}

	len = readlink(destination, target_path, sizeof(target_path)-1);

	if (len != -1) {
		target_path[len] = '\0';
		if (strcmp(source, target_path) == 0) {
			fprintf(stderr, "both files are same (no action)\n");
			exit(1);
		}
	}
	
	if (access(source, R_OK) != 0) {
		fprintf(stderr, "%s is not readable (access denied)\n", source);
		exit(1);
	}
	
	stat(source, &buf);

	if(!S_ISREG(buf.st_mode)) {
		fprintf(stderr, "Not a valid file: %s\n", source);
		exit(1);
	}
	
	inputFD = open(source, O_RDONLY);

	if(inputFD == -1) {
		fprintf(stderr, "Error opening file: %s\n", source);
		exit(1);
	}

	if(access((dirname(strdup(destination))), W_OK) != 0) {
		fprintf(stderr, "%s is not writable(access denied)", dirname(strdup(destination)));
		exit(1);
	}

	stat(destination, &buf);

	mkdir(dirname(strdup(destination)),0777);

	if(S_ISDIR(buf.st_mode)) {
		strcat(destination,"/");
		strcat(destination,basename(strdup(source)));
	}

	outputFD = open(destination, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	
	if(outputFD == -1) {
		fprintf(stderr, "Error opening file:%s\n", destination);
		exit(1);
	}

	while((number = read(inputFD, filebuf, BUF_SIZE)) > 0) {
		if(write(outputFD, filebuf, number) != number) {
			fprintf(stderr, "Error writing file\n");
			exit(1);
		}
	}

	if(number == -1) {
		fprintf(stderr, "Error writing file\n");
		exit(1);
	}

	if(close(inputFD) == -1) {
		fprintf(stderr, "Error closing file:%s\n", source);
		exit(1);
	}

	if(close(outputFD) == -1) {
		fprintf(stderr, "Error closing file:%s\n", destination);
		exit(1);
	}

	return(0);
}

