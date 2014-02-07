#include <stdio.h>
#include <stdlib.h>
#include <libnet.h>
#include <stdint.h>
#include <string.h>

#include "DEFS.H"
#include "HEI.H"

int Send_HAP_Packet(HEIDevice *pDevice, BYTE *pPacket, WORD PacketSize, BYTE *pResponse, int *pResponseSize, BOOL WaitForResponse, BOOL ReturnWarnings, libnet_t *l);
int InsertCRC(BYTE *pBuffer, WORD Len);
WORD CalcCRC(WORD icrc, BYTE* icp, WORD icnt);
u_int16_t send_UDP(const uint8_t *payload, u_int16_t sizeof_payload, uint16_t dst_port,uint32_t dst_ip, libnet_t *l);
u_int16_t crc16(const uint8_t *msg, u_int16_t size) ;
int configure(HEIDevice **pDevice, libnet_t **l);
int send_respond_polling_all(HEIDevice **pDevice, libnet_t **l);
int send_respond_addressed_broadcast(HEIDevice **pDevice, libnet_t **l);
void mem_print(const char* label, void* data, u_int length);
int send_respond_query_setup_data(HEIDevice **pDevice, libnet_t **l);
int send_respond_ccm_request(HEIDevice **pDevice, libnet_t **l);
int send_respond_kseq_request(HEIDevice **pDevice, libnet_t **l);
int send_read_setup_data(HEIDevice **pDevice, libnet_t **l);
int send_read_device_info(HEIDevice **pDevice, libnet_t **l);
int send_read_version_info(HEIDevice **pDevice, libnet_t **l);
int send_read_estats(HEIDevice **pDevice, libnet_t **l);
void stoh(char* str, unsigned char* dst_hex, int strlen);
