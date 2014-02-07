//gcc -Wall -o rawtcp.o rawtcp.c checksum.c -lnfnetlink  -lnetfilter_queue 
#include "firewall.h"
#include <syslog.h>

struct nfq_handle *h;
struct nfq_q_handle *qh;
int global_UDP_cntr = 0;

// Simple checksum function, may use others such as
// Cyclic Redundancy Check, CRC

unsigned short csum(unsigned short *buf, int len)
{
unsigned long sum;
for(sum=0; len>0; len--)
sum += *buf++;
sum = (sum >> 16) + (sum &0xffff);
sum += (sum >> 16);
return (unsigned short)(~sum);
}

/* returns packet id */
#if PACKET_INFO
static u_int32_t print_data (struct nfq_data *tb)
{
    int id = 0;
    struct nfqnl_msg_packet_hdr *ph;
    struct nfqnl_msg_packet_hw *hwph;
    u_int32_t mark,ifi; 
    int ret;
    char *data;

    ph = nfq_get_msg_packet_hdr(tb);
    if (ph) {
            id = ntohl(ph->packet_id);
            printf("hw_protocol=0x%04x hook=%u id=%u ",
                    ntohs(ph->hw_protocol), ph->hook, id);
    }

    hwph = nfq_get_packet_hw(tb);
    if (hwph) {
            int i, hlen = ntohs(hwph->hw_addrlen);

            printf("hw_src_addr=");
            for (i = 0; i < hlen-1; i++)
                    printf("%02x:", hwph->hw_addr[i]);
            printf("%02x ", hwph->hw_addr[hlen-1]);
    }

    mark = nfq_get_nfmark(tb);
    if (mark)
            printf("mark=%u ", mark);

    ifi = nfq_get_indev(tb);
    if (ifi)
            printf("indev=%u ", ifi);

    ifi = nfq_get_outdev(tb);
    if (ifi)
            printf("outdev=%u ", ifi);
    ifi = nfq_get_physindev(tb);
    if (ifi)
            printf("physindev=%u ", ifi);

    ifi = nfq_get_physoutdev(tb);
    if (ifi)
            printf("physoutdev=%u ", ifi);

    ret = nfq_get_payload(tb, (char**)&data);
    if (ret >= 0)
    {	printf("payload_len=%d ", ret);
		//mem_print("data", &data, ret);	
			
	}

    fputc('\n', stdout);


	//print ip and tcp specific info
	struct iphdr *ip;
	struct tcphdr *tcp;
	ip = malloc(sizeof(struct iphdr));
	tcp = malloc(sizeof(struct tcphdr));

	//struct in_addr ipa;

	id = ntohl((nfq_get_msg_packet_hdr(tb))->packet_id);
	nfq_get_payload(tb, (char **)&ip);

	tcp = (struct tcphdr *)((uint32_t *) ip + ip->ihl);
	mem_print("ip header", ip, 20);
	mem_print("tcp header", tcp, tcp->doff*4);
	
	//free(ip);
	//free(tcp);
    return id;
}
#endif

static int cb(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg, struct nfq_data *nfa, void *data)
{
    u_int32_t id;

		#if PACKET_INFO
		printf("entering callback\n");
		#endif
		
	uint32_t os_ret;
	int ret;
	
	struct ip *ip;
	ip = malloc(sizeof(struct ip));
	struct response_dict* resp_dict = data;
	

	//match incoming packet to an IP struct
	id = ntohl((nfq_get_msg_packet_hdr(nfa))->packet_id);
	nfq_get_payload(nfa, (u_char **)&ip);

	#if SYSLOG
	char raw_bytes[2048];
	memset(raw_bytes,0,2048);
	u_int len_tol = ntohs(ip->ip_len);	//
	hex_encode(raw_bytes, ip,len_tol-1);
	syslog (LOG_INFO, "ID(%d) nfq: PKT BYTES(%s)", HPID, raw_bytes);
	#endif
	
	//printf("inside outfilter\n");
	switch(ip->ip_p)
	{
	case IPPROTO_TCP:

		#if PACKET_INFO
		id = print_data(nfa);
		#endif
		while(0);{}		//nop to avoid declaring a struct as first thing in the case (not allowed)
		struct tcphdr *tcp;
		tcp = malloc(sizeof(struct tcphdr));
		tcp = (struct tcphdr *)((uint32_t *) ip + ip->ip_hl);
	
		os_ret = OS_scan(nfa, resp_dict, (char*)ip);
		break;
		
	case IPPROTO_UDP:
	  	while(0);{}		//nop to avoid declaring a struct as first thing in the case (not allowed)
		struct udphdr *udp;
		udp = malloc(sizeof(struct udphdr));
		udp = (struct udphdr *)((uint32_t *) ip + ip->ip_hl);
		os_ret = OS_scan_alt(nfa, resp_dict, (char*)ip);

		
		
	default:
	//if not a TCP or UDP packet, accept
		os_ret = 0;
		break;

	}



	if(os_ret==1)
		ret = nfq_set_verdict2(qh, id, NF_DROP,1, 0, NULL);
	else
		ret = nfq_set_verdict2(qh, id, NF_ACCEPT,0x1, (uint32_t)ntohs(ip->ip_len), (unsigned char *)ip);


	//free(ip);

	return ret;

}

