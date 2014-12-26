/*
 * trpclientfast.c
 *
 *  Created on: 27-Jul-2013
 *      Author: Swapniel
 *  This file implements the Fast TRP protocol.
 *  Arguments: <src ip> <interface> <dst ip> <no. of packets>
 */

#include <arpa/inet.h>
#include <asm/byteorder.h>
#include <asm-generic/int-ll64.h>
#include <libio.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netpacket/packet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct _trphdr
{
	__u16 packno;
	__u16 prcssid;

//#if defined(__LITTLE_ENDIAN_BITFIELD)		//Ideally this should work but server is coded wrongly.
//	__u16 flag:2,burstseq:7,maxburst:7;
//#elif defined (__BIG_ENDIAN_BITFIELD)
	__u16 maxburst:7,burstseq:7,flag:2;
//#else
//#error  "Please fix <asm/byteorder.h>"
//#endif

	__u16 chksum;
}trphead;

typedef struct iphdr iphead;

inline int min(int a, int b)
{
	return a<b?a:b;
}

//inline int myceil(float a)
//{
//	return (int)(a+0.5f);
//}

unsigned short comp_csum(char *ptr, int len)
{
	unsigned short *addr = (unsigned short *)ptr;
	long sum = 0;
	while (len > 1)
	{
		sum += *(addr++);
		len -= 2;
	}
	if (len > 0)
		sum += *addr;
	while (sum >> 16)
		sum = ((sum & 0xffff) + (sum >> 16));
	sum = ~sum;
	return ((u_short) sum);
}

void BindToInterface(int raw , char *device , int protocol)
{
	struct sockaddr_ll sll;
	struct ifreq ifr;
	memset(&sll, 0, sizeof(sll));
	memset(&ifr, 0, sizeof(ifr));
	strncpy((char *)ifr.ifr_name ,device , IFNAMSIZ);
	if((ioctl(raw , SIOCGIFINDEX , &ifr)) == -1)
	{
		perror("Unable to find interface index");
		exit(-1);
	}
	if (setsockopt (raw, SOL_SOCKET, SO_BINDTODEVICE, &ifr, sizeof (ifr)) < 0)
	{
		perror ("setsockopt() failed to bind to interface ");
		exit (EXIT_FAILURE);
	}
}

//void print_packet(char *buff)
//{
//	iphead *ip=(iphead *)buff;
//	printf("SRC IP = %s\n",inet_ntoa(*(struct in_addr *)&ip->saddr));
//	printf("Data = %s\n",buff + sizeof(iphead) + sizeof(trphead));
//}

int main(int argc, char *argv[])
{
	iphead *ip;
	trphead *trp, *trprcv;
	char messg[101]={0};
	int sock, on = 1, bseq, count, acked=0, rcvd, max, tobesent, success, temp;
	char sbuffer[1024],rbuffer[1024];
	struct sockaddr_in daddr, saddr;
	struct timeval tv;
	if (argc != 5)
	{
		fprintf(stderr, "Usage: %s <src-addr> <local interface name> <dest-addr> <no. of packets>\n", argv[0]);
		return 1;
	}

	count = atoi(argv[4]);
//	mode = atoi(argv[5]);
//	mode = 1;

	inet_pton(AF_INET, argv[3], (struct in_addr *)&daddr.sin_addr.s_addr);
	inet_pton(AF_INET, argv[1], (struct in_addr *)&saddr.sin_addr.s_addr);

	sock = socket(PF_INET, SOCK_RAW, 254);
	if (sock == -1)
	{
		perror("socket() failed");
		return 1;
	}
	else
		printf("socket() ok\n");

	BindToInterface(sock,argv[2],IPPROTO_RAW);

	tv.tv_sec=1;
	tv.tv_usec=0;

	if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) == -1 || setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) == -1)
	{
		perror("setsockopt() failed");
		return 2;
	}
	else
		printf("setsockopt() ok\n");

	memset(messg,'k',100);
	messg[100]='\0';
	memset(sbuffer,0,1024);

	int iphdrlen = sizeof(iphead);		//20
	int trphdrlen = sizeof(trphead);	//8
	int datalen=strlen(messg);		//100

	//..............Initialise sbuffer.....................//
	ip = (iphead*) sbuffer;
	ip->version = 4;
	ip->ihl = 5;
	ip->tos = 0x0;
	ip->tot_len = htons(iphdrlen+trphdrlen+datalen);		// 20 + 8 + 100 = 128
	ip->id = 0;							//Not necessary to set. Network card does it.
	ip->frag_off = 0;
	ip->ttl = 255;
	ip->protocol = 254;
	ip->saddr = saddr.sin_addr.s_addr;
	ip->daddr = daddr.sin_addr.s_addr;
	ip->check = 0;
//	ip->check = comp_csum(sbuffer,iphdrlen);			//Useless. Network card does it.

	trp = (trphead*) (sbuffer+iphdrlen);
	trp->packno = htons(0);						////// to be updated, htons
	trp->prcssid = htons((int)(getpid()));				////// htons
	trp->maxburst = 0;						////// to be updated
	trp->burstseq = 0;						////// to be updated
	trp->flag = 0;
	trp->chksum = 0;						////// to be updated,  htons

	strcpy(sbuffer+iphdrlen+trphdrlen,messg);

	trprcv = (trphead*) (rbuffer+iphdrlen);				//Receiving buffer

	daddr.sin_family = AF_INET;

	max=temp=4;
	while(acked<count)
	{
//		max = min(mode?min(32,myceil(max+success*0.5f)):4,count-acked);
		printf("Next burst = %d\n",max);
		tobesent=acked+1;
		trp->maxburst=temp;
		for(bseq=1;bseq<=max;bseq++)
		{
			trp->packno = htons(tobesent);
			trp->burstseq = bseq;
			trp->chksum = 0;
			trp->chksum = htons(comp_csum(sbuffer+iphdrlen,trphdrlen+datalen));

			if(sendto(sock,sbuffer,iphdrlen+trphdrlen+datalen,0,(struct sockaddr*) &daddr,sizeof(struct sockaddr)) == -1)
			{
				perror("sendto() failed");
				return 1;
			}

			printf("Sent packet %d\n",tobesent);
			tobesent++;
		}

		printf("Sent %d packets\n",max);

		success=0;
		while(1)
		{
			memset(rbuffer,0,1024);
			if(recv(sock, rbuffer, sizeof(rbuffer), 0) == -1)
				break;
			rcvd=ntohs(trprcv->packno);
			if(rcvd>acked&&trprcv->flag==1)
			{
				printf("Ack for %d received\n",rcvd);
				success+=rcvd-acked;
				acked=rcvd;
			}
			if(rcvd==count)
				break;
//			print_packet(rbuffer);
		}
		
		printf("Successfully sent %d packets in last burst\n",success);
//		printf("max = %d, temp = %d\n",max,temp);
		temp = (temp+success*0.5)+0.5;
		temp = temp>32?32:temp;
		max = min(temp,count-acked);
//		printf("max = %d, temp = %d\n",max,temp);
//		printf("new max = %d\n",max);
	}

	return 0;
}

