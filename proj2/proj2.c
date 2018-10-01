//main class for project 2
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

#define ERROR 1
#define BUF_LEN 1024
#define PROTOCOL "tcp"
#define RESPONSE_CODE_LENGTH 3

const int DEFAULT_PORT_NUMBER = 80;
const char* DEFAULT_FILENAME = "/";

int port;
char* hostname;
char* url_filename;
char* local_filename;
int responseCode;
int dOption = 0;
int rOption = 0;
int bigRoption = 0;

void error(char *msg){
	fprintf(stderr, msg);
	fprintf(stderr, "\n"); 
	exit(ERROR);
}

//separates URL into hostname, port, path to file
//return 0 if url parsed successfully, 1 otherwise
int parseUrl(url){
	const int protocolLen = 7;
	char* slash = "/";
	char* colon = ":";
	
	int urlLen = strlen(url) + 1;
	if(urlLen < 8){
		fprintf(stderr, "URL must begin with http://");
		return ERROR;
	}

	//convert url to char array
	char fullUrl [urlLen];
	strncpy(fullUrl, url, sizeof(fullUrl) - 1);
	fullUrl[sizeof(fullUrl) - 1] = '\0';

	//check that http protocol is specified, case insensitive
	char* expectedProtocol = "http://";
	if(strncasecmp(fullUrl, expectedProtocol, protocolLen) != 0){
		fprintf(stderr, "HTTP protocol must be used\n");
		return ERROR;
	}

	//chop off http://
	//http://hi.com:90/helloThere -> becomes: hi.com:90/helloThere
	char *noProtocolUrl = fullUrl + protocolLen;
	
    //extract hostname
    //check if colon exists
    if(strstr(noProtocolUrl, colon)){
        //everything after the hostname
        char *endOfHostname = strstr(noProtocolUrl, colon);
        
        char tempHostnameHolder[strlen(noProtocolUrl) - strlen(endOfHostname) +1 ];
        strncpy(tempHostnameHolder, noProtocolUrl, sizeof(tempHostnameHolder) - 1);
        tempHostnameHolder[sizeof(tempHostnameHolder) - 1] = '\0';
        hostname = (char *)malloc(strlen(tempHostnameHolder)+1);
        strcpy(hostname, tempHostnameHolder);
       
        //if web filename exists
        if(strstr(endOfHostname, slash)){
            char endOfHostnameArray[strlen(endOfHostname)];
            endOfHostnameArray[sizeof(endOfHostnameArray) - 1] = '\0';
            char *webFilename = strstr(endOfHostname, slash);
            url_filename = (char *)malloc(strlen(webFilename) + 1);
            strcpy(url_filename,webFilename);
            
            //grab port number
            char portStringWithColon[strlen(endOfHostname) - strlen(url_filename) + 1];
            strncpy(portStringWithColon, endOfHostname, sizeof(portStringWithColon) - 1);
            portStringWithColon[sizeof(portStringWithColon) - 1] = '\0';
            //advance portString pointer by 1 to remove the colon
            int colonOffset = 1;
            char *portString = portStringWithColon + colonOffset;
            port = atoi(portString);
        }
        else{
            //web filename doesn't exist
            char endOfHostnameArray[strlen(endOfHostname)];
            endOfHostnameArray[sizeof(endOfHostnameArray) - 1] = '\0';
            strcpy(endOfHostnameArray, endOfHostname);
            char *portString = endOfHostnameArray + 1;
            port = atoi(portString);
            url_filename = (char *)malloc(strlen(slash) + 1);
            strcpy(url_filename, slash);
        }
    }else{
        port = 80;
        //if web filename exists
        if(strstr(noProtocolUrl, slash)){
            //extract hostname from url to endOfHostname
            char *endOfHostname = strstr(noProtocolUrl, slash);
			char tempHostnameHolder[strlen(noProtocolUrl) - strlen(endOfHostname) +1 ];
			strncpy(tempHostnameHolder, noProtocolUrl, sizeof(tempHostnameHolder) - 1);
            tempHostnameHolder[sizeof(tempHostnameHolder) - 1] = '\0';
            hostname = (char *)malloc(strlen(tempHostnameHolder)+1);
            strcpy(hostname, tempHostnameHolder);
            
            //extract url_filename
            url_filename = (char *)malloc(strlen(endOfHostname) + 1);
            strcpy(url_filename,endOfHostname);
        }else{
            hostname = (char *)malloc(strlen(noProtocolUrl) + 1);
            strcpy(hostname, noProtocolUrl);
            url_filename = (char *)malloc(strlen(slash) + 1);
            strcpy(url_filename, slash);
            
        }
        
    }

	
	return 0;
}