//////////////////
u_int32_t OS_scan_alt (struct nfq_data *nfa, struct response_dict* resp_dict, char* ip_from_cb)
{
    u_int32_t id;
	struct ip *ip_in;
	struct udphdr *udp_in;
	int scan_flag = 255;
	
	id = ntohl((nfq_get_msg_packet_hdr(nfa))->packet_id);
	nfq_get_payload(nfa, (u_char **)&ip_in);

	//put data received from iptables into ip_in and udp_in
	ip_in = (struct ip*)ip_from_cb;
	udp_in = (struct udphdr *)((uint32_t *) ip_in + ip_in->ip_hl);
	
	switch(ip_in->ip_p)
	{

	case IPPROTO_UDP:
		
		#if PACKET_INFO
		mem_print("udp header", udp_in, htons(udp_in->len));
		printf("UDP dest port %04x\n",htons(udp_in->dest));
		printf("UDP length %02x\n",htons(udp_in->len));
		printf("IP DF flag %04x\n",((ip_in->ip_off) && IP_DF));
		#endif
		
		if(htons(udp_in->source) == 0x7070)	//port 28784
		{
			if((ip_in->ip_id) != 0x0000)	//4
			{
			#if PACKET_INFO
			printf("ipid of %04x received\n", ip_in->ip_id);
			#endif
			  scan_flag = 0;
			}
		}
		break;
	}



	if(scan_flag!=255)
	{

		// No data, just datagram
		char buffer[PCKT_LEN];
		// The size of the headers
		struct iphdr *ip = (struct iphdr *) buffer;
		struct sockaddr_in sin, din;
		memset(buffer, 0, PCKT_LEN);
		//initialize the packet with recorded response 

		switch(scan_flag){

		case 0:
			//copy incoming packet 'ip_in' to new, outgoing packet 'ip'
			memcpy(ip,ip_in,htons(ip_in->ip_len));
			
			(ip->id) = 0x0000;
			#if PACKET_INFO
			printf("ipid of changed to %04x \n", ip->id);
			#endif
			break;

		default:
			return 0; //return 0 means set verdict to ACCEPT
			break;
		}

		//now fill in all the fields with info pertinent to this incoming query
		// Address family
		sin.sin_family = AF_INET;
		din.sin_family = AF_INET;
		
		//maintain ports
		sin.sin_port = udp_in->source;
		din.sin_port = udp_in->dest;

		//maintain ip addresses
		sin.sin_addr = ip_in->ip_src;
		din.sin_addr = ip_in->ip_dst;

		//set ipid
		ip->frag_off = 0x0040;
		
		//zero out the checksum so we can calulate the new checksum 
		ip->check = 0x0000;
		ip->check = csum((u_short*)ip, 10);
		
		
		


		#if PACKET_INFO
		mem_print("ip pkt after ini   ", ip, htons(ip->tot_len));
		printf("Using:::::Source IP: %08x Target IP: %08x.\n", ip_in->ip_dst, ip_in->ip_src);
		mem_print("sending this data  ", buffer, htons(ip->tot_len));
		#endif
		
		//replace 'ip_in' the data we built in our temp memory space called 'ip'
		memcpy(ip_in,ip,htons(ip_in->ip_len));

		#if DEBUG
		printf("verdict = DROP\n");
		printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
		#endif
		return 1;//return 1 means set verdict to DROP because we just sent our own packet
	}
	#if DEBUG
	printf("verdict = ACCEPT\n");
	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

	#endif
	return 0;	//return 0 means set verdict to ACCEPT
	
}




