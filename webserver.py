#~2012 Capt Bob Jaromin
#~python webserver to emulate a koyo directlogic device

import sys
import os
import pynetlinux
import fcntl

#check for required libraries in ../../lib
base_dir = os.path.dirname(__file__) or '.'
lib_dir = os.path.join(base_dir, '../lib')
sys.path.insert(0, lib_dir)

import string,time, logging, socket, struct, subprocess
from cgi import parse_header
from os import curdir, sep
from BaseHTTPServer import BaseHTTPRequestHandler, HTTPServer
import urlparse
from urlparse import urlparse, parse_qs, parse_qsl
from string import Template,upper
import re
import urllib 
from configfileio import configfileio

import cProfile

PYNETIFCONF = pynetlinux.ifconfig.Interface
# Configuration:
iface = 'eth6'
logging = 0
HPID = 222

#syslog stuff:
import syslog
#logging.getLogger("scapy").setLevel(1)

syslog.openlog("HP", syslog.LOG_CONS, syslog.LOG_LOCAL0)
syslog.syslog(syslog.LOG_ALERT, "ID(%s) web:Webserver Started.  Listening on Port 80..." % HPID)

#urllib.unquote_plus: removes + signs from url encoding

 

#may not need to worry about the quote, unquote...looks like it is robust 
# to spaces because of the 1 in "split(' ', 1)"


