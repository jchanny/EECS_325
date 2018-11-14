//Jeremy Chan jsc126
//EECS 325 Proj 4
//Oct 24, 2018
//Program to analyze packets
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
#include <arpa/inet.h>
#include <sys/socket.h>

#define NOT_PRESENT 0
#define PRESENT 1
#define ERROR 1
#define SUCCESS 1
#define MAX_PKT_SIZE 1600
#define READ_MODE "r"
#define IP_PKT_SIZE 34
#define IP_TYPE 0x0800
#define TCP_PROTOCOL_ID 6
#define UDP_PROTOCOL_ID 17
#define MIN_IP_HDR_LEN 5
#define MAX_IP_HDR_LEN 6
#define MIN_TCP_HEADER_SIZE 20
#define UDP_HEADER_SIZE 8

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


double formatUsecs(int usecs){
	double usecsD = usecs;
	usecsD = usecsD/pow(10,9);
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

	double formattedTime = ntohl((double)meta.secs);
	double usecs = ntohl(meta.usecs);
	usecs = usecs/pow(10,6);
	formattedTime = formattedTime + usecs;
	pinfo->now = formattedTime;
	
	
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

	//handle IP packet
	if(pinfo->caplen >= IP_PKT_SIZE && pinfo->ethh->ether_type == IP_TYPE){
		pinfo->iph = (struct iphdr *)(pinfo->pkt + sizeof(struct ether_header));

		//store correct transport packet in pinfo
		if(pinfo->iph->protocol == TCP_PROTOCOL_ID){
			pinfo->tcph = (struct tcphdr *)(pinfo->pkt + sizeof(struct iphdr) + sizeof(struct ether_header));
		}
		else if(pinfo->iph->protocol == UDP_PROTOCOL_ID){
			pinfo->udph = (struct udphdr *)(pinfo->pkt + sizeof(struct iphdr) + sizeof(struct ether_header));
		}
	}

	return (1);
}

//returns data from summary mode
int summaryMode(char *traceFile){
	struct pkt_info pinfo;

	int pktCount = 0;
	int ipPkts = 0;
	
	int fd = open(traceFile, O_RDONLY);

	if(fd < 0){
		errexit("Error opening file.");
	}

	next_packet(fd, &pinfo);
	if(pinfo.caplen > 0)
		pktCount++;
	if(pinfo.caplen >= IP_PKT_SIZE && pinfo.ethh->ether_type == IP_TYPE){
		ipPkts++;
	}
	
	double first_time = pinfo.now;
	double last_time = first_time;
	
	while(next_packet(fd, &pinfo)){
		last_time = pinfo.now;
		if(pinfo.caplen > 0)
			pktCount++;
		if(pinfo.caplen >= IP_PKT_SIZE && pinfo.ethh->ether_type == IP_TYPE)
			ipPkts++;
	}

	close(fd);
	
	printf("TIME SPAN: %f - %f\n", first_time, last_time);
	printf("TOTAL PACKETS: %i\n", pktCount);
	printf("IP PACKETS: %i\n", ipPkts);
	
	exit(SUCCESS);
}

int readIpPacket(struct pkt_info *pinfo){
	double ts = pinfo->now;
	int caplen = pinfo->caplen;
	int ip_len = -1;
	int iphl = -1;
	char transport = '-';
	int protocol = -1;
	//check if entire IP header is present
	if(caplen >= IP_PKT_SIZE){
		ip_len = ntohs(pinfo->iph->tot_len);
		iphl = pinfo->iph->ihl;

		if(iphl == MIN_IP_HDR_LEN)
			iphl = 20;
		if(iphl == MAX_IP_HDR_LEN)
			iphl = 24;
		
		protocol = pinfo->iph->protocol;
		
		if(protocol == TCP_PROTOCOL_ID)
			transport = 'T';
		else if(protocol == UDP_PROTOCOL_ID)
			transport = 'U';
		else
			transport = '?';

		printf("%f %i %i %i %c", ts, caplen, ip_len, iphl, transport);

		if(transport == '?'){
			printf(" ? ?");
		}
		else{
			int trans_hl = ip_len - iphl;
			if((transport == 'T' && trans_hl < MIN_TCP_HEADER_SIZE) || (transport == 'U' && trans_hl < UDP_HEADER_SIZE))
				printf(" -");
			else
				printf(" %i", trans_hl);

			//if transport is ?, then payload len ?
			//if TCP or UDP header not presnet print 0
		}
	}//IP hdr not present fully
	else{
		printf("%f %i - - - - -", ts, caplen);
	}

	printf("\n");
	return 0;
}

//print out data about IP packets in trace file
int lengthMode(char *traceFile){
	struct pkt_info pinfo;
	
	int fd = open(traceFile, O_RDONLY);
	if(fd < 0){
		errexit("Error opening file.");
	}

	while(next_packet(fd, &pinfo)){
		//if ethernet header not present or not IP packet, ignore
		if(pinfo.caplen >= sizeof(struct ether_header) && pinfo.ethh->ether_type == IP_TYPE){
			readIpPacket(&pinfo);
		}
	}
	
	exit(SUCCESS);
}

//helper method that transforms int32 to ip address string
char *intToIpAddress(u_int32_t ipAddr){
	struct in_addr ip_addr;
	ip_addr.s_addr = ipAddr;
	char *ip_addr_str = inet_ntoa(ip_addr);
	//now reverse string
	return ip_addr_str;
}

int processTcpPacket(struct pkt_info *pinfo){
	double ts = pinfo->now;

	char *src_ip = intToIpAddress(pinfo->iph->saddr);
	char *dst_ip = intToIpAddress(pinfo->iph->daddr);
	
	int src_port = ntohs(pinfo->tcph->th_sport);
	int dest_port = ntohs(pinfo->tcph->th_dport);
	
}

int packetPrintingMode(char *traceFile){
	struct pkt_info pinfo;
	
	int fd = open(traceFile, O_RDONLY);
	if(fd < 0)
		errexit("Error opening file.");

	while(next_packet(fd, &pinfo)){
		if(pinfo.iph->protocol == TCP_PROTOCOL_ID){
			processTcpPacket(&pinfo);
		}
	}
	
	exit(SUCCESS);
}

int trafficMatrixMode(){
	exit(SUCCESS);
}

int parseargs(int argc, char *argv[]){
	int opt;

	int numModeArgs = 0;
	int tFlag = NOT_PRESENT;

	if(argc == 1)
		errexit("Mode and trace filename need to be specified");
	
	while((opt = getopt(argc, argv, "t:slpm")) != -1){
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
		errexit("Trace filename must be specified.");
	}
	if(numModeArgs > 1){
		errexit("Only 1 mode can be used.");
	}
	if(numModeArgs == 0){
		errexit("Mode must be specified.");
	}

	return 0;
}

int main(int argc, char *argv []){
	parseargs(argc, argv);
	
	if(mode == 's'){
		summaryMode(traceFilename);
	}
	if(mode == 'l'){
		lengthMode(traceFilename);
	}
	if(mode == 'p'){
		packetPrintingMode(traceFilename);
	}
	if(mode == 'm'){
		trafficMatrixMode();
	}
	return 0;
}
