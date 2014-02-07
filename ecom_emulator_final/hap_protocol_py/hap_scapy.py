#~2012 Capt Bob Jaromin
#~Python emulator for partial implementation of the Host Automation Products protocol

import sys
import os
#check for required libraries in ../../lib
base_dir = os.path.dirname(__file__) or '.'
lib_dir = os.path.join(base_dir, '../lib')
sys.path.insert(0, lib_dir)



import socket, platform, random
from struct import *
import sys
import re
import crc16pure
import binascii
import struct


from scapy.all import *

#custom files:
from configfileio import configfileio

function_code_enum = {0x5:'FUN_POLLING_ALL', 0x15:'FUN_ADDRESSED_BROADCAST', 0x55:'FUN_QUERY_RESPONSE',
0x0:'FUN_POLLING', 0x4:'FUN_READ_DEVICE_INFO', 0x32:'FUN_COMM_REQ_ACK', 0x1a:'FUN_KSEQ_REQUEST', 0xb:'FUN_READ_SETUP_DATA'}

function_code = {'FUN_POLLING_ALL':0x5, 'FUN_ADDRESSED_BROADCAST':0x15, 'FUN_QUERY_RESPONSE':0x55,
'FUN_POLLING':0x0, 'FUN_READ_DEVICE_INFO':0x4, 'FUN_COMM_REQ_ACK':0x32, 'FUN_KSEQ_REQUEST':0x1a, 'FUN_READ_SETUP_DATA':0xb}

SetupType_enum = {0x10:'IP_ADDRESS',0x20:'NODE_NUMBER',0x16:'NODE_NAME',0x26:'DESCRIPTION',0x4:'READ_DEVICE_INFO'}
SetupType = {'IP_ADDRESS':0x10,'NODE_NUMBER':0x20,'NODE_NAME':0x16,'DESCRIPTION':0x26,'READ_DEVICE_INFO':0x4}


class HAP(Packet):
    name = "HAP"
    fields_desc = [ X3BytesField("HAP", 0x484150),
                    LEShortField("AppVal",0x0000),
                    LEShortField("CRC",0x0000),
                    LEFieldLenField("Length",None,length_of="Data",adjust=lambda pkt,x:(x+1)),
                    ByteEnumField("Fun_Code",0x00,function_code_enum),
                    StrLenField("Data","",length_from=lambda pkt:(pkt.Length))
                    ]
                    
                    



class response1:
    def wait_for_packets(self):
        print'nothing'



#{0x5:'FUN_POLLING_ALL', 0x15:'FUN_ADDRESSED_BROADCAST', 0x55:'FUN_QUERY_RESPONSE',
#0x0:'FUN_POLLING', 0x4:'FUN_READ_DEVICE_INFO', 0x32:'FUN_COMM_REQ_ACK', 0x1a:'FUN_KSEQ_REQUEST'}
def decide_response(packet):
    try:
        if packet.haslayer(UDP):
            if packet[UDP].dport == 28784:
                if packet.haslayer(HAP):
                    #print'**we have HAP packets**'      
                    print'-------------------- new packet ---------------------------'
                    print 'appVal: ', hex(packet[HAP].AppVal)
                    print 'fun_code: ', hex(packet[HAP].Fun_Code), function_code_enum[packet[HAP].Fun_Code]
                    print 'data: ', packet[HAP].Data.encode("hex")
                                        
                    if packet[HAP].Fun_Code ==function_code['FUN_POLLING_ALL']:
                        #print'respond poll all'
                        response_polling_all(packet)
                    elif packet[HAP].Fun_Code ==function_code['FUN_ADDRESSED_BROADCAST']:
                        #print'respond addressed broadcast'
                        response_addressed_broadcast(packet)
                    elif packet[HAP].Fun_Code ==function_code['FUN_QUERY_RESPONSE']:
                        print'respond query request'
                        #respons0x4:'FUN_READ_DEVICE_INFO'e_query_response(packet)    
                    elif packet[HAP].Fun_Code ==function_code['FUN_POLLING']:
                        print'respond polling'
                        #response_polling(packet)
                    elif packet[HAP].Fun_Code ==function_code['FUN_READ_DEVICE_INFO']:
                        print'tryting to enter respond read device info'
                        response_device_info(packet)
                    elif packet[HAP].Fun_Code ==function_code['FUN_COMM_REQ_ACK']:
                        print'respond comm req ack'
                        #response_comm_req_ack(packet)
                    elif packet[HAP].Fun_Code ==function_code['FUN_KSEQ_REQUEST']:
                        print'respond kseq request'
                        #response_kseq_request(packet)
                    else:
                        print'unknown function code'
    except:
        print'something broken in decide_response'
        pass
        
                            
def is_for_me(packet, global_params):
    hex_mac = global_params['mac'].replace(' ', '').decode('hex')
    lower_mac = str.lower(global_params['mac'].replace(' ', ''))    
    
    if  lower_mac in packet[HAP].Data.encode('hex'):
        print packet[IP].src,' is talking to me...'        
        return 1
    else:
        print packet[IP].src,' is talking to someone else...'        
        return 0

def send_response(packet, fun_code, data, to_crc):
    print'in send response'
    hap = HAP()
    hap.AppVal = packet[HAP].AppVal
    hap.Fun_Code = fun_code
    hap.Data = data
    print'crc_16 start...'    
    print crc16_pure.crc16xmodem(to_crc)
    print'...crc_16 done'    
    
    hap.CRC = crc16_pure.crc16xmodem(to_crc)  
    udp = UDP()
    udp.dport = packet[UDP].sport
    udp.sport = 28784
    ip = IP()
    ip.dst = packet[IP].src
    response = Ether()/ip/udp/hap
    sendp(response)
        