class MyHandler(BaseHTTPRequestHandler):
    
    
    # Override.  Disable logging DNS lookups to speed up response
    def address_string(self):
        return str(self.client_address[0])
    # Override.  Logs messages to syslog
    def log_message(self, format, *args):
        try:
            if(logging ==1):
                syslog.syslog(syslog.LOG_ALERT, "ID(%s) web:%s %s" % (HPID,self.address_string(), format%args)) #end of the road, excption triggered	
        #sys.stderr.write("%s - - [%s] %s\n" %                     (self.address_string(), self.log_date_time_string(), format%args))
        except:
            pass
        
    def do_GET(self):
        
        
        self.client_address
        configio = configfileio()
        
        global_template = {'dollarsign':'$'}
        submit ={'submit_btn':'<INPUT type="submit" value="Send">'}
        no_submit ={'submit_btn':''}
        global_params = configio.get_global_parameters()
        
        if global_params['submit'] == 'yes':
            global_template.update(submit)
        else:
            global_template.update(no_submit)

        if global_params['webenabled'] == 'no':
            return
        
        
        #fulldict=configio.get_all_parameters()
        
        req = urlparse(self.path)
        self.log_message("1-%s", self.requestline)
        #print 'Request: '+ self.requestline
        try:

       
            #default page
            if req.path=="/" or req.path=="":
                
                
                f = open(curdir + sep + 'index.html')  
                template = Template(f.read())
                main_index = configio.get_all_parameters() 
                pophtml= template.substitute(main_index)   

                self.wfile.write('HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n'+ pophtml)
                self.close_connection = 1   #because http 1.1, not closed automatically       
                
                


                f.close()
            #favicon 
            if req.path==("/favicon.ico"):
                f = open(curdir + sep + self.path,'rb')
                self.close_connection = 1   #because http 1.1, not closed automatically           
                self.send_response(200)
                self.end_headers()
                self.wfile.write(f.read())
                f.close()
            #other .html pages
            if self.path.endswith(".html"):
            #update if not end in .html
                
                if "/getp2p" in req.path:
					if req.path=="/getp2p0.html":
						f = open(curdir + sep + '/getp2p0.html')
					else: 
						f = open(curdir + sep + '/getp2p1.html')
						##############################
						setp2p = configio.get_setp2p_parameters()
						#build template replacement dictionary (more complicated than the others)
						offset = int(req.path[7]) * 8
						if int(req.path[7]) !=0:
							offset -= 1 #kludge to get right numbers to show up on template

						if req.path[8] == '0':
							offset = 79     #avoid interpreting '10' as '1'
						if (offset > 80) or (offset < 0):
							print'failed!!! offset is '+ offset
						
						getp2pdict={}
						for dex1 in range(1, 9):
							#replaces the ${} delimters in the getp2p template
							getp2pdict['d'+ str(dex1)]=str(dex1+offset)
							getp2pdict['i'+ str(dex1)]='i' +str(dex1+offset)
							getp2pdict['r'+ str(dex1)]='r' +str(dex1+offset)
							getp2pdict['u'+ str(dex1)]='u' +str(dex1+offset)  
							getp2pdict['pn'+ str(dex1)]='p' +str(dex1+offset)
							#replaces the data
							getp2pdict['ii'+ str(dex1)]=setp2p['i' +str(dex1+offset)]
							getp2pdict['rr'+ str(dex1)]=setp2p['r' +str(dex1+offset)]
							getp2pdict['uu'+ str(dex1)]=setp2p['u' +str(dex1+offset)]
							getp2pdict['p'+ str(dex1)+'_0']=setp2p['p' +str(dex1+offset)+'_0']
							getp2pdict['p'+ str(dex1)+'_1']=setp2p['p' +str(dex1+offset)+'_1']
						
						pophtml= template.substitute(dict(global_template,**getp2pdict))
						########################################
                else:
                    f = open(curdir + sep + self.path)               

                #get the .html template that we will populate with the data
                template = Template(f.read())
                
                #populate template based on what data is displayed on screen
                if not (("getp2p" or "testsmtp" or "main" or "index" or "peerlink") in req.path): 
                    tempVar = configio.get_parameters(req.path[1:-5])
                    pophtml= template.substitute(dict(global_template,**tempVar))
                # elif req.path=="/setname.html":
                    # setname = configio.get_setname_parameters()                           
                    # pophtml= template.substitute(dict(global_template,**setname))
                # elif req.path=="/setdesc.html":
                    # setdesc = configio.get_setdesc_parameters() 
                    # pophtml= template.substitute(dict(global_template,**setdesc))
                # elif req.path=="/setip.html":
                    # setip = configio.get_setip_parameters() 
                    # pophtml= template.substitute(dict(global_template,**setip))                    
                # elif req.path=="/setsmtp.html":
                    # setsmtp = configio.get_setsmtp_parameters() 
                    # pophtml= template.substitute(dict(global_template,**setsmtp))                    
                elif req.path=="/testsmtp.html": 
                    pophtml= template.substitute(global_template)
                # elif "/getp2p" in req.path:
                    # setp2p = configio.get_setp2p_parameters()
                    ##build template replacement dictionary (more complicated than the others)
                    # offset = int(req.path[7]) * 8
                    # if int(req.path[7]) !=0:
                        # offset -= 1 #kludge to get right numbers to show up on template

                    # if req.path[8] == '0':
                        # offset = 79     #avoid interpreting '10' as '1'
                    # if (offset > 80) or (offset < 0):
                        # print'failed!!! offset is '+ offset
                    
                    # getp2pdict={}
                    # for dex1 in range(1, 9):
                        ##replaces the ${} delimters in the getp2p template
                        # getp2pdict['d'+ str(dex1)]=str(dex1+offset)
                        # getp2pdict['i'+ str(dex1)]='i' +str(dex1+offset)
                        # getp2pdict['r'+ str(dex1)]='r' +str(dex1+offset)
                        # getp2pdict['u'+ str(dex1)]='u' +str(dex1+offset)  
                        # getp2pdict['pn'+ str(dex1)]='p' +str(dex1+offset)
                        ##replaces the data
                        # getp2pdict['ii'+ str(dex1)]=setp2p['i' +str(dex1+offset)]
                        # getp2pdict['rr'+ str(dex1)]=setp2p['r' +str(dex1+offset)]
                        # getp2pdict['uu'+ str(dex1)]=setp2p['u' +str(dex1+offset)]
                        # getp2pdict['p'+ str(dex1)+'_0']=setp2p['p' +str(dex1+offset)+'_0']
                        # getp2pdict['p'+ str(dex1)+'_1']=setp2p['p' +str(dex1+offset)+'_1']
                    
                    # pophtml= template.substitute(dict(global_template,**getp2pdict))
                    
                # elif req.path=="/advanced.html":
                    # advanced = configio.get_advanced_parameters()
                    # pophtml= template.substitute(dict(global_template,**advanced))
                # elif req.path=="/setupsmtp.html":
                    # setsmtp = configio.get_setsmtp_parameters()
                    # pophtml= template.substitute(dict(global_template,**setsmtp))
                elif req.path=="/peerlink.html":
                    peerlink = configio.get_peerlink_parameters()
                    pophtml= template.substitute(dict(global_template,**peerlink))                    
                elif req.path=="/main.html" or req.path=="/index.html":
                    main_index = configio.get_all_parameters() 
                    pophtml= template.substitute(main_index)   
                else:				
                    #sleep emulates the PLC DoS upon bad URL
                    #print'sleep 60 a'
                    #time.sleep(60)
                    return            


                #self.send_response(200)
                #self.send_header('Content-type',	'text/html')
                #self.end_headers()
            
                self.wfile.write('HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n'+ pophtml)
                self.close_connection = 1   #because http 1.1, not closed automatically       
                #self.wfile.write('<HTML><HEAD><TITLE>${title}</TITLE></HEAD><CENTER>${message}</CENTER><BR><CENTER><B><A HREF=main.html>Return to main setup page.</A></B></CENTER><BR></BODY></HTML>')
                


                f.close()
                
                return
            if req.query!= "":        
                if global_params['submit']=='no':
                   self.close_connection = 1   #because http 1.1, not closed automatically
                   self.send_response(200)
                   self.send_header('Content-type',	'text/html')
                   self.end_headers()

                   f = open(curdir + sep +'/complete.html')
                   template = Template(f.read())
                   pophtml = template.substitute({'title':'http://IP'+req.path+ req.query,'message':'Currently in Read-Only Mode.'})
                   self.wfile.write(pophtml)
                   f.close() 
                
                #if have new user input, save data to file and display success
                elif req.path=="/setid.html":          
                    setid = configio.get_setid_parameters()                              
                    setid['n']= dict(parse_qsl(req.query))['n']
                    setid['h']= '0x'+string.upper(str("{0:x}".format(int(setid['n']))))
                    configio.save_setid_parameters(setid)                                        
                    #map(lambda x:x.upper(),[])
                    
                    self.close_connection = 1   #because http 1.1, not closed automatically                   
                    self.send_response(200)
                    self.send_header('Content-type',	'text/html')
                    self.end_headers()
                    self.log_message(" +PARAMS: ID=%s", dict(parse_qsl(req.query))['n'])
                    
                    f = open(curdir + sep +'/complete.html')
                    template = Template(f.read())
                    pophtml = template.substitute({'title':'Set Module ID','message':'Setup Module ID complete.'})
                    self.wfile.write(pophtml) 
                    f.close()                  
                
                elif req.path=="/setname.html":
                    setname = configio.get_setname_parameters()                                        
                    setname['n']= (dict(parse_qsl(req.query))['n'])
                    
                    configio.save_setname_parameters(setname)                                        
                                       
                    self.close_connection = 1   #because http 1.1, not closed automatically                   
                    self.send_response(200)
                    self.send_header('Content-type',	'text/html')
                    self.end_headers()
                    self.log_message(" +PARAMS: NAME=%s", dict(parse_qsl(req.query))['n'])
                    
                    
                    f = open(curdir + sep +'/complete.html')
                    template = Template(f.read())
                    pophtml = template.substitute({'title':'Set Module Name','message':'Module Name setup complete.'})
                    self.wfile.write(pophtml) 
                    f.close()   

                
                elif req.path=="/setdesc.html":
                    setdesc = configio.get_setdesc_parameters()                                        
                    setdesc['n']= (dict(parse_qsl(req.query))['n'])
                    configio.save_setdesc_parameters(setdesc)                                        
                    
                    self.close_connection = 1   #because http 1.1, not closed automatically                   
                    self.send_response(200)
                    self.send_header('Content-type',	'text/html')
                    self.end_headers()
                    self.log_message(" +PARAMS: DESCRIP=%s", dict(parse_qsl(req.query))['n'])
                    
                    f = open(curdir + sep +'/complete.html')
                    template = Template(f.read())
                    pophtml = template.substitute({'title':'Set Module Description','message':'Module Description setup complete.'})
                    self.wfile.write(pophtml)   
                    f.close() 
                                    
                #need to implement DHCP stuff here!!
                elif req.path=="/setip.html":
                    setip = configio.get_setip_parameters()
                    
                                                            
                    for item in dict(parse_qsl(req.query)):
                        setip[item]= (dict(parse_qsl(req.query))[item])                            
                        print item,(dict(parse_qsl(req.query))[item]) 
                                
                            
                    if (dict(parse_qsl(req.query))['m']) == '1':
                        #config by DHCP
                        setip['dhcp'] = 1                                
                        setip['n']='checked'
                        setip['m']=""
                        
                        #get new ipaddress via dhcp
                        #subprocess.call(["dhclient", iface,"-r"])
                        #subprocess.call(["dhclient", iface,"-nw"])
                        print'at 1'
                        #refresh all ip values 
                        setip['i'] = PYNETIFCONF(iface).get_ip()
                        setip['g'] = pynetlinux.route.get_default_gw()
                        setip['s'] = socket.inet_ntoa(struct.pack(">L", (1<<32) - (1<<32>>PYNETIFCONF(iface).get_netmask())))
                        global_params['ip'] = PYNETIFCONF(iface).get_ip()
                        global_params['gateway']=pynetlinux.route.get_default_gw()
                        global_params['subnet']=socket.inet_ntoa(struct.pack(">L", (1<<32) - (1<<32>>PYNETIFCONF(iface).get_netmask())))

                        
                        
                        #save values to config file
                        global_params['suppliedbydhcp'] = '(Supplied by DHCP)' 
                        configio.save_setip_parameters(setip)                             
                        configio.save_global_parameters(global_params)                              
                        
                        self.close_connection = 1   #because http 1.1, not closed automatically                   
                        self.send_response(200)
                        self.send_header('Content-type',	'text/html')
                        self.end_headers()
                        
                        #special complete message for DHCP
                        f = open(curdir + sep +'/ipcomplete.html')
                        template = f.read()
                        self.wfile.write(template)
                        f.close()     

                                                        
                    elif (dict(parse_qsl(req.query))['m']) == '0':

                        #manual config      
                        setip['dhcp'] = 0                              
                        setip['n']=""
                        setip['m']='checked'
                        #set ip and subnet of gumstix (dont know how to set gateway manually...)
                        print 'ip',setip['i']
                        print 'gateway',setip['g']
                        
                        try:
                            print 'subnet', PYNETIFCONF(iface).set_netmask(int(get_netmask(setip['i'], setip['s'])))
                        except:
                            print 'bad subnet', setip['s']
                        
                        
                        #TODO:fix setting IPs:
                        
                        
                        
                        
                        try:
                            PYNETIFCONF(iface).set_ip(setip['i'])
                            PYNETIFCONF(iface).set_netmask(int(get_netmask(setip['i'], setip['s'])))
                        except:
                            pass       
                        #set globals to reflect same as setip vars
                        global_params['ip'] = PYNETIFCONF(iface).get_ip()
                        global_params['gateway']=setip['g']
    #                               global_params['subnet']=socket.inet_ntoa(struct.pack(">L", (1<<32) - (1<<32>>PYNETIFCONF(iface).get_netmask())))
                        global_params['subnet'] = setip['s'] # want to save subnet even if it is bad... to mimic PLC
                        global_params['suppliedbydhcp'] = ''
                        
                        configio.save_global_parameters(global_params)
                        configio.save_setip_parameters(setip)                                     
                        self.log_message(" +PARAMS: DHCP=%d IP=%s SUBNET=%s GATEWAY=%s", setip['dhcp'],dict(parse_qsl(req.query))['i'],dict(parse_qsl(req.query))['s'], dict(parse_qsl(req.query))['g'])
                        
                        self.close_connection = 1   #because http 1.1, not closed automatically                   
                        self.send_response(200)
                        self.send_header('Content-type',	'text/html')
                        self.end_headers()
                        f = open(curdir + sep +'/complete.html')
                        template = Template(f.read())
                        pophtml = template.substitute({'title':'Set IP Configuration','message':'Setup IP Configuration complete.'})
                        self.wfile.write(pophtml)
                        f.close()  
   
                
                elif req.path=="/setsmtp.html":                                  
                    setsmtp = configio.get_setsmtp_parameters()
                    #must treat fields and checkbox differently here because they can be blank
                    # and are only in URL if present
                    for item in setsmtp:
                        if item != 'p' and item != 't':                                       
                            setsmtp[item]= "\"\""                 
                    
                    for item in dict(parse_qsl(req.query)):                                        
                        if item != 'a':
                            setsmtp[item]= (dict(parse_qsl(req.query))[item])                                                 
                        elif item == 'a':
                            setsmtp[item]= 'checked'
                    
                    #handle blank input cases here
                    if ( setsmtp['n']== "\"\""):
                        global_params['email_sendername']='\"-- undefined --\"'
                    else:
                        global_params['email_sendername'] = setsmtp['n'] 
                    
                    
                    if  (setsmtp['e']== "\"\""):
                        global_params['email_senderaddr']='-- undefined --'
                    else:
                        global_params['email_senderaddr'] = setsmtp['e']                                
                    
                    
                    if ( setsmtp['c']== "\"\""):    
                        global_params['email_cc']='-- undefined --'
                    else:
                        global_params['email_cc'] = setsmtp['c']     
                    
                    
                    if (setsmtp['i']=="\"\""):    
                        global_params['email_serverip']='0.0.0.0'
                    else:
                        global_params['email_serverip']=setsmtp['i']
                    
                  
                    #if (setsmtp['t']=="\"\""):    
                    #    global_params['email_timeout']= '60'
                    #    setsmtp['t']= '60'
                    #else:
                    global_params['email_timeout']=setsmtp['t']

                    
                    #if (setsmtp['p']=="\"\""):    
                    #    global_params['email_port']='25'
                    #   setsmtp['p']= '25'
                    #else:
                    global_params['email_port']=setsmtp['p'] 
                                  
                    
                    configio.save_global_parameters(global_params)
                    configio.save_setsmtp_parameters(setsmtp)


                    self.close_connection = 1   #because http 1.1, not closed automatically                   
                    self.send_response(200)
                    self.send_header('Content-type',	'text/html')
                    self.end_headers()
                    self.log_message(" +PARAMS: SVR_IP=%s SENDR_NAME=%s SENDR_ADDR=%s CC=%s PORT=%s TOUT=%s AUTH=%s UN=%s PW=%s", dict(parse_qsl(req.query))['i'],
                    dict(parse_qsl(req.query))['n'],dict(parse_qsl(req.query))['e'],dict(parse_qsl(req.query))['c'],dict(parse_qsl(req.query))['p'],
                    dict(parse_qsl(req.query))['t'],setsmtp['a'],dict(parse_qsl(req.query))['u'],dict(parse_qsl(req.query))['s'])
                   
                  
                    f = open(curdir + sep +'/complete.html')
                    template = Template(f.read())
                    pophtml = template.substitute({'title':'Set EMail Configuration','message':'Setup EMail Configuration complete.'})
                    self.wfile.write(pophtml) 
                    f.close() 

                
                #TODO:  implement EMAIL stuff here!!
                elif req.path=="/testsmtp.html":                                    
                    
                    
                    if (dict(parse_qsl(req.query))['n'])!= '':
                        test_email_addr= (dict(parse_qsl(req.query))['n'])                                   
                        
                        # logic here to check that settings from setsmtp are correct
                        # logic here to send email

                        
                        self.close_connection = 1   #because http 1.1, not closed automatically                   
                        self.send_response(200)
                        self.send_header('Content-type',	'text/html')
                        self.end_headers()
                        self.log_message(" +PARAMS: EMAILSEND_ADDR=%s", dict(parse_qsl(req.query))['n'])

                        f = open(curdir + sep +'/complete.html')
                        template = Template(f.read())           #fake data here                            and here
                        pophtml = template.substitute({'title':'Send Test EMail','message':'TestEMail has been sent.'})

                        self.wfile.write(pophtml)  
                        f.close() 
                    else:
                        #do something here based on real response from PLC
                        pass    

                elif req.path=="/advanced.html":
                    advanced = configio.get_advanced_parameters()
                    #must handle checkboxes (queries g, h, i) differently 
                    # because they are only in URL if they are a 'checked'
                    advanced['g'] = "\'\'"
                    advanced['h'] = "\'\'"
                    advanced['i'] = "\'\'"
                    
                    #print'global params bob', global_params
                    for item in dict(parse_qsl(req.query)):
                            if item != 'g' and item !='h' and item !='i':
                                advanced[item]= (dict(parse_qsl(req.query))[item])                            
                            elif item == 'h':
                                #readonly web option selected
                                #No web mechanism to turn OFF readonly mode
                                advanced['readonly_web_checkmark'] = 'DISABLED'
                                global_params['submit'] = 'no'
                                #print'submit should now be off', global_params['submit']
                                configio.save_global_parameters(global_params) 
                                advanced[item]= 'checked'
                            else:
                                advanced[item]= 'checkedcalcDottedNetmask'
                                   
                    configio.save_advanced_parameters(advanced)
                                      
                    self.close_connection = 1   #because http 1.1, not closed automatically                   
                    self.send_response(200)
                    self.send_header('Content-type',	'text/html')
                    self.end_headers()
                    self.log_message(" +PARAMS: ACK_TOUT=%s RESP_TOUT=%s RTRIES=%s KSEQ_RTRIES=%s MODMST_TOUT=%s MODSLV_TOUT=%s FOR_10T=%s WEB_RONLY=%s DIP=%s", 
                     dict(parse_qsl(req.query))['a'],dict(parse_qsl(req.query))['b'],dict(parse_qsl(req.query))['c'],dict(parse_qsl(req.query))['d'],
                     dict(parse_qsl(req.query))['e'],dict(parse_qsl(req.query))['f'],advanced['g'],advanced['h'],advanced['i'])

                    f = open(curdir + sep +'/complete.html')
                    template = Template(f.read())
                    pophtml = template.substitute({'title':'Set ECOM Advanced Settings','message':'ECOM Advanced Setup complete.'})
                    self.wfile.write(pophtml)  
                    f.close() 

                elif req.path=="/peerlink.html":
                    peerlink = configio.get_peerlink_parameters()
                    #must handle checkbox (query e) differently
                    # because it is only in URL if it is 'checked'
                    peerlink['e'] = "\'\'"
                    for item in dict(parse_qsl(req.query)):                                        
                        if item != 'e':
                            peerlink[item]= (dict(parse_qsl(req.query))[item])                 
                        elif item == 'e':
                            peerlink[item]= 'checked'
                    
                    configio.save_peerlink_parameters(peerlink)                                        
                    
                    self.close_connection = 1   #because http 1.1, not closed automatically                   
                    self.send_response(200)
                    self.send_header('Content-type',	'text/html')
                    self.end_headers()
           
                    self.log_message(" +PARAMS: VMEM=%s ENAB=%s BK0=%s BK1=%s BK2=%s BK3=%s BK4=%s BK5=%s BK6=%s BK7=%s BK8=%s BK9=%s BK10=%s BK11=%s BK12=%s BK13=%s BK14=%s BK15=%s", 
                    peerlink['a'], peerlink['e'],dict(parse_qsl(req.query))['b0'],dict(parse_qsl(req.query))['b1'],dict(parse_qsl(req.query))['b2'],
                    dict(parse_qsl(req.query))['b3'],dict(parse_qsl(req.query))['b4'],dict(parse_qsl(req.query))['b6'],dict(parse_qsl(req.query))['b6'],
                    dict(parse_qsl(req.query))['b7'],dict(parse_qsl(req.query))['b8'],dict(parse_qsl(req.query))['b9'],dict(parse_qsl(req.query))['b10'], 
                    dict(parse_qsl(req.query))['b11'],dict(parse_qsl(req.query))['b12'],dict(parse_qsl(req.query))['b13'],dict(parse_qsl(req.query))['b14'], 
                    dict(parse_qsl(req.query))['b15'])
                                        
                    f = open(curdir + sep +'/complete.html')
                    template = Template(f.read())
                    pophtml = template.substitute({'title':'Set Peerlink Settings','message':'Peerlink Setup Complete.'})
                    self.wfile.write(pophtml)  
                    f.close() 
                    
                elif 'setp2p' in req.path:
                    setp2p = configio.get_setp2p_parameters()
                    log_string = ''
                    #print 'setp2p[i7] before is',setp2p[i7]
                    for item in dict(parse_qsl(req.query)):                                                               
                        log_string += item+'=' +setp2p[item]+' '
                        setp2p[item]= (dict(parse_qsl(req.query))[item])                  
                                                
                    #print 'setp2p[i7] after is',setp2p[i7]    
                    configio.save_setp2p_parameters(setp2p)                                        
                    
                    self.close_connection = 1   #because http 1.1, not closed automatically                   
                    self.send_response(200)
                    self.send_header('Content-type',	'text/html')
                    self.end_headers()
                    self.log_message(" +PARAMS: %s", log_string)
                    
                    f = open(curdir + sep +'/complete.html')
                    template = Template(f.read())
                    pophtml = template.substitute({'title':'Set Peerlink Settings','message':'Peerlink Setup Complete.'})
                    self.wfile.write(pophtml)  
                    f.close()     
                    
                    
                elif req.path=="/main.html" or req.path=="/index.html":                                        
                    setid['n']= (dict(parse_qsl(req.query))['n'])
                    configio.save_setid_parameters(setid)                                        
                    
                    self.close_connection = 1   #because http 1.1, not closed automatically                   
                    self.send_response(200)
                    self.send_header('Content-type',	'text/html')
                    self.end_headers()
                    
                    
                    
                    
                    
                    f = open(curdir + sep +'/complete.html')
                    template = Template(f.read())
                    pophtml = template.substitute({'title':'Set Peer to Peer','message':'Peer to Peer setup complete.'})
                    self.wfile.write(pophtml)  
                    f.close() 
                else:
                    pass
                    #this is commented out because the actual PLC sends no 404 replys, it just times out!
                    #print'file not found 2'
                    #self.send_error(404,'File Not Found: %s' % self.path)
                    #print'sleep 60 b'
                    #time.sleep(60)	#sleep emulates the PLC DoS upon bad URL
                    
                
                return
    
            else:
                pass
                #this is commented out because the actual PLC sends no 404 replys, it just times out!
                #print'file not found 3'
                #self.send_error(404,'File Not Found: %s' % self.path)
                #print'sleep 60 c'
                #time.sleep(60)	#sleep emulates the PLC DoS upon bad URL
                
        except:
            
                #this is commented out because the actual PLC sends no 404 replys, it just times out!
                #print'exception file not found 4'
                #self.send_error(404,'File Not Found: %s' % self.path)
                print'exception occured!'
                #time.sleep(60)	#sleep emulates the PLC DoS upon bad URL
                raise
            
    def do_POST(self):
        global rootnode
        try:
            ctype, pdict = cgi.parse_header(self.headers.getheader('content-type'))
            if ctype == 'multipart/form-data':
                query=cgi.parse_multipart(self.rfile, pdict)
            self.send_response(301)
            
            self.end_headers()
            upfilecontent = query.get('upfile')
            print "filecontent", upfilecontent[0]
            self.wfile.write("<HTML>POST OK.<BR><BR>");
            self.wfile.write(upfilecontent[0]);
            
        except :
            raise
 
 
