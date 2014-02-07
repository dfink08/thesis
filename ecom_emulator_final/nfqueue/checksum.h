#include <stdio.h>

/* set ip checksum of a given ip header*/
void compute_ip_checksum(struct iphdr* iphdrp);
/*validate ip checksum, result should be 0 if IP header is correct*/
unsigned short validate_ip_checksum(struct iphdr* iphdrp);
/* set tcp checksum: given IP header and TCP header, and TCP data */
void compute_tcp_checksum(struct iphdr *pIph, unsigned short *ipPayload);
/* set tcp checksum: given IP header and TCP header, and TCP data */
void compute_udp_checksum(struct iphdr *pIph, unsigned short *ipPayload);

