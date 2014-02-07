//from: http://www.roman10.net/how-to-calculate-iptcpudp-checksumpart-3-usage-example-and-validation/
#include <stdio.h>
#include <linux/types.h>
#include <netinet/in.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>

/* Compute checksum for count bytes starting at addr, using one's complement of one's complement sum*/
static unsigned short compute_checksum(unsigned short *addr, unsigned int count) {
  register unsigned long sum = 0;
  while (count > 1) {
    sum += * addr++;
    count -= 2;
  }
  //if any bytes left, pad the bytes and add
  if(count > 0) {
    sum += ((*addr)&htons(0xFF00));
  }
  //Fold sum to 16 bits: add carrier to result
  while (sum>>16) {
  	sum = (sum & 0xffff) + (sum >> 16);
  }
  //one's complement
  sum = ~sum;
  return ((unsigned short)sum);
}

/* set ip checksum of a given ip header*/
void compute_ip_checksum(struct iphdr* iphdrp){
  iphdrp->check = 0;
  iphdrp->check = compute_checksum((unsigned short*)iphdrp, iphdrp->ihl<<2);
}

/*validate ip checksum, result should be 0 if IP header is correct*/
u_int16_t validate_ip_checksum(struct iphdr* iphdrp){
  return compute_checksum((unsigned short*)iphdrp, iphdrp->ihl<<2);
}

/* set tcp checksum: given IP header and tcp segment */
void compute_tcp_checksum(struct iphdr *pIph, unsigned short *ipPayload) {
	register unsigned long sum = 0;
    unsigned short tcpLen = ntohs(pIph->tot_len) - (pIph->ihl<<2);
	struct tcphdr *tcphdrp = (struct tcphdr*)(ipPayload);
	//add the pseudo header 
	//the source ip
	sum += (pIph->saddr>>16)&0xFFFF;
	sum += (pIph->saddr)&0xFFFF;
	//the dest ip
	sum += (pIph->daddr>>16)&0xFFFF;
	sum += (pIph->daddr)&0xFFFF;
	//protocol and reserved: 6
	sum += htons(IPPROTO_TCP);
	//the length
	sum += htons(tcpLen);

	//add the IP payload
	//initialize checksum to 0
	tcphdrp->check = 0;
	while (tcpLen > 1) {
		sum += * ipPayload++;
		tcpLen -= 2;
	}
    //if any bytes left, pad the bytes and add
    if(tcpLen > 0) {
		//printf("+++++++++++padding, %d\n", tcpLen);
        sum += ((*ipPayload)&htons(0xFF00));
    }
  	//Fold 32-bit sum to 16 bits: add carrier to result
  	while (sum>>16) {
  		sum = (sum & 0xffff) + (sum >> 16);
  	}
  	sum = ~sum;
	//set computation result
	tcphdrp->check = (unsigned short)sum;
}

/* set udp checksum: given IP header and UDP datagram */
void compute_udp_checksum(struct iphdr *pIph, unsigned short *ipPayload) {
	register unsigned long sum = 0;
	struct udphdr *udphdrp = (struct udphdr*)(ipPayload);
	unsigned short udpLen = htons(udphdrp->len);
	//printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~udp len=%d\n", udpLen);
	//add the pseudo header 
	//printf("add pseudo header\n");
	//the source ip
	sum += (pIph->saddr>>16)&0xFFFF;
	sum += (pIph->saddr)&0xFFFF;
	//the dest ip
	sum += (pIph->daddr>>16)&0xFFFF;
	sum += (pIph->daddr)&0xFFFF;
	//protocol and reserved: 17
	sum += htons(IPPROTO_UDP);
	//the length
	sum += udphdrp->len;

	//add the IP payload
	//printf("add ip payload\n");
	//initialize checksum to 0
	udphdrp->check = 0;
	while (udpLen > 1) {
		sum += * ipPayload++;
		udpLen -= 2;
	}
    //if any bytes left, pad the bytes and add
    if(udpLen > 0) {
		//printf("+++++++++++++++padding: %d\n", udpLen);
        sum += ((*ipPayload)&htons(0xFF00));
    }
  	//Fold sum to 16 bits: add carrier to result
	//printf("add carrier\n");
  	while (sum>>16) {
  		sum = (sum & 0xffff) + (sum >> 16);
  	}
	//printf("one's complement\n");
  	sum = ~sum;
	//set computation result
	udphdrp->check = ((unsigned short)sum == 0x0000)?0xFFFF:(unsigned short)sum;
}

