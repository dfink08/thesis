//gcc -Wall -o rawtcp.o rawtcp.c checksum.c -lnfnetlink  -lnetfilter_queue 
#include "firewall.h"
#include <syslog.h>

struct nfq_handle *h;
struct nfq_q_handle *qh;
int global_UDP_cntr = 0;

unsigned short ini_responses(struct response_dict* resp_dict)
{

struct tcp_pkt_response tcp_resp[13];

//Test Packet 1
memcpy(&tcp_resp[0].ip_hdr ,"\x45\x00\x00\x2c\x00\x00\x40\x00\xff\x06\x66\xa2\x0a\x01\x00\x7b\x0a\x01\x00\xad",20);
memcpy(&(tcp_resp[0].tcp_hdr),"\x00\x50\xb6\x02\x6d\xc9\xec\x43\x21\x2f\x4d\x2e\x60\x12\x08\x00\x41\xa7\x00\x00",20);
memcpy(&(tcp_resp[0].tcp_options), "\x02\x04\x02\x00\x00\x00",6);
tcp_resp[0].len= htons(tcp_resp[0].ip_hdr.tot_len);

//Test Packet 2
memcpy(&tcp_resp[1].ip_hdr ,"\x45\x00\x00\x2c\x00\x00\x40\x00\xff\x06\x66\xa2\x0a\x01\x00\x7b\x0a\x01\x00\xad",20);
memcpy(&(tcp_resp[1].tcp_hdr),"\x00\x50\xb6\x03\x6d\xf3\x34\xb9\x21\x2f\x4d\x2f\x60\x12\x08\x00\xce\x7e\x00\x00",20);
memcpy(&(tcp_resp[1].tcp_options), "\x02\x04\x02\x00\x00\x00",6);
tcp_resp[1].len=htons(tcp_resp[1].ip_hdr.tot_len);

//Test Packet 3
memcpy(&tcp_resp[2].ip_hdr ,"\x45\x00\x00\x2c\x00\x00\x40\x00\xff\x06\x66\xa2\x0a\x01\x00\x7b\x0a\x01\x00\xad",20);
memcpy(&(tcp_resp[2].tcp_hdr),"\x00\x50\xb6\x04\x6e\x1c\x7d\x32\x21\x2f\x4d\x30\x60\x12\x08\x00\x5b\x54\x00\x00",20);
memcpy(&(tcp_resp[2].tcp_options), "\x02\x04\x02\x00\x00\x00",6);
tcp_resp[2].len=htons(tcp_resp[2].ip_hdr.tot_len);

//Test Packet 4
memcpy(&tcp_resp[3].ip_hdr ,"\x45\x00\x00\x2c\x00\x00\x40\x00\xff\x06\x66\xa2\x0a\x01\x00\x7b\x0a\x01\x00\xad",20);
memcpy(&(tcp_resp[3].tcp_hdr),"\x00\x50\xb6\x04\x6e\x45\xc5\xad\x21\x2f\x4d\x30\x60\x12\x08\x00\x5b\x54\x00\x00\x02\x04\x02\x00\x00\x00",20);
memcpy(&(tcp_resp[3].tcp_options), "\x02\x04\x02\x00\x00\x00",6);
tcp_resp[3].len=htons(tcp_resp[3].ip_hdr.tot_len);

//Test Packet 5
memcpy(&tcp_resp[4].ip_hdr ,"\x45\x00\x00\x2c\x00\x00\x40\x00\xff\x06\x66\xa2\x0a\x01\x00\x7b\x0a\x01\x00\xad",20);
memcpy(&(tcp_resp[4].tcp_hdr),"\x00\x50\xb6\x06\x6e\x6f\x0e\x2a\x21\x2f\x4d\x32\x60\x12\x08\x00\x74\xf7\x00\x00",20);
memcpy(&(tcp_resp[4].tcp_options), "\x02\x04\x02\x00\x00\x00",6);
tcp_resp[4].len=htons(tcp_resp[4].ip_hdr.tot_len);

//Test Packet 6
memcpy(&tcp_resp[5].ip_hdr ,"\x45\x00\x00\x2c\x00\x00\x40\x00\xff\x06\x66\xa2\x0a\x01\x00\x7b\x0a\x01\x00\xad",20);
memcpy(&(tcp_resp[5].tcp_hdr),"\x00\x50\xb6\x07\x6e\x98\x56\xaa\x21\x2f\x4d\x33\x60\x12\x08\x00\x02\xbd\x00\x00",20);
memcpy(&(tcp_resp[5].tcp_options), "\x02\x04\x01\x09\x00\x00",6);
tcp_resp[5].len=htons(tcp_resp[5].ip_hdr.tot_len);

//Test Packet ECN --scan flag 6
memcpy(&tcp_resp[6].ip_hdr ,"\x45\x00\x00\x2c\x00\x00\x40\x00\xff\x06\x66\xa2\x0a\x01\x00\x7b\x0a\x01\x00\xad",20);
memcpy(&(tcp_resp[6].tcp_hdr),"\x00\x50\xb6\x0e\x00\x0d\xcb\x46\x21\x2f\x4d\x2e\x60\x12\x08\x00\x8e\x91\x00\x00",20);
memcpy(&(tcp_resp[6].tcp_options), "\x02\x04\x02\x00\x00\x00",6);
tcp_resp[6].len=htons(tcp_resp[6].ip_hdr.tot_len);


////Test Packet T2 --scan flag 7 (NULL TCP)  //these memcpy values are not used because a null tcp packet is just dropped in this PLC implementation
memcpy(&tcp_resp[7].ip_hdr ,"\x45\x00\x00\x2c\x00\x00\x40\x00\xff\x06\x66\xa2\x0a\x01\x00\x7b\x0a\x01\x00\xad",20);
memcpy(&(tcp_resp[7].tcp_hdr),"\x00\x50\xb6\x11\x00\x0e\x3e\x79\x21\x2f\x4d\x2e\x60\x12\x08\x00\x1c\x52\x00\x00",20);
memcpy(&(tcp_resp[7].tcp_options), "\x02\x04\x01\x09\x00\x00",6);
tcp_resp[7].len=htons(tcp_resp[7].ip_hdr.tot_len);

////Test Packet T3 (SYN, FIN, URG, PSH)
memcpy(&tcp_resp[8].ip_hdr ,"\x45\x00\x00\x2c\x00\x00\x40\x00\xff\x06\x66\xa2\x0a\x01\x00\x7b\x0a\x01\x00\xad",20);
memcpy(&(tcp_resp[8].tcp_hdr),"\x00\x50\xb6\x11\x00\x0e\x3e\x79\x21\x2f\x4d\x2e\x60\x12\x08\x00\x1c\x52\x00\x00",20);
memcpy(&(tcp_resp[8].tcp_options), "\x02\x04\x01\x09\x00\x00",6);
tcp_resp[8].len=htons(tcp_resp[8].ip_hdr.tot_len);

////Test Packet T3 (ACK)
memcpy(&tcp_resp[9].ip_hdr ,"\x45\x00\x00\x28\x00\x00\x40\x00\xff\x06\x66\xa6\x0a\x01\x00\x7b\x0a\x01\x00\xad",20);
memcpy(&(tcp_resp[9].tcp_hdr),"\x00\x50\xb6\x12\xd3\xeb\xc0\xd1\x21\x2f\x4d\x2d\x50\x14\x08\x00\xd9\x2a\x00\x00",20);
memcpy(&(tcp_resp[9].tcp_options), "\x00\x00\x00\x00\x00\x00",6);
tcp_resp[9].len=htons(tcp_resp[9].ip_hdr.tot_len);

//--------closed port tests (nmap chooses port 1)

////Test Packet T5 --scan flag 99
memcpy(&tcp_resp[10].ip_hdr ,"\x45\x00\x00\x28\x00\x00\x40\x00\xff\x06\x66\xa6\x0a\x01\x00\x7b\x0a\x01\x00\xad",20);
memcpy(&(tcp_resp[10].tcp_hdr),"\x00\x01\xb6\x13\xd3\xeb\xc0\xd0\x21\x2f\x4d\x2e\x50\x14\x08\x00\xd9\x78\x00\x00",20);
memcpy(&(tcp_resp[10].tcp_options), "\x00\x00\x00\x00\x00\x00",6);
tcp_resp[10].len=htons(tcp_resp[10].ip_hdr.tot_len);

////Test Packet T6 --scan flag 98
memcpy(&tcp_resp[11].ip_hdr ,"\x45\x00\x00\x28\x00\x00\x40\x00\xff\x06\x66\xa6\x0a\x01\x00\x7b\x0a\x01\x00\xad",20);
memcpy(&(tcp_resp[11].tcp_hdr),"\x00\x01\xb6\x14\xd3\xeb\xc0\xd0\x21\x2f\x4d\x2d\x50\x14\x08\x00\xd9\x78\x00\x00",20);
memcpy(&(tcp_resp[11].tcp_options), "\x00\x00\x00\x00\x00\x00",6);
tcp_resp[11].len=htons(tcp_resp[11].ip_hdr.tot_len);

////Test Packet T7 --scan flag 97
memcpy(&tcp_resp[12].ip_hdr ,"\x45\x00\x00\x28\x00\x00\x40\x00\xff\x06\x66\xa6\x0a\x01\x00\x7b\x0a\x01\x00\xad",20);
memcpy(&(tcp_resp[12].tcp_hdr),"\x00\x01\xb6\x15\xd3\xeb\xc0\xd0\x21\x2f\x4d\x2e\x50\x14\x08\x00\xd9\x76\x00\x00",20);
memcpy(&(tcp_resp[12].tcp_options), "\x00\x00\x00\x00\x00\x00",6);
tcp_resp[12].len=htons(tcp_resp[12].ip_hdr.tot_len);


/////////////////////// ICMP Packets
struct icmp_pkt_response icmp_resp[3];
//ICMP Packet 1
memset(&icmp_resp[0].ip_hdr,0,512);
memcpy(&icmp_resp[0].ip_hdr ,"\x45\x00\x00\x94\xf1\x31\x40\x00\xff\x01\x75\x0d\x0a\x01\x00\x7b\x0a\x01\x00\xad",20);
memcpy(&(icmp_resp[0].icmp_hdr),"\x00\x09\x1c\xac\xe2\x23\x01\x27",8);
memcpy(&(icmp_resp[0].icmp_data_sec), "\x00",1);
icmp_resp[0].len= htons(icmp_resp[0].ip_hdr.tot_len);

//ICMP Packet 2
memset(&icmp_resp[1].ip_hdr,0,512);
memcpy(&icmp_resp[1].ip_hdr ,"\x45\x04\x00\xb2\x5e\x42\x00\x00\xff\x01\x47\xdb\x0a\x01\x00\x7b\x0a\x01\x00\xad",20);
memcpy(&(icmp_resp[1].icmp_hdr),"\x00\x00\x1c\xb3\xe2\x24\x01\x28",8);
memcpy(&(icmp_resp[1].icmp_data_sec), "\x00",1);
icmp_resp[1].len=htons(icmp_resp[1].ip_hdr.tot_len);

//UDP Packet 1 (responds with an ICMP packet)
memset(&icmp_resp[2].ip_hdr,1,512);
memcpy(&icmp_resp[2].ip_hdr ,"\x45\x00\x00\x38\x00\x00\x40\x00\xff\x01\x66\x9b\x0a\x01\x00\x7b\x0a\x01\x00\xad",20);
memcpy(&(icmp_resp[2].icmp_hdr),"\x03\x03\x68\x79\x00\x51\x13\xe1",8);
memcpy(&(icmp_resp[2].icmp_data_sec), "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x03\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x03\x11",28);
icmp_resp[2].len= htons(icmp_resp[2].ip_hdr.tot_len);

memcpy(&(resp_dict->os_tcp_resp0), &tcp_resp[0], sizeof(struct tcp_pkt_response));
memcpy(&(resp_dict->os_tcp_resp1), &tcp_resp[1], sizeof(struct tcp_pkt_response));
memcpy(&(resp_dict->os_tcp_resp2), &tcp_resp[2], sizeof(struct tcp_pkt_response));
memcpy(&(resp_dict->os_tcp_resp3), &tcp_resp[3], sizeof(struct tcp_pkt_response));
memcpy(&(resp_dict->os_tcp_resp4), &tcp_resp[4], sizeof(struct tcp_pkt_response));
memcpy(&(resp_dict->os_tcp_resp5), &tcp_resp[5], sizeof(struct tcp_pkt_response));
memcpy(&(resp_dict->os_tcp_resp6), &tcp_resp[6], sizeof(struct tcp_pkt_response));
memcpy(&(resp_dict->os_tcp_resp7), &tcp_resp[7], sizeof(struct tcp_pkt_response));
memcpy(&(resp_dict->os_tcp_resp8), &tcp_resp[8], sizeof(struct tcp_pkt_response));
memcpy(&(resp_dict->os_tcp_resp9), &tcp_resp[9], sizeof(struct tcp_pkt_response));
memcpy(&(resp_dict->os_tcp_resp99), &tcp_resp[10], sizeof(struct tcp_pkt_response));
memcpy(&(resp_dict->os_tcp_resp98), &tcp_resp[11], sizeof(struct tcp_pkt_response));
memcpy(&(resp_dict->os_tcp_resp97), &tcp_resp[12], sizeof(struct tcp_pkt_response));

memcpy(&(resp_dict->os_icmp_resp0), &icmp_resp[0], sizeof(struct icmp_pkt_response));
memcpy(&(resp_dict->os_icmp_resp1), &icmp_resp[1], sizeof(struct icmp_pkt_response));
memcpy(&(resp_dict->os_icmp_resp2), &icmp_resp[2], sizeof(struct icmp_pkt_response));
return 1;



/*
test 1	r r	r  r	r	 r 	  r	 r	c	 c		  c		450400b25e420000ff0147db0a01007b0a0100ad	 c    c    c	    c        r r r  r    c    r    r
query: 	4 5 00 003c 4f9d 0000 36 06 1ff6 0a0100ad 0a01007b - b602 0050 212f4d2d d3ebc0d0 a 0 02 0001 7070 0000 03030a01020405b4080affffffff000000000402
resp:	4 5 00 002c 0000 4000 ff 06 66a2 0a01007b 0a0100ad - 0050 b602 000b183f 212f4d2e 6 0 12 0800 41a7 0000 020402000000

test 2	
query: 	4 5 00 003c 5bf3 0000 2c 06 1da0 0a0100ad 0a01007b - b603 0050 212f4d2e d3ebc0d0 a 0 02 003f 7a6d 0000 020405780303000402080affffffff0000000000
resp:	4 5 00 002c 0000 4000 ff 06 66a2 0a01007b 0a0100ad - 0050 b603 000b8b65 212f4d2f 6 0 12 0800 ce7e 0000 020402000000
 
test 3
query: 	4 5 00 003c a4af 0000 2a 06 d6e3 0a0100ad 0a01007b - b604 0050 212f4d2f d3ebc0d0 a 0 02 0004 7b9e 0000 080affffffff0000000001010303050102040280
resp:	4 5 00 002c 0000 4000 ff 06 66a2 0a01007b 0a0100ad - 0050 b604 000bfe8d 212f4d30 6 0 12 0800 5b54 0000 020402000000
 
test 4
query: 	4 5 00 0038 7368 0000 36 06 fc2e 0a0100ad 0a01007b - b605 0050 212f4d30 d3ebc0d0 9 0 02 0004 8824 0000 0402080affffffff0000000003030a00
resp:	4 5 00 002c 0000 4000 ff 06 66a2 0a01007b 0a0100ad - 0050 b605 000c71b8 212f4d31 6 0 12 0800 e826 0000 020402000000

test 5
query: 	4 5 00 003c 1df6 0000 32 06 559d 0a0100ad 0a01007b - b606 0050 212f4d31 d3ebc0d0 a 0 02 0010 73f6 0000 020402180402080affffffff0000000003030a00
resp:	4 5 00 002c 0000 4000 ff 06 66a2 0a01007b 0a0100ad - 0050 b606 000ce4e5 212f4d32 6 0 12 0800 74f7 0000 020402000000

test 6
query: 	4 5 00 0038 ec24 0000 38 06 8172 0a0100ad 0a01007b - b607 0050 212f4d32 d3ebc0d0 9 0 02 0200 901a 0000 020401090402080affffffff00000000
resp:	4 5 00 002c 0000 4000 ff 06 66a2 0a01007b 0a0100ad - 0050 b607 000d5814 212f4d33 6 0 12 0800 02bd 0000 020401090000
//////////////////
test 7-ECN
query: 	4500003471c700003b06f8d30a0100ad0a01007b b60e0050212f4d2d0000000088c200032b7af7f5 03030a01020405b404020101
resp:	4500002c00004000ff0666a20a01007b0a0100ad 0050b60e000dcb46212f4d2e601208008e910000 020402000000

test T2-TCP null
query: 	4500003cde4a400029065e480a0100ad0a01007b b6100050212f4d2dd3ebc0d0a0000080749000000 3030a0102040109080affffffff000000000402
resp:	none

test T3-(fin, syn, psh, urg)
query: 	4500003c411d000039062b760a0100ad0a01007b b6110050212f4d2dd3ebc0d0a02b010073e40000 03030a0102040109080affffffff000000000402 (fin, syn, psh, urg)
resp:	4500002c00004000ff0666a20a01007b0a0100ad 0050b611000e3e79212f4d2e601208001c520000 020401090000 (syn, ack)

test T4-(ack)
query:	4500003c98bf4000360696d30a0100ad0a01007b b6120050212f4d2dd3ebc0d0a010040070fe0000 03030a0102040109080affffffff000000000402 
resp:	4500002800004000ff0666a60a01007b0a0100ad 0050b612d3ebc0d1212f4d2d50140800d92a0000 000000000000 (reset, ack)

// 97
// 4500002800004000ff0666a60a01007b0a0100ad 0001b613d3ebc0d0212f4d2e50140800d9780000 00 00 00 00 00 00

// 98
// 4500002800004000ff0666a60a01007b0a0100ad 0001b614d3ebc0d0212f4d2d50140800d9780000 000000000000

// 99
// 4500002800004000ff0666a60a01007b0a0100ad 0001b615d3ebc0d0212f4d2e50140800d9760000 000000000000

// ICMP 1
// 45000094f13140003a013a0e0a0100ad0a01007b 080914ace2230127 000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
// 45000094f1314000ff01750d0a01007b0a0100ad 00091cace2230127 000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000 
// ICMP 2
// 450400b25e4200002e0118dc0a0100ad0a01007b 080014b3e2240128 000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
// 450400b25e420000ff0147db0a01007b0a0100ad 00001cb3e2240128 000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
// UDP 1
// 45000148104200004011543a0a0100ad0a01007b cd6a89e7013427a14343434 (300 43s)
// 4500003800004000ff01669b0a01007b0a0100ad 030368a3005113e1 45000148104200004011543a0a0100ad0a01007bcd6a89e7013427a1
// 4500003800004000ff01669b0a01007b0a0100ad 030324fa33f423e7 45000148104200003b11593a0a0100ad0a01007bcd6a89e7013427a1
// 4500003800004000ff01669b0a01007b0a0100ad 030315e3e62680cb 450001481042000037115d3a0a0100ad0a01007bcd6a89e7013427a1

	BYTE RetBuffer[600];
	memset(Buffer, 0, 600);
	int ResponseSize = sizeof(RetBuffer);
	BYTE FromAddress[20];
	int Error;
	WORD Count = 0;
	DWORD StartTime;
	int Retval;

	char str[1024];
	memset(str, 0, 1024);

	char keyVal_hex[64];
	memset(keyVal_hex,0,64);
	char keyVal_str[128];
	memset(keyVal_str,0,128);

	DataOffset = ((HEIDevice*)*pDevice)->DataOffset;
	Total = DataOffset;

	//ack
	Buffer[DataOffset] = FUN_ACK; 		// This is the function code! 
	Total = DataOffset+1;			
	memcpy(&Buffer[DataOffset+1], "\x1a\x00\x00\x00\x00", 5);
	Total+=5;
	Error = Send_HAP_Packet(*pDevice, Buffer, Total, RetBuffer, &ResponseSize, TRUE, FALSE, *l);	

	//reset offset
	DataOffset = ((HEIDevice*)*pDevice)->DataOffset;
	Total = DataOffset;
	memset(Buffer, 0, 600);

	memcpy(&keyVal_hex,&(((HEIDevice*)*pDevice)->kseqQuery.padding2), 1 +((HEIDevice*)*pDevice)->kseqQuery.length);	//kseq size is variable
//	printf("kseq.length: %d\n", 1 +((HEIDevice*)*pDevice)->kseqQuery.length);		
//	printf("strlen(keyVal_hex): %d\n", strlen(keyVal_hex));	
//	mem_print("Kseq query: ", keyVal_hex, 1 +((HEIDevice*)*pDevice)->kseqQuery.length);	
	
	hex_encode(keyVal_str, keyVal_hex, 1 +((HEIDevice*)*pDevice)->kseqQuery.length);	//kseq size is variable
	printf("Kseq Query: %s\n", keyVal_str);	

	//get the response from the file
	ini_gets("KSEQ_READ",keyVal_str ,"default_val", str, 512, "kseqdictionary.txt");
	printf("Kseq Response: %s\n", str);
//	printf("strlen(ascii response): %d\n", strlen(str));	



	//response
	Buffer[DataOffset] = FUN_RESPONSE; 	// This is the function code! 
	Total = DataOffset+1;			
	
	Buffer[DataOffset+1] = FUN_KSEQ_REQUEST; 	// response to kseq 
	Total++;
	Buffer[DataOffset+2] = FALSE;
	Total++;

	//memcpy(&Buffer[DataOffset+1], &str, strlen(str));
	stoh(str, &Buffer[DataOffset+3], strlen(str)/2);
	//mem_print("str of response in hex: ", &Buffer[DataOffset], strlen(str)/2);
	Total+=257;
*/

}



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

	
	
	//uncomment this to test speed of pulling packet into userspace and back into kernel.  this accepts all pacekts with no analysis.
	  //nfq_set_verdict(qh, id, NF_ACCEPT, (uint32_t)ntohs(ip->ip_len), (unsigned char *)ip);
	
	
	#if SYSLOG
	char raw_bytes[2048];
	memset(raw_bytes,0,2048);
	u_int len_tol = ntohs(ip->ip_len);	//
	hex_encode(raw_bytes, ip,len_tol-1);
	syslog (LOG_INFO, "ID(%d) nfq: PKT BYTES(%s)", HPID, raw_bytes);
	#endif
	switch(ip->ip_p)
	{

	case IPPROTO_UDP:
	case IPPROTO_ICMP:
		os_ret = OS_scan_alt(nfa, resp_dict, (char*)ip);
		break;

	case IPPROTO_TCP:

		#if PACKET_INFO
		id = print_data(nfa);
		#endif
		while(0);{}		//nop
		struct tcphdr *tcp;
		tcp = malloc(sizeof(struct tcphdr));
		tcp = (struct tcphdr *)((uint32_t *) ip + ip->ip_hl);
	
		os_ret = OS_scan(nfa, resp_dict, (char*)ip);
		
		
		break;

	}
	
