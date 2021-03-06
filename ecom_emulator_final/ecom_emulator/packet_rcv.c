/**********************************************************************
*
* Compile with:
* didnt use this-> gcc -Wall -pedantic disect2.c -lpcap 
* gcc libpcap_filter.c filterlibpcap.o -lnet -lpcap -o filterlibpcap
*
* Usage:
* a.out (# of packets) "filter string"
*
**********************************************************************/

#include <libnet.h>
#include "HAP_emulator.h"
#include "libpcap_filter.h"
#include "libnet_send.h"
#include <unistd.h>
#include <net/route.h>
#include <syslog.h>
#include <sys/time.h>
#include "minIni.h"


//#include <arpa/inet.h>
//#include <netinet/in.h>
//#include <stdio.h>
//#include <sys/types.h>
//#include <sys/socket.h>




//int respond_polling_all(struct hap_simple* hap_simple, const u_char* packet);
int respond_read_version_info(struct hap_simple* hap_simple, const u_char* packet, struct ecomDevice *eDevice);
int respond_polling_all(struct hap_simple* hap_simple, const u_char* packet, struct ecomDevice *eDevice);
int respond_read_setup_data(struct hap_simple* hap_simple, const u_char* packet, struct ecomDevice *eDevice);

/* looking at ethernet headers */
void my_callback(u_char *args,const struct pcap_pkthdr* pkthdr,const u_char* packet)
{
	#if DEBUG
	printf("*~*~ libpcap_filter.c - my_callback\n");
	#endif

	
	/*struct timespec req={0};
	req.tv_sec=0;
	req.tv_nsec=000000L;
	while(nanosleep(&req,&req)==-1)	
	      continue;
	*/
	
	

	
	
	
	
    struct ecomDevice *eDevice = (struct ecomDevice*)args;

    u_int16_t type = handle_ethernet(args,pkthdr,packet);
    u_int16_t udp_port;
    struct my_hap* hap_response;

    u_int16_t data_len;
    u_char* hap_data;
    struct hap_simple* hap_simple_ptr; 
    hap_simple_ptr =malloc(sizeof(struct hap_simple));
    memset( hap_simple_ptr, 0, sizeof(struct hap_simple));

    if(type == ETHERTYPE_IP)
    {/* handle IP packet */
        type = handle_IP(args,pkthdr,packet);
	if (type == IPPROTO_UDP)
	{
		udp_port= handle_UDP(args,pkthdr,packet);
		if (udp_port == 28784)
		{
			hap_response = handle_HAP(args,pkthdr,packet);
			data_len = 256*hap_response->hap_length2+hap_response->hap_length1;				
			hap_data = (u_char*)hap_response;	//hap_data is now just the hap data

			#if DATA_PRINT
			mem_print("HAP data", hap_data, 9+data_len);
			#endif
			
			//check for hap header
			if(hap_data[0] == 'H' && hap_data[1] == 'A' && hap_data[2] == 'P')
			{
				hap_simple_ptr->hap_crc = 256*hap_response->hap_crc2+hap_response->hap_crc1;					
				hap_simple_ptr->hap_length = data_len;
				hap_simple_ptr->hap_AppVal = 256*hap_response->hap_AppVal2+hap_response->hap_AppVal1;
				hap_simple_ptr->hap_Fun_Code = hap_response->hap_Fun_Code;

				memcpy(&(hap_simple_ptr->hap_data_buff), &(hap_response->hap_data_buff), data_len);	
    				
				#if DATA_PRINT
				mem_print("HAP Data section", &hap_simple_ptr->hap_data_buff, data_len);
				#endif

				#if SYSLOG
				char ascii_HAP[256];
				memset(ascii_HAP,0,256);
				hex_encode(ascii_HAP, (char*)&hap_simple_ptr->hap_data_buff,data_len-1);
				syslog (LOG_INFO, "ID(%d) hap: APPVAL(%004x) HAPCRC(%004x) LEN(%004x) FUNC(%004x) QUERY(%s)", HPID, hap_simple_ptr->hap_AppVal, hap_simple_ptr->hap_crc, 
													hap_simple_ptr->hap_length, hap_simple_ptr->hap_Fun_Code,
													ascii_HAP);

				char raw_bytes[1024];
				memset(raw_bytes,0,1024);
			        int len_tol = (sizeof(struct ether_header)+ sizeof(struct my_ip)+ sizeof(struct my_udp)+10+data_len);	//10=size of hap header
				hex_encode(raw_bytes, packet,len_tol-1);
				syslog (LOG_INFO, "ID(%d) hap: PKT BYTES(%s)", HPID, raw_bytes);

				#endif

				decide_response(hap_simple_ptr, packet, eDevice);

	
			}		
			else
				printf("bad HAP header\n");				
				//invalid header, drop packet
			

		}
	}

    }else if(type == ETHERTYPE_ARP)
    {/* handle arp packet */
    }
    else if(type == ETHERTYPE_REVARP)
    {/* handle reverse arp packet */
    }


    free(hap_simple_ptr);

}

