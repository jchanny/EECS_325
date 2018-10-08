//simple web server
//By Jeremy Chan, jsc126
#define SERVER_SHUTTING_DOWN 200
#define OK 200
#define MALFORMED_REQUEST 400
#define OPERATION_FORBIDDEN 403
#define FILE_NOT_FOUND 404
#define UNSUPPORTED_METHOD 405
#define INVALID_FILENAME 406
#define PROTOCOL_NOT_IMPLEMENTED 501

int port;
char *directory;
char *authToken;

int pFlag = 0;
int dFlag = 0;
int aFlag = 0;

int parseargs(int argc, char *argv[]){
	int opt;

	while((opt = getopt(argc, argv, "p:d:a:")) != -1){
		switch(opt){
		case 'p' :
			port = optarg;
			if(port < 0){
				fprintf(stderr, "Port number must be between 1024 and 49151\n");
				return 1;
			}
			if(port <= 1024){
				fprintf(stderr, "Ports 0-1024 are reserved and cannot be used\n");
				return 1;
			}
			break;
		case 'd':
			directory = optarg;
			break;
		case 'a':
			authToken = optarg;
			break;
		}
		
	}

	if(pFlag == 0){
		fprintf(stderr, "Port number must be specified\n");
		return 1;
	}
	if(dFlag == 0){
		fprintf(stderr, "Directory must be specified\n");
		return 1;
	}
	if(aFlag == 0){
		fprintf(stderr, "Authentication token needs to be provided\n");
		return 1;
	}
	
	return 0;
}

int main(int argc, char *argv []){
	parseargs(argc, argv);
	return 0;
}
