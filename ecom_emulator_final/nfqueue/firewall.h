#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <linux/types.h>
#include <linux/netfilter.h>            /* for NF_ACCEPT */
#include <libnetfilter_queue/libnetfilter_queue.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include "checksum.h"



//OPTIONS
#define PCKT_LEN 8192	// Packet length
#define DEBUG 0
#define PACKET_INFO 0
#define PRINT_INFO 0
#define SYSLOG 0
#define HPID 222




struct tcp_pkt_response{
	struct iphdr		ip_hdr;
	struct tcphdr		tcp_hdr;
	char				tcp_options[20];
	u_int16_t			len;
};

struct udp_pkt_response{
	struct iphdr		ip_hdr;
	struct udphdr		udp_hdr;
	char				udp_data[512];
	u_int16_t			len;
};

struct icmp_pkt_response{
	struct iphdr		ip_hdr;
	struct icmp			icmp_hdr;
	char				icmp_data_sec[512];
	u_int16_t			len;
};


struct response_dict{
	struct tcp_pkt_response		os_tcp_resp0;
	struct tcp_pkt_response		os_tcp_resp1;
	struct tcp_pkt_response		os_tcp_resp2;
	struct tcp_pkt_response		os_tcp_resp3;
	struct tcp_pkt_response		os_tcp_resp4;
	struct tcp_pkt_response		os_tcp_resp5;
	struct tcp_pkt_response		os_tcp_resp6;
	struct tcp_pkt_response		os_tcp_resp7;
	struct tcp_pkt_response		os_tcp_resp8;
	struct tcp_pkt_response		os_tcp_resp9;
	struct tcp_pkt_response		os_tcp_resp99;
	struct tcp_pkt_response		os_tcp_resp98;
	struct tcp_pkt_response		os_tcp_resp97;

	struct icmp_pkt_response	os_icmp_resp0;
	struct icmp_pkt_response	os_icmp_resp1;
	struct icmp_pkt_response	os_icmp_resp2;

	//struct udp_pkt_response		os_udp_resp0;

};


void graceful_exit();
uint16_t icmp_checksum(uint16_t *buffer, uint32_t size);
u_int32_t OS_scan (struct nfq_data *nfa, struct response_dict* resp_dict, char *ip_from_cb);
u_int32_t OS_scan_alt (struct nfq_data *nfa, struct response_dict* resp_dict, char *ip_from_cb);
void mem_print(const char* label, void* data, u_int length);
void hex_encode(u_char* output, u_char* input, int input_len);
