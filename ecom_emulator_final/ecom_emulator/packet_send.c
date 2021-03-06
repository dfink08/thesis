/**********************************************************************
*
* Compile with:
* gcc packet_send.c -lnet -o packetsend.o
* !!! to make object file for linking: gcc -c packet_send.c -lpcap -o packetsend.o 
*
*
* Usage:
* ./sendlibnet
*
**********************************************************************/
#include <libnet.h>
#include "libnet_send.h"	//all other #includes are in this file
#include "HAP_emulator.h"
#include "minIni.h"
#include <syslog.h>
#include <sys/time.h>	//added for time delay


void get_gumstix_ip(HEIDevice **pDevice);

void stoh(char* str, unsigned char* dst_hex, int strlen)
{
	#if DEBUG
	printf("*~*~ libnet_send.c - stoh\n");
	#endif
	char str1[512];
	int uChar = 0;
	int x;
	char *p1 = str1, *p2 = str; 

	//remove spaces
	do 
		while(*p2==' ')
			p2++;
	while ((*p1++ = *p2++));

	
	//convert
	for (x = 0; x < strlen; x++)
	{
		//mem_print("dst_hex", dst_hex, strlen);
		sscanf(&str1[2*x], "%02x", &uChar);
		dst_hex[x] = (unsigned char) uChar;

	}
}


static BYTE hextab[512] = "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f202122232425262728292a2b2c2d2e2f303132333435363738393a3b3c3d3e3f404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f606162636465666768696a6b6c6d6e6f707172737475767778797a7b7c7d7e7f808182838485868788898a8b8c8d8e8f909192939495969798999a9b9c9d9e9fa0a1a2a3a4a5a6a7a8a9aaabacadaeafb0b1b2b3b4b5b6b7b8b9babbbcbdbebfc0c1c2c3c4c5c6c7c8c9cacbcccdcecfd0d1d2d3d4d5d6d7d8d9dadbdcdddedfe0e1e2e3e4e5e6e7e8e9eaebecedeeeff0f1f2f3f4f5f6f7f8f9fafbfcfdfeff";



void hex_encode(char* output, char* input, int input_len)
{
	#if DEBUG
	printf("*~*~ libnet_send.c - hex_encode\n");
	#endif
	
	u_char* u_output =(u_char*)output;
	u_char* u_input =(u_char*)input;
	
	u_int16_t i;
	for(i = 0; i<input_len; i++)
	{	
		u_output[i*2] = hextab[u_input[i]*2];
		u_output[(i*2)+1] = hextab[(u_input[i]*2)+1];
		//printf("%c%c", hextab[input[i]*2], hextab[(input[i]*2)+1]);
	}
	//printf("\n");
}


