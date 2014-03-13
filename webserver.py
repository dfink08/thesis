# 2014 Deanna Fink
# This file generates a dictionary of input fields from html pages gathered by wget. 
#	It writes this dictionary to a text file called configDict.txt
# Uses python 3.3.3

import sys, os, inspect

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

currentdir = os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe())))
parentdir = os.path.dirname(currentdir)
sys.path.insert(0, parentdir)
from variableFile import responses


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
		
	req = urlparse(self.path)
        self.log_message("1-%s", self.requestline)
        
        try:
	    
	    if req.path=="/" or req.path=="":
	      proc = subprocess.Popen(["file",req.path],stdout=subprocess.PIPE)
	      output = proc.stdout.read()
	    else: 
	      proc = subprocess.Popen(["file",req.path[1:]],stdout=subprocess.PIPE)
	      output = proc.stdout.read()
   
            #default page
            if req.path=="/" or req.path=="":
		f = open(curdir + sep + 'index.html')  
		
                #f = open(curdir + "/user/web"+sep + 'home.html')  
                fileText = f.read()
                template = Template(fileText)
                
                main_index = configio.get_all_parameters() 
                #pophtml= template.substitute(main_index) 
                pophtml = fileText

                self.wfile.write(responses[req.path]+pophtml)
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
			
	    #if self.path.endswith(".html") or self.path.endswith(".asp"):
	    if ("HTML" in output):
		f = open(curdir + sep + self.path)               
	        print self.path
		#get the .html template that we will populate with the data
		template = Template(f.read())
		if ("main" or "index") in req.path:
			main_index = configio.get_all_parameters() 
			print "Request from get_param: ",main_index
			pophtml= template.safe_substitute(main_index)
			  
		
		elif not (("main" or "index") in req.path):
			request = configio.get_parameters(req.path[1:-5]) 
			print "Request from get_param: ",request
			pophtml= template.safe_substitute(dict(global_template,**request))
			print "pophtml: ", pophtml
	    
		else:
		  print "The first part is broken"
		  return
		
		self.wfile.write(responses[req.path]+pophtml)
		
		self.close_connection = 1
		print "Closes the connection in the first part"
		f.close()	
	    
		return
		
	    if ("SGML" in output):
		f = open(curdir + sep + self.path)               

		#get the .html template that we will populate with the data
		template = Template(f.read())
		if ("main" or "index" or "home") in req.path:
			main_index = configio.get_all_parameters() 
			pophtml= template.safe_substitute(main_index)
		
		elif not (("main" or "index" or "home") in req.path):
			request = configio.get_parameters(req.path[1:-5]) 
			pophtml= template.safe_substitute(dict(global_template,**request))
				
	    
		else:
		  print "The first part is broken"
		  return
		
		self.wfile.write(responses[req.path]+pophtml)
		self.close_connection = 1
		f.close()	
	    
		return
		
	    if req.query!= "":   
		
		#if global_params['submit']=='no':
		    #closeAndSendHeader(self)

		    #f = open(curdir + sep +'/complete.html')
		    #template = Template(f.read())
		    #pophtml = template.substitute({'title':'http://IP'+req.path+ req.query,'message':'Currently in Read-Only Mode.'})
		    #self.wfile.write(pophtml)
		    #f.close() 
		
		if req.path.endswith('.html'):
		    
		    request, type = req.path.split('.',1)
		    requestDict = configio.get_parameters(request)
		    newDictionary = {}
		    #print "It makes the dict"
		    #print "request: ",request
		    # for input in parse_qsl(req.query):
			    # print "It gets to the parsing loop"
			    # requestDict[str(req.path+input[0])] = input[1]
			    # print str(req.path+input[0])
			    # print input[1]
				
		    for item in dict(parse_qsl(req.query)):
                        requestDict[request[1:]+'_'+item]= (dict(parse_qsl(req.query))[item])                            
                        
		    configio.save_parameters(requestDict)
		    
		    #closeAndSendHeader(self)
		    self.close_connection = 1   #because http 1.1, not closed automatically                   
		    self.send_response(200)
		    self.send_header('Content-type',	'text/html')
		    self.end_headers()
		    
		    #f = open(curdir + sep +'/complete.html')
		    #template = Template(f.read())
		    # pophtml = template.substitute({'title':'Set Module Description','message':'Module Description setup complete.'})
		    #self.wfile.write(pophtml)   
		    #f.close() 
		    
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
	    
	except:
	  print "Exception Occured!"
	  raise
				
    def do_POST(self):
	#print "It goes into do_POST"
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
			
def closeAndSendHeader(server):
	server.close_connection = 1   #because http 1.1, not closed automatically                   
	server.send_response(200)
	server.send_header('Content-type',	'text/html')
	server.end_headers()
	
def getHwAddr(ifname):
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    info = fcntl.ioctl(s.fileno(), 0x8927,  struct.pack('256s', ifname[:15]))
    return ''.join(['%02X ' % ord(char) for char in info[18:24]])[:-1]

def run_ini():
	configio = configfileio()
	#print "ConfigIO: ", configio
        setip = configio.get_parameters("setip")
        #print "SETIP ",setip
        #global_params = configio.get_parameters("global_")
        setip['setip_i'] = PYNETIFCONF(iface).get_ip()
        if (pynetlinux.route.get_default_gw() != None):
            print pynetlinux.route.get_default_gw()
            setip['setip_g'] = pynetlinux.route.get_default_gw()
        setip['setip_s'] = socket.inet_ntoa(struct.pack(">L", (1<<32) - (1<<32>>PYNETIFCONF(iface).get_netmask())))
        #global_params['ip'] = PYNETIFCONF(iface).get_ip()
        #global_params['gateway']=pynetlinux.route.get_default_gw()        
        #global_params['subnet']=socket.inet_ntoa(struct.pack(">L", (1<<32) - (1<<32>>PYNETIFCONF(iface).get_netmask())))

        #global_params['gateway']=setip['g']

        #retrieve actual MAC address from interface, and store it in the config file      
       # global_params['mac'] = getHwAddr(iface)
        
        configio.save_setip_parameters(setip)                             
        #configio.save_global_parameters(global_params)

def main():
    try:
        run_ini()
        server = HTTPServer(('', 80), MyHandler)
        print '+++++HTTP Server Started.  Listening on Port 80.'
      
		# self.log_message("Webserver Started.  Listening on Port 80...")

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