u_int32_t OS_scan (struct nfq_data *nfa, struct response_dict* resp_dict, char *ip_from_cb)
{
  
    //NOTE: Type of 'ip_in' is 'ip', Type of 'ip' is 'ip_hdr', same for tcp...
    
    u_int32_t id;
	struct ip *ip_in;
	struct tcphdr *tcp_in;
	int scan_flag = 255;

	id = ntohl((nfq_get_msg_packet_hdr(nfa))->packet_id);
	
	//put data received from iptables into ip_in
	ip_in = (struct ip*)ip_from_cb;
	

	//nfq_get_payload(nfa, (char **)&ip_in);
	
	tcp_in = (struct tcphdr *)((uint32_t *) ip_in + ip_in->ip_hl);

	//printf("~~~~~~tcp offset %02x\n", (tcp_in->doff));	
#if PACKET_INFO
printf("TCP dest port %04x\n",htons(tcp_in->dest));
printf("TCP offset %02x\n",(tcp_in->doff));
printf("IP DF flag %04x\n",((ip_in->ip_off) && IP_DF));
#endif


if(htons(tcp_in->source) == 0x0050)	//port 80
	{
		if((ip_in->ip_id) != 0x0000)	//4
		{
		 #if PACKET_INFO
		 printf("ipid of %04x received\n", ip_in->ip_id);
		 #endif
		  scan_flag = 0;
		 }

	}


	if(scan_flag!=255)
	{

		// No data, just datagram
		char buffer[PCKT_LEN];
		// The size of the headers
		struct iphdr *ip = (struct iphdr *) buffer;
		struct sockaddr_in sin, din;
		memset(buffer, 0, PCKT_LEN);

		//initialize the packet with recorded response 

		switch(scan_flag){

		case 0:
			//copy incoming packet 'ip_in' to new, outgoing packet 'ip'
			memcpy(ip,ip_in,htons(ip_in->ip_len));
			
			(ip->id) = 0x0000;
			#if PACKET_INFO
			printf("ipid of changed to %04x \n", ip->id);
			#endif
			
			break;
		default:
			return 0; //return 0 means set verdict to ACCEPT
			break;
			
		}

		//now fill in all the fields with info pertinent to this outgoing query
		// Address family
		sin.sin_family = AF_INET;
		din.sin_family = AF_INET;

		//maintain ports
		sin.sin_port = tcp_in->source;
		din.sin_port = tcp_in->dest;

		//maintain ip addresses
		sin.sin_addr = ip_in->ip_src;
		din.sin_addr = ip_in->ip_dst;
		
		ip->check = 0x0000;
		ip->check = csum((u_short*)ip, 10);

		    

		#if PACKET_INFO
		printf("Using:::::Source IP: %08x port: %d, Target IP: %08x port: %d.\n", ip_in->ip_dst, htons(tcp_in->source), ip_in->ip_src,  htons(tcp_in->dest));
		mem_print("sending this data  ", buffer, htons(ip->tot_len));
		#endif
		
		//replace 'ip_in' the data we built in our temp memory space called 'ip'
		memcpy(ip_in,ip,htons(ip_in->ip_len));


		#if DEBUG
		printf("verdict = ACCEPT (w/ Changes)\n");
		printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
		#endif
		return 0;//return 0 since we want to accept the packet with our changes made
	}
	#if DEBUG
	printf("verdict = ACCEPT\n");
	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	#endif
	return 0;	//return 0 means set verdict to ACCEPT
}




