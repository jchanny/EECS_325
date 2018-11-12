//Jeremy Chan jsc126
//EECS 325 Proj 4
//Oct 24, 2018
#include <fcntl.h>
#include <math.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <netinet/in.h>

#define NOT_PRESENT 0
#define PRESENT 1
#define ERROR 1
#define SUCCESS 1
#define MAX_PKT_SIZE 1600
#define READ_MODE "r"
#define IP_PKT_SIZE 34

struct pkt_info{
	//captured length of packet
	unsigned short caplen;
	double now;
	unsigned char pkt [MAX_PKT_SIZE];
	struct ether_header *ethh;
	struct iphdr *iph;
	struct tcphdr *tcph;
	struct udphdr *udph;
};

struct meta_info{
	unsigned short caplen;
	unsigned short ignored;
	unsigned int secs;
	unsigned int usecs;
};
	
char mode;
char *traceFilename;

int errexit(char *msg){
	fprintf(stderr, "%s\n", msg);
	exit(ERROR);
}

int isIpPacket(){
	
}

double formatTime(int secs, int usecs){
	double newTime = secs;
	
}

unsigned short next_packet(int fd, struct pkt_info *pinfo){
	struct meta_info meta;
	int bytes_read;

	memset(pinfo, 0x0, sizeof(struct pkt_info));
	memset(&meta, 0x0, sizeof(struct meta_info));

	bytes_read = read(fd, &meta, sizeof(meta));

	if(bytes_read == 0)
		return (0);
	if(bytes_read < sizeof(meta))
		errexit("Cannot read meta information");
	pinfo->caplen = ntohs(meta.caplen);

	double formattedTime = formatTime(meta.secs, meta.usecs);
	pinfo->now = ntohl(formattedTime);
	
	
	if(pinfo->caplen == 0)
		return (1);
	if(pinfo->caplen > MAX_PKT_SIZE)
		errexit("Packet too big");
	
	bytes_read = read(fd, pinfo->pkt, pinfo->caplen);
	
	if(bytes_read < pinfo->caplen)
		errexit("unexpected end of file");
	if(bytes_read < sizeof(struct ether_header))
		return (1);
	
	pinfo->ethh = (struct ether_header *)pinfo->pkt;
	pinfo->ethh->ether_type = ntohs(pinfo->ethh->ether_type);
	
	
/*possibly set pinfo->iph and handle byte order */
	/* possibly set pinfo->udph and handle byte order */
    /* possibly set pinfo->tcph and handle byte order */
	return (1);
}

//returns data from summary mode
int summaryMode(char *traceFile){
	struct pkt_info pinfo;

	int pktCount = 0;
	int ipPkts = 0;
	
	int fd = open("409.trace", O_RDONLY);

	if(fd < 0){
		errexit("Error opening file.");
	}

	next_packet(fd, &pinfo);
	if(pinfo.caplen > 0)
		pktCount++;
	if(pinfo.caplen >= IP_PKT_SIZE && pinfo.ethh->ether_type == 0x0800){
		ipPkts++;
	}
	
	double first_time = pinfo.now;
	double last_time = first_time;
	
	while(next_packet(fd, &pinfo)){
		last_time = pinfo.now;
		if(pinfo.caplen > 0)
			pktCount++;
		if(pinfo.caplen >= IP_PKT_SIZE && pinfo.ethh->ether_type == 0x0800)
			ipPkts++;
	}

	close(fd);
	
	printf("TIME SPAN: %f - %f\n", first_time, last_time);
	printf("TOTAL PACKETS: %i\n", pktCount);
	printf("IP PACKETS: %i\n", ipPkts);
	
	exit(SUCCESS);
}

int lengthMode(){
	exit(SUCCESS);
}

int packetPrintingMode(){
	exit(SUCCESS);
}

int trafficMatrixMode(){
	exit(SUCCESS);
}

int parseargs(int argc, char *argv[]){
	int opt;

	int numModeArgs = 0;
	int tFlag = NOT_PRESENT;
	
	while((opt = getopt(argc, argv, "t:slpm")) != 1){
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
		errexit("Trace filename must be specified.\n");
	}
	if(numModeArgs > 1){
		errexit("Only 1 mode can be used.\n");
	}
	if(numModeArgs == 0){
		errexit("Mode must be specified.\n");
	}

}

int main(int argc, char *argv []){
	char *filenm = "/shout.trace";
	summaryMode(filenm);
	/* parseargs(argc, argv); */
	
	/* if(mode == 's'){ */
	/* 	summaryMode(); */
	/* } */
	/* if(mode == 'l'){ */
	/* 	lengthMode(); */
	/* } */
	/* if(mode == 'p'){ */
	/* 	packetPrintingMode(); */
	/* } */
	/* if(mode == 'm'){ */
	/* 	trafficMatrixMode(); */
	/* } */
	return 0;
}