//os_ret = 0;

	if(os_ret==1)
		ret = nfq_set_verdict2(qh, id, NF_DROP,1, 0, NULL);
	else
		ret = nfq_set_verdict2(qh, id, NF_ACCEPT,1, (uint32_t)ntohs(ip->ip_len), (unsigned char *)ip);


	//free(ip);

	return ret;

}
//start//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		      
u_int32_t OS_scan_alt (struct nfq_data *nfa, struct response_dict* resp_dict, char *ip_from_cb)
{
    u_int32_t id;
	struct ip *ip_in;
	struct udphdr *udp_in;
	struct icmphdr	  *icmp_in;
	int scan_flag = 255;
	
	const char* ICMP_alt_hdr[5];		//rotating list of headers, see below for reasoning
	ICMP_alt_hdr[0] ="\x03\x03\x68\x79\x62\xe6\xb3\x90";
	ICMP_alt_hdr[1] ="\x03\x03\x68\x79\x63\xf2\xb4\x03";
	ICMP_alt_hdr[2] ="\x03\x03\x68\x79\x64\xfe\xb6\xa0";
	ICMP_alt_hdr[3] ="\x03\x03\x68\x79\x66\x0a\xbc\x3e";
	ICMP_alt_hdr[4] ="\x03\x03\x68\x79\x67\x16\xc4\x4d";
	

	id = ntohl((nfq_get_msg_packet_hdr(nfa))->packet_id);
	nfq_get_payload(nfa, (u_char **)&ip_in);


	switch(ip_in->ip_p)
	{

	case IPPROTO_UDP:
		udp_in = (struct udphdr *)((uint32_t *) ip_in + ip_in->ip_hl);
		#if PACKET_INFO
		mem_print("udp header", udp_in, htons(udp_in->len));
		printf("UDP dest port %04x\n",htons(udp_in->dest));
		printf("UDP length %02x\n",htons(udp_in->len));
		printf("IP DF flag %04x\n",((ip_in->ip_off) && IP_DF));
		#endif
		
		if(htons(udp_in->len) == 0x0134)	//308 bytes long
		{
			if ((memcmp(&(((char*)udp_in)[8]),"\x43\x43\x43\x43\x43\x43\x43\x43\x43\x43\x43\x43\x43\x43\x43\x43\x43\x43\x43\x43",20)==0))
			{	//OS scan packet UDP
				#if PACKET_INFO
				printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~OS scan packet UDP test\n");
				#endif
				#if SYSLOG
				syslog (LOG_INFO, "ID(%d) nfq: ~~ OS scan packet UDP test", HPID);
				#endif
				scan_flag = 2;

			}
		}
		break;

	case IPPROTO_ICMP:
		icmp_in = (struct icmphdr *)((uint32_t *) ip_in + ip_in->ip_hl);	
		#if PACKET_INFO
		mem_print("icmp header", icmp_in, htons(ip_in->ip_len)-(ip_in->ip_hl));
		printf("IP DF flag %04x\n",((ip_in->ip_off) && IP_DF));
		printf("icmp type %02x\n",icmp_in->type);
		printf("icmp code %02x\n",icmp_in->code);		
		#endif	
		if(  ((ip_in->ip_off && IP_DF) ==1) && (icmp_in->type == 0x08)&& (icmp_in->code == 0x09) && (memcmp(&(((char*)icmp_in)[6]),"\x01\x27",2)==0))
			{//OS scan packet #1
					#if PACKET_INFO
					printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~OS scan packet icmp ping test 1\n");
					#endif
					#if SYSLOG
					syslog (LOG_INFO, "ID(%d) nfq: ~~ OS scan ICMP ping test 1", HPID);
					#endif
					scan_flag = 0;
			}

		else if((icmp_in->type == 0x08)&& (icmp_in->code == 0x00) && (memcmp(&(((char*)icmp_in)[6]),"\x01\x28",2)==0))
			{//OS scan packet #2
					#if PACKET_INFO
					printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~OS scan packet icmp ping test 2\n");
					#endif
					#if SYSLOG
					syslog (LOG_INFO, "ID(%d) nfq: ~~ OS scan ICMP ping test 2", HPID);
					#endif
					scan_flag = 1;
			}

		break;
	}



	if(scan_flag!=255)
	{
		int sd;
		// No data, just datagram
		char buffer[PCKT_LEN];
		// The size of the headers
		struct iphdr *ip = (struct iphdr *) buffer;
		struct icmphdr *icmp_var = (struct icmphdr *) (buffer + sizeof(struct iphdr));
		struct sockaddr_in sin, din;
		int one = 1;
		const int *val = &one;
		memset(buffer, 0, PCKT_LEN);
		sd = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP);
		if(sd < 0)
		{
			perror("socket() error");
			exit(-1);
		}
		else
		#if PACKET_INFO
		printf("socket()-SOCK_RAW and icmp protocol is OK.\n");
		printf("icmp_resp0.len: %d\n", resp_dict->os_icmp_resp0.len);	
		#endif
		//initialize the packet with recorded response 

		switch(scan_flag){

		case 0:
			memcpy(ip,&resp_dict->os_icmp_resp0,resp_dict->os_icmp_resp0.len);
		#if PACKET_INFO
			mem_print("ip packet data     ", ip, resp_dict->os_icmp_resp0.len);
		#endif
			// ICMP id
			printf("id, case 0 before   %04x\n", icmp_var->un.echo.id);
			icmp_var->un.echo.id = icmp_in->un.echo.id;
			printf("id, case 0 after    %04x\n", icmp_var->un.echo.id);
			
			icmp_var->un.echo.sequence = icmp_in->un.echo.sequence;
			
			//change IPID to be the same as the ping request's IPID
			(ip->id) = ip_in->ip_id;
			break;

		case 1:
			memcpy(ip,&resp_dict->os_icmp_resp1,resp_dict->os_icmp_resp1.len);
		#if PACKET_INFO
			mem_print("ip packet data     ", ip, resp_dict->os_icmp_resp1.len);
		#endif
			//Change ICMP Seq Number to be same as ping request's ICMP sequence number
			icmp_var->un.echo.sequence = icmp_in->un.echo.sequence;
			
			//Change ICMP id to be same as ping request's ICMP id
			printf("id, case 1 before   %04x\n", icmp_var->un.echo.id);
			icmp_var->un.echo.id = icmp_in->un.echo.id;
			printf("id, case 1 after    %04x\n", icmp_var->un.echo.id);

			//change IPID to be the same as the ping request's IPID
			(ip->id) = ip_in->ip_id;
			break;

		case 2:
			//this replaces the ICMP header with a rotating list of headers based on ICMP_alt_hdr array initialized above.  This is done because the last 4 bytes of this header are fingerprintable
			//  and NMAP might try this UDP test more than once (the most I've seen is 5 times), so we have 5 different rotating values to spit back!
			memcpy(&(resp_dict->os_icmp_resp2.icmp_hdr),ICMP_alt_hdr[global_UDP_cntr%5],8);
			
			memcpy(ip,&resp_dict->os_icmp_resp2,28);
			
			//this is an ICMP unreachable, port unreachable response in this implementation.  Therefore first 28 bytes of last message (ie, the udp query) are to be sent as icmp payload (RFC 2003)
			memcpy(&(((char*)ip)[28]),ip_in, 28);
			

			
			global_UDP_cntr++;	//used up one from the special array of values, next request spits out a different value
		#if PACKET_INFO
			mem_print("ip packet data     ", ip, resp_dict->os_icmp_resp0.len);
		#endif
			// ICMP id
			//icmp_var->un.echo.id = icmp_in->un.echo.id;  //use default
			break;
		default:
			return 0; //return 0 means set verdict to ACCEPT
			break;
		}

		//now fill in all the fields with info pertinent to this incoming query
		// Address family
		sin.sin_family = AF_INET;
		din.sin_family = AF_INET;

		//reverse ip addresses
		sin.sin_addr = ip_in->ip_dst;
		din.sin_addr = ip_in->ip_src;
/*
		// IP structure
		ip->ihl = 5;
		ip->version = 4;
		ip->tos = 16;
		ip->tot_len = sizeof(struct ipheader) + sizeof(struct tcpheader);
		ip->id = htons(00000);
		ip->frag_off = 0;
		ip->ttl = 64;
		ip->protocol = 6; // TCP
		ip->check = 0; // Done by kernel
*/
		//printf("print3-a    %04x\n", icmp_in->un.echo.id;);


		//zero out the checksum so we can calulate the new checksum below
		icmp_var->checksum = 0;	

		// Source IP, modify as needed
		ip->saddr = ip_in->ip_dst.s_addr;

		// Destination IP, modify as needed
		ip->daddr = ip_in->ip_src.s_addr;

		#if PACKET_INFO
		mem_print("ip pkt after ini   ", ip, htons(ip->tot_len));
		#endif
		
		// IP checksum calculation
		//ip->check = csum((unsigned short *) buffer, (sizeof(struct iphdr) + sizeof(struct icmphdr)));

		// ICMP checksum calculation	
		icmp_var->checksum = icmp_checksum((uint16_t*) icmp_var,htons(ip->tot_len)-ip->ihl);

		// Inform the kernel do not fill up the headers's structure, we fabricated our own
		if(setsockopt(sd, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0)
		{
			perror("setsockopt() error");
			exit(-1);
		}
		else
		{
		#if DEBUG
		printf("setsockopt() is OK\n");
		#endif
		}		
		#if PACKET_INFO
		printf("Using:::::Source IP: %08x Target IP: %08x.\n", ip_in->ip_dst, ip_in->ip_src);
		#endif

		#if DEBUG
		mem_print("sending this data  ", buffer, htons(ip->tot_len));
		#endif
		if(sendto(sd, buffer, htons(ip->tot_len), 0, (struct sockaddr *)&din, sizeof(din)) < 0)
		// Verify
		{
			perror("sendto() error");
			exit(-1);
		}
		else
		{	
			#if DEBUG
			printf("Send Successful\n");
			#endif
		}
		close(sd);
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

//end//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


u_int32_t OS_scan (struct nfq_data *nfa, struct response_dict* resp_dict, char *ip_from_cb)
{
    u_int32_t id;
	struct ip *ip_in;
	struct tcphdr *tcp_in;
	int scan_flag = 255;

	id = ntohl((nfq_get_msg_packet_hdr(nfa))->packet_id);
	nfq_get_payload(nfa, (u_char **)&ip_in);
	tcp_in = (struct tcphdr *)((uint32_t *) ip_in + ip_in->ip_hl);

	//printf("~~~~~~tcp offset %02x\n", (tcp_in->doff));	
#if PACKET_INFO
printf("TCP dest port %04x\n",htons(tcp_in->dest));
printf("TCP offset %02x\n",(tcp_in->doff));
printf("IP DF flag %04x\n",((ip_in->ip_off) && IP_DF));
#endif
if(htons(tcp_in->dest) == 0x0050)
	{
		if((tcp_in->doff) == 0x0a)	//40
		{

			if ((memcmp(&(((char*)tcp_in)[20]),"\x03\x03\x0a\x01\x02\x04\x05\xb4\x08\x0a\xff\xff\xff\xff\x00\x00\x00\x00\x04\x02",20)==0))
			{	//OS scan packet #1
				#if PACKET_INFO
				printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~OS scan packet #1\n");
				#endif
				#if SYSLOG
				syslog (LOG_INFO, "ID(%d) nfq: ~~ OS scan packet #1 (on port 80)", HPID);
				#endif
				scan_flag = 0;
			}

			else if ((memcmp(&(((char*)tcp_in)[20]),"\x02\x04\x05\x78\x03\x03\x00\x04\x02\x08\x0a\xff\xff\xff\xff\x00\x00\x00\x00\x00",20)==0))
			{	//OS scan packet #2
				#if PACKET_INFO
				printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~OS scan packet #2\n");
				#endif
				#if SYSLOG
				syslog (LOG_INFO, "ID(%d) nfq: ~~ OS scan packet #2 (on port 80)", HPID);
				#endif
				scan_flag = 1;
			}

			else if ((memcmp(&(((char*)tcp_in)[20]),"\x08\x0a\xff\xff\xff\xff\x00\x00\x00\x00\x01\x01\x03\x03\x05\x01\x02\x04\x02\x80",20)==0))
			{	//OS scan packet #3
				#if PACKET_INFO
				printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~OS scan packet #3\n");
				#endif
				#if SYSLOG
				syslog (LOG_INFO, "ID(%d) nfq: ~~ OS scan packet #3 (on port 80)", HPID);
				#endif
				scan_flag = 2;
			}

			else if ((memcmp(&(((char*)tcp_in)[20]),"\x02\x04\x02\x18\x04\x02\x08\x0a\xff\xff\xff\xff\x00\x00\x00\x00\x03\x03\x0a\x00",20)==0))
			{	//OS scan packet #5
				#if PACKET_INFO
				printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~OS scan packet #5\n");
				#endif
				#if SYSLOG
				syslog (LOG_INFO, "ID(%d) nfq: ~~ OS scan packet #5 (on port 80)", HPID);
				#endif
				scan_flag = 4;
			}	
		

	//start of T2-T7 tests

			else if ( (memcmp(&(((char*)tcp_in)[12]),"\xa0\x00\x00\x80",4)==0) && (memcmp(&(((char*)tcp_in)[18]),"\x00\x00\x03\x03\x0a\x01\x02\x04\x01\x09\x08\x0a\xff\xff\xff\xff\x00\x00\x00\x00\x04\x02",22)==0) )
			{	//OS scan packet #T2 no TCP flags set, window 128, and options set accordingly
				#if PACKET_INFO
				printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~OS scan packet T2 (null TCP)\n");
				#endif
				#if SYSLOG
				syslog (LOG_INFO, "ID(%d) nfq: ~~ OS scan packet T2 (null TCP) (on port 80)", HPID);
				#endif
				//this is a null TCP packet, just drop it (by returing 0)
				//scan_flag = 7;  //left in for completeness
				#if PACKET_INFO
				printf("verdict = DROP	(with no response)\n");
				printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
				#endif
				return 1;
			}

			else if ( (ip_in->ip_off && IP_DF)==0 && (memcmp(&(((char*)tcp_in)[12]),"\xa0\x2b\x01\x00",4)==0) && (memcmp(&(((char*)tcp_in)[18]),"\x00\x00\x03\x03\x0a\x01\x02\x04\x01\x09\x08\x0a\xff\xff\xff\xff\x00\x00\x00\x00\x04\x02",22)==0) )
			{	//OS scan packet #T3, IP_DF flag not set, 4 TCP flags set, window 256, and options set accordingly
				#if PACKET_INFO
				printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~OS scan packet T3 (SYN, FIN, URG, PSH)\n");
				#endif
				#if SYSLOG
				syslog (LOG_INFO, "ID(%d) nfq: ~~ OS scan packet T3 (SYN, FIN, URG, PSH) (on port 80)", HPID);
				#endif
				scan_flag = 8;
			}
			//tcp header:::c4:d6:00:50:bb:1a:a3:93:93:1e:81:8d:a0:10:04:00:f2:22:                      00:00:03:03:0a:01:02:04:01:09:08:0a:ff:ff:ff:ff:00:00:00:00:04:02:

			else if ( (ip_in->ip_off && IP_DF)==1 && (memcmp(&(((char*)tcp_in)[12]),"\xa0\x10\x04\x00",4)==0) && (memcmp(&(((char*)tcp_in)[18]),"\x00\x00\x03\x03\x0a\x01\x02\x04\x01\x09\x08\x0a\xff\xff\xff\xff\x00\x00\x00\x00\x04\x02",22)==0) )
			{	//OS scan packet #T4 IP_DF flag set, ACK TCP flags set, window 1024, and options set accordingly
				#if PACKET_INFO
				printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~OS scan packet T4 (ACK) \n");
				#endif
				#if SYSLOG
				syslog (LOG_INFO, "ID(%d) nfq: ~~ OS scan packet T4 (ACK) (on port 80)", HPID);
				#endif
				scan_flag = 9;
			}			
		
					
		}

		else if((tcp_in->doff) == 0x09)	//36
		{
			if ((memcmp(&(((char*)tcp_in)[20]),"\x04\x02\x08\x0a\xff\xff\xff\xff\x00\x00\x00\x00\x03\x03\x0a\x00",16)==0))
			{	//OS scan packet #4
				#if PACKET_INFO
				printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~OS scan packet #4\n");
				#endif
				#if SYSLOG
				syslog (LOG_INFO, "ID(%d) nfq: ~~ OS scan packet #4 (on port 80)", HPID);
				#endif
				scan_flag = 3;
			}

			else if ((memcmp(&(((char*)tcp_in)[20]),"\x02\x04\x01\x09\x04\x02\x08\x0a\xff\xff\xff\xff\x00\x00\x00\x00",16)==0))
			{	//OS scan packet #6
				#if PACKET_INFO
				printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~OS scan packet #6\n");
				#endif
				#if SYSLOG
				syslog (LOG_INFO, "ID(%d) nfq: ~~ OS scan packet #6 (on port 80)", HPID);
				#endif
				scan_flag = 5;
			}

		}
		else if((tcp_in->doff) == 0x08)	//32
		{	//4500003471c700003b06f8d30a0100ad0a01007b b60e0050212f4d2d0000000088c200032b7af7f5 03030a01020405b404020101

			if ( (memcmp(&(((char*)tcp_in)[12]),"\x88\xc2",2)==0) && (memcmp(&(((char*)tcp_in)[18]),"\xf7\xf5\x03\x03\x0a\x01\x02\x04\x05\xb4\x04\x02\x01\x01",14)==0) )
			{	//OS scan packet ECN Test
				#if PACKET_INFO
				printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~OS scan packet ECN Test\n");
				#endif
				#if SYSLOG
				syslog (LOG_INFO, "ID(%d) nfq: ~~ OS scan packet ECN Test (on port 80)", HPID);
				#endif
				scan_flag = 6;
			}
		}
	}

//else if((htons(tcp_in->dest) != 0xf601) || (htons(tcp_in->dest) != 0x0050) || (htons(tcp_in->dest) != 0x7070))

else if(htons(tcp_in->dest) == 0x0001)		//nmap chooses a closed port to perform these tests on.  When using nmap -p 1-66535, nmap chooses port 1 to perform its "closed port OS scans" (tests T5, T6, T7) upon
	{					//TODO: this would be better handled with an iptables rule that says if not ports 502, 80, 28784, and .  I was trying to avoid pulling every packet thrown at a closed port into the queue (huge time loss)
						//	The problem is that ip tables cannot match on the DF flag (ip option) 
		if((tcp_in->doff) == 0x0a)	//40
		{

			if ( (ip_in->ip_off && IP_DF)==0 && (memcmp(&(((char*)tcp_in)[12]),"\xa0\x02\x7a\x69",4)==0) && (memcmp(&(((char*)tcp_in)[18]),"\x00\x00\x03\x03\x0a\x01\x02\x04\x01\x09\x08\x0a\xff\xff\xff\xff\x00\x00\x00\x00\x04\x02",22)==0) )
			{	//OS scan packet #T5 IP_DF flag not set, SYN, window 31337, and options set accordingly
				#if PACKET_INFO
				printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~OS scan packet T5\n");
				#endif
				#if SYSLOG
				syslog (LOG_INFO, "ID(%d) nfq: ~~ OS scan packet T5 (on port 1)", HPID);
				#endif
				scan_flag = 99;	//using different sequence of scan flags since these are sent to a closed port, as opposed to an open port 
			}

			if ( (ip_in->ip_off && IP_DF)==1 && (memcmp(&(((char*)tcp_in)[12]),"\xa0\x10\x80\x00",4)==0) && (memcmp(&(((char*)tcp_in)[18]),"\x00\x00\x03\x03\x0a\x01\x02\x04\x01\x09\x08\x0a\xff\xff\xff\xff\x00\x00\x00\x00\x04\x02",22)==0) )
			{	//OS scan packet #T6 IP_DF flag set, ACK, window 32768, and options set accordingly
				#if PACKET_INFO
				printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~OS scan packet T6 (on port 1)\n");
				#endif
				#if SYSLOG
				syslog (LOG_INFO, "ID(%d) nfq: ~~ OS scan packet T6", HPID);
				#endif
				scan_flag = 98;
			}

				//tcp header:::f3:49:00:01:2b:66:d4:63:58:3d:aa:8f:a0:29:ff:ff:33:a9:                                                         00: 00: 03: 03: 0f: 01: 02: 04: 01: 09: 08: 0a: ff: ff: ff: ff: 00: 00: 00: 00: 04: 02:
			if ( (ip_in->ip_off && IP_DF)==0 && (memcmp(&(((char*)tcp_in)[12]),"\xa0\x29\xff\xff",4)==0) && (memcmp(&(((char*)tcp_in)[18]),"\x00\x00\x03\x03\x0f\x01\x02\x04\x01\x09\x08\x0a\xff\xff\xff\xff\x00\x00\x00\x00\x04\x02",22)==0) )
			{	//OS scan packet #T7 IP_DF flag not set, FIN, PSH, URG, window 65535, and options set accordingly
				#if PACKET_INFO
				printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~OS scan packet T7 (on port 1)\n");
				#endif
				#if SYSLOG
				syslog (LOG_INFO, "ID(%d) nfq: ~~ OS scan packet T7", HPID);
				#endif
				scan_flag = 97;
			}
		}		
	}


	if(scan_flag!=255)
	{
		int sd;
		// No data, just datagram
		char buffer[PCKT_LEN];
		// The size of the headers
		struct iphdr *ip = (struct iphdr *) buffer;
		struct tcphdr *tcp = (struct tcphdr *) (buffer + sizeof(struct iphdr));
		struct sockaddr_in sin, din;
		int one = 1;
		const int *val = &one;
		memset(buffer, 0, PCKT_LEN);
		sd = socket(PF_INET, SOCK_RAW, IPPROTO_TCP);
		if(sd < 0)
		{
			perror("socket() error");
			exit(-1);
		}
		else
		#if PACKET_INFO
		printf("socket()-SOCK_RAW and icmp protocol is OK.\n");	
		#endif

		//initialize the packet with recorded response 

		switch(scan_flag){

		case 0:
			//Pkt #1 (part of T1-first)
			memcpy(ip,&resp_dict->os_tcp_resp0,resp_dict->os_tcp_resp0.len);
			#if PACKET_INFO
			mem_print("ip packet data     ", ip, resp_dict->os_tcp_resp0.len);
			#endif

			// TCP ack number (this is an indicator in nmap OS scan: A, in probe for this PLC implementation, the value is S+
			//    which means that the ack# is the same as the seq# plus one of incoming packet)
			tcp->ack_seq = htonl(htonl((tcp_in->seq))+1);   

			// TCP sequence number (this is an indicator in nmap OS scan: S, in probe for this PLC implementation, the value is O
			//    which means that the seq# something different)
			//tcp->seq = random;	//could generate one, but just dont set it, and use what we got from the PLC recording

			break;

		case 1:
			//Pkt #2 (part of T1)
			memcpy(ip,&resp_dict->os_tcp_resp1,resp_dict->os_tcp_resp1.len);
			#if PACKET_INFO
			mem_print("ip packet data     ", ip, resp_dict->os_tcp_resp1.len);
			#endif			
			// TCP ack number 
			tcp->ack_seq = htonl(htonl((tcp_in->seq))+1);   
			// TCP sequence number 
			//USE RECORDED-tcp->seq = htonl(htonl((tcp_in->ack_seq))+0);
			break;

		case 2:
			//Pkt #3 (part of T1)
			memcpy(ip,&resp_dict->os_tcp_resp2,resp_dict->os_tcp_resp2.len);
			#if PACKET_INFO
			mem_print("ip packet data     ", ip, resp_dict->os_tcp_resp2.len);
			#endif
			// TCP ack number 
			tcp->ack_seq = htonl(htonl((tcp_in->seq))+1);   
			// TCP sequence number 
			//USE RECORDED-tcp->seq = htonl(htonl((tcp_in->ack_seq))+0);
			break;

		case 3:
			//Pkt #4 (part of T1)
			memcpy(ip,&resp_dict->os_tcp_resp3,resp_dict->os_tcp_resp3.len);
			#if PACKET_INFO
			mem_print("ip packet data     ", ip, resp_dict->os_tcp_resp3.len);
			#endif
			// TCP ack number 
			tcp->ack_seq = htonl(htonl((tcp_in->seq))+1);   
			// TCP sequence number 
			//USE RECORDED-tcp->seq = htonl(htonl((tcp_in->ack_seq))+0);
			break;

		case 4:
			//Pkt #5 (part of T1)
			memcpy(ip,&resp_dict->os_tcp_resp4,resp_dict->os_tcp_resp4.len);
			#if PACKET_INFO
			mem_print("ip packet data     ", ip, resp_dict->os_tcp_resp4.len);
			#endif
			// TCP ack number 
			tcp->ack_seq = htonl(htonl((tcp_in->seq))+1);   
			// TCP sequence number 
			//USE RECORDED-tcp->seq = htonl(htonl((tcp_in->ack_seq))+0);
			break;

		case 5:
			//Pkt #6 (part of T1-last)
			memcpy(ip,&resp_dict->os_tcp_resp5,resp_dict->os_tcp_resp5.len);
			#if PACKET_INFO
			mem_print("ip packet data     ", ip, resp_dict->os_tcp_resp5.len);
			#endif
			// TCP ack number 
			tcp->ack_seq = htonl(htonl((tcp_in->seq))+1);   
			// TCP sequence number 
			//USE RECORDED-tcp->seq = htonl(htonl((tcp_in->ack_seq))+0);
			break;

		case 6:
			//ECN
			memcpy(ip,&resp_dict->os_tcp_resp6,resp_dict->os_tcp_resp6.len);
			#if PACKET_INFO
			mem_print("ip packet data     ", ip, resp_dict->os_tcp_resp6.len);
			#endif
			// TCP ack number 
			tcp->ack_seq = htonl(htonl((tcp_in->seq))+0);   
			// TCP sequence number 
			//USE RECORDED-tcp->seq = htonl(htonl((tcp_in->ack_seq))+0);
			break;

		case 7:
			//T2 Null TCP
			memcpy(ip,&resp_dict->os_tcp_resp7,resp_dict->os_tcp_resp7.len);
			#if PACKET_INFO
			mem_print("ip packet data     ", ip, resp_dict->os_tcp_resp7.len);
			#endif
			break;
				
		case 8:
			//T3
			memcpy(ip,&resp_dict->os_tcp_resp8,resp_dict->os_tcp_resp8.len);
			#if PACKET_INFO
			mem_print("ip packet data     ", ip, resp_dict->os_tcp_resp8.len);
			#endif
			// TCP ack number (this is an indicator in nmap OS scan: A, in probe for this PLC implementation, the value is S+
			//    which means that the ack# is 1 + seq of incoming packet)
			tcp->ack_seq = htonl(htonl((tcp_in->seq))+1);   

			// TCP sequence number (this is an indicator in nmap OS scan: S, in probe for this PLC implementation, the value is O
			//    which means that the seq# something different)
			//tcp->seq = random;	//could generate one, but just dont set it, and use what we got from the PLC recording
			break;		

		case 9:
			//T4
			memcpy(ip,&resp_dict->os_tcp_resp9,resp_dict->os_tcp_resp9.len);
			#if PACKET_INFO
			mem_print("ip packet data     ", ip, resp_dict->os_tcp_resp9.len);
			#endif
			// TCP ack number (this is an indicator in nmap OS scan: A, in probe for this PLC implementation, the value is S
			//    which means that the ack# seq of incoming packet)
			tcp->ack_seq = htonl(htonl((tcp_in->seq))+0);   

			// TCP sequence number (this is an indicator in nmap OS scan: S, in probe for this PLC implementation, the value is A+
			//    which means that the seq# something different)
			tcp->seq = htonl(htonl((tcp_in->ack_seq))+1);

			break;		

		case 99:
			//T5
			memcpy(ip,&resp_dict->os_tcp_resp99,resp_dict->os_tcp_resp99.len);
			#if PACKET_INFO
			mem_print("ip packet data     ", ip, resp_dict->os_tcp_resp99.len);
			#endif
			// TCP ack number (this is an indicator in nmap OS scan: A, in probe for this PLC implementation, the value is S+
			//    which means that the ack# seq of incoming packet)
			tcp->ack_seq = htonl(htonl((tcp_in->seq))+1);   


			// TCP sequence number (this is an indicator in nmap OS scan: S, in probe for this PLC implementation, the value is A
			//    which means that the seq# something different)
			tcp->seq = htonl(htonl((tcp_in->ack_seq))+0);

			break;

		case 98:
			//T6
			memcpy(ip,&resp_dict->os_tcp_resp98,resp_dict->os_tcp_resp98.len);
			#if PACKET_INFO
			mem_print("ip packet data     ", ip, resp_dict->os_tcp_resp98.len);
			#endif
			// TCP ack number (this is an indicator in nmap OS scan: A, in probe for this PLC implementation, the value is S
			//    which means that the ack# seq of incoming packet)
			tcp->ack_seq = htonl(htonl((tcp_in->seq))+0);   

			// TCP sequence number (this is an indicator in nmap OS scan: S, in probe for this PLC implementation, the value is A
			//    which means that the seq# something different)
			tcp->seq = htonl(htonl((tcp_in->ack_seq))+0);
			break;	


		case 97:
			//T7
			memcpy(ip,&resp_dict->os_tcp_resp97,resp_dict->os_tcp_resp97.len);
			#if PACKET_INFO
			mem_print("ip packet data     ", ip, resp_dict->os_tcp_resp97.len);
			#endif
			// TCP ack number (this is an indicator in nmap OS scan: A, in probe for this PLC implementation, the value is S+
			//    which means that the ack# seq of incoming packet)
			tcp->ack_seq = htonl(htonl((tcp_in->seq))+1);   

			// TCP sequence number (this is an indicator in nmap OS scan: S, in probe for this PLC implementation, the value is A
			//    which means that the seq# something different)
			tcp->seq = htonl(htonl((tcp_in->ack_seq))+0);
			break;			

		default:
			return 0; //return 0 means set verdict to ACCEPT
			break;
		}

		//now fill in all the fields with info pertinent to this incoming query
		// Address family
		sin.sin_family = AF_INET;
		din.sin_family = AF_INET;

		//reverse ports
		sin.sin_port = tcp_in->dest;
		din.sin_port = tcp_in->source;

		//reverse ip addresses
		sin.sin_addr = ip_in->ip_dst;
		din.sin_addr = ip_in->ip_src;
/*
		// IP structure
		ip->ihl = 5;
		ip->version = 4;
		ip->tos = 16;
		ip->tot_len = sizeof(struct ipheader) + sizeof(struct tcpheader);
		ip->id = htons(00000);
		ip->frag_off = 0;
		ip->ttl = 64;
		ip->protocol = 6; // TCP
		ip->check = 0; // Done by kernel
*/


		// Source IP, modify as needed
		ip->saddr = ip_in->ip_dst.s_addr;

		// Destination IP, modify as needed
		ip->daddr = ip_in->ip_src.s_addr;

		// TCP structure
		tcp->source = tcp_in->dest;

		// The destination port
		tcp->dest = tcp_in->source;  
		#if DEBUG
		mem_print("ip pkt after ini   ", ip, htons(ip->tot_len));
		printf("ip tot_len: %d\n", htons(ip->tot_len));
		#endif

/*
		tcp->seq = htonl(1);		
		tcp->doff = 5;
		tcp->syn = 1;
		tcp->ack = 0;
		tcp->window = htons(32767);
		tcp->check = 0; // Done by kernel
		tcp->urg_ptr = TCP offset 080;
*/
		// IP checksum calculation
		ip->check = 0x0000;
		ip->check = csum((unsigned short *) buffer, (sizeof(struct iphdr) + sizeof(struct tcphdr)));

		// Inform the kernel do not fill up the headers's structure, we fabricated our own
		if(setsockopt(sd, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0)
		{
			perror("setsockopt() error");
			exit(-1);
		}
		else
{
		#if DEBUG
		printf("setsockopt() is OK\n");
		#endif
}
		#if PACKET_INFO
		printf("Using:::::Source IP: %08x port: %d, Target IP: %08x port: %d.\n", ip_in->ip_dst, htons(tcp_in->source), ip_in->ip_src,  htons(tcp_in->dest));
		mem_print("sending this data  ", buffer, htons(ip->tot_len));
		#endif

		if(sendto(sd, buffer, htons(ip->tot_len), 0, (struct sockaddr *)&din, sizeof(din)) < 0)
		// Verify
		{
			perror("sendto() error");
			exit(-1);
		}
		else
		{	
			#if DEBUG
			printf("Send Successful\n");
			#endif
		}
		close(sd);
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




int main(int argc, char **argv)
{

    struct nfnl_handle;
    int fd;
    int rv;
    char buf[4096] __attribute__ ((aligned));

	struct response_dict* resp_dict_ptr;	
	resp_dict_ptr= malloc( sizeof(struct response_dict));

	ini_responses(resp_dict_ptr);

	// Set up signal handler
	signal(SIGINT,graceful_exit);
	signal(SIGHUP,graceful_exit);
	
	
printf("\n+++++Starting infilterall.o now.");

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
            //exit(1);
    }
#if PRINT_INFO    
printf("opening library handle\n");
printf("binding nfnetlink_queue as nf_queue handler for AF_INET\n");
#endif
    if (nfq_bind_pf(h, AF_INET) < 0) {
            fprintf(stderr, "in_filter.o: error during nfq_bind_pf()\n");
	    perror("");
           // exit(1);
    }
#if PRINT_INFO    
    printf("binding this socket to queue '1'\n");
#endif
    qh = nfq_create_queue(h,  1, &cb, resp_dict_ptr);							///set queue number here
    if (!qh) {
            fprintf(stderr, "error during nfq_create_queue() on Queue 1 (incoming filter)\n");
            //exit(1);
    }
#if PRINT_INFO
    printf("setting copy_packet mode\n");
#endif
    if (nfq_set_mode(qh, NFQNL_COPY_PACKET, 0xffff) < 0) {
            fprintf(stderr, "can't set packet_copy mode\n");
            exit(1);
    }

    fd = nfq_fd(h);
printf("  Firewall Active.\n");
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



