int parseargs(int argc, char *argv []){
	int opt;

	int urlPresent = 0;
	int localFilenamePresent = 0;
	char* urlString;
	
	while ((opt = getopt(argc, argv, "u:do:rR")) != -1){
		switch(opt){
		case 'u':
			if(strlen(optarg) == 0){
				fprintf(stderr,"missing URL");
				return ERROR;
			}
			urlString = (char *)malloc(strlen(optarg) + 1);
			strcpy(urlString, optarg);
			urlPresent = 1;
			break;
		case 'd' :
			dOption = 1;
			break;
		case 'o' :
			local_filename = optarg;
			localFilenamePresent = 1;
			break;
		case 'r':
			rOption = 1;
			break;
		case 'R':
			bigRoption = 1;
			break;
		case '?':
			if(optopt == 'u')
				fprintf(stderr, "URL needs to be specified.\n");
			if(optopt == 'o')
				fprintf(stderr, "Output filename needs to be specified.\n");
			else
				fprintf(stderr, "Unsupported command flag used.\n");
			return ERROR;
		}
		
	}

	if(urlPresent == 0){
		fprintf(stderr, "Missing URL parameter.\n");
		return ERROR;
	}
	if(localFilenamePresent == 0){
		fprintf(stderr, "Missing write file parameter.\n");
		return ERROR;
	}

	return parseUrl(urlString);
}

//-d option
void printCommandLineParams(){
	printf("DET: hostname = %s \n", hostname);
	printf("DET: port = %i \n", port);
	printf("DET: web_filename = %s \n", url_filename);
	printf("DET: output_filename = %s \n", local_filename);
}

//-r option, print out HTTP request
void printHttpRequest(){
	printf("REQ: GET %s HTTP/1.0\n", url_filename);
	printf("REQ: Host: %s\n", hostname);
	printf("REQ: User-Agent: CWRU EECS 325 Client 1.0\n");
}

void printResponseHeader(char responseHeader[]){
	char *tok;
	char *delim = "\r\n";
	tok = strtok(responseHeader, delim);

	while(tok != NULL){
		printf("RSP: %s\n",tok);
		tok = strtok(NULL, delim);
	}
}

//takes in raw response buffer from server, separates response header from content
int parseResponse(char buffer[]){
	//parse response code, content length
	int contentLength;
	char* httpProtocolAndVersion = "HTTP/1.0 ";
	char responseCodeStr[4];
	strncpy(responseCodeStr, buffer + strlen(httpProtocolAndVersion), 3);
	responseCodeStr[3] = '\0';
	responseCode = atoi(responseCodeStr);

	//extract and separate out response header
	char* breakoffLine = "\r\n\r\n";
	char* responseContent = strstr(buffer, breakoffLine);
	//delete 2 empty spaces from responseContent
	char* content = responseContent + 4;
   
	char responseHeader[strlen(buffer) - strlen(responseContent) + 1];
	strncpy(responseHeader, buffer, strlen(buffer) - strlen(responseContent));
	responseHeader[sizeof(responseHeader) - 1] = '\0';
	
	
	if(bigRoption){
		printResponseHeader(responseHeader);
	}
	//write content to file if response code = 200
	if(responseCode == 200){
		FILE *fp = fopen(local_filename, "w");
		if(fp < 0){
			fprintf(stderr, "Error opening file.\n");
			return 1;
		}

		fprintf(fp, content);
		fclose(fp);
	}else{
		fprintf(stderr, "Response code was %i. There will be nothing written to output file.\n", responseCode);
		return 1;
	}

	return 0;
}

//method opens socket, initiates connection, processes request
int makeGetRequest(){
	struct sockaddr_in serv_addr;
	struct protoent *protoinfo;
	struct hostent *hinfo;
	char buffer[BUF_LEN];
   
	int sock, ret, n;
		
	hinfo = gethostbyname(hostname);
	if(hinfo == NULL){
		error("Host not found.");
		return 1;
	}

	
	memset((char *)&serv_addr, 0x0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	memcpy((char *)&serv_addr.sin_addr, hinfo->h_addr, hinfo->h_length);

	if((protoinfo = getprotobyname(PROTOCOL)) == NULL){
		error("Cannot find protocol info");
		return 1;
	}

	sock = socket(PF_INET, SOCK_STREAM, protoinfo->p_proto);
	if(sock < 0){
		error("Cannot create socket");
		return 1;
	}

	if(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
		error("Socket connection error. Check your internet connection");
		return 1;
	}

	//write get request to buffer
	snprintf(buffer, sizeof(buffer), "GET %s HTTP/1.0\r\nHost: %s\r\nUser-Agent: CWRU EECS 325 Client 1.0\r\n\r\n", url_filename, hostname);
	n = write(sock, buffer, strlen(buffer));
	if(n < 0){
		error("Error writing to socket");
		return 1;
	}
		
	//read buffer
	memset(buffer, 0x0, BUF_LEN);
	ret = read(sock, buffer, BUF_LEN - 1);
	
	if(ret < 0){
		error("Error reading buffer.");
		return 1;
	}

	parseResponse(buffer);
	if(responseCode == 200){
		FILE *fp = fopen(local_filename, "a");
		if(fp < 0){
			fprintf(stderr, "Error writing to file.\n");
			return 1;
		}
		
		while(ret > 0){
			memset(buffer, 0x0, BUF_LEN);
			ret = read(sock, buffer, BUF_LEN -1);
			fprintf(fp, "%s", buffer);
		}
		fclose(fp);
	}
	
	close(sock);
	
	return 0;
}

int main(int argc, char *argv []){

	port = DEFAULT_PORT_NUMBER;
	url_filename = DEFAULT_FILENAME;
	
	if(parseargs(argc, argv))
		return ERROR;

	if(dOption){
		printCommandLineParams();
	}
	if(rOption){
		printHttpRequest();
	}

	if(makeGetRequest())
		return ERROR;
		
	//here do the actual fetch stuff
	return 0;
}