def response_polling_all(packet):
    #HEIQueryDevices
    # PLC responds with "I'm Alive" packet with mac and IP
    try:
        print'respond poll all'

        
        #get globals so we can get mac and IP
        configio = configfileio()
        global_params = configio.get_global_parameters()
        hex_mac = global_params['mac'].replace(' ', '').decode('hex')
        str_ip = global_params['ip'].split('.')
        hex_ip =(''.join('%02x'%int(i) for i in str_ip)).decode('hex')
        
        data = "\xaa"+hex_mac + "\x00\x01" + hex_ip + "\x00"
        fun_code = function_code['FUN_QUERY_RESPONSE']
        
        #convert fun_code and data to strings to combine
        combo = chr(fun_code).encode('hex')+str(data.encode('hex'))
        to_crc = combo.decode('hex')
        print "combo:", combo
        print "tocrc:", to_crc
        #sent response packet
        send_response(packet,fun_code,data,to_crc)   
    except:
        print 'broken response polling all'
        pass
        
    
def response_addressed_broadcast(packet):
    try:
        configio = configfileio()
        global_params = configio.get_global_parameters()

            
        if is_for_me(packet, global_params):            
            #7th byte determines what kind of querey this is
            fun_code2= packet[HAP].Data.encode('hex')[12]
            fun_code2+=packet[HAP].Data.encode('hex')[13]
            fun_code2=int(fun_code2,16)
            print'2nd function code: ', function_code_enum[fun_code2]
            
            if function_code_enum[fun_code2] =='FUN_READ_DEVICE_INFO':       
                fun_code = int(global_params['Htype'])    #model type: 2,3, or 4
                print'Etype: ', global_params['Etype']
                Etype = chr(int(global_params['Etype']))   #2=EBC, 1=ECOM
                
                print 'Ftype: ', global_params['Ftype']                 
                Ftype = chr(int(global_params['Ftype']))   # 0= "",  1= "-F"
                               
                hex_mac = global_params['mac'].replace(' ', '').decode('hex')
                data = "\x01"+ Etype +"\x00"+ hex_mac+"\x40\x00\x00\x02\x00\x00\x00\x00"+Ftype+"\x00\x00\x00\x00\x00\x00\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                
                #convert fun_code and data to strings to combine
                combo = chr(fun_code).encode('hex')+str(data.encode('hex'))
                to_crc = combo.decode('hex')
                
                #sent response packet
                send_response(packet,fun_code,data,to_crc)
                
            elif function_code_enum[fun_code2] =='FUN_READ_SETUP_DATA':ef response_addressed_broadcast(packet):
    try:
        configio = configfileio()
        global_params = configio.get_global_parameters()

                fun_type = packet[HAP].Data.encode('hex')[16]
                fun_type += packet[HAP].Data.encode('hex')[17]
                fun_type=int(fun_type,16)
                print'setup type raw byte: ', hex(fun_type)
                print'setup_type: ', SetupType_enum[fun_type]
                
                if SetupType_enum[fun_type] == 'IP_ADDRESS':
                    #clever way to convert ip to hex bytes, other way: socket.inet_ntoa(struct.pack("!I", long(1074015239)))
                    str_ip = global_params['ip'].split('.')
                    hex_ip =(''.join('%02x'%int(i) for i in str_ip)).decode('hex')
                    print'hexip: ', hex_ip            
                    
                    fun_code = function_code['FUN_POLLING']
                    data = "\x00"+hex_ip
                    
                    #sent response packet
                    send_response(packet,fun_code,data,data)                    
                    
                elif SetupType_enum[fun_type] == 'NODE_NUMBER':
                    setid = configio.get_setid_parameters()
                    print'in node number'
                    int_id = int(setid['n'])
                    hex_id = hex(int_id)
                    packed_id = struct.pack("<L",int_id )                    
                    print R"packed_id", packed_id.encode('hex')
                    
                    fun_code = function_code['FUN_POLLING']
                    data = "\x00"+packed_id

                    #sent response packet
                    send_response(packet,fun_code,data,data)   
                    
                elif SetupType_enum[fun_type] == 'NODE_NAME':
                    setname = configio.get_setname_parameters()
                    padded_name = setname['n'].ljust(256,'\x00')
                    
                    fun_code = function_code['FUN_POLLING']
                    data = "\x00"+padded_name

                    #sent response packet
                    send_response(packet,fun_code,data,data)   
    
                elif SetupType_enum[fun_type] == 'DESCRIPTION':
                    setdesc = configio.get_setdesc_parameters()
                    padded_desc = setdesc['n'].ljust(256,'\x00')
                    
                    fun_code = function_code['FUN_POLLING']
                    data = "\x00"+padded_desc
                    
                    #sent response packet
                    send_response(packet,fun_code,data,data)                       
                else:
                    print'setup type not implemented'
    except:
        print'~~~~~~~~~~~~~~~~~~~~~~~~~~~ something broke! ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~'
        pass

def main():
    try:
        
        bind_layers(UDP,HAP,sport = 28784)
        bind_layers(UDP,HAP,dport = 28784)
        
        print'sniffing'
        sniff(iface="eth2", filter="udp and port 28784", store = 0, prn=lambda x:decide_response(x))
        print'end sniffing'
        
        
        
    except KeyboardInterrupt:        
        print'nothing here'
        pass
        
if __name__ == '__main__':
    main()



