#include "sws.h"

int
main(int argc, char* argv[]) {
	int opt = 0;

	while((opt = getopt(argc, argv,"ialp")) != -1) {  
        	switch(opt) {
			case 'a':
				break;
			default:
				break;
		}
	}

	printf("%d\n", argc);
	return 0;
}
