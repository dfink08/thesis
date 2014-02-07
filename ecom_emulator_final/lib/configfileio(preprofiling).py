import sys
import os


import string,cgi,time 
from os import curdir, sep
from BaseHTTPServer import BaseHTTPRequestHandler, HTTPServer
import urlparse
from urlparse import urlparse, parse_qs, parse_qsl
from string import Template,upper
import re
import urllib 

base_dir = os.path.dirname(__file__) or '.'
config_file = os.path.join(base_dir, '../config.txt')
config_file = os.path.normpath(config_file)





class configfileio:
    
    
    
    def get_all_parameters(self):
        try:    
            fulldict = {}
            with open(config_file)as f:
                for line in f:
                    param, value = line.strip().split('=', 1)                                                                                                                                
                    fulldict[param] = urllib.unquote_plus(value)
                    #print 'fulldict1: ',param,fulldict[param]
                #print 'fulldict2: ',fulldict['setid_n']
                return fulldict
        except:
            pass
            
    def get_setid_parameters(self):
        try:    
            partialdict = {}
            with open(config_file)as f:
                for line in f:
                    param, value = line.strip().split('=', 1) 
                    #print param,value
                    if "setid_" in param:                   
                        fun, query=param.split('_',1)
                        partialdict[query] = value
                return partialdict
        except:
            pass

    def save_setid_parameters(self,tosave):
        try:
            setid_dict={'n':'setid_n','h':'setid_h'}
            savedict={}
            for item in setid_dict:
                savedict[setid_dict[item]]=tosave[item]

            #union this with savedict and keep overlapping values of saveedict
            fulldict  = self.get_all_parameters()
            newdict = dict(fulldict,**savedict)
            with open(config_file,'w') as f:      
                for item in newdict:
                    printer = item+'='+ str(newdict[item]) +'\n'
                    f.write(printer)
            
        except:
            print'save setid broken!'
            pass
            
              
    def get_setname_parameters(self):
        try: 
            partialdict = {}
            with open(config_file)as f:
                for line in f:
                    param, value = line.strip().split('=', 1) 
                    #print param,value
                    if "setname_" in param:            
                        fun, query=param.split('_',1)
                        #unquote_plus removes + signs from url encoding
                        partialdict[query] = urllib.unquote_plus(value)
                return partialdict
        except:
            print'get setname broken!'
            pass             
            
    def save_setname_parameters(self,tosave):
        try:    
            partialdict={'n':'setname_n'}
            savedict={}
            for item in partialdict:
                savedict[partialdict[item]]=tosave[item]

            #union this with savedict and keep overlapping values of saveedict
            fulldict  = self.get_all_parameters()
            newdict = dict(fulldict,**savedict)
            with open(config_file,'w') as f:      
                for item in newdict:
                    printer = item+'='+ (str(newdict[item])) +'\n'
                    f.write(printer)
        except:
            print'save setname broken!'
            pass
             
    def get_global_parameters(self):
        try: 
            partialdict = {}
            with open(config_file)as f:
                for line in f:
                    param, value = line.strip().split('=',1) 
                    #print param,value
                    if "global_" in param: 
                        #print'found global'                    
                        fun, query=param.split('_',1)
                        partialdict[query] = urllib.unquote_plus(value)
                        
               #print 'about to return'
               #print 'after get globals partial dict', partialdict
                return partialdict
                
        except:
            print'get globals broken!'
            pass            


            
    def save_global_parameters(self,tosave):
        try:
            partialdict={'submit':'global_submit','gateway':'global_gateway','cpurev':'global_cpurev',
            'email_timeout':'global_email_timeout','PWB':'global_PWB','PWB':'global_PLD','email_sendername':'global_email_sendername',
            'mac':'global_mac','subnet':'global_subnet','email_port':'global_email_port','Htype':'global_Htype','Etype':'global_Etype',
            'boot':'global_boot','email_serverip':'global_email_serverip','OSver':'global_OSver', 'Ftype':'global_Ftype',
            'email_cc':'global_email_cc','email_senderaddr':'global_email_senderaddr','ip':'global_ip',
            'ver_boot':'global_ver_boot','ver_firmware':'global_ver_firmware','build_boot':'global_build_boot','build_firmware':'global_build_firmware',
            'password':'global_password','locked':'global_locked', 'suppliedbydhcp':'global_suppliedbydhcp'} #'type_n':'global_type_n',
            savedict={}
            #print'saving in global partial dict', partialdict
            #print'partialdict[\'gateway\']: ',partialdict['gateway']         
            
            
            for item in partialdict:
             #   print'the key is',item
              #  print'saving...before unquote', tosave[item]
                savedict[partialdict[item]]=urllib.quote_plus(tosave[item])
               # print'saving...after unquote', savedict[partialdict[item]]

            #union this with savedict and keep overlapping values of saveedict
            fulldict  = self.get_all_parameters()
            newdict = dict(fulldict,**savedict)
            #print'fulldict',fulldict
            #print '\n\n'
            
            #print'newdict',newdict
            with open(config_file,'w') as f:
                for item in newdict:
                    printer = item+'='+ str(newdict[item]) +'\n'
                    f.write(printer)      
            
        except:
            print'saving globals broken!'             
            raise 
            
            
    def get_advanced_parameters(self):
        try: 
            partialdict = {}
            with open(config_file)as f:
                for line in f:
                    param, value = line.strip().split('=', 1) 
                    #print param,value
                    if "advanced_" in param:                     
                        fun, query=param.split('_',1)
                        partialdict[query] = value
                return partialdict
        except:
            pass             
            
    def save_advanced_parameters(self,tosave):
        try:
            partialdict={'a':'advanced_a','b':'advanced_b','c':'advanced_c','d':'advanced_d',
            'e':'advanced_e','f':'advanced_f','g':'advanced_g','h':'advanced_h',
            'i':'advanced_i','readonly_web_checkmark':'advanced_readonly_web_checkmark'}
            savedict={}
            for item in partialdict:
                savedict[partialdict[item]]=tosave[item]

            #union this with savedict and keep overlapping values of saveedict
            fulldict  = self.get_all_parameters()
            newdict = dict(fulldict,**savedict)
            with open(config_file,'w') as f:      
                for item in newdict:
                    printer = item+'='+ str(newdict[item]) +'\n'
                    f.write(printer)
            
        except:
            pass  
            
    def get_peerlink_parameters(self):
        try: 
            partialdict = {}
            with open(config_file)as f:
                for line in f:
                    param, value = line.strip().split('=', 1) 
                    #print param,value
                    if "peerlink_" in param:                     
                        fun, query=param.split('_',1)
                        partialdict[query] = value

            #make 3 new dictionary entries out of the single config entry to
            # accomadate the radio buttons in the .html template
                for dex1 in range(0,16):
                    for dex2 in '012':
                        partialdict.update({'b' + str(dex1)+ '_' + dex2:"\'\'"})
                        #print 'b'+str(dex1)+dex2
                        #print 'dict so far', partialdict
                    temp = 'b' + str(dex1)+ '_' + partialdict['b' + str(dex1)]
                    partialdict[temp] = 'checked'
                
 #               print 'peerlink dict', partialdict
                return partialdict
        except:
            print'peerlink get broken'
            pass             
            
            
    def save_peerlink_parameters(self,tosave):
        try:
            partialdict={'a':'peerlink_a','e':'peerlink_e','b0':'peerlink_b0','b1':'peerlink_b1','b2':'peerlink_b2',
            'b3':'peerlink_b3','b4':'peerlink_b4','b5':'peerlink_b5','b6':'peerlink_b6','b7':'peerlink_b7',
            'b8':'peerlink_b8','b9':'peerlink_b9','b10':'peerlink_b10','b11':'peerlink_b11','b12':'peerlink_b12',
            'b13':'peerlink_b13','b14':'peerlink_b14','b15':'peerlink_b15'}
            savedict={}
            for item in partialdict:
                savedict[partialdict[item]]=tosave[item]

            #union this with savedict and keep overlapping values of saveedict
            fulldict  = self.get_all_parameters()
            newdict = dict(fulldict,**savedict)
            with open(config_file,'w') as f:      
                for item in newdict:
                    printer = item+'='+ str(newdict[item]) +'\n'
                    f.write(printer)
            
        except:
            pass  
            
    def get_setdesc_parameters(self):
        try: 
            partialdict = {}
            with open(config_file)as f:
                for line in f:
                    param, value = line.strip().split('=', 1) 
                    #print param,value
                    if "setdesc_" in param:                   
                        fun, query=param.split('_',1)
                        partialdict[query] = urllib.unquote_plus(value)
                return partialdict
        except:
            print'broken get_setdesc'
            pass             
            
    def save_setdesc_parameters(self,tosave):
        try:
            partialdict={'n':'setdesc_n'}
            savedict={}
            for item in partialdict:
                savedict[partialdict[item]]=tosave[item]

            #union this with savedict and keep overlapping values of saveedict
            fulldict  = self.get_all_parameters()
            newdict = dict(fulldict,**savedict)
            with open(config_file,'w') as f:      
                for item in newdict:
                    printer = item+'='+ (str(newdict[item])) +'\n'
                    f.write(printer)
            
        except:
            print'save setdesc broken!'
            pass  
            
            
    def get_setp2p_parameters(self):
        try: 
            partialdict = {}
            with open(config_file)as f:
                for line in f:
                    param, value = line.strip().split('=', 1) 
                    #print param,value
                    if "setp2p_" in param:                     
                        fun, query=param.split('_',1)
                        partialdict[query] = value
 
           #make 2 new dictionary entries out of the single config entry to
            # accomadate the radio buttons in the .html template
                for dex1 in range(1,88):
                    for dex2 in '01':
                        partialdict.update({'p' + str(dex1)+ '_' + dex2:"\'\'"})
                        #print 'p'+str(dex1)+'_'+dex2
                        #print 'dict so far', partialdict
                    temp = 'p' + str(dex1)+ '_' + partialdict['p' + str(dex1)]
                    partialdict[temp] = 'checked'                
                
                return partialdict
        except:
            print'get setp2p is broken!'
            pass             
            
    def save_setp2p_parameters(self,tosave):
        try:
            partialdict={'i1':'setp2p_i1','i10':'setp2p_i10','i11':'setp2p_i11','i12':'setp2p_i12','i13':'setp2p_i13','i14':'setp2p_i14','i15':'setp2p_i15',
            'i16':'setp2p_i16','i17':'setp2p_i17','i18':'setp2p_i18','i19':'setp2p_i19','i2':'setp2p_i2','i20':'setp2p_i20','i21':'setp2p_i21','i22':'setp2p_i22',
            'i23':'setp2p_i23','i24':'setp2p_i24','i25':'setp2p_i25','i26':'setp2p_i26','i27':'setp2p_i27','i28':'setp2p_i28','i29':'setp2p_i29','i3':'setp2p_i3',
            'i30':'setp2p_i30','i31':'setp2p_i31','i32':'setp2p_i32','i33':'setp2p_i33','i34':'setp2p_i34','i35':'setp2p_i35','i36':'setp2p_i36','i37':'setp2p_i37',
            'i38':'setp2p_i38','i39':'setp2p_i39','i4':'setp2p_i4','i40':'setp2p_i40','i41':'setp2p_i41','i42':'setp2p_i42','i43':'setp2p_i43','i44':'setp2p_i44',
            'i45':'setp2p_i45','i46':'setp2p_i46','i47':'setp2p_i47','i48':'setp2p_i48','i49':'setp2p_i49','i5':'setp2p_i5','i50':'setp2p_i50','i51':'setp2p_i51',
            'i52':'setp2p_i52','i53':'setp2p_i53','i54':'setp2p_i54','i55':'setp2p_i55','i56':'setp2p_i56','i57':'setp2p_i57','i58':'setp2p_i58','i59':'setp2p_i59',
            'i6':'setp2p_i6','i60':'setp2p_i60','i61':'setp2p_i61','i62':'setp2p_i62','i63':'setp2p_i63','i64':'setp2p_i64','i65':'setp2p_i65','i66':'setp2p_i66',
            'i67':'setp2p_i67','i68':'setp2p_i68','i69':'setp2p_i69','i7':'setp2p_i7','i70':'setp2p_i70','i71':'setp2p_i71','i72':'setp2p_i72','i73':'setp2p_i73',
            'i74':'setp2p_i74','i75':'setp2p_i75','i76':'setp2p_i76','i77':'setp2p_i77','i78':'setp2p_i78','i79':'setp2p_i79','i8':'setp2p_i8','i80':'setp2p_i80',
            'i81':'setp2p_i81','i82':'setp2p_i82','i83':'setp2p_i83','i84':'setp2p_i84','i85':'setp2p_i85','i86':'setp2p_i86','i87':'setp2p_i87','i9':'setp2p_i9',
            'p1':'setp2p_p1','p10':'setp2p_p10','p11':'setp2p_p11','p12':'setp2p_p12','p13':'setp2p_p13','p14':'setp2p_p14','p15':'setp2p_p15','p16':'setp2p_p16',
            'p17':'setp2p_p17','p18':'setp2p_p18','p19':'setp2p_p19','p2':'setp2p_p2','p20':'setp2p_p20','p21':'setp2p_p21','p22':'setp2p_p22','p23':'setp2p_p23',
            'p24':'setp2p_p24','p25':'setp2p_p25','p26':'setp2p_p26','p27':'setp2p_p27','p28':'setp2p_p28','p29':'setp2p_p29','p3':'setp2p_p3','p30':'setp2p_p30',
            'p31':'setp2p_p31','p32':'setp2p_p32','p33':'setp2p_p33','p34':'setp2p_p34','p35':'setp2p_p35','p36':'setp2p_p36','p37':'setp2p_p37','p38':'setp2p_p38',
            'p39':'setp2p_p39','p4':'setp2p_p4','p40':'setp2p_p40','p41':'setp2p_p41','p42':'setp2p_p42','p43':'setp2p_p43','p44':'setp2p_p44','p45':'setp2p_p45',
            'p46':'setp2p_p46','p47':'setp2p_p47','p48':'setp2p_p48','p49':'setp2p_p49','p5':'setp2p_p5','p50':'setp2p_p50','p51':'setp2p_p51','p52':'setp2p_p52',
            'p53':'setp2p_p53','p54':'setp2p_p54','p55':'setp2p_p55','p56':'setp2p_p56','p57':'setp2p_p57','p58':'setp2p_p58','p59':'setp2p_p59','p6':'setp2p_p6',
            'p60':'setp2p_p60','p61':'setp2p_p61','p62':'setp2p_p62','p63':'setp2p_p63','p64':'setp2p_p64','p65':'setp2p_p65','p66':'setp2p_p66','p67':'setp2p_p67',
            'p68':'setp2p_p68','p69':'setp2p_p69','p7':'setp2p_p7','p70':'setp2p_p70','p71':'setp2p_p71','p72':'setp2p_p72','p73':'setp2p_p73','p74':'setp2p_p74',
            'p75':'setp2p_p75','p76':'setp2p_p76','p77':'setp2p_p77','p78':'setp2p_p78','p79':'setp2p_p79','p8':'setp2p_p8','p80':'setp2p_p80','p81':'setp2p_p81',
            'p82':'setp2p_p82','p83':'setp2p_p83','p84':'setp2p_p84','p85':'setp2p_p85','p86':'setp2p_p86','p87':'setp2p_p87','p9':'setp2p_p9','r1':'setp2p_r1',
            'r10':'setp2p_r10','r11':'setp2p_r11','r12':'setp2p_r12','r13':'setp2p_r13','r14':'setp2p_r14','r15':'setp2p_r15','r16':'setp2p_r16','r17':'setp2p_r17',
            'r18':'setp2p_r18','r19':'setp2p_r19','r2':'setp2p_r2','r20':'setp2p_r20','r21':'setp2p_r21','r22':'setp2p_r22','r23':'setp2p_r23','r24':'setp2p_r24',
            'r25':'setp2p_r25','r26':'setp2p_r26','r27':'setp2p_r27','r28':'setp2p_r28','r29':'setp2p_r29','r3':'setp2p_r3','r30':'setp2p_r30','r31':'setp2p_r31',
            'r32':'setp2p_r32','r33':'setp2p_r33','r34':'setp2p_r34','r35':'setp2p_r35','r36':'setp2p_r36','r37':'setp2p_r37','r38':'setp2p_r38','r39':'setp2p_r39',
            'r4':'setp2p_r4','r40':'setp2p_r40','r41':'setp2p_r41','r42':'setp2p_r42','r43':'setp2p_r43','r44':'setp2p_r44','r45':'setp2p_r45','r46':'setp2p_r46',
            'r47':'setp2p_r47','r48':'setp2p_r48','r49':'setp2p_r49','r5':'setp2p_r5','r50':'setp2p_r50','r51':'setp2p_r51','r52':'setp2p_r52','r53':'setp2p_r53',
            'r54':'setp2p_r54','r55':'setp2p_r55','r56':'setp2p_r56','r57':'setp2p_r57','r58':'setp2p_r58','r59':'setp2p_r59','r6':'setp2p_r6','r60':'setp2p_r60',
            'r61':'setp2p_r61','r62':'setp2p_r62','r63':'setp2p_r63','r64':'setp2p_r64','r65':'setp2p_r65','r66':'setp2p_r66','r67':'setp2p_r67','r68':'setp2p_r68',
            'r69':'setp2p_r69','r7':'setp2p_r7','r70':'setp2p_r70','r71':'setp2p_r71','r72':'setp2p_r72','r73':'setp2p_r73','r74':'setp2p_r74','r75':'setp2p_r75',
            'r76':'setp2p_r76','r77':'setp2p_r77','r78':'setp2p_r78','r79':'setp2p_r79','r8':'setp2p_r8','r80':'setp2p_r80','r81':'setp2p_r81','r82':'setp2p_r82',
            'r83':'setp2p_r83','r84':'setp2p_r84','r85':'setp2p_r85','r86':'setp2p_r86','r87':'setp2p_r87','r9':'setp2p_r9','u1':'setp2p_u1','u10':'setp2p_u10',
            'u11':'setp2p_u11','u12':'setp2p_u12','u13':'setp2p_u13','u14':'setp2p_u14','u15':'setp2p_u15','u16':'setp2p_u16','u17':'setp2p_u17','u19':'setp2p_u19',
            'u2':'setp2p_u2','u20':'setp2p_u20','u21':'setp2p_u21','u22':'setp2p_u22','u23':'setp2p_u23','u24':'setp2p_u24','u25':'setp2p_u25','u26':'setp2p_u26',
            'u27':'setp2p_u27','u28':'setp2p_u28','u29':'setp2p_u29','u3':'setp2p_u3','u30':'setp2p_u30','u31':'setp2p_u31','u32':'setp2p_u32','u33':'setp2p_u33',
            'u34':'setp2p_u34','u35':'setp2p_u35','u36':'setp2p_u36','u37':'setp2p_u37','u38':'setp2p_u38','u39':'setp2p_u39','u4':'setp2p_u4','u40':'setp2p_u40',
            'u41':'setp2p_u41','u42':'setp2p_u42','u43':'setp2p_u43','u44':'setp2p_u44','u45':'setp2p_u45','u46':'setp2p_u46','u47':'setp2p_u47','u48':'setp2p_u48',
            'u49':'setp2p_u49','u5':'setp2p_u5','u50':'setp2p_u50','u51':'setp2p_u51','u52':'setp2p_u52','u53':'setp2p_u53','u54':'setp2p_u54','u55':'setp2p_u55',
            'u56':'setp2p_u56','u57':'setp2p_u57','u58':'setp2p_u58','u59':'setp2p_u59','u6':'setp2p_u6','u60':'setp2p_u60','u61':'setp2p_u61','u62':'setp2p_u62',
            'u63':'setp2p_u63','u64':'setp2p_u64','u65':'setp2p_u65','u66':'setp2p_u66','u67':'setp2p_u67','u68':'setp2p_u68','u69':'setp2p_u69','u7':'setp2p_u7',
            'u70':'setp2p_u70','u71':'setp2p_u71','u72':'setp2p_u72','u73':'setp2p_u73','u74':'setp2p_u74','u75':'setp2p_u75','u76':'setp2p_u76','u77':'setp2p_u77',
            'u78':'setp2p_u78','u79':'setp2p_u79','u8':'setp2p_u8','u80':'setp2p_u80','u81':'setp2p_u81','u82':'setp2p_u82','u83':'setp2p_u83','u84':'setp2p_u84',
            'u85':'setp2p_u85','u86':'setp2p_u86','u87':'setp2p_u87','u9':'setp2p_u9'}
            savedict={}
            for item in partialdict:
                savedict[partialdict[item]]=tosave[item]

            #union this with savedict and keep overlapping values of saveedict
            fulldict  = self.get_all_parameters()
            newdict = dict(fulldict,**savedict)
            with open(config_file,'w') as f:      
                for item in newdict:
                    printer = item+'='+ str(newdict[item]) +'\n'
                    f.write(printer)
            
        except:
            print'save setp2p broken!'
            pass  
            
    def get_setip_parameters(self):
        try: 
            partialdict = {}
            with open(config_file)as f:
                for line in f:
                    param, value = line.strip().split('=', 1) 
                    #print param,value
                    if "setip_" in param:                     
                        fun, query=param.split('_',1)
                        partialdict[query] = value
                return partialdict
        except:
            pass             
            
    def save_setip_parameters(self,tosave):
        try:
            partialdict={'g':'setip_g','i':'setip_i','m':'setip_m',
            's':'setip_s','n':'setip_n', 'dhcp':'setip_dhcp'}
            savedict={}
            for item in partialdict:
                savedict[partialdict[item]]=tosave[item]

            #union this with savedict and keep overlapping values of saveedict
            fulldict  = self.get_all_parameters()
            newdict = dict(fulldict,**savedict)
            with open(config_file,'w') as f:      
                for item in newdict:
                    printer = item+'='+ str(newdict[item]) +'\n'
                    f.write(printer)
            
        except:
            print'setip save broken'
            pass  
            
    def get_setsmtp_parameters(self):
        try: 
            partialdict = {}
            with open(config_file)as f:
                for line in f:
                    param, value = line.strip().split('=', 1) 
                    #print param,value
                    if "setsmtp_" in param:                     
                        fun, query=param.split('_',1)
                        partialdict[query] = urllib.unquote_plus(value)
                return partialdict
        except:
            pass             
            
    def save_setsmtp_parameters(self,tosave):
        try:
            partialdict={'c':'setsmtp_c','e':'setsmtp_e','i':'setsmtp_i','n':'setsmtp_n',
            'p':'setsmtp_p','s':'setsmtp_s','t':'setsmtp_t','u':'setsmtp_u','a':'setsmtp_a'}
            savedict={}
            for item in partialdict:
                savedict[partialdict[item]]=tosave[item]

            #union this with savedict and keep overlapping values of saveedict
            fulldict  = self.get_all_parameters()
            newdict = dict(fulldict,**savedict)
            with open(config_file,'w') as f:      
                for item in newdict:
                    printer = item+'='+ str(newdict[item]) +'\n'
                    f.write(printer)
            
        except:
            print'save setsmtp is broken!'
            pass  
            
    def get_aadvanced_parameters(self):
        try: 
            partialdict = {}
            with open(config_file)as f:
                for line in f:
                    param, value = line.strip().split('=', 1) 
                    #print param,value
                    if "advanced_" in param:                     
                        fun, query=param.split('_',1)
                        partialdict[query] = value
                return partialdict
        except:
            pass             
            
    def save_aadvanced_parameters(self,tosave):
        try:
            partialdict={'a':'advanced_a','b':'advanced_b','c':'advanced_c','d':'advanced_d',
            'e':'advanced_e','f':'advanced_f','g':'advanced_g',}
            savedict={}
            for item in partialdict:
                savedict[partialdict[item]]=tosave[item]

            #union this with savedict and keep overlapping values of saveedict
            fulldict  = self.get_all_parameters()
            newdict = dict(fulldict,**savedict)
            with open(config_file,'w') as f:      
                for item in newdict:
                    printer = item+'='+ str(newdict[item]) +'\n'
                    f.write(printer)
            
        except:
            pass  
            
    def get_aadvanced_parameters(self):
        try: 
            partialdict = {}
            with open(config_file)as f:
                for line in f:
                    param, value = line.strip().split('=', 1) 
                    #print param,value
                    if "advanced_" in param:                     
                        fun, query=param.split('_',1)
                        partialdict[query] = value
                return partialdict
        except:
            pass             
            
    def save_aadvanced_parameters(self,tosave):
        try:
            partialdict={'a':'advanced_a','b':'advanced_b','c':'advanced_c','d':'advanced_d',
            'e':'advanced_e','f':'advanced_f','g':'advanced_g',}
            savedict={}
            for item in partialdict:
                savedict[partialdict[item]]=tosave[item]

            #union this with savedict and keep overlapping values of saveedict
            fulldict  = self.get_all_parameters()
            newdict = dict(fulldict,**savedict)
            with open(config_file,'w') as f:      
                for item in newdict:
                    printer = item+'='+ str(newdict[item]) +'\n'
                    f.write(printer)
            
        except:
            pass  
            
    def get_aadvanced_parameters(self):
        try: 
            partialdict = {}
            with open(config_file)as f:
                for line in f:
                    param, value = line.strip().split('=', 1) 
                    #print param,value
                    if "advanced_" in param:                     
                        fun, query=param.split('_',1)
                        partialdict[query] = value
                return partialdict
        except:
            pass             
            
    def save_aadvanced_parameters(self,tosave):
        try:
            partialdict={'a':'advanced_a','b':'advanced_b','c':'advanced_c','d':'advanced_d',
            'e':'advanced_e','f':'advanced_f','g':'advanced_g',}
            savedict={}
            for item in partialdict:
                savedict[partialdict[item]]=tosave[item]

            #union this with savedict and keep overlapping values of saveedict
            fulldict  = self.get_all_parameters()
            newdict = dict(fulldict,**savedict)
            with open(config_file,'w') as f:      
                for item in newdict:
                    printer = item+'='+ str(newdict[item]) +'\n'
                    f.write(printer)
            
        except:
            pass  
            
    def get_aadvanced_parameters(self):
        try: 
            partialdict = {}
            with open(config_file)as f:
                for line in f:
                    param, value = line.strip().split('=', 1) 
                    #print param,value
                    if "advanced_" in param:                     
                        fun, query=param.split('_',1)
                        partialdict[query] = value
                return partialdict
        except:
            pass             
            
    def save_aadvanced_parameters(self,tosave):
        try:
            partialdict={'a':'advanced_a','b':'advanced_b','c':'advanced_c','d':'advanced_d',
            'e':'advanced_e','f':'advanced_f','g':'advanced_g',}
            savedict={}
            for item in partialdict:
                savedict[partialdict[item]]=tosave[item]

            #union this with savedict and keep overlapping values of saveedict
            fulldict  = self.get_all_parameters()
            newdict = dict(fulldict,**savedict)
            with open(config_file,'w') as f:      
                for item in newdict:
                    printer = item+'='+ str(newdict[item]) +'\n'
                    f.write(printer)
            
        except:
            pass  
