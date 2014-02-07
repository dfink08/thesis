#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h> 
#include <net/ethernet.h>
#include <netinet/ether.h> 
#include <netinet/ip.h> 


/* tcpdump header (ether.h) defines ETHER_HDRLEN) */
#ifndef ETHER_HDRLEN 
#define ETHER_HDRLEN 14
#endif


#define P1		printf("Print Marker 1\n");
#define P2		printf("Print Marker 2\n");
#define P3		printf("Print Marker 3\n");
#define P4		printf("Print Marker 4\n");
#define P5		printf("Print Marker 5\n");


struct my_ip {
	u_int8_t	ip_vhl;		/* header length, version */
#define IP_V(ip)	(((ip)->ip_vhl & 0xf0) >> 4)
#define IP_HL(ip)	((ip)->ip_vhl & 0x0f)
	u_int8_t	ip_tos;		/* type of service */
	u_int16_t	ip_len;		/* total length */
	u_int16_t	ip_id;		/* identification */
	u_int16_t	ip_off;		/* fragment offset field */
#define	IP_DF 0x4000			/* dont fragment flag */
#define	IP_MF 0x2000			/* more fragments flag */
#define	IP_OFFMASK 0x1fff		/* mask for fragmenting bits */
	u_int8_t	ip_ttl;		/* time to live */
	u_int8_t	ip_p;		/* protocol */
	u_int16_t	ip_sum;		/* checksum */
	struct	in_addr ip_src,ip_dst;	/* source and dest address */
};

struct my_udp {
	u_int16_t	udp_srcprt;		/* source port */
	u_int16_t	udp_dstprt;		/* destination port */
	u_int16_t	udp_len;		/* udp length */
	u_int16_t	udp_crc;		/* udp checksum */
};

struct my_hap {
	u_int8_t	hap_H;		/* H */
	u_int8_t	hap_A;		/* A */
	u_int8_t	hap_P;		/* P */
	u_int8_t	hap_AppVal1;	/* hap AppVal */
	u_int8_t	hap_AppVal2;	/* hap AppVal */
	u_int8_t	hap_crc1;	/* hap crc */
	u_int8_t	hap_crc2;	/* hap crc */
	u_int8_t	hap_length1;	/* hap length */
	u_int8_t	hap_length2;	/* hap length */
	u_int8_t	hap_Fun_Code;	/* hap function code*/
	u_char		hap_data_buff[600];	/* hap data*/
};

struct hap_simple {
	u_int16_t	hap_AppVal;	/* hap AppVal */
	u_int16_t	hap_crc;	/* hap crc */
	u_int16_t	hap_length;	/* hap length */
	u_int8_t	hap_Fun_Code;	/* hap function code*/
	u_char		hap_data_buff[600];	/* hap data*/
};


struct ecomDevice {
	libnet_t*	ecom_l_ptr;
	HEIDevice* 	ecom_pDevice;
};







u_int16_t handle_ethernet (u_char *args,const struct pcap_pkthdr* pkthdr,const u_char* packet);
u_int16_t handle_IP (u_char *args,const struct pcap_pkthdr* pkthdr,const u_char* packet);
u_int16_t handle_UDP (u_char *args,const struct pcap_pkthdr* pkthdr,const u_char* packet);
int decide_response(struct hap_simple* hap_simple, const u_char* packet, struct ecomDevice *eDevice);
void swap_ip_struct(IPSetup* current);
int respond_addressed_broadcast(struct hap_simple* hap_simple, const u_char* packet, struct ecomDevice *eDevice);
int respond_write_setup_data(struct hap_simple* hap_simple, const u_char* packet, struct ecomDevice *eDevice);
int respond_query_setup_data(struct hap_simple* hap_simple, const u_char* packet, struct ecomDevice *eDevice);
int respond_read_device_info(struct hap_simple* hap_simple, const u_char* packet, struct ecomDevice *eDevice);
int respond_ccm_request(struct hap_simple* hap_simple, const u_char* packet, struct ecomDevice *eDevice);
int respond_kseq_request(struct hap_simple* hap_simple, const u_char* packet, struct ecomDevice *eDevice);
int respond_read_estats(struct hap_simple* hap_simple, const u_char* packet, struct ecomDevice *eDevice);

struct my_hap* handle_HAP (u_char *args,const struct pcap_pkthdr* pkthdr,const u_char* packet);


