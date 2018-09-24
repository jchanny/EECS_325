//main class for project 2
#include <stdio.h>
#include <unistd.h>

char* url;

int parseargs(int argc, char *argv []){
	int opt;

	if(argc == 1){
		fprintf(stderr, "-u flag and URL are required.\n");
		return 1;
	}
	
	while ((opt = getopt(argc, argv, "u:")) != -1){
		switch(opt){
		case 'u':
			url = optarg;
			break;
		case '?':
			if(optopt == 'u')
				fprintf(stderr, "URL needs to be specified.\n");
			else
				fprintf(stderr, "Unsupported command flag used.\n");
			return 1;
		}
		
	}
	return 0;
}

int main(int argc, char *argv []){
	parseargs(argc, argv);
	return 0;
}
