#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

struct pH{
	u_int32_t scrAddr;
	u_int32_t destAddr;
	u_int8_t placeHolder;
	u_int8_t protocol;
	u_int16_t udpLen;
};

unsigned short CheckSum(unsigned short *ptr,int nbytes) 
{
	register long sum;
	unsigned short oddbyte;
	register short answer; 
	sum=0;
	while(nbytes>1) {
		sum+=*ptr++;
		nbytes-=2;
	}
	if(nbytes==1) {
		oddbyte=0;
		*((u_char*)&oddbyte)=*(u_char*)ptr;
		sum+=oddbyte;
	}
	sum = (sum>>16)+(sum & 0xffff);
	sum = sum + (sum>>16);
	answer=(short)~sum;
	return(answer);
}

void UdpPacketSend(struct sockaddr_in *srcHost, struct sockaddr_in *destHost, char *udpData)
{
	int st;
	int ipLen;
	int udpDataLen;
	int pseLen;
	const int on = 1;
	char buf[128]={0}, *pse;
	struct ip *ipHeader;
	struct udphdr *udpHeader;
	struct pH psh;
	if((st = socket(AF_INET, SOCK_RAW, IPPROTO_UDP))<0){
		perror("CREATE ERROR");
		exit(1);
	}
	if(setsockopt(st, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on))<0){
		perror("HDRINCL ERROR");
		exit(1);
	}
	setuid(getpid());
	udpDataLen = strlen(udpData);
	ipLen = sizeof(struct ip) + sizeof(struct udphdr) + udpDataLen;
	//IP首部参数修改
	ipHeader = (struct ip*)buf;
	ipHeader->ip_v = IPVERSION;
	ipHeader->ip_hl = sizeof(struct ip)>>2;
	ipHeader->ip_tos = 0;
	ipHeader->ip_len = htons(ipLen);
	ipHeader->ip_id = 0;
	ipHeader->ip_off = 0;
	ipHeader->ip_ttl = MAXTTL;
	ipHeader->ip_p = IPPROTO_UDP;
	ipHeader->ip_sum = 0;
	ipHeader->ip_dst = destHost->sin_addr;
	ipHeader->ip_src = srcHost->sin_addr;
	//UDP首部参数修改
	udpHeader = (struct udphdr*)(buf + sizeof(struct ip));
	udpHeader->source = srcHost->sin_port;
	udpHeader->dest=destHost->sin_port;
	udpHeader->len = htons(ipLen - sizeof(struct ip));
	udpHeader->check = 0;
	if(udpDataLen>0){
		memcpy(buf+sizeof(struct ip)+sizeof(struct udphdr),udpData,udpDataLen); 
	}
	//UDP校验和计算
	psh.scrAddr = srcHost->sin_addr.s_addr;
	psh.destAddr = destHost->sin_addr.s_addr;	
	psh.placeHolder = 0;
	psh.protocol = IPPROTO_UDP;
	psh.udpLen = htons(sizeof(struct udphdr ) + udpDataLen);
	pseLen = sizeof(struct pH) + sizeof(struct udphdr) + udpDataLen;
	pse = malloc(pseLen);
	memcpy(pse, (char*)&psh, sizeof(struct pH));
	memcpy(pse + sizeof(struct pH), udpHeader, sizeof(struct udphdr) + udpDataLen);
	udpHeader->check = CheckSum((unsigned short*)pse, pseLen);
        //循环执行sento()可以控制数据包个数
	sendto(st,buf,ipLen,0,(struct sockaddr*)destHost,sizeof(struct sockaddr_in));
	close(st);
}

int main(int argc,char** argv)
{
	struct sockaddr_in srcHost, destHost;
	struct hostent *host, *host2;
	char *Data="Too young, too simple! Sometimes naive!!";
	bzero(&srcHost, sizeof(struct sockaddr_in));
	bzero(&destHost, sizeof(struct sockaddr_in));
	destHost.sin_family = AF_INET;
	destHost.sin_port = htons(atoi(argv[4]));
	if(inet_aton(argv[3], &destHost.sin_addr)!=1){
		host = gethostbyname(argv[3]);
		if(host==NULL){
			printf("ERROR");
			exit(1);
		}
		destHost.sin_addr = *(struct in_addr *)(host->h_addr_list[0]);
	}
       	srcHost.sin_family = AF_INET;
	srcHost.sin_port = htons(atoi(argv[2]));
	if(inet_aton(argv[1], &srcHost.sin_addr)!=1){
		host2 = gethostbyname(argv[0]);
		if(host==NULL){
			printf("ERROR");
			exit(1);
		}
		srcHost.sin_addr = *(struct in_addr *)(host2->h_addr_list[0]);
	}
	UdpPacketSend(&srcHost, &destHost, Data);
	return 0;
}
