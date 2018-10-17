//simple web server, proj3
//By Jeremy Chan, jsc126
//Oct 8, 2018
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define SHUTTING_DOWN "HTTP/1.1 200 Server Shutting Down\r\n\r\n"
#define MALFORMED_REQUEST "HTTP/1.1 400 Malformed Request\r\n\r\n"
#define OPERATION_FORBIDDEN "HTTP/1.1 403 Operation Forbidden\r\n\r\n"
#define FILE_NOT_FOUND "HTTP/1.1 404 File Not Found\r\n\r\n"
#define UNSUPPORTED_METHOD "HTTP/1.1 405 Unsupported Method\r\n\r\n"
#define INVALID_FILENAME "HTTP/1.1 406 Invalid Filename\r\n\r\n"
#define PROTOCOL_NOT_IMPLEMENTED "HTTP/1.1 501 Protocol Not Implemented\r\n\r\n"
#define NOT_PRESENT 0
#define PRESENT 1
#define SUCCESS 0
#define ERROR 1
#define PROTOCOL "tcp"
#define QLEN 5
#define BUF_LEN 8096
#define DEFAULT_FILENAME "/default.html"
#define READ_MODE "r"

int port;
char *directory;
char *authToken;

int pFlag = NOT_PRESENT;
int dFlag = NOT_PRESENT;
int aFlag = NOT_PRESENT;

int error(char *arg){
	fprintf(stderr, "%s\n", arg);
	exit(ERROR);
}

int parseargs(int argc, char *argv[]){
	int opt;

	while((opt = getopt(argc, argv, "p:d:a:")) != -1){
		switch(opt){
		case 'p' :
			port = atoi(optarg);
			if(port < 0 || port > 65535){
				error("Port number must be between 1025 and 65535\n");
			}
			if(port <= 1024){
				error("Ports 0-1024 are reserved and cannot be used\n");
			}
			pFlag = PRESENT;
			break;
		case 'd':
			directory = optarg;
			dFlag = PRESENT;
			break;
		case 'a':
			authToken = optarg;
			aFlag = PRESENT;
			break;
		}
		
	}

	if(pFlag == NOT_PRESENT){
		error("Port number must be specified\n");
	}
	if(dFlag == NOT_PRESENT){
		error("Directory must be specified\n");
	}
	if(aFlag == NOT_PRESENT){
		error("Authentication token needs to be provided\n");
	}
	
	return SUCCESS;
}

//writes message to socket
int sendResponse(int socket, char *message){
	int bytesWritten;
	char buffer[BUF_LEN];
	snprintf(buffer, sizeof(buffer), "%s", message);
	bytesWritten = write(socket, buffer, strlen(buffer));
	if(bytesWritten < 0){
		error("Error writing to socket.");
		return 1;
	}
	return 0;
}

//attempts to open file, and send file to client
int getFile(int socket, char *filename){
		//if filename doesn't begin with /
	if(filename[0] != '/'){
		sendResponse(socket, INVALID_FILENAME);
		return 1;
	}
	FILE *fp;

	char *fullPath;
	//check if default filename should be used
	if(strcmp(filename, "/") == 0){
		//look for default.html
		fullPath = malloc(strlen(directory) + strlen(DEFAULT_FILENAME) + 1);
		strcpy(fullPath, directory);
		strcat(fullPath, DEFAULT_FILENAME);
	}else{
		fullPath  = malloc(strlen(directory) + strlen(filename) + 1);
		strcpy(fullPath, directory);
		strcat(fullPath, filename);
	}
	
	fp = fopen(fullPath, READ_MODE);
	
	if(fp < 0){
		error("Error opening file");
	}


	//now write fp to socket
	return 0;
}

int quitOperation(int socket, char *token){
	if(strcmp(token, authToken) == 0){
		sendResponse(socket, SHUTTING_DOWN);
		exit(SUCCESS);
	}
	else{
		sendResponse(socket, OPERATION_FORBIDDEN);
		return 1;
	}
	return 0;
}

//do what client requests
int completeRequest(int socket, char *firstLine){
	//this forces protocol not implemented to have higher priority
	if(strstr(firstLine, " HTTP/") == NULL){
		sendResponse(socket, PROTOCOL_NOT_IMPLEMENTED);
		return 1;
	}
	
	if(strstr(firstLine, "GET") != NULL){
		char *filenamePtr = strstr(firstLine, "GET ");
		char *endFilenamePtr = strstr(firstLine, " HTTP/");
						
		char *tmpFilename = filenamePtr + strlen("GET ");

		//the requested filename
		char filename[strlen(tmpFilename) - strlen(endFilenamePtr)];
		filename[sizeof(filename) - 1] = '\0';
		strncpy(filename, tmpFilename, sizeof(filename));

		getFile(socket, filename);
		
	}else if(strstr(firstLine, "QUIT") != NULL){
		char *quitPtr = strstr(firstLine, "QUIT ");
		char *endTokenPtr = strstr(firstLine, " HTTP/");
		
		char *tmpToken = quitPtr + strlen("QUIT ");
		char token[strlen(tmpToken) - strlen(endTokenPtr)];
		token[sizeof(token) - 1] = '\0';
		strncpy(token, tmpToken, sizeof(token));

		quitOperation(socket, token);
		
	}
	else{
		sendResponse(socket, UNSUPPORTED_METHOD);
		return 1;
	}
	
	return 0;

}

//handles connection with client
int handleClientConnection(int socket){
	char buffer[BUF_LEN];
	int bytesRead;
	char *newLine = "\r\n";

	memset(buffer, 0x0, BUF_LEN);
	bytesRead = read(socket, buffer, BUF_LEN - 1);

	if(bytesRead < 0){
		//request is empty, throw error
		return 1;
	}
	
	//extract request line, copy buffer fist
	char bufferCpy[BUF_LEN];
	strncpy(bufferCpy, buffer, BUF_LEN);
	char *request = strtok(bufferCpy, "\r\n");

	if(strstr(buffer, "\r\n\r\n") == NULL){
		return sendResponse(socket, MALFORMED_REQUEST);
	}
	
	if(request){
		return completeRequest(socket, request);
	}
	else{
		return sendResponse(socket, MALFORMED_REQUEST);
	}
	
	return 0;
}

int main(int argc, char *argv []){
	parseargs(argc, argv);

	struct sockaddr_in sin;
	struct sockaddr addr;
	struct protoent *protoinfo;
	unsigned int addrlen;
	int sd, sd2;

	if((protoinfo = getprotobyname(PROTOCOL)) == NULL)
		error("No protocol info found.");
	
	memset((char *)&sin, 0x0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(port);

	//create socket
	sd = socket(PF_INET, SOCK_STREAM, protoinfo->p_proto);
	if(sd < 0)
		error("Socket could not be created");

	if(bind(sd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
		error("Could not bind socket to port specified.");

	if(listen(sd, QLEN) < 0){
		error("Could not listen on port.");
	}
	
	while(1){
		sd2 = accept(sd, &addr, &addrlen);
		if(sd2 < 0){
			close(sd2);
			close(sd);
			error("Error accepting connection.");
		}

		handleClientConnection(sd2);
		
		//close socket, wait for next connection
		close(sd2);
	}

	close(sd);
	exit(SUCCESS);
}
