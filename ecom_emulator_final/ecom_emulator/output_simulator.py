#!/usr/bin/env python
import sys
import os

#check for required libraries in ../../lib
base_dir = os.path.dirname(__file__) or '.'
lib_dir = os.path.join(base_dir, '../lib')
sys.path.insert(0, lib_dir)


import random
import ConfigParser
import time

config_kseq = ConfigParser.RawConfigParser()
config_ccm = ConfigParser.RawConfigParser()

config_ccm.add_section('CCM_READ')
config_ccm.add_section('CCM_WRITE')


#if status in directsoft isnt working, its probably because this path is broken.
ccm_file = './ecom_emulator/ccmdictionary.txt'


#print"\n--------------------------------------------------------------------"
#print"* output_simulator.py                                              *" 
#print"* manipulates ccmdictionary.txt to simulate i/o 	                 *"
#print"--------------------------------------------------------------------"
print '+++++Output Simulator Started.  Changing values in ccmdictionary.txt file.'

#these don't change:
config_ccm.set("CCM_READ", "1e01777736", "60000")            
config_ccm.set("CCM_READ", "1e010f0036", "00005a")   
config_ccm.set("CCM_READ", "1e06ee0f31", "0000000000000000")   
config_ccm.set("CCM_READ", "1e01840132", "000000")   
config_ccm.set("CCM_WRITE", "200140043680", "060000000000") 
config_ccm.set("CCM_WRITE", "200140043600", "060000000000") 


a = range(1000)
counter1 = 0
counter2=4

strHex = "0x%0.2X" % 255


inputs = [0x1,0x3,0xb,0x7,0xb,0x7,0xb,0x7,0xb,0x7,0xb,0x7,0xb,0x7,0xb,0x7,0xb,0x7,0xb,0x7,0xb,0x7,0x0,0x7,0xb,0x7,0xb,0x7,0xb,0x7,0xb,0x7,0xb,0x7,0xb,0x7,0xb,0x7,0xb,0x7,0xb,0x7,0x1,0x00]
i=0

while (1):
    i+=1
    #print i
    config_ccm.set("CCM_READ", "1e01010132", "%06x" % inputs[i%14])     #inputs register
    if (inputs[i%43]&1):
        counter1+=1
    else:
	#print "clear counter 1"
	counter1 = 0
 
    if (inputs[i%43]&2):
        counter2+=i%2
    else:
	#print "clear counter 2"
	counter2=0

    config_ccm.set("CCM_READ", "1e04010031", "00"+str(counter1).zfill(4)+str(counter2).zfill(4)+"00") 
    
    if (inputs[i%43]&8):
        config_ccm.set("CCM_READ", "1e01010133", "000001")       
    else:
        config_ccm.set("CCM_READ", "1e01010133", "000000")       
    
    if (inputs[i%43]&4):
        config_ccm.set("CCM_READ", "1e01810133", "000001") 
    else:
        config_ccm.set("CCM_READ", "1e01810133", "000000") 
  
    config_ccm.set("CCM_READ", "1e01010333", "%06x" % inputs[i%13]) 
    
    # Writing new configuration file 
    with open(ccm_file, 'wb') as configfile:
        config_ccm.write(configfile)
    
   
#    print "1e01010132 = " ,config_ccm.get('CCM_READ', "1e01010132")    
#    print "1e04010031 = " ,config_ccm.get('CCM_READ', "1e04010031")    
#    print "1e01010133 = " ,config_ccm.get('CCM_READ', "1e01010133")    
#    print "1e01810133 = " ,config_ccm.get('CCM_READ', "1e01810133")    
#    print "1e01010333 = " ,config_ccm.get('CCM_READ', "1e01010333")    
    
    time.sleep(.1+random.random())








