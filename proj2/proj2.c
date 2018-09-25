//main class for project 2
#include <stdio.h>
#include <unistd.h>

const int DEFAULT_PORT_NUMBER = 80;
const char* DEFAULT_FILENAME = "/";

char* url;
int port;
char* url_filename;
char* local_filename;

int dOption = 0;

//-d option
void printCommandLineParams(){
	printf("DET: hostname = %s \n", url);
	printf("DET: port = %i \n", port);
	printf("DET: web_filename = %s \n", url_filename);
	printf("DET: output_filename = %s \n", local_filename);
}

int parseargs(int argc, char *argv []){
	int opt;

	int urlPresent = 0;
	int localFilenamePresent = 0;
	
	while ((opt = getopt(argc, argv, "u:do:")) != -1){
		switch(opt){
		case 'u':
			url = optarg;
			urlPresent = 1;
			break;
		case 'd' :
			dOption = 1;
			break;
		case 'o' :
			local_filename = optarg;
			localFilenamePresent = 1;
			break;
		case '?':
			if(optopt == 'u')
				fprintf(stderr, "URL needs to be specified.\n");
			if(optopt == 'o')
				fprintf(stderr, "Output filename needs to be specified.\n");
			else
				fprintf(stderr, "Unsupported command flag used.\n");
			return 1;
		}
		
	}

	if(urlPresent == 0){
		fprintf(stderr, "Missing URL parameter.\n");
		return 1;
	}
	if(localFilenamePresent == 0){
		fprintf(stderr, "Missing write file parameter.\n");
		return 1;
	}
	
	return 0;
}

int main(int argc, char *argv []){
	port = DEFAULT_PORT_NUMBER;
	url_filename = DEFAULT_FILENAME;
	
	parseargs(argc, argv);

	if(dOption){
		printCommandLineParams();
	}
	return 0;
}