def run_ini():
        configio = configfileio()
        setip = configio.get_setip_parameters()
        global_params = configio.get_global_parameters()
        setip['i'] = PYNETIFCONF(iface).get_ip()
        if (pynetlinux.route.get_default_gw() != None):
            print pynetlinux.route.get_default_gw()
            setip['g'] = pynetlinux.route.get_default_gw()
        setip['s'] = socket.inet_ntoa(struct.pack(">L", (1<<32) - (1<<32>>PYNETIFCONF(iface).get_netmask())))
        global_params['ip'] = PYNETIFCONF(iface).get_ip()
        #global_params['gateway']=pynetlinux.route.get_default_gw()        
        global_params['subnet']=socket.inet_ntoa(struct.pack(">L", (1<<32) - (1<<32>>PYNETIFCONF(iface).get_netmask())))

        global_params['gateway']=setip['g']

        #retrieve actual MAC address from interface, and store it in the config file      
        global_params['mac'] = getHwAddr(iface)
        
        configio.save_setip_parameters(setip)                             
        configio.save_global_parameters(global_params)  
             

def getHwAddr(ifname):
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    info = fcntl.ioctl(s.fileno(), 0x8927,  struct.pack('256s', ifname[:15]))
    return ''.join(['%02X ' % ord(char) for char in info[18:24]])[:-1]


def main():
    try:
        run_ini()
        server = HTTPServer(('', 80), MyHandler)
        print '+++++HTTP Server Started.  Listening on Port 80.'
        
           

#        self.log_message("Webserver Started.  Listening on Port 80...")

        server.serve_forever()
    except KeyboardInterrupt:
        print '^C received, shutting down server'
        server.socket.close()

def get_net_size(netmask):
    binary_str = ''
    for octet in netmask:
        binary_str += bin(int(octet))[2:].zfill(8)
    return str(len(binary_str.rstrip('0')))

def get_netmask(ip, sub):
    ipaddr = ip.split('.')
    netmask = sub.split('.')
    # return netmask
    return get_net_size(netmask)



if __name__ == '__main__':
    main()
    #cProfile.run('main()')