void get_gumstix_ip(HEIDevice **pDevice)
{
  
	int fd;
	struct ifreq ifr;
	unsigned long unit_ip_hex;
	struct in_addr sa;
	char* unit_ip_str;
	unit_ip_str = malloc(20);
	int n;
  
	//check what ip addresses are and store those addresses into the struct and save them to the config file
	fd = socket(AF_INET, SOCK_DGRAM, 0);

	/* I want to get an IPv4 IP address */
	ifr.ifr_addr.sa_family = AF_INET;

	/* I want IP address attached to "eth0" */
	strncpy(ifr.ifr_name, INET_INTERFACE, IFNAMSIZ-1);

	ioctl(fd, SIOCGIFADDR, &ifr);

	
	memcpy(&(((HEIDevice*)*pDevice)->IPinfo.IPAddress), (char*)&(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), 4);
	sa.s_addr = *((u_int32_t*)(((HEIDevice*)*pDevice)->IPinfo.IPAddress));
	inet_ntop(AF_INET, &(sa), unit_ip_str, INET_ADDRSTRLEN);
	n = ini_puts("", "global_ip",unit_ip_str, CONFIG_FILE);

	ioctl(fd, SIOCGIFNETMASK, &ifr);                      
	memcpy(&(((HEIDevice*)*pDevice)->IPinfo.Subnet), (char*)&(((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr), 4);
	sa.s_addr = *((u_int32_t*)(((HEIDevice*)*pDevice)->IPinfo.Subnet));
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
		iface = strndup(line + 73, 4);

		// Find line with the gateway
		if(strcmp("0.0.0.0        ", destination) == 0 && strcmp(iface, INET_INTERFACE) == 0) 
			gateway = strndup(line + 16, 15);	// Extract gateway

	}

	
	if (gateway != NULL)
	{
	unit_ip_hex = inet_network(gateway);
	//endian swap ip address
	unit_ip_hex = htonl(unit_ip_hex);
	((HEIDevice*)*pDevice)->gateway=unit_ip_hex;	
	
		 
	memcpy(&(((HEIDevice*)*pDevice)->IPinfo.Gateway), (char*) &unit_ip_hex, 4);
	n = ini_puts("", "global_gateway",gateway, CONFIG_FILE);
	}
	
	else
	{
	 // printf("Error: Invalid Internet Gateway.  Try running dhclient on listening interface.\n");
char str[256];
	char command[30];
	char * temp = (char*) &unit_ip_hex;
	//if no gateway set, then set the gateway using the config file value
	n = ini_gets("", "global_gateway", "dummy", str, sizearray(str), CONFIG_FILE);
	unit_ip_hex = inet_network(str);
   	
	//endian swap ip address
	unit_ip_hex = htonl(unit_ip_hex);
	((HEIDevice*)*pDevice)->gateway=unit_ip_hex;	
	memcpy(&(((HEIDevice*)*pDevice)->IPinfo.Gateway), temp, 4);
	memcpy(&(((HEIDevice*)*pDevice)->IPinfo.BackupGateway), temp, 4);
	
	  
	//set default gateway
	sa.s_addr = *((u_int32_t*)(((HEIDevice*)*pDevice)->IPinfo.Gateway));
	inet_ntop(AF_INET, &(sa), unit_ip_str, INET_ADDRSTRLEN);
	
	sprintf(command, "sudo route add default gw %s %s",unit_ip_str, INET_INTERFACE);
   	printf("change gateway using command: %s\n", command);    
    	system(command);	  
	}
	
	pclose(fp);
	close(fd);
	
}




int configure(HEIDevice **pDevice, libnet_t **l) {

	#if DEBUG
	printf("*~*~ libnet_send.c - configure\n");
	#endif

        char errbuf[LIBNET_ERRBUF_SIZE];
        unsigned long unit_ip_hex;
        u_int16_t id;	
	char * temp = (char*) &unit_ip_hex;
	long n;        
  	//unsigned char* hexString;

  	char str[256];
  	unsigned char mac_hex[18];
        memset(str,'\00',sizeof(str));


	//get the IP addresses that are currently used on the gumstix interface, store them to the file, and fill the struct
	get_gumstix_ip(pDevice);

	
	memset(&(((HEIDevice*)*pDevice)->NodeName),'\00',256);
	memset(&(((HEIDevice*)*pDevice)->NodeDescription),'\00',256);

	//unit mac
	n = ini_gets("", "global_mac", "dummy", str, sizearray(str), CONFIG_FILE);
	//convert ascii mac to hex representation
	stoh(str, mac_hex,6);
	memcpy(&(((HEIDevice*)*pDevice)->ENetAddress), &mac_hex, 6);
			

	//unit ip
	n = ini_gets("", "global_ip", "dummy", str, sizearray(str), CONFIG_FILE);
	unit_ip_hex = inet_network(str);
   	 //endian swap ip address
	unit_ip_hex = htonl(unit_ip_hex);
	((HEIDevice*)*pDevice)->Address.AddressIP.AddressingType.lAddr=unit_ip_hex;	//unit ip address
	memcpy(&(((HEIDevice*)*pDevice)->IPinfo.IPAddress), temp, 4);
	memcpy(&(((HEIDevice*)*pDevice)->IPinfo.BackupIPAddress), temp, 4);
	
	//unit subnet mask
	n = ini_gets("", "global_subnet", "dummy", str, sizearray(str), CONFIG_FILE);
	unit_ip_hex = inet_network(str);
   	 //endian swap ip address
	unit_ip_hex = htonl(unit_ip_hex);
	((HEIDevice*)*pDevice)->subnetMask=unit_ip_hex;	//unit ip address
	memcpy(&(((HEIDevice*)*pDevice)->IPinfo.Subnet), temp, 4);
	memcpy(&(((HEIDevice*)*pDevice)->IPinfo.BackupSubnet), temp, 4);

	//unit gateway 
	n = ini_gets("", "global_gateway", "dummy", str, sizearray(str), CONFIG_FILE);
	unit_ip_hex = inet_network(str);
   	 //endian swap ip address
	unit_ip_hex = htonl(unit_ip_hex);
	((HEIDevice*)*pDevice)->gateway=unit_ip_hex;	
	memcpy(&(((HEIDevice*)*pDevice)->IPinfo.Gateway), temp, 4);
	memcpy(&(((HEIDevice*)*pDevice)->IPinfo.BackupGateway), temp, 4);

	//number
	((HEIDevice*)*pDevice)->NodeNumber= (WORD)ini_getl("", "setid_n",-1, CONFIG_FILE);

	//name
        memset(str,'\0',sizeof(str)); //clear str
	n = ini_gets("", "setname_n", "dummy", str, sizearray(str), CONFIG_FILE);
	#if DATA_PRINT
	printf("setname_n: %s\n",str);
	#endif
	memcpy(&(((HEIDevice*)*pDevice)->NodeName), &str, 256);

	//description
	memset(str,'\00',256);  //clear str
	n = ini_gets("", "setdesc_n", "dummy", str, sizearray(str), CONFIG_FILE);
	memcpy(&(((HEIDevice*)*pDevice)->NodeDescription), &str, 256);

	//ethernet device info
	((HEIDevice*)*pDevice)->HType= (BYTE)ini_getl("", "global_HType",-1, CONFIG_FILE);	
	((HEIDevice*)*pDevice)->EType= (BYTE)ini_getl("", "global_EType",-1, CONFIG_FILE);
	((HEIDevice*)*pDevice)->FType= (BYTE)ini_getl("", "global_FType",-1, CONFIG_FILE);

	//module type name
        memset(str,'\0',sizeof(str)); //clear str
	n = ini_gets("", "global_type_n", "dummy", str, sizearray(str), CONFIG_FILE);
	memcpy(&(((HEIDevice*)*pDevice)->TypeName), &str, 29);

	//dhcp yes/no
	if ((WORD)ini_getl("", "setip_dhcp",0, CONFIG_FILE))
		((HEIDevice*)*pDevice)->IPinfo.Flags= 4;	//magic number that means we are using dhcp
	else	
		((HEIDevice*)*pDevice)->IPinfo.Flags= 0;	//magic number that means we are using dhcp

	//version info
	memset(str,'\0',sizeof(str)); //clear str
	((HEIDevice*)*pDevice)->verInfo.SizeofVersionInfo = 50;
	((HEIDevice*)*pDevice)->verInfo.BootVersion.MajMinVersion= (WORD)ini_getl("", "global_ver_boot",-1, CONFIG_FILE);
	((HEIDevice*)*pDevice)->verInfo.BootVersion.BuildVersion= (WORD)ini_getl("", "global_build_boot",-1, CONFIG_FILE);

	((HEIDevice*)*pDevice)->verInfo.OSVersion.MajMinVersion= (WORD)ini_getl("", "global_ver_firmware",-1, CONFIG_FILE);
	((HEIDevice*)*pDevice)->verInfo.OSVersion.BuildVersion= (WORD)ini_getl("", "global_build_firmware",-1, CONFIG_FILE);
	((HEIDevice*)*pDevice)->verInfo.NumOSExtensions = 0;
	for(n=0; n<10; n++)
		{((HEIDevice*)*pDevice)->verInfo.OSExt[n].MajMinVersion=0xffff;
		((HEIDevice*)*pDevice)->verInfo.OSExt[n].BuildVersion=0x00ff;}

	//ethernet stats (displayed on main screen of netedit)
	((HEIDevice*)*pDevice)->eStats.SizeofEthernetStats = 0x1a;
	((HEIDevice*)*pDevice)->eStats.MissedFrameCount = ini_getl("", "estats_missed_frame",-1, CONFIG_FILE);
	((HEIDevice*)*pDevice)->eStats.TransmitCollisionCount = ini_getl("", "estats_transmit_coll",-1, CONFIG_FILE);
	((HEIDevice*)*pDevice)->eStats.DiscardedPackets = ini_getl("", "estats_discard_pkt",-1, CONFIG_FILE);
	((HEIDevice*)*pDevice)->eStats.BadCRCCount = ini_getl("", "estats_bad_crc",-1, CONFIG_FILE);
	((HEIDevice*)*pDevice)->eStats.UnknownTypeCount = ini_getl("", "estats_unknown_type",-1, CONFIG_FILE);
	((HEIDevice*)*pDevice)->eStats.SendErrorCount = ini_getl("", "estats_send_err",-1, CONFIG_FILE);

	//populate the advanced settings files in netedit
	((HEIDevice*)*pDevice)->settings_adv.SizeofSettings = sizeof(HEISettings);
	((HEIDevice*)*pDevice)->settings_adv.Flags = ini_getl("", "settings_flags",10, CONFIG_FILE);
	((HEIDevice*)*pDevice)->settings_adv.RXWXACKTimeout = (WORD)ini_getl("", "settings_RXWXACKTimeout",250, CONFIG_FILE);
	((HEIDevice*)*pDevice)->settings_adv.RXWXResponseTimeout = (WORD)ini_getl("", "settings_RXWXResponseTimeout",1, CONFIG_FILE);
	((HEIDevice*)*pDevice)->settings_adv.RXWXMaxRetrys = (WORD)ini_getl("", "settings_RXWXMaxRetrys",10, CONFIG_FILE);
	((HEIDevice*)*pDevice)->settings_adv.KSeqMaxRetrys = (WORD)ini_getl("", "settings_KSeqMaxRetrys",10, CONFIG_FILE);
	((HEIDevice*)*pDevice)->settings_adv.ModbusMasterTimeout = (WORD)ini_getl("", "settings_ModbusMasterTimeout",1000, CONFIG_FILE);
	((HEIDevice*)*pDevice)->settings_adv.ModbusSlaveTimeout = (WORD)ini_getl("", "settings_ModbusSlaveTimeout",200000, CONFIG_FILE);	

   	  //check to see if web configuration is readonly, and set flag appropriately
        memset(str,'\0',sizeof(str)); //clear str
	n = ini_gets("", "advanced_h", "dummy", str, sizearray(str), CONFIG_FILE);
	//mem_print("readonly config", str, 15);
	if(strcmp(str,"checked"))
		((HEIDevice*)*pDevice)->settings_adv.Flags = ((((HEIDevice*)*pDevice)->settings_adv.Flags)| 0x2);  //website not readonly
	else
		((HEIDevice*)*pDevice)->settings_adv.Flags = ((((HEIDevice*)*pDevice)->settings_adv.Flags)| 0x2);  //website is readonly

	//Status of PLC, 1=locked 0=unlocked.  Correct password changes this from 1 to 0
	((HEIDevice*)*pDevice)->PLCLocked = (BYTE)ini_getl("", "global_locked",-1, CONFIG_FILE);

	#if DEBUG
	printf("PLC lock status CONFIG %d\n", ((HEIDevice*)*pDevice)->PLCLocked);
	#endif
	
	//get the password
    memset(str,'\0',sizeof(str)); //clear str
	n = ini_gets("", "global_password", "Bad_Value", str, sizearray(str), CONFIG_FILE);

	#if DEBUG	
	printf("Password: %s\n",str);
	#endif
	
	memcpy(&(((HEIDevice*)*pDevice)->Password), &str, 8);
	#if DEBUG
	mem_print("PLC password CONFIG",&(((HEIDevice*)*pDevice)->Password),8);
	#endif	
	
	
	//valid:
	((HEIDevice*)*pDevice)->Address.AddressIP.Port	=0x7070;
	((HEIDevice*)*pDevice)->DataOffset		= PACKET_HEADER_SIZE;		//9 bytes
	((HEIDevice*)*pDevice)->UseAddressedBroadcast  = 0;				//zero for initial query


	//generate libnet context
        *l = libnet_init(LIBNET_RAW4, INET_INTERFACE, errbuf);

        if ( *l == NULL ) 
	{
		fprintf(stderr, "libnet_init() failed: %s\n", errbuf);
                exit(EXIT_FAILURE);
        }

        /* Generating a random id */				


        libnet_seed_prand (*l);
        id = (u_int16_t)libnet_get_prand(LIBNET_PR16);
	
	
	#if DEBUG	
	printf("Configure CCM test\n");
	ini_gets("CCM_READ","1e010f0036" ,"path_to_ccmdictionary.txt_broken!", str, 512, CCM_FILE);
	printf("CCM Response: %s\n", str);

	printf("Configure KSEQ test\n");
	ini_gets("KSEQ_READ","0001510140040100801795" ,"path_to_kseqdictionary.txt_broken!", str, 512, KSEQ_FILE);
	printf("KSEQ Response: %s\n", str);
	#endif
	return 1;

}
	

int send_read_setup_data(HEIDevice **pDevice, libnet_t **l)
{//Response definition for query devices

	#if DEBUG
	printf("*~*~ libnet_send.c - send_read_setup_data\n");
	#endif 	

	//mystery info that contains the version of PLC, could be dissected
	BYTE version_info[128] = "\x01\x00\x00\x00\x02\x00\x02\x00\x01\x00\x01\x00\x00\x00\x84\x27\x40\x00\x14\xfd\x02\x00\x2a\xfd\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";

	WORD DataOffset;	
	//BYTE Buffer[600];
	BYTE *Buffer;
	Buffer = malloc(600);
	memset(Buffer, 0, 600);
	WORD Total;

	DataOffset = ((HEIDevice*)*pDevice)->DataOffset;

	Buffer[DataOffset] = FUN_POLLING; 	/* This is the function code! */
	Total = DataOffset+1;			//9 bytes plus function code = 10 bytes
	
	/// fill in the rest of the payload here ///
	
	Buffer[DataOffset+1] = FALSE; 		//not sure what this byte is, but it is always zero
	Total++; 

	switch(((HEIDevice*)*pDevice)->setupType)
	{

	case 0x20:
	//node number 4byte
	memcpy(&Buffer[DataOffset+2], &((HEIDevice*)*pDevice)->NodeNumber, 4);
	Total+=4;
	#if SYSLOG
	syslog (LOG_INFO, "ID(%d) hap: ~~~ NodeNumber value", HPID);
	#endif
		break;


	case 0x30:	
	//subnet mask 4b	BYTE FromAddress[20];yte
	memcpy(&Buffer[DataOffset+2], (char*) &(((HEIDevice*)*pDevice)->subnetMask), 4);
	Total+=4;
	#if SYSLOG
	syslog (LOG_INFO, "ID(%d) hap: ~~~ Subnet value", HPID);
	#endif
	
		break;

	case 0x10:
	//IP address 4byte
	memcpy(&Buffer[DataOffset+2], (char*) &(((HEIDevice*)*pDevice)->Address.AddressIP.AddressingType), 4);
	Total+=4;
	#if SYSLOG
	syslog (LOG_INFO, "ID(%d) hap: ~~~ IP value", HPID);
	#endif

		break;
	
	case 0x16:
	//node name 256byte
	memcpy(&Buffer[DataOffset+2], ((HEIDevice*)*pDevice)->NodeName, 256);
	Total+=256;
	#if SYSLOG
	syslog (LOG_INFO, "ID(%d) hap: ~~~ NodeName value", HPID);
	#endif
		break;

	case 0x33:
	//Type name 32byte
	memcpy(&Buffer[DataOffset+2], ((HEIDevice*)*pDevice)->TypeName, 32);
	Total+=32;
	#if SYSLOG
	syslog (LOG_INFO, "ID(%d) hap: ~~~ TypeName value", HPID);
	#endif
		break;

	case 0x40:	
	//gateway 4byte
	memcpy(&Buffer[DataOffset+2], (char*) &(((HEIDevice*)*pDevice)->gateway), 4);
	Total+=4;
	#if SYSLOG
	syslog (LOG_INFO, "ID(%d) hap: ~~~ Gateway value", HPID);
	#endif
		break;

	case 0x26:
	//node description 256byte
	memcpy(&Buffer[DataOffset+2], ((HEIDevice*)*pDevice)->NodeDescription, 256);
	Total+=256;
	#if SYSLOG
	syslog (LOG_INFO, "ID(%d) hap: ~~~ Description value", HPID);
	#endif
		break;
	
	case 0x8035:	
	//dont know exactly what this is!
	memcpy(&Buffer[DataOffset+2], version_info, 128);
	Total+=128;
	#if SYSLOG
	syslog (LOG_INFO, "ID(%d) hap: ~~~ VersInvo value", HPID);
	#endif

		break;
	case 0x36:	
	//IPinfo 4byte
	memcpy(&Buffer[DataOffset+2], (char*) &(((HEIDevice*)*pDevice)->IPinfo), 256);
	Total+=256;
	#if SYSLOG
	syslog (LOG_INFO, "ID(%d) hap: ~~~ IP_Info value", HPID);
	#endif
		break;

	case 0x0015:	
	//settings
	memcpy(&Buffer[DataOffset+2], (char*) &(((HEIDevice*)*pDevice)->settings_adv), 128);
	Total+=128;
	#if SYSLOG
	syslog (LOG_INFO, "ID(%d) hap: ~~~ ECOM_settings value", HPID);
	#endif
		break;

	default:
	#if MINIMAL_PRINT
	printf("setup type not known\n");
	#endif
	#if SYSLOG
	syslog (LOG_INFO, "ID(%d) hap: ~~~ Unknown datatype request", HPID);
	#endif
		break;
	}

	BYTE RetBuffer[600];
	int ResponseSize = sizeof(RetBuffer);
	int Error;

	
	//pDevice- pointer to device description
	//Buffer- Bytes including header to be sent to HMI
	//Total- size of HAP packet including header and data
	//RetBuffer- not used
	//&responseSize- size of RetBuffer, also not used
	//(Bool literal 1)- care about timeout, not used
	//(Bool literal 2)- care about returns, not used
	//*l- libnet context


	Error = Send_HAP_Packet(*pDevice, Buffer, Total, RetBuffer, &ResponseSize, TRUE, FALSE, *l);

	free(Buffer);
	return 0;
}


int send_read_version_info(HEIDevice **pDevice, libnet_t **l)
{//Response definition for query devices

	#if DEBUG
	printf("*~*~ libnet_send.c - send_read_version_info\n");
	#endif 	

	WORD DataOffset;	
	BYTE Buffer[600];
	WORD Total;

	DataOffset = ((HEIDevice*)*pDevice)->DataOffset;

	memcpy(&Buffer[DataOffset], &((HEIDevice*)*pDevice)->verInfo, ((HEIDevice*)*pDevice)->verInfo.SizeofVersionInfo);
	Total = DataOffset+((HEIDevice*)*pDevice)->verInfo.SizeofVersionInfo;
	
	BYTE RetBuffer[600];
	int ResponseSize = sizeof(RetBuffer);
	int Error;
	
	//pDevice- pointer to device description
	//Buffer- Bytes including header to be sent to HMI
	//Total- size of HAP packet including header and data
	//RetBuffer- not used
	//&responseSize- size of RetBuffer, also not used
	//(Bool literal 1)- care about timeout?, not used
	//(Bool literal 2)- care about returns?, not used
	//*l- libnet contextP4


	//mem_print("(read device info) data to send", Buffer, Total);

	Error = Send_HAP_Packet(*pDevice, Buffer, Total, RetBuffer, &ResponseSize, TRUE, FALSE, *l);

	return 0;
}

int send_respond_ccm_request(HEIDevice **pDevice, libnet_t **l)
{//Response definition for query devices

	#if DEBUG
	printf("*~*~ libnet_send.c - send_read_version_info\n");
	#endif 	

	WORD DataOffset;	
	BYTE Buffer[600];
	WORD Total;

	BYTE RetBuffer[600];
	memset(Buffer, 0, 600);
	int ResponseSize = sizeof(RetBuffer);
	int Error;


	char str[512];
	char keyVal_hex[32];
	memset(keyVal_hex,0,32);
	char keyVal_str[64];
	memset(keyVal_str,0,64);

	DataOffset = ((HEIDevice*)*pDevice)->DataOffset;

	Total = DataOffset;


	if(((HEIDevice*)*pDevice)->ccmQuery.r_w == 0x1e)
	{//read data

		//ack
		memcpy(&Buffer[DataOffset], "\x20\x19\x00\x00\x00\x00", 6);
		Total+=6;
		Error = Send_HAP_Packet(*pDevice, Buffer, Total, RetBuffer, &ResponseSize, TRUE, FALSE, *l);	

		//reset offset
		DataOffset = ((HEIDevice*)*pDevice)->DataOffset;
		Total = DataOffset;
		memset(Buffer, 0, 600);

		memcpy(&keyVal_hex,&(((HEIDevice*)*pDevice)->ccmQuery.r_w), 5);	//ccm request is always 5 bytes	
		hex_encode(keyVal_str, keyVal_hex, 5);				//ccm request is always 5 bytes
		printf("CCM Query: %s\n", keyVal_str);	

		ini_gets("CCM_READ",keyVal_str ,"default_val", str, 512, CCM_FILE);
		printf("CCM Response: %s\n", str);

		//response
		Buffer[DataOffset] = FUN_RESPONSE; 	/* This is the function code! */
		Total = DataOffset+1;			
	
		Buffer[DataOffset+1] = FUN_CCM_REQUEST; /* response to ccm */
		Total++;

		//memcpy(&Buffer[DataOffset], &str, strlen(str));
		stoh(str, &Buffer[DataOffset+2], strlen(str));

		#if SYSLOG
		if((strncmp("1e01010133", keyVal_str, 10)==0) || (strncmp("1e01010132", keyVal_str, 10)==0)|| (strncmp("1e04010031", keyVal_str, 10)==0))
		{
		  syslog (LOG_INFO, "ID(%d) hap: ~~ Reading Status of I/O", HPID);
		}
		#endif


		Total+=strlen(str)/2;

		Error = Send_HAP_Packet(*pDevice, Buffer, Total, RetBuffer, &ResponseSize, TRUE, FALSE, *l);

	}


	if(((HEIDevice*)*pDevice)->ccmQuery.r_w == 0x20)
	{//write data  //right now this is exactly the same as read except for the "CCM_WRITE" section designator
		//ack
		memcpy(&Buffer[DataOffset], "\x20\x19\x00\x00\x00\x00", 6);
		Total+=6;
		Error = Send_HAP_Packet(*pDevice, Buffer, Total, RetBuffer, &ResponseSize, TRUE, FALSE, *l);	

		//reset offset
		DataOffset = ((HEIDevice*)*pDevice)->DataOffset;
		Total = DataOffset;
		memset(Buffer, 0, 600);

		memcpy(&keyVal_hex,&(((HEIDevice*)*pDevice)->ccmQuery.r_w), 5+((HEIDevice*)*pDevice)->ccmQuery.length);	//ccm write may have more than 5 bytes	
		
		hex_encode(keyVal_str, keyVal_hex, 5+((HEIDevice*)*pDevice)->ccmQuery.length);	//ccm write may have more than 5 bytes
		printf("CCM Query: %s\n", keyVal_str);		

		ini_gets("CCM_WRITE",keyVal_str ,"default_val", str, 512, CCM_FILE);
		printf("CCM Response: %s\n", str);



		//response
		Buffer[DataOffset] = FUN_RESPONSE; 	/* This is the function code! */
		Total = DataOffset+1;			
	
		Buffer[DataOffset+1] = FUN_CCM_REQUEST; /* response to ccm */
		Total++;

		stoh(str, &Buffer[DataOffset+2], strlen(str));
		Total+=strlen(str)/2;

		Error = Send_HAP_Packet(*pDevice, Buffer, Total, RetBuffer, &ResponseSize, TRUE, FALSE, *l);

	}


	return 0;
}

int send_respond_kseq_request(HEIDevice **pDevice, libnet_t **l)
{//Response definition for query devices

	#if DEBUG
	printf("*~*~ libnet_send.c - send_read_version_info\n");
	#endif 	

	WORD DataOffset;	
	BYTE Buffer[600];
	WORD Total;
	
	BYTE RetBuffer[600];
	memset(Buffer, 0, 600);
	int ResponseSize = sizeof(RetBuffer);
	int Error;

	char str[1024];
	memset(str, 0, 1024);

	char keyVal_hex[64];
	memset(keyVal_hex,0,64);
	char keyVal_str[128];
	memset(keyVal_str,0,128);

	DataOffset = ((HEIDevice*)*pDevice)->DataOffset;
	Total = DataOffset;
	
	//set our default response if the querey is not understood by the honeypot, use response from 1795 since that seems to be a common response...
	char default_response[1024];
	ini_gets("KSEQ_READ","0001510140040100801795" ,"path_to_kseqdictionary_broken!", str, 512, KSEQ_FILE);
	memcpy(default_response, str, 1024);
	
	

	//send ack
	Buffer[DataOffset] = FUN_ACK; 		/* This is the function code! */
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

	//is PLC locked?
	printf("PLC lock status %d\n", ((HEIDevice*)*pDevice)->PLCLocked);
	
	if (((HEIDevice*)*pDevice)->PLCLocked==1)
	{
	  //if yes, check if they are checking the lock state
	  if(strncmp("00015001020001001752", keyVal_str, 20)==0)
	  	 {
		   //replace unlocked bits with locked magic bits
		   ini_gets("KSEQ_READ",keyVal_str ,default_response, str, 512, KSEQ_FILE);
		   str[10] = '0';
		   str[11] = '3';
		   str[14] = 'd';
		   str[15] = '1';

		 }
	  
	  //check if this is a password attempt
	else if((memcmp("0001510119020400", &keyVal_str, 16)==0) && (memcmp("17", &keyVal_str[24],2 )==0))
	  {	
		#if SYSLOG
		char pass[9];
		pass[9] = '\x00';
		memcpy(&pass,&keyVal_str[16],8);
		syslog (LOG_INFO, "ID(%d) hap: ~~Attempting Password: %s", HPID, pass);
		#endif

		mem_print("password",&((HEIDevice*)*pDevice)->Password[1], 7);
		//check if we have a valid password
		if((((HEIDevice*)*pDevice)->Password[0]=='A') &&(memcmp(&((HEIDevice*)*pDevice)->Password[1], &keyVal_str[17],7 )==0))
	  	 {
		   //if match, unlock PLC
		    #if SYSLOG
		    syslog (LOG_INFO, "ID(%d) hap: ~~~ PLC Unlocked", HPID);
		    #endif
		   (((HEIDevice*)*pDevice)->PLCLocked) = 0;
		   //send response.  The response is exactly the same as for the 1795 type packet, so just grab the response from that packet, despite correct password or not
		    ini_gets("KSEQ_READ","0001510140040100801795" ,default_response, str, 512, KSEQ_FILE);
		    printf("Kseq Response: %s\n", str);    
		 }
	  	 else
		 {
		   //send response.  The response is exactly the same as for the 1795 type packet, so just grab the response from that packet, despite correct password or not
		    ini_gets("KSEQ_READ","0001510140040100801795" ,default_response, str, 512, KSEQ_FILE);
		    printf("Kseq Response: %s\n", str);    
		 }
	  }
			   
	//these queries are ok when locked		   
	  else if((strncmp("00015001000001001750", keyVal_str, 20)==0) || (strncmp("000150010a010100175b", keyVal_str, 20)==0)|| 
	    (strncmp("000150010f000100175f", keyVal_str, 20)==0)|| (strncmp("0001510140040100801795", keyVal_str, 22)==0)|| (strncmp("0001500141041800170c", keyVal_str, 20)==0)) 
	  {		 
	      ini_gets("KSEQ_READ",keyVal_str ,default_response, str, 512, KSEQ_FILE);
	      printf("Kseq Response: %s\n", str);
	  }


	  
	}
	else
	{
	  
	  if(memcmp("0001510119020400ffffffff174f", &keyVal_str, 28)==0)  //if a lock command is sent, then lock the PLC
	  {
	    #if SYSLOG
	    syslog (LOG_INFO, "ID(%d) hap: ~~~ PLC Locked", HPID);
	    #endif
	    ((HEIDevice*)*pDevice)->PLCLocked=1;
	  }
	  
	  //get the response from the file
	  ini_gets("KSEQ_READ",keyVal_str ,default_response, str, 512, KSEQ_FILE);
	  printf("Kseq Response: %s\n", str);
	}


	//response
	Buffer[DataOffset] = FUN_RESPONSE; 	/* This is the function code! */
	Total = DataOffset+1;			
	
	Buffer[DataOffset+1] = FUN_KSEQ_REQUEST; 	/* response to kseq */
	Total++;
	Buffer[DataOffset+2] = FALSE;
	Total++;

	//memcpy(&Buffer[DataOffset+1], &str, strlen(str));
	stoh(str, &Buffer[DataOffset+3], strlen(str)/2);
	//mem_print("str of response in hex: ", &Buffer[DataOffset], strlen(str)/2);
	Total+=257;	
	Error = Send_HAP_Packet(*pDevice, Buffer, Total, RetBuffer, &ResponseSize, TRUE, FALSE, *l);

	return 0;
}


int send_read_estats(HEIDevice **pDevice, libnet_t **l)
{//Response definition for query devices

	#if DEBUG
	printf("*~*~ libnet_send.c - send_ethernet_stats\n");
	#endif 	

	WORD DataOffset;	
	BYTE Buffer[600];
	WORD Total;

	DataOffset = ((HEIDevice*)*pDevice)->DataOffset;

	memcpy(&Buffer[DataOffset], &((HEIDevice*)*pDevice)->eStats, ((HEIDevice*)*pDevice)->eStats.SizeofEthernetStats);
	Total = DataOffset+((HEIDevice*)*pDevice)->eStats.SizeofEthernetStats;

 	BYTE RetBuffer[600];
	int ResponseSize = sizeof(RetBuffer);
	int Error;

	//pDevice- pointer to device description
	//Buffer- Bytes including header to be sent to HMI
	//Total- size of HAP packet including header and data
	//RetBuffer- not used
	//&responseSize- size of RetBuffer, also not used
	//(Bool literal 1)- care about timeout, not used
	//(Bool literal 2)- care about returns, not used
	//*l- libnet contextP4


	//mem_print("(read device info) data to send", Buffer, Total);

	Error = Send_HAP_Packet(*pDevice, Buffer, Total, RetBuffer, &ResponseSize, TRUE, FALSE, *l);

	return 0;
}


int send_respond_polling_all(HEIDevice **pDevice, libnet_t **l)
{//Response definition for query devices 	
	#if DEBUG
	printf("*~*~ libnet_send.c - send_respond_polling_all\n");
	#endif
 
	WORD DataOffset;	
	u_char *Buffer;
	Buffer=malloc(600);
	WORD Total;

	
	DataOffset = ((HEIDevice*)*pDevice)->DataOffset;

	Buffer[DataOffset] = FUN_QUERY_RESPONSE;	 	/* This is the function code! */
	Total = DataOffset+1;			//9 bytes plus function code = 10 bytes
	
	/// fill in the rest of the payload here ///
	Buffer[DataOffset+1] = 0xaa;	//response to broadcast
	Total++;

	//put mac address in 
	memcpy(&Buffer[DataOffset+2], ((HEIDevice*)*pDevice)->ENetAddress, 6);
	Total+=6;

	//put 2 control bits in	
	Buffer[DataOffset+8] = FALSE;	//if IP address is set, then this bit is 1
	Buffer[DataOffset+9] = TRUE;	//this is zero
	Total+=2;

	//put in IP
	memcpy(&Buffer[DataOffset+10], (char*) &(((HEIDevice*)*pDevice)->Address.AddressIP.AddressingType), 4);
	Total+=4;

	//put in zero 
	Buffer[DataOffset+14] = FALSE;
	Total++;

	BYTE RetBuffer[600];
	int ResponseSize = sizeof(RetBuffer);
	BYTE FromAddress[20];
	int Error;


	((HEIDevice*)*pDevice)->pData = FromAddress;
	((HEIDevice*)*pDevice)->SizeOfData = sizeof(FromAddress);

	#if MEM_PRINT
	mem_print("data to send", Buffer, Total);
	#endif

	Error = Send_HAP_Packet(*pDevice, Buffer, Total, RetBuffer, &ResponseSize, TRUE, FALSE, *l);
	
	free(Buffer);
	return 0;
}

int send_read_device_info(HEIDevice **pDevice, libnet_t **l)
{//Response definition for query devices 	
	#if DEBUG
	printf("*~*~ libnet_send.c - send_read_device_info\n");
	#endif

	WORD DataOffset;	
	BYTE Buffer[600];
	WORD Total;

	DataOffset = ((HEIDevice*)*pDevice)->DataOffset;

	Buffer[DataOffset+0] = ((HEIDevice*)*pDevice)->HType; 	/* This is instead of the function code! */
	Total = DataOffset+1;			//9 bytes plus function code = 10 bytes
	
	/// fill in the rest of the payload here ///
	
	Buffer[DataOffset+1] = TRUE; 		//probably some parameter I dont know about
	Total++; 
	
	Buffer[DataOffset+2] = ((HEIDevice*)*pDevice)->EType; 		//probably some parameter I dont know about
	Total++; 	

	Buffer[DataOffset+3] = FALSE; 		//probably some parameter I dont know about
	Total++; 

	//put mac address in 
	memcpy(&Buffer[DataOffset+4], ((HEIDevice*)*pDevice)->ENetAddress, 6);
	Total+=6;

	//more params I don't know about (in little endian)
	memcpy(&Buffer[DataOffset+10], "\x40\x00\x00\x02\x00\x00\x00\x00\x00", 9);
	Total+=9;		       

	//Ftype
	Buffer[DataOffset+19] = ((HEIDevice*)*pDevice)->FType;
	Total++; 	

	//more params I don't know about (in little endian)
	memcpy(&Buffer[DataOffset+20], "\x00\x00\x00\x00\x00\x01\x01\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 22);
	Total+=22;		       
	
	BYTE RetBuffer[600];
	int ResponseSize = sizeof(RetBuffer);
	int Error;

	//pDevice- pointer to device description
	//Buffer- Bytes including header to be sent to HMI
	//Total- size of HAP packet including header and data
	//RetBuffer- not used
	//&responseSize- size of RetBuffer, also not used
	//(Bool literal 1)- care about timeout, not used
	//(Bool literal 2)- care about returns, not used
	//*l- libnet contextP4

	#if MEM_PRINT
	mem_print("data to send", Buffer, Total);
	#endif


	Error = Send_HAP_Packet(*pDevice, Buffer, Total, RetBuffer, &ResponseSize, TRUE, FALSE, *l);

	return 0;
}


int send_respond_query_setup_data(HEIDevice **pDevice, libnet_t **l)
{//Response definition for query devices 
	#if DEBUG
	printf("*~*~ libnet_send.c - send_respond_address_broadcast\n");
	#endif	

	WORD DataOffset;	
	BYTE Buffer[600];
	WORD Total;
#if DATA_PRINT
	printf("(response) app val: %x\n", ((HEIDevice*)*pDevice)->LastAppVal);	
#endif
	DataOffset = ((HEIDevice*)*pDevice)->DataOffset;

	Buffer[DataOffset] = FUN_QUERY_RESPONSE;	 	/* This is the function code! */
	Total = DataOffset+1;			//9 bytes plus function code = 10 bytes
	
	/// fill in the rest of the payload here ///
	
	Buffer[DataOffset+1] = 0xaa;	//response to broadcast
	Total++;

	//put mac address in 
	memcpy(&Buffer[DataOffset+2], ((HEIDevice*)*pDevice)->ENetAddress, 6);
	Total+=6;

	//put 2 control bits in	int send_read_device_info(HEIDevice *pDevice, libnet_t **l)
	Buffer[DataOffset+8] = 0;	//this is zero
	Buffer[DataOffset+9] = 0;	//if IP address is set, then this bit is 1
	Total+=2;

	/// fill in the rest of the payload here ///
	
//	Buffer[DataOffset+10] = FALSE; 		//not sure what this byte is, but it is always zero
//	Total++; 

	BYTE RetBuffer[600];
	int ResponseSize = sizeof(RetBuffer);
	BYTE FromAddress[20];
	int Error;
	

	((HEIDevice*)*pDevice)->pData = FromAddress;
	((HEIDevice*)*pDevice)->SizeOfData = sizeof(FromAddress);


	
	//pDevice- pointer to device description
	//Buffer- Bytes including header to be sent to HMI
	//Total- size of HAP packet including header and data
	//RetBuffer- not used
	//&responseSize- size of RetBuffer, also not used
	//(Bool literal 1)- care about timeout, not used
	//(Bool literal 2)- care about returns, not used
	//*l- libnet contextP4


#if DATA_PRINT
	int i;
	printf("\nraw buffer:::");
    	for (i=0; i<=(Total-1); i++)	//-1 to account for not starting at 1
    	{
		printf("%02x:", Buffer[i]);
	}
	printf("::end\n");    
#endif

	Error = Send_HAP_Packet(*pDevice, Buffer, Total, RetBuffer, &ResponseSize, TRUE, FALSE, *l);

	return 0;
}



int send_respond_addressed_broadcast(HEIDevice **pDevice, libnet_t **l)
{//Response definition for query devices 
	#if DEBUG
	printf("*~*~ libnet_send.c - send_respond_address_broadcast\n");
	#endif	

	WORD DataOffset;	
	BYTE Buffer[600];
	WORD Total;
#if DATA_PRINT
	printf("(response) app val: %x\n", ((HEIDevice*)*pDevice)->LastAppVal);	
#endif
	DataOffset = ((HEIDevice*)*pDevice)->DataOffset;

	Buffer[DataOffset] = FUN_QUERY_RESPONSE;	 	/* This is the function code! */
	Total = DataOffset+1;			//9 bytes plus function code = 10 bytes
	
	/// fill in the rest of the payload here ///
	
	Buffer[DataOffset+1] = 0xaa;	//response to broadcast
	Total++;

	//put mac address in 
	memcpy(&Buffer[DataOffset+2], ((HEIDevice*)*pDevice)->ENetAddress, 6);
	Total+=6;

	//put 2 control bits in	int send_read_device_info(HEIDevice *pDevice, libnet_t **l)
	Buffer[DataOffset+8] = FALSE;	//if IP address is set, then this bit is 1
	Buffer[DataOffset+9] = TRUE;	//this is zero
	Total+=2;

	//put in IP
	memcpy(&Buffer[DataOffset+10], (char*) &(((HEIDevice*)*pDevice)->Address.AddressIP.AddressingType), 4);
	Total+=4;

	//put in zero 
	Buffer[DataOffset+14] = FALSE;
	Total++;

	BYTE RetBuffer[600];
	int ResponseSize = sizeof(RetBuffer);
	BYTE FromAddress[20];
	int Error;



	((HEIDevice*)*pDevice)->pData = FromAddress;
	((HEIDevice*)*pDevice)->SizeOfData = sizeof(FromAddress);


	
	//pDevice- pointer to device description
	//Buffer- Bytes including header to be sent to HMI
	//Total- size of HAP packet including header and data
	//RetBuffer- not used
	//&responseSize- size of RetBuffer, also not used
	//(Bool literal 1)- care about timeout, not used
	//(Bool literal 2)- care about returns, not used
	//*l- libnet contextP4


#if DATA_PRINT
	int i;
	printf("\nraw buffer:::");
    	for (i=0; i<=(Total-1); i++)	//-1 to account for not starting at 1
    	{
		printf("%02x:", Buffer[i]);
	}
	printf("::end\n");    
#endif

	Error = Send_HAP_Packet(*pDevice, Buffer, Total, RetBuffer, &ResponseSize, TRUE, FALSE, *l);

	return 0;
}


int Send_HAP_Packet(HEIDevice *pDevice, BYTE *pPacket, WORD PacketSize, BYTE *pResponse, int *pResponseSize, BOOL WaitForResponse, BOOL ReturnWarnings, libnet_t *l)
	{
	#if DEBUG
	printf("*~*~ libnet_send.c - send_HAP_Packet\n");
	#endif
	// pPacket is the pointer to the actual packet data, and PacketSize is the
	// number of bytes in the packet to be sent.
	BYTE *ptr;
	WORD *pWord;
	unsigned short ThisAppVal;
	unsigned short *pWordAppVal;
	BYTE *pBegin = pPacket;
	WORD PreHeaderBytes = (((HEIDevice*)pDevice)->DataOffset) - PACKET_HEADER_SIZE; 

	if (((HEIDevice*)pDevice)->UseAddressedBroadcast)
		PreHeaderBytes -= 7;
			
	pPacket += PreHeaderBytes;			
	ptr = pPacket;
			
	if (((HEIDevice*)pDevice)->UseAddressedBroadcast)
		{
		pPacket[PACKET_HEADER_SIZE] = FUN_ADDRESSED_BROADCAST;
		memcpy(&pPacket[PACKET_HEADER_SIZE+1], ((HEIDevice*)pDevice)->ENetAddress, 6);
		}	
	*ptr++ = 'H';
	*ptr++ = 'A';
	*ptr   = 'P';
			
	// Calculate CRC 
	InsertCRC(pPacket, (WORD) (PacketSize-PreHeaderBytes));


	//Insert the number of bytes which were checksumed. 
	pWord = (WORD *) (pPacket+7);
		
	*pWord = (WORD) PacketSize - PreHeaderBytes - PACKET_HEADER_SIZE;	// This is the bytes to follow/checksum 
		
	pWordAppVal = (unsigned short *) (pPacket+3);
	ThisAppVal = ((HEIDevice*)pDevice)->LastAppVal;
	#if DATA_PRINT
	printf("(send packet) app val: %x\n", ThisAppVal);
	#endif
	*pWordAppVal = ThisAppVal;

	
	u_int32_t ip_dst;
	
        ip_dst = libnet_name2addr4(l, ((HEIDevice*)pDevice)->IPdst, LIBNET_DONT_RESOLVE);
//        ip_dst = libnet_name2addr4(l, "255.255.255.255", LIBNET_DONT_RESOLVE);	//all IPs
	#if DATA_PRINT
	printf("Port: %x \n",((HEIDevice*)pDevice)->srcPrt);
	#endif
	send_UDP(pBegin, PacketSize ,((HEIDevice*)pDevice)->srcPrt,ip_dst , l);


	return HEIE_TIMEOUT;
	}

	
	
int InsertCRC(BYTE *pBuffer, WORD Len)
	{
	
	WORD *pWord;
	pWord = (WORD *) (pBuffer+5);
	
	if (1)
		{
		(*pWord) = CalcCRC(0, (BYTE *) (pBuffer+PACKET_HEADER_SIZE), (WORD) (Len - PACKET_HEADER_SIZE));
		}
	else
		{
		(*pWord) = 0;
		}
	
	return 0;
	}	

	
/* number of bits in CRC: don't change it.  */
#define W 16

/* this the number of bits per char: don't change it. */
#define B 8

static unsigned short crctab[1<<B] = { 
    0x0000,  0x1021,  0x2042,  0x3063,  0x4084,  0x50a5,  0x60c6,  0x70e7,
    0x8108,  0x9129,  0xa14a,  0xb16b,  0xc18c,  0xd1ad,  0xe1ce,  0xf1ef,
    0x1231,  0x0210,  0x3273,  0x2252,  0x52b5,  0x4294,  0x72f7,  0x62d6,
    0x9339,  0x8318,  0xb37b,  0xa35a,  0xd3bd,  0xc39c,  0xf3ff,  0xe3de,
    0x2462,  0x3443,  0x0420,  0x1401,  0x64e6,  0x74c7,  0x44a4,  0x5485,
    0xa56a,  0xb54b,  0x8528,  0x9509,  0xe5ee,  0xf5cf,  0xc5ac,  0xd58d,
    0x3653,  0x2672,  0x1611,  0x0630,  0x76d7,  0x66f6,  0x5695,  0x46b4,
    0xb75b,  0xa77a,  0x9719,  0x8738,  0xf7df,  0xe7fe,  0xd79d,  0xc7bc,
    0x48c4,  0x58e5,  0x6886,  0x78a7,  0x0840,  0x1861,  0x2802,  0x3823,
    0xc9cc,  0xd9ed,  0xe98e,  0xf9af,  0x8948,  0x9969,  0xa90a,  0xb92b,
    0x5af5,  0x4ad4,  0x7ab7,  0x6a96,  0x1a71,  0x0a50,  0x3a33,  0x2a12,
    0xdbfd,  0xcbdc,  0xfbbf,  0xeb9e,  0x9b79,  0x8b58,  0xbb3b,  0xab1a,
    0x6ca6,  0x7c87,  0x4ce4,  0x5cc5,  0x2c22,  0x3c03,  0x0c60,  0x1c41,
    0xedae,  0xfd8f,  0xcdec,  0xddcd,  0xad2a,  0xbd0b,  0x8d68,  0x9d49,
    0x7e97,  0x6eb6,  0x5ed5,  0x4ef4,  0x3e13,  0x2e32,  0x1e51,  0x0e70,
    0xff9f,  0xefbe,  0xdfdd,  0xcffc,  0xbf1b,  0xaf3a,  0x9f59,  0x8f78,
    0x9188,  0x81a9,  0xb1ca,  0xa1eb,  0xd10c,  0xc12d,  0xf14e,  0xe16f,
    0x1080,  0x00a1,  0x30c2,  0x20e3,  0x5004,  0x4025,  0x7046,  0x6067,
    0x83b9,  0x9398,  0xa3fb,  0xb3da,  0xc33d,  0xd31c,  0xe37f,  0xf35e,
    0x02b1,  0x1290,  0x22f3,  0x32d2,  0x4235,  0x5214,  0x6277,  0x7256,
    0xb5ea,  0xa5cb,  0x95a8,  0x8589,  0xf56e,  0xe54f,  0xd52c,  0xc50d,
    0x34e2,  0x24c3,  0x14a0,  0x0481,  0x7466,  0x6447,  0x5424,  0x4405,
    0xa7db,  0xb7fa,  0x8799,  0x97b8,  0xe75f,  0xf77e,  0xc71d,  0xd73c,
    0x26d3,  0x36f2,  0x0691,  0x16b0,  0x6657,  0x7676,  0x4615,  0x5634,
    0xd94c,  0xc96d,  0xf90e,  0xe92f,  0x99c8,  0x89e9,  0xb98a,  0xa9ab,
    0x5844,  0x4865,  0x7806,  0x6827,  0x18c0,  0x08e1,  0x3882,  0x28a3,
    0xcb7d,  0xdb5c,  0xeb3f,  0xfb1e,  0x8bf9,  0x9bd8,  0xabbb,  0xbb9a,
    0x4a75,  0x5a54,  0x6a37,  0x7a16,  0x0af1,  0x1ad0,  0x2ab3,  0x3a92,
    0xfd2e,  0xed0f,  0xdd6c,  0xcd4d,  0xbdaa,  0xad8b,  0x9de8,  0x8dc9,
    0x7c26,  0x6c07,  0x5c64,  0x4c45,  0x3ca2,  0x2c83,  0x1ce0,  0x0cc1,
    0xef1f,  0xff3e,  0xcf5d,  0xdf7c,  0xaf9b,  0xbfba,  0x8fd9,  0x9ff8,
    0x6e17,  0x7e36,  0x4e55,  0x5e74,  0x2e93,  0x3eb2,  0x0ed1,  0x1ef0
    };

WORD CalcCRC(WORD    icrc,  BYTE   *icp,  WORD    icnt)
	{
	register WORD crc = icrc;
	register BYTE *cp = icp;
	register WORD cnt = icnt;

	while( cnt-- )
		crc = (crc<<B) ^ crctab[(crc>>(W-B)) ^ *cp++];


	return( crc );
	}	


u_int16_t crc16(const uint8_t *msg, u_int16_t size)
{
  int i = 0;
  //unsigned short crc = 0xffffU;
  unsigned short crc = 0x0000U;
  while(size--) {
      crc ^= (unsigned short) *msg++ << 8;

    for(i = 0; i < 8; ++i)
      crc = crc << 1 ^ (crc & 0x8000U ? 0x1021U : 0U);
  }
  return(crc & 0xffffU);
}

	
u_int16_t send_UDP(const uint8_t *payload, u_int16_t sizeof_payload, uint16_t dst_port,uint32_t dst_ip, libnet_t *l_ptr)
{
	//printf("size of UDP payload: %d\n", sizeof_payload);

	#if DEBUG
	printf("*~*~ libnet_send.c - send_UDP\n");
	#endif

	#if DATA_PRINT
	printf("IP destination: %d\n",dst_ip);
	#endif 

      	int bytes_written;
	
        /* Building UDP header */


	uint16_t src_port = 28784;
	//printf("address l-7: %p\n",&l);

	uint16_t udp_length = LIBNET_UDP_H+sizeof_payload;
#if MINIMAL_PRINT
	int i;
	printf("  payload: (");
	for(i =0; i<= sizeof_payload; i++)
	printf("%02x:", payload[i] );
	printf(")  size of payload: (%d)", sizeof_payload);
	printf("  length: (%d)", udp_length);
	printf("  src_port: (%d)", src_port);
	printf("  dst_port: (%d)\n", dst_port);
#endif

	//if (libnet_build_udp (51, 51, 8, 0, NULL,0, l,0) ==-1)
        if (libnet_build_udp (src_port, dst_port, udp_length, 0, (u_int8_t*)payload,sizeof_payload, l_ptr,0) ==-1)
        {   
		
                fprintf(stderr, "Error building UDP header: %s\n", libnet_geterror(l_ptr));
                libnet_destroy(l_ptr);
                exit(EXIT_FAILURE);
        }

        // Building IP header 

        if ( libnet_autobuild_ipv4(LIBNET_IPV4_H + LIBNET_UDP_H + sizeof_payload, IPPROTO_UDP, dst_ip, l_ptr) == -1 )
        {
                fprintf(stderr, "Error building IP header: %s\n", libnet_geterror(l_ptr));
                libnet_destroy(l_ptr);
                exit(EXIT_FAILURE);
        }
        
        
        


	u_int32_t sleeper;
	sleeper=0;
	while(sleeper <515000)
	sleeper++;




        // Writing packet 
        bytes_written = libnet_write(l_ptr);
		int a;
        if ( bytes_written != -1 )
		{
		a = 1;
		#if DATA_PRINT
		printf("(%d bytes written)\n", LIBNET_ETH_H+LIBNET_IPV4_H + LIBNET_UDP_H + sizeof_payload); 
		#endif 
		}
        else
                fprintf(stderr, "Error writing packet: %s\n", libnet_geterror(l_ptr));

	libnet_clear_packet(l_ptr);
        //ibnet_destroy(l_ptr);
        return 0;
}
/*
int msleep(unsigned long milisec)
{
    struct timespec req={0};
    time_t sec=(int)(milisec/100000);
    milisec=milisec-(sec*100000);
    req.tv_sec=sec;
    req.tv_nsec=milisec*10000L;
    while(nanosleep(&req,&req)==-1)
         continue;
    return 1;
}
*/


 



