void mem_print(const char* label, void* data, u_int length)
{	
	#if DEBUG
	printf("*~*~ libpcap_filter.c - mem_print\n");
	#endif


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


int decide_response(struct hap_simple* hap_simple, const u_char* packet, struct ecomDevice *eDevice)
{
	#if DEBUG
	printf("*~*~ libpcap_filter.c - decide_response\n");
	#endif	

	#if DEBUG
	printf("received func_code (%02x) ", hap_simple->hap_Fun_Code);
	#endif

	switch(hap_simple->hap_Fun_Code)
	{
	case FUN_POLLING_ALL:
		respond_polling_all(hap_simple, packet, eDevice);
		break;
	
	case FUN_ADDRESSED_BROADCAST:
		#if (DEBUG + MINIMAL_PRINT)
		printf("(Addressed Broadcast Function)\n");	
		#endif
		
		#if SYSLOG
		syslog (LOG_INFO, "ID(%d) hap: ~~ Addressed Broadcast Function", HPID);
		#endif
		

		respond_addressed_broadcast(hap_simple, packet, eDevice);
		break;

	case FUN_QUERY_SETUP_DATA:
		#if (DEBUG + MINIMAL_PRINT)
		printf("(Query Setup Data Function)\n");
		#endif
		#if SYSLOG
		syslog (LOG_INFO, "ID(%d) hap: ~~ Query Setup Data Function", HPID);
		#endif
		
		respond_query_setup_data(hap_simple, packet, eDevice);
		break;

	case FUN_POLLING:
		//respond_polling_all(struct hap_simple* hap_simple, const u_char* packet);
		break;

	case FUN_READ_SETUP_DATA:
		#if (DEBUG + MINIMAL_PRINT)
		printf("(Read Setup Data Function)\n");
		#endif
		#if SYSLOG
		syslog (LOG_INFO, "ID(%d) hap: ~~ Read Setup Data Function", HPID);
		#endif
		
		
		respond_read_setup_data(hap_simple, packet, eDevice);
		break;

	case FUN_WRITE_SETUP_DATA:
		#if (DEBUG + MINIMAL_PRINT)
		printf("(Write Setup Data Function)\n");
		#endif
		#if SYSLOG
		syslog (LOG_INFO, "ID(%d) hap: ~~ Write Setup Data Function", HPID);
		#endif
		respond_write_setup_data(hap_simple, packet, eDevice);
		break;

	case FUN_READ_DEVICE_INFO:
		#if (DEBUG + MINIMAL_PRINT)
		printf("(Read Device Info Funtion)\n");
		#endif
		#if SYSLOG
		syslog (LOG_INFO, "ID(%d) hap: ~~ Read Device Info Funtion", HPID);
		#endif
		respond_read_device_info(hap_simple, packet, eDevice);
		break;

	case FUN_CCM_REQUEST:
		#if (DEBUG + MINIMAL_PRINT)
		printf("(CCM Request Function)\n");
		#endif
		#if SYSLOG
		syslog (LOG_INFO, "ID(%d) hap: ~~ CCM Request Function", HPID);
		#endif
		respond_ccm_request(hap_simple, packet, eDevice);
		//respond_polling_all(struct hap_simple* hap_simple, const u_char* packet);
		break;

	case FUN_KSEQ_REQUEST:
		#if (DEBUG + MINIMAL_PRINT)
		printf("(KSeq request Function)\n");
		#endif
		#if SYSLOG
		syslog (LOG_INFO, "ID(%d) hap: ~~ KSeq request Function", HPID);
		#endif
		respond_kseq_request(hap_simple, packet, eDevice);
		//respond_polling_all(struct hap_simple* hap_simple, const u_char* packet);
		break;

	case FUN_READ_VER_INFO:
		#if (DEBUG + MINIMAL_PRINT)
		printf("(Read Version Info Function)\n");
		#endif
		#if SYSLOG
		syslog (LOG_INFO, "ID(%d) hap: ~~ Read Version Info Function", HPID);
		#endif 
		respond_read_version_info(hap_simple, packet, eDevice);
		break;

	case FUN_READ_ETHERNET_STATS:
		#if (DEBUG + MINIMAL_PRINT)
		printf("(Read Ethernet Stats Function)\n");
		#endif
		#if SYSLOG
		syslog (LOG_INFO, "ID(%d) hap: ~~ Read Ethernet Stats Function", HPID);
		#endif
		respond_read_estats(hap_simple, packet, eDevice);
		break;

	default:
		#if (DEBUG + MINIMAL_PRINT)
		printf("(func_code not recognized) \n");
		#endif		
		#if SYSLOG
		syslog (LOG_INFO, "ID(%d) hap: ~~ unknown function request", HPID);
		#endif
		break;
	}
	return 0;
}

int local_config(HEIDevice **pDevice, libnet_t **l_ptr, const u_char* packet) 
{

	#if DEBUG
	printf("*~*~ libpcap_filter.c - local_config\n");
	#endif	
	/*This is done for every packet sent*/	
        const struct my_ip* ip;
        const struct my_udp* udp;  
	u_char* IPdst;

	ip = (struct my_ip*)(packet + sizeof(struct ether_header));
	udp = (struct my_udp*)(packet + sizeof(struct ether_header) + sizeof(struct my_ip));
	
	((HEIDevice*)*pDevice)->srcPrt = htons(udp->udp_srcprt);

	IPdst = (u_char*)inet_ntoa(ip->ip_src);

	memcpy(&((HEIDevice*)*pDevice)->IPdst, IPdst,18);

return 1;
}

int is_for_me(HEIDevice **pDevice, struct hap_simple* hap_simple)
{
	#if DEBUG
	printf("*~*~ libpcap_filter.c - is_for_me\n");
	#endif	

	if (memcmp(((HEIDevice*)*pDevice)->ENetAddress, hap_simple->hap_data_buff,6)==0)
		return 1;


	else
		return 0;
}


int respond_polling_all(struct hap_simple* hap_simple, const u_char* packet, struct ecomDevice *eDevice)
{
	#if DEBUG
	printf("*~*~ libpcap_filter.c - respond_polling_all\n");
	#endif	
	
	//do setup that is required for all packets
	local_config(&(eDevice->ecom_pDevice), &(eDevice->ecom_l_ptr), packet);

	//set app val
	(eDevice->ecom_pDevice)->LastAppVal = hap_simple->hap_AppVal;
	
	//send response
	send_respond_polling_all(&(eDevice->ecom_pDevice), &(eDevice->ecom_l_ptr));

	return 0 ;
}

int respond_query_setup_data(struct hap_simple* hap_simple, const u_char* packet, struct ecomDevice *eDevice)
{
	#if DEBUG
	printf("*~*~ libpcap_filter.c - respond_query_setup_data\n");
	#endif	
	
	//do setup that is required for all packets
	local_config(&(eDevice->ecom_pDevice), &(eDevice->ecom_l_ptr), packet);

	//set app val
	(eDevice->ecom_pDevice)->LastAppVal = hap_simple->hap_AppVal;

	//save the querey field request (1st word of data field)
	(eDevice->ecom_pDevice)->setupType = 256*hap_simple->hap_data_buff[2]+hap_simple->hap_data_buff[1];

	switch((eDevice->ecom_pDevice)->setupType)
	{

	case 0x20:
	//node number 4byte

	mem_print("num in buff",  &hap_simple->hap_data_buff[5], 4);
	mem_print("num in struct",  &(eDevice->ecom_pDevice)->NodeNumber, 4);
	if (memcmp(&(eDevice->ecom_pDevice)->NodeNumber, &hap_simple->hap_data_buff[5], 4)==0)
		//send response		
		send_respond_query_setup_data(&(eDevice->ecom_pDevice), &(eDevice->ecom_l_ptr));

		break;

	case 0x16:
	//node name
	mem_print("name in buff",  &hap_simple->hap_data_buff[5], 32);
	mem_print("name in struct",  &(eDevice->ecom_pDevice)->NodeName, 32);
	if (memcmp(&(eDevice->ecom_pDevice)->NodeName, &hap_simple->hap_data_buff[5],32)==0)
		//send response		
		send_respond_query_setup_data(&(eDevice->ecom_pDevice), &(eDevice->ecom_l_ptr));
		break;

	default:
		break;	
	}

	return 0;
}


int respond_ccm_request(struct hap_simple* hap_simple, const u_char* packet, struct ecomDevice *eDevice)
{
	#if DEBUG
	printf("*~*~ libpcap_filter.c - respond_ccm_request\n");
	#endif	
	
	//do setup that is required for all packets
	local_config(&(eDevice->ecom_pDevice), &(eDevice->ecom_l_ptr), packet);

	//set app val
	(eDevice->ecom_pDevice)->LastAppVal = hap_simple->hap_AppVal;

	//put this request into the struct
	memcpy(&((eDevice->ecom_pDevice)->ccmQuery), &(hap_simple->hap_data_buff[2]), sizeof(ccm_request));
	
	//send response
	send_respond_ccm_request(&(eDevice->ecom_pDevice), &(eDevice->ecom_l_ptr));

	return 0 ;
}

int respond_kseq_request(struct hap_simple* hap_simple, const u_char* packet, struct ecomDevice *eDevice)
{
	#if DEBUG
	printf("*~*~ libpcap_filter.c - respond_kseq_request\n");
	#endif	
	
	//do setup that is required for all packets
	local_config(&(eDevice->ecom_pDevice), &(eDevice->ecom_l_ptr), packet);

	//set app val
	(eDevice->ecom_pDevice)->LastAppVal = hap_simple->hap_AppVal;

	//put this request into the struct
	memcpy(&((eDevice->ecom_pDevice)->kseqQuery), &(hap_simple->hap_data_buff[0]), (hap_simple->hap_data_buff[1])+3);
	printf("kseq length: %d\n", (eDevice->ecom_pDevice)->kseqQuery.length);
	mem_print("kseq query: ",&((eDevice->ecom_pDevice)->kseqQuery), (eDevice->ecom_pDevice)->kseqQuery.length+3);
	

	//send response
	send_respond_kseq_request(&(eDevice->ecom_pDevice), &(eDevice->ecom_l_ptr));
	return 0 ;
}



int respond_addressed_broadcast(struct hap_simple* hap_simple, const u_char* packet, struct ecomDevice *eDevice){
	#if DEBUG
	printf("*~*~ libpcap_filter.c - respond_addressed_broadcast\n");
	#endif	
	//const char CONFIG_FILE = CONFIG_FILE;
  	char str[100];
  	unsigned char mac_hex[100];
	long n;

  	// read mac from config file
	n = ini_gets("", "global_mac", "dummy", str, sizearray(str), CONFIG_FILE);
	//convert ascii mac to hex representation
	stoh(str, mac_hex,6);



	if (memcmp(mac_hex, hap_simple->hap_data_buff,6)==0)
	{
		#if DEBUG
		printf("query is for me.\n");
		#endif

		memcpy(&(hap_simple->hap_Fun_Code), &(hap_simple->hap_data_buff[6]), 1);
		memmove(&(hap_simple->hap_data_buff[0]), &(hap_simple->hap_data_buff[7]), 256);

		#if DEBUG
		mem_print("hap data buff post \'addressed\' slice", &hap_simple->hap_data_buff, 80);
		printf("func code post \'addressed\' slice: %02x \n", hap_simple->hap_Fun_Code);
		#endif

		decide_response(hap_simple, packet, eDevice);
		return 1;

	}

	else 
		#if DEBUG
		printf("query is for someone else.\n");
		#endif
	return 0 ;


}


int respond_read_setup_data(struct hap_simple* hap_simple, const u_char* packet, struct ecomDevice *eDevice)
{
	#if DEBUG
	printf("*~*~ libpcap_filter.c - respond_read_setup_data\n");
	#endif	
	

	//do setup that is required for all packets
	local_config(&(eDevice->ecom_pDevice), &(eDevice->ecom_l_ptr), packet);

	//set app val
	(eDevice->ecom_pDevice)->LastAppVal = hap_simple->hap_AppVal;
	
	//save the querey field request (1st word of data field)
	(eDevice->ecom_pDevice)->setupType = 256*hap_simple->hap_data_buff[2]+hap_simple->hap_data_buff[1];
	
	//send response
	send_read_setup_data(&(eDevice->ecom_pDevice), &(eDevice->ecom_l_ptr));

	return 0 ;
}

int respond_write_setup_data(struct hap_simple* hap_simple, const u_char* packet, struct ecomDevice *eDevice)
{
	#if DEBUG
	printf("*~*~ libpcap_filter.c - respond_write_setup_data\n");
	#endif	
	int n;

	char* unit_ip_str;

        unit_ip_str = malloc(20);
	struct in_addr sa;
	char* hexid;
	hexid = malloc(10);
	char command[30];

	//determine what kind of data is to follow
	switch(256*hap_simple->hap_data_buff[2]+hap_simple->hap_data_buff[1])
	{

	case 0x20:
	//node number 4byte
	memcpy((char*) &((eDevice->ecom_pDevice)->NodeNumber), (char*) &(hap_simple->hap_data_buff[3]) , 4);
	//printf("new number: %ld\n",  *((u_int32_t*)&(hap_simple->hap_data_buff[3])) );
	n = ini_putl("", "setid_n",*((u_int32_t*)&(hap_simple->hap_data_buff[3])), CONFIG_FILE);
	//also store number in hex to be displayed by the web interface correctly
	sprintf(hexid, "0x%X",*((u_int32_t*)&(hap_simple->hap_data_buff[3])));
	n = ini_puts("", "setid_h",hexid, CONFIG_FILE);
		break;

	case 0x30:	
	//subnet mask 4byte
	//memcpy(&Buffer[DataOffset+2], (char*) &(((HEIDevice*)*pDevice)->subnetMask), 4);
	//Total+=4;
		break;

	case 0x10:
	//IP address 4byte
	//memcpy(&Buffer[DataOffset+2], (char*) &(((HEIDevice*)*pDevice)->Address.AddressIP.AddressingType), 4);
	//Total+=4;
		break;
	
	case 0x16:
	//node name 256byte
	memcpy((char*) &((eDevice->ecom_pDevice)->NodeName), (char*) &(hap_simple->hap_data_buff[3]) , 256);
	n = ini_puts("", "setname_n",(char*)(eDevice->ecom_pDevice)->NodeName, CONFIG_FILE);
		break;

	case 0x33:
	//Type name 32byte
	//memcpy(&Buffer[DataOffset+2], ((HEIDevice*)*pDevice)->TypeName, 32);
	//Total+=32;
		break;

	case 0x40:	
	//gateway 4byte
	//memcpy(&Buffer[DataOffset+2], (char*) &(((HEIDevice*)*pDevice)->gateway), 4);
	//Total+=4;
		break;

	case 0x26:
	//node description 256byte
	memcpy((char*) &((eDevice->ecom_pDevice)->NodeDescription), (char*) &(hap_simple->hap_data_buff[3]) , 256);
	n = ini_puts("", "setdesc_n",(char*)(eDevice->ecom_pDevice)->NodeDescription, CONFIG_FILE);

		break;
	
	case 0x8035:	
	//dont know what this is!
	//memcpy(&Buffer[DataOffset+2], version_info, 128);
	//Total+=128;		

		break;
	case 0x36:	
	//IPinfo 4byte
	memcpy((char*) &((eDevice->ecom_pDevice)->IPinfo), (char*) &(hap_simple->hap_data_buff[3]) , 84);
	
	char str[20];
	int fd;
	struct ifreq ifr;
	struct sockaddr_in sin;
        unsigned long unit_ip_hex;
	int s;

	ini_gets("", "setip_dhcp", "dummy", str, 5, CONFIG_FILE);

	printf("dhcp set: %d\n ", strncmp(str,"1",1));

	//dhcp yes/no
	if (*((u_int32_t*) (&(eDevice->ecom_pDevice)->IPinfo.Flags))==4)
	{	P1

		if(strncmp(str,"1",0)==0)//if used to be manual address, swap within the struct
			swap_ip_struct(&((eDevice->ecom_pDevice)->IPinfo));			
	
		n= ini_putl("", "setip_dhcp",1, CONFIG_FILE);	//magic number that means we are using dhcp

	//DHCP IPs

		#if SYSLOG
		syslog (LOG_INFO, "ID(%d) hap: ~~ Attempting DHCP client", HPID);
		#endif
		
		//get New Addresses from dhcp
		sprintf(command, "sudo dhclient %s", INET_INTERFACE);
   		printf("%s\n", command);    
    		system(command);
		
		//check what those addresses are and store those addresses into the struct and save them to the config file
		fd = socket(AF_INET, SOCK_DGRAM, 0);

		/* I want to get an IPv4 IP address */
		ifr.ifr_addr.sa_family = AF_INET;

		/* I want IP address attached to "eth0" */
		strncpy(ifr.ifr_name, INET_INTERFACE, IFNAMSIZ-1);

		ioctl(fd, SIOCGIFADDR, &ifr);
		memcpy(&((eDevice->ecom_pDevice)->IPinfo.IPAddress), (char*)&(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), 4);
		sa.s_addr = *((u_int32_t*)((eDevice->ecom_pDevice)->IPinfo.IPAddress));
		inet_ntop(AF_INET, &(sa), unit_ip_str, INET_ADDRSTRLEN);
		n = ini_puts("", "global_ip",unit_ip_str, CONFIG_FILE);

		ioctl(fd, SIOCGIFNETMASK, &ifr);                      
		memcpy(&((eDevice->ecom_pDevice)->IPinfo.Subnet), (char*)&(((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr), 4);
		sa.s_addr = *((u_int32_t*)((eDevice->ecom_pDevice)->IPinfo.Subnet));
		inet_ntop(AF_INET, &(sa), unit_ip_str, INET_ADDRSTRLEN);
		
		n = ini_puts("", "global_subnet",unit_ip_str, CONFIG_FILE);

		//get the gateway by parsing the netstat command
		char* gateway = NULL;
		FILE* fp = popen("netstat -rn", "r");
		char line[256]={0x0};
		while(fgets(line, sizeof(line), fp) != NULL)
		{    
			char* destination;
			destination = strndup(line, 15);

			char* iface;
// 			iface = strndup(line + 73, 4);

			// Find line with the gateway
			if(strcmp("0.0.0.0        ", destination) == 0 && strcmp(iface, INET_INTERFACE) == 0) 
				gateway = strndup(line + 16, 15);	// Extract gateway
	
		}

		if (gateway != NULL)
		{
		    unit_ip_hex = inet_network(gateway);
	    
		    //endian swap ip address
		    unit_ip_hex = htonl(unit_ip_hex);
		    (eDevice->ecom_pDevice)->gateway=unit_ip_hex;	

		    memcpy(&((eDevice->ecom_pDevice)->IPinfo.Gateway), (char*) &unit_ip_hex, 4);
		    n = ini_puts("", "global_gateway",gateway, CONFIG_FILE);
		}
		else
		{
		  printf("Error: Invalid Internet Gateway.  Try running dhclient on listening interface.\n");
		  
		}


		pclose(fp);
		close(fd);


	}


	else
	{

		//if(strncmp(str,"1",1)== 0)	//if used to be DHCP address, swap within the struct
			swap_ip_struct(&((eDevice->ecom_pDevice)->IPinfo));

		n= ini_putl("", "setip_dhcp",0, CONFIG_FILE);	//magic number that means we are not using dhcp


	//manually coded IPs
		
		sa.s_addr = *((u_int32_t*)((eDevice->ecom_pDevice)->IPinfo.IPAddress));
		memcpy(&((eDevice->ecom_pDevice)->IPinfo.BackupIPAddress), &((eDevice->ecom_pDevice)->IPinfo.IPAddress), 4);	
		inet_ntop(AF_INET, &(sa), unit_ip_str, INET_ADDRSTRLEN);
		n = ini_puts("", "global_ip",unit_ip_str, CONFIG_FILE);
		#if SYSLOG
		syslog (LOG_INFO, "ID(%d) hap: ~~ Setting IP to %s", HPID), unit_ip_str;
		#endif

		sa.s_addr = *((u_int32_t*)((eDevice->ecom_pDevice)->IPinfo.Subnet));
		memcpy(&((eDevice->ecom_pDevice)->IPinfo.BackupSubnet), &((eDevice->ecom_pDevice)->IPinfo.Subnet), 4);	
		inet_ntop(AF_INET, &(sa), unit_ip_str, INET_ADDRSTRLEN);
		n = ini_puts("", "global_subnet",unit_ip_str, CONFIG_FILE);
		#if SYSLOG
		syslog (LOG_INFO, "ID(%d) hap: ~~ Setting SUBNET to %s", HPID), unit_ip_str;
		#endif

		sa.s_addr = *((u_int32_t*)((eDevice->ecom_pDevice)->IPinfo.Gateway));
		memcpy(&((eDevice->ecom_pDevice)->IPinfo.BackupGateway), &((eDevice->ecom_pDevice)->IPinfo.Gateway), 4);
		inet_ntop(AF_INET, &(sa), unit_ip_str, INET_ADDRSTRLEN);
		n = ini_puts("", "global_gateway",unit_ip_str, CONFIG_FILE);
		#if SYSLOG
		syslog (LOG_INFO, "ID(%d) hap: ~~ Setting GATEWAY to %s", HPID), unit_ip_str;
		#endif

		//now actually change the settings on the interface!

		s = socket(AF_INET,SOCK_DGRAM,0); //temporary socket 

		memset(&ifr, 0x0, sizeof(struct ifreq));
		memset(&sin, 0x0, sizeof(struct sockaddr));
		strncpy(ifr.ifr_name, INET_INTERFACE, IF_NAMESIZE-1);
		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = *((u_int32_t*)((eDevice->ecom_pDevice)->IPinfo.IPAddress));
		memcpy(&ifr.ifr_addr, &sin, sizeof(struct sockaddr));

		if(ioctl(s, SIOCSIFADDR, &ifr)< 0){ //set IP-Adress and check for errors
		    P2
		    close(s);
		    perror("ioctl()");}


		memset(&ifr, 0x0, sizeof(struct ifreq));
		memset(&sin, 0x0, sizeof(struct sockaddr));
		strncpy(ifr.ifr_name, INET_INTERFACE, IF_NAMESIZE-1);
		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = *((u_int32_t*)((eDevice->ecom_pDevice)->IPinfo.Subnet));
		memcpy(&ifr.ifr_addr, &sin, sizeof(struct sockaddr));
			    //SIOCSIFNETMASK
		if(ioctl(s, SIOCSIFNETMASK, &ifr)< 0){ //set IP-Adress and check for errors
		    P1
		    close(s);
		    perror("ioctl()");}
		
		//set default gateway
		sa.s_addr = *((u_int32_t*)((eDevice->ecom_pDevice)->IPinfo.Gateway));
		inet_ntop(AF_INET, &(sa), unit_ip_str, INET_ADDRSTRLEN);
		
		sprintf(command, "sudo route add default gw %s %s",unit_ip_str, INET_INTERFACE);
   		printf("change gateway using command: %s\n", command);    
    		system(command);	    
		    

	}
	//put final ip address into global Address within struct
	memcpy((char*) &((eDevice->ecom_pDevice)->Address.AddressIP.AddressingType),(char*) &((eDevice->ecom_pDevice)->IPinfo.IPAddress), 4);
		break;

	case 0x0015:	
	//settings
	memcpy((char*) &((eDevice->ecom_pDevice)->settings_adv), (char*) &(hap_simple->hap_data_buff[3]) , 129);

	n = ini_putl("", "settings_flags",(eDevice->ecom_pDevice)->settings_adv.Flags, CONFIG_FILE);
//#if DEBUG
	printf("flags %d\n",(u_int8_t)((eDevice->ecom_pDevice)->settings_adv.Flags));
//#endif
	int readonly = (u_int8_t)((eDevice->ecom_pDevice)->settings_adv.Flags)&0x2;
	int webenabled = (u_int8_t)((eDevice->ecom_pDevice)->settings_adv.Flags)&0x10;
	if(readonly !=2)  //website not readonly
	{
	  n = ini_puts("", "advanced_readonly_web_checkmark","ENABLED", CONFIG_FILE);
	  n = ini_puts("", "global_submit","yes", CONFIG_FILE);
	  n = ini_puts("", "advanced_h","\'\'", CONFIG_FILE);
	 	  
	}
	
	if(webenabled ==16)  //website enabled
	{
	  n = ini_puts("", "global_webenabled","yes", CONFIG_FILE);	 	  
	}
	if(webenabled !=16)  //website enabled
	{
	  n = ini_puts("", "global_webenabled","no", CONFIG_FILE);	 	  
	}

	n = ini_putl("", "settings_RXWXACKTimeout",(eDevice->ecom_pDevice)->settings_adv.RXWXACKTimeout, CONFIG_FILE);
	n = ini_putl("", "settings_RXWXACKTimeout",(eDevice->ecom_pDevice)->settings_adv.RXWXACKTimeout, CONFIG_FILE);
	n = ini_putl("", "settings_RXWXResponseTimeout",(eDevice->ecom_pDevice)->settings_adv.RXWXResponseTimeout, CONFIG_FILE);
	n = ini_putl("", "settings_RXWXMaxRetrys",(eDevice->ecom_pDevice)->settings_adv.RXWXMaxRetrys, CONFIG_FILE);
	n = ini_putl("", "settings_KSeqMaxRetrys",(eDevice->ecom_pDevice)->settings_adv.KSeqMaxRetrys, CONFIG_FILE);
	n = ini_putl("", "settings_ModbusMasterTimeout",(eDevice->ecom_pDevice)->settings_adv.ModbusMasterTimeout, CONFIG_FILE);
	n = ini_putl("", "settings_ModbusSlaveTimeout",(eDevice->ecom_pDevice)->settings_adv.ModbusSlaveTimeout, CONFIG_FILE);	
		break;

	default:
		#if MINIMAL_PRINT
		printf("setup type not known\n");
		#endif
		break;
	}


	//do setup that is required for all packets
	local_config(&(eDevice->ecom_pDevice), &(eDevice->ecom_l_ptr), packet);

	//set app val
	(eDevice->ecom_pDevice)->LastAppVal = hap_simple->hap_AppVal;

	//send 2 blank packets in acknowledgement
	WORD DataOffset;	
	BYTE *Buffer;
	Buffer = malloc(600);
        memset(Buffer, 0, 600);
	WORD Total;

	DataOffset = (eDevice->ecom_pDevice)->DataOffset;

	Buffer[DataOffset] = 0; 	
	Total = DataOffset+1;		
	Buffer[DataOffset+1] = 0; 
	Total++; 

	BYTE RetBuffer[600];
	int ResponseSize = sizeof(RetBuffer);
	int Error;	
	Error = Send_HAP_Packet((eDevice->ecom_pDevice), Buffer, Total, RetBuffer, &ResponseSize, TRUE, FALSE, eDevice->ecom_l_ptr);
	usleep(50000);
	Error = Send_HAP_Packet(eDevice->ecom_pDevice, Buffer, Total, RetBuffer, &ResponseSize, TRUE, FALSE, eDevice->ecom_l_ptr);
	return 0 ;
}



void swap_ip_struct(IPSetup* current)
{	
	IPSetup* temp;
	temp = malloc(256);
	

	memcpy(temp, current, 256);

	memcpy(&(current->Gateway), &(current->BackupGateway), 4);
	memcpy(&(current->IPAddress), &(current->BackupIPAddress), 4);
	memcpy(&(current->Subnet), &(current->BackupSubnet), 4);

	memcpy(&(current->BackupGateway), &(temp->Gateway), 4);
	memcpy(&(current->BackupIPAddress), &(temp->IPAddress), 4);
	memcpy(&(current->BackupSubnet), &(temp->Subnet), 4);
	
	free(temp);

	printf("swapped!\n");
}
	
int respond_read_device_info(struct hap_simple* hap_simple, const u_char* packet, struct ecomDevice *eDevice)
{
	#if DEBUG
	printf("*~*~ libpcap_filter.c - respond_read_device_info\n");
	#endif	

	//do setup that is required for all packets
	local_config(&(eDevice->ecom_pDevice), &(eDevice->ecom_l_ptr), packet);

	//set app val
	(eDevice->ecom_pDevice)->LastAppVal = hap_simple->hap_AppVal;

	//send response
	send_read_device_info(&(eDevice->ecom_pDevice), &(eDevice->ecom_l_ptr));

	return 0 ;
}

int respond_read_version_info(struct hap_simple* hap_simple, const u_char* packet, struct ecomDevice *eDevice)
{
	#if DEBUG
	printf("*~*~ libpcap_filter.c - respond_read_version_info\n");
	#endif	

	//do setup that is required for all packets
	local_config(&(eDevice->ecom_pDevice), &(eDevice->ecom_l_ptr), packet);

	//set app val
	(eDevice->ecom_pDevice)->LastAppVal = hap_simple->hap_AppVal;

	//send response
	send_read_version_info(&(eDevice->ecom_pDevice), &(eDevice->ecom_l_ptr));

	return 0 ;
}

int respond_read_estats(struct hap_simple* hap_simple, const u_char* packet, struct ecomDevice *eDevice)
{
	#if DEBUG
	printf("*~*~ libpcap_filter.c - respond_read_version_info\n");
	#endif	

	//do setup that is required for all packets
	local_config(&(eDevice->ecom_pDevice), &(eDevice->ecom_l_ptr), packet);

	//set app val
	(eDevice->ecom_pDevice)->LastAppVal = hap_simple->hap_AppVal;

	//if 2nd byte is 1, query is a reset command, so reset stats
	if(hap_simple->hap_data_buff[1]==1){
	ini_putl("", "estats_missed_frame",0, CONFIG_FILE);
	ini_putl("", "estats_transmit_coll",0, CONFIG_FILE);
	ini_putl("", "estats_discard_pkt",0, CONFIG_FILE);
	ini_putl("", "estats_bad_crc",0, CONFIG_FILE);
	ini_putl("", "estats_unknown_type",0, CONFIG_FILE);
	ini_putl("", "estats_send_err",0, CONFIG_FILE);
	}

	//(eDevice->ecom_pDevice)->setupType = 256*hap_simple->hap_data_buff[2]+hap_simple->hap_data_buff[1];

	//send response
	send_read_estats(&(eDevice->ecom_pDevice), &(eDevice->ecom_l_ptr));

	return 0 ;
}


struct my_hap  *handle_HAP(u_char *args,const struct pcap_pkthdr* pkthdr,const u_char* packet)
{
	#if DEBUG
	printf("*~*~ libpcap_filter.c - handle_HAP\n");
	#endif	
     struct my_hap* hap;
    u_int length = pkthdr->len;
    u_int16_t data_len = 0, hdr_len=0, hap_len=0, len_tol=0, AppVal = 0, crc = 0;


    /* jump pass other headers */
    hap = (struct my_hap*)(packet + sizeof(struct ether_header) + sizeof(struct my_ip)+sizeof(struct my_udp));
    length -= (sizeof(struct ether_header)+ sizeof(struct my_ip)+ sizeof(struct my_udp)); 
    /* check to see we have a packet of valid length */

    //deal with Endianess
    data_len = 256*hap->hap_length2+hap->hap_length1;
    hdr_len = 9;
    //printf("data_len %d\n", data_len);
    hap_len = hdr_len + data_len;


    len_tol = (sizeof(struct ether_header)+ sizeof(struct my_ip)+ sizeof(struct my_udp)+hap_len);
    if (length < hap_len)
    {
	#if DEBUG
        printf("truncated hap %d",length);
	#endif
        return hap;
    }


    AppVal = 256*hap->hap_AppVal2+hap->hap_AppVal1;
    crc = 256*hap->hap_crc2+hap->hap_crc1;


    #if MEM_PRINT
    mem_print("raw packet", packet, len_tol);
    #endif

    #if DEBUG
    // see if we have as much packet as we should 
   if(length < data_len)
        printf("\ntruncated HAP - %d bytes missing\n",data_len - length);
    #endif

    #if MINIMAL_PRINT
    printf("  HAP: ");
    printf("HAP(%x%x%x)", hap->hap_H, hap->hap_A, hap->hap_P);
    printf("AppVal(%004x) HAP_crc(%004x) length(%004x) Fun_Code(%004x)", AppVal, crc, data_len, hap->hap_Fun_Code);

    #endif

    return hap;
}


u_int16_t handle_UDP(u_char *args,const struct pcap_pkthdr* pkthdr,const u_char* packet)
{
	#if DEBUG
	printf("*~*~ libpcap_filter.c - handle_UDP\n");
	#endif	
    const struct my_udp* udp;
    u_int length = pkthdr->len;
    u_int16_t src_port, dst_port;

    int len, crc;

    /* jump pass the ethernet header */
    udp = (struct my_udp*)(packet + sizeof(struct ether_header) + sizeof(struct my_ip));
    length -= (sizeof(struct ether_header)+ sizeof(struct my_ip)); 

    /* check to see we have a packet of valid length */
    if (length < sizeof(struct my_udp))
    {
	#if DEBUG
        printf("truncated udp %d",length);
	#endif
        return -1;
    }

    len      = ntohs(udp->udp_len);
    src_port = udp->udp_srcprt;
    dst_port = udp->udp_dstprt;
    dst_port = htons(dst_port);
    crc      = udp->udp_crc;

    /* see if we have as much packet as we should */

    if(length < len)
	    printf("\ntruncated UDP - %d bytes missing\n",len - length);

    #if MINIMAL_PRINT
    printf(" UDP: ");
    printf("src_port(%ld) ", src_port);
    printf("dst_port(%ld) seg_len(%d) udp_CRC(%x)\n", dst_port, len,crc);
    #endif

/* Done by iptables
    #if SYSLOG
    syslog (LOG_INFO, "ID(%d) hap: UDP HDDR: SPORT(%ld) DPORT(%ld)", HPID, src_port, dst_port);
    #endif
*/


    return dst_port;
}




u_int16_t handle_IP(u_char *args,const struct pcap_pkthdr* pkthdr,const u_char* packet)
{
    const struct my_ip* ip;
    u_int length = pkthdr->len;
    u_int hlen,off,version, proto;
    int len, crc;
    char ip_dst[18];
    memset(ip_dst,0,18);
    char ip_src[18];
    memset(ip_src,0,18);
    /* jump pass the ethernet header */
    ip = (struct my_ip*)(packet + sizeof(struct ether_header));
    length -= sizeof(struct ether_header); 
    /* check to see we have a packet of valid length */
    if (length < sizeof(struct my_ip))
    {
        printf("truncated ip %d",length);
        return -1;
    }
    len     = ntohs(ip->ip_len);
    hlen    = IP_HL(ip); /* header length */
    version = IP_V(ip);/* ip version */
    proto   = ip->ip_p;	/* protocol of encapsulated segment */
    crc     = ip->ip_sum; /* IP checksum */
    /* check version */
    if(version != 4)
    {
      fprintf(stdout,"Unknown version %d\n",version);
      return -1;
    }
    /* check header length */
    if(hlen < 5 )
        fprintf(stdout,"bad-hlen %d \n",hlen);
    /* see if we have as much packet as we should */
    if(length < len)
        printf("\ntruncated IP - %d bytes missing\n",len - length);
    /* Check to see if we have the first fragment */
    off = ntohs(ip->ip_off);

    if((off & 0x1fff) == 0 )/* aka no 1's in first 13 bits */
    {/* print SOURCE DESTINATION hlen version len offset */
    #if MINIMAL_PRINT
        fprintf(stdout,"IP: ");
	sprintf(ip_src, inet_ntoa(ip->ip_src));
	sprintf(ip_dst,inet_ntoa(ip->ip_dst));
        fprintf(stdout,"src(%s) dst(%s) hlen(%d) version(%d) segment len(%d) offset(%d) IP_crc(%x) encaps_proto(%d)\n", ip_src, ip_dst, hlen,version,len,off, crc, proto);
    #endif

/* Done by iptables
    #if SYSLOG
    //syslog (LOG_INFO, "ID(%d) hap: IP HDDR: SRC(%s) DST(%s) HDDRLEN(%d) VERS(%d) SEGLEN(%d) OFF(%d) IPCRC(%x) PROTO(%d)", HPID, ip_src, ip_dst, hlen,version,len,off, crc, proto);
    syslog (LOG_INFO, "ID(%d) hap: IP HDDR: SRC(%s) DST(%s)", HPID, ip_src, ip_dst);
    #endif
*/

    }

    return proto;
}

/* handle ethernet packets, much of this code gleaned from
 * print-ether.c from tcpdump source
 */


u_int16_t handle_ethernet (u_char *args,const struct pcap_pkthdr* pkthdr,const u_char*  packet)
{
	#if DEBUG
	printf("*~*~ libpcap_filter.c - handle_ethernet\n");
	#endif	
    u_int caplen = pkthdr->caplen;
    struct ether_header *eptr;  /* net/ethernet.h */
    u_short ether_type;
    char eth_dst[18];
    memset(eth_dst,0,18);
    char eth_src[18];
    memset(eth_src,0,18);

    if (caplen < ETHER_HDRLEN)
    {
        printf("Packet length less than ethernet header length\n");
        return -1;
    }

    /* lets start with the ether header... */
    eptr = (struct ether_header *) packet;
    ether_type = ntohs(eptr->ether_type);

    #if MINIMAL_PRINT
    /* Lets print SOURCE DEST TYPE LENGTH */
    printf("ETH: ");
    sprintf(eth_src, ether_ntoa((struct ether_addr*)eptr->ether_shost));
    sprintf(eth_dst,ether_ntoa((struct ether_addr*)eptr->ether_dhost));

    printf("SRC(%s) DST(%s)",eth_src, eth_dst);

    /* check to see if we have an ip packet */
    if (ether_type == ETHERTYPE_IP)
    {
        printf("(IP)");
    }else  if (ether_type == ETHERTYPE_ARP)
    {
        printf("(ARP)");
    }else  if (eptr->ether_type == ETHERTYPE_REVARP)
    {
        printf("(RARP)");
    }else {
        printf("(?)");
    }
    printf(" LEN(%d)\n",length);
    #endif
/* Done by iptables
    #if SYSLOG
    syslog (LOG_INFO, "ID(%d) hap: ETH HDDR: SRC(%s) DST(%s)", HPID, eth_src, eth_dst);
    #endif
*/
    return ether_type;
}


int main(int argc,char **argv)
{ 
	#if DEBUG
	printf("*~*~ libpcap_filter.c - main\n");
	#endif	

    char *dev; 
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t* descr;
    struct bpf_program fp;      /* hold compiled program     */
    bpf_u_int32 maskp;          /* subnet mask               */
    bpf_u_int32 netp;           /* ip                        */

    struct ecomDevice *eDevice;
    eDevice = malloc(sizeof(struct ecomDevice));
    memset( eDevice, 0, sizeof(struct ecomDevice));
    eDevice->ecom_pDevice = malloc(sizeof(HEIDevice));
    eDevice->ecom_l_ptr = malloc(sizeof(libnet_t));

#if SYSLOG
    //openlog ("HP:", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL0);
    openlog ("HP", LOG_CONS, LOG_LOCAL0);
#endif	

    configure(&(eDevice->ecom_pDevice), &(eDevice->ecom_l_ptr));

    u_char* args = (u_char*) eDevice;

    /* Options must be passed in as a string because I am lazy */
    if(argc < 2){ 
        fprintf(stdout,"Usage: %s numpackets \"options\"\n",argv[0]);
        return 0;
    }

/* grab a device to peek into... */
    dev = pcap_lookupdev(errbuf);
    dev = INET_INTERFACE;
    if(dev == NULL)
    { printf("%s\n",errbuf); exit(1); }

    /* ask pcap for the network address and mask of the device */
    pcap_lookupnet(dev,&netp,&maskp,errbuf);

    /* open device for reading. NOTE: defaulting to
     * promiscuous mode*/
    descr = pcap_open_live(dev,BUFSIZ,1,-1,errbuf);
    if(descr == NULL)
    { printf("pcap_open_live(): %s\n",errbuf); exit(1); }


    if(argc > 2)
    {
        /* Lets try and compile the program.. non-optimized */
        if(pcap_compile(descr,&fp,argv[2],0,netp) == -1)
        { fprintf(stderr,"Error calling pcap_compile\n"); exit(1); }

        /* set the compiled program as the filter */
        if(pcap_setfilter(descr,&fp) == -1)
        { fprintf(stderr,"Error setting filter\n"); exit(1); }
    }


//open socket on 28784, although we never actually use this because we build our own raw packets.  Simply starts a listener on HAP port.
    struct sockaddr_in si_me;
    int s;

    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
      printf("opening socket on 28784\n");

    memset((char *) &si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(28784);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(s, &si_me, sizeof(si_me))==-1)
      printf("error binding to 28784\n");

//do our modbus.pyx process a favor and listen on TCP port 502 since, for some reason, python has trouble binding to ports try netstat -nutl to see for yourself...
    struct sockaddr_in s1i_me;
    int s1;

    if ((s1=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))==-1)
      printf("error opening socket on 502\n");

    memset((char *) &s1i_me, 0, sizeof(s1i_me));
    s1i_me.sin_family = AF_INET;
    s1i_me.sin_port = htons(502);
    s1i_me.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(s1, &s1i_me, sizeof(s1i_me))==-1)
      printf("error binding to 502\n");
    listen(s1,3);
    
    
    /* ... and loop */ 
    printf("+++++Industrial Protocol Emulator Started.  Listening on Port 28784.\n");
    #if SYSLOG
    syslog (LOG_INFO, "ID(%d) hap: Industrial Protocol Emulator Started.  Listening on Port 28784)", HPID);
    #endif

    pcap_loop(descr,atoi(argv[1]),my_callback,args);

    fprintf(stdout,"\nfinished\n");
    close(s);

    libnet_destroy(eDevice->ecom_l_ptr);
    free(eDevice->ecom_pDevice);
    free(eDevice->ecom_l_ptr);

    free(eDevice);
    return 0;

}