int main(int argc, char **argv)
{

    struct nfnl_handle;
    int fd;
    int rv;
    char buf[4096] __attribute__ ((aligned));

	struct response_dict* resp_dict_ptr;	
	resp_dict_ptr= malloc( sizeof(struct response_dict));

	//ini_responses(resp_dict_ptr);

	// Set up signal handler
	signal(SIGINT,graceful_exit);
	signal(SIGHUP,graceful_exit);
	
	
printf("\n+++++Starting outfilter80.o now.");

#if SYSLOG
    openlog ("HP", LOG_CONS, LOG_LOCAL0);
#endif	


#if PRINT_INFO
printf("opening library handle\n");
#endif
    h = nfq_open();
    if (!h) {
            fprintf(stderr, "error during nfq_open()\n");
            exit(1);
    }
#if PRINT_INFO
printf("opening library handle\n");
printf("unbinding existing nf_queue handler for AF_INET (if any)\n");
#endif
    if (nfq_unbind_pf(h, AF_INET) < 0) {
            fprintf(stderr, "error during nfq_unbind_pf()\n");
            exit(1);
    }
#if PRINT_INFO    
printf("opening library handle\n");
printf("binding nfnetlink_queue as nf_queue handler for AF_INET\n");
#endif
    if (nfq_bind_pf(h, AF_INET) < 0) {
            fprintf(stderr, "out_filter.o error during nfq_bind_pf()\n");
	    perror("");
            //exit(1);
    }
#if PRINT_INFO    
    printf("binding this socket to queue '2'\n");
#endif
    qh = nfq_create_queue(h,  2, &cb, resp_dict_ptr);							///set queue number here
    if (!qh) {
            fprintf(stderr, "error during nfq_create_queue()\n");
            exit(1);
    }
#if PRINT_INFO
    printf("setting copy_packet mode\n");
#endif
    if (nfq_set_mode(qh, NFQNL_COPY_PACKET, 0xffff) < 0) {
            fprintf(stderr, "can't set packet_copy mode\n");
            exit(1);
    }

    fd = nfq_fd(h);
printf("  Port 80 output mangling started.\n");
#if SYSLOG
syslog (LOG_INFO, "ID(%d) nfq: Firewall Started.  Listening...", HPID);
#endif

    while ((rv = recv(fd, buf, sizeof(buf), 0)) && rv >= 0) {
#if PACKET_INFO
            printf("pkt received\n");
#endif
            nfq_handle_packet(h, buf, rv);
	    //printf("rv = %d \n", rv);
    }

/*    while (1) {
//#if PACKET_INFO
	    rv = recv(fd, buf, sizeof(buf),0);
            printf("pkt received\n");
//#endif
            nfq_handle_packet(h, buf, rv);
	    printf("rv = %d \n", rv);
    }
*/
printf("done!");

//free(resp_dict_ptr);
    exit(0);
}

void graceful_exit () {
//======================================================================
// Called by a SIGHUP or SIGINT, unbind the queue and exit
//======================================================================

	printf ("Signal caught, destroying queue ...");
	nfq_destroy_queue(qh);
	printf ("Closing handle \n");
	nfq_close(h);
	exit(0);
}

void mem_print(const char* label, void* data, u_int length)
{	
	u_char* data_char;
	data_char = (u_char*) data;
	int i; 

	printf("%s:::", label);
	    for (i=0; i<=(length-1); i++)	//-1 to account for not starting at 1
	    {
		printf("%02x:", data_char[i]);
	    }
		printf("\n"); 

}



uint16_t icmp_checksum(uint16_t *buffer, uint32_t size) 
{
    unsigned long cksum=0;
    while(size >1) 
    {
        cksum+=*buffer++;
        size -=sizeof(u_short);
    }
    if(size ) 
    {
        cksum += *(u_char*)buffer;
    }
    cksum = (cksum >> 16) + (cksum & 0xffff);
    cksum += (cksum >>16);
    return (uint16_t)(~cksum);
}


static char hextab[512] = "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f202122232425262728292a2b2c2d2e2f303132333435363738393a3b3c3d3e3f404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f606162636465666768696a6b6c6d6e6f707172737475767778797a7b7c7d7e7f808182838485868788898a8b8c8d8e8f909192939495969798999a9b9c9d9e9fa0a1a2a3a4a5a6a7a8a9aaabacadaeafb0b1b2b3b4b5b6b7b8b9babbbcbdbebfc0c1c2c3c4c5c6c7c8c9cacbcccdcecfd0d1d2d3d4d5d6d7d8d9dadbdcdddedfe0e1e2e3e4e5e6e7e8e9eaebecedeeeff0f1f2f3f4f5f6f7f8f9fafbfcfdfeff";

void hex_encode(u_char* output, u_char* input, int input_len)
{
	u_int16_t i;
	for(i = 0; i<input_len; i++)
	{	
		output[i*2] = hextab[input[i]*2];
		output[(i*2)+1] = hextab[(input[i]*2)+1];
		//printf("%c%c", hextab[input[i]*2], hextab[(input[i]*2)+1]);
	}
	//printf("\n");
}



















