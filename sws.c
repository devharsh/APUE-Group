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

	CURL *curl;
	CURLcode res;

	curl = curl_easy_init();
	
	if(!curl) {
		perror("CURL Init failed");
		exit(1);
	} else {
		curl_easy_setopt(curl, CURLOPT_URL, "https://www.yahoo.com");
		curl_easy_-setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		res = curl_easy_perform(curl);
		printf("%d\n", atoi(curl));
		curl_easy_cleanup(curl);
		curl = NULL;
	}

	return 0;
}
