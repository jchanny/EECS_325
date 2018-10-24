//Jeremy Chan jsc126
//EECS 325 Proj 4
//Oct 24, 2018
#include <stdio.h>
#include <stdlib.h>

#DEFINE NOT_PRESENT 0
#DEFINE PRESENT 1
#DEFINE ERROR 1

char mode;
char *traceFilename;

int error(char *arg){
	fprintf(stderr, "%s", arg);
	exit(ERROR);
}

int parseargs(int argc, char *argv[]){
	int opt;

	int numModeArgs = 0;
	int tFlag = NOT_PRESENT;
	
	while((opt = getopt(argc, argv, "t:slpm"))! = 1){
		switch(opt){
		case 't':
			traceFilename = optarg;
			tFlag = PRESENT;
			break;
		case 's':
			mode = 's';
			numModeArgs ++;
			break;
		case 'l':
			mode = 'l';
			numModeArgs++;
			break;
		case 'p':
			mode = 'p';
			numModeArgs++;
			break;
		case 'm':
			mode = 'm';
			numModeArgs++;
			break;
		}
	}

	if(tFlag == NOT_PRESENT){
		error("Trace filename must be specified.\n");
	}
	if(numModeArgs > 1){
		error("Only 1 mode can be used.\n");
	}
	if(numModeArgs == 0){
		error("Mode must be specified.\n");
	}
}

int main(int argc, char *argv []){
	parseargs(argc, argv);
	
	return 0;
}
