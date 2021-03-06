#! /usr/bin/python

### Dustin Berman
### AFIT/ENG
### Masters of Cyber Operations, June 2012
### -Modified by Capt Bob Jaromin, Nov 2012

### File Information
### canary.py
### Emulates a PLC with the following commands: Read Coil, Read Discrete Inputs, Read Holding Registers, Read Input Registers, Write Single Coil, Write Single Register
### This will also log any connections to the syslog

# Imports
import logging, platform, random
import syslog
from struct import *
from bitstring import BitArray
import socket	


# Designed with Scapy 2.2.0
logging.getLogger("scapy").setLevel(1)
from scapy.all import *

# Display the version of Python and Scapy being used
#print "Python %s\tScapy %s" % (platform.python_version(), conf.version)

#Global Variables
numcoils = 100
numdinputs = 100
numinputregisters = 100
numholdregisters = 100
coil = ['0']*numcoils
dinputs = ['0']*numdinputs
inputregister = ['\x00\x00']*numinputregisters
holdregister = ['\x00\x00']*numholdregisters
vendorname = "Koyo"
productcode = "Directlogic405"
majorminorrevision = "V1.12.1"

HPID = '222'
logging = 0


# Dictionaries
# Need to add in all function codes here
function_code_enum = {1:"Read Coil", 2:"Read Discrete Inputs", 3:"Read Holding Registers", 4:"Read Input Registers", 5:"Write Single Coil", 6:"Write Single Register", 43:"Encapsulated Interface Transport"} 
function_code = {"Read Coil":1, "Read Discrete Inputs":2, "Read Holding Registers":3, "Read Input Registers":4, "Write Single Coil":5, "Write Single Register":6, "Encapsulated Interface Transport":43}

# Modbus Header
class Modbus(Packet):
	name = "Modbus"
	fields_desc = [ShortField("transaction", 0),
		ShortField("protocol", 0),
		ShortField("length", 0),
		ByteField("unit", 0),
		ByteEnumField("function", 1, function_code_enum)
		]

	# This will determine how to dissect the rest of the packet
	def guess_payload_class(self, payload):
		if self.function == function_code['Read Coil']:
			return ReadCoil
		elif self.function == function_code['Read Discrete Inputs']:
			return ReadDiscreteInputs
		elif self.function == function_code['Read Holding Registers']:
			return ReadHoldingRegisters
		elif self.function == function_code['Read Input Registers']:
			return ReadInputRegisters
		elif self.function == function_code['Write Single Coil']:
			return WriteSingleCoil
		elif self.function == function_code['Write Single Register']:
			return WriteSingleRegister
		elif self.function == function_code['Encapsulated Interface Transport']:
			return EncapsulatedInterfaceRequest
		else:
			return Packet.guess_payload_class(self,payload)

# Read Coil Payload
class ReadCoil(Packet):
	name= "ReadCoil"
	fields_desc = [ShortField("startcoil", 0),
		ShortField("quantitycoils", 0)
		]

# Read Coil Response Payload
class ReadCoilResponse(Packet):
	name= "ReadCoilResponse"
	fields_desc = [ByteField("bytecount", 0),
		StrField("status", "")
		]

# Read Discrete Inputs Payload
class ReadDiscreteInputs(Packet):
	name= "ReadDiscreteInputs"
	fields_desc = [ShortField("startinput", 0),
		ShortField("quantityinputs", 0)
		]

# Read Discrete Inputs Response Payloads
class ReadDiscreteInputsResponse(Packet):
	name= "ReadDiscreteInputsResponse"
	fields_desc = [ByteField("bytecount", 0),
		StrField("status", "")
		]

# Read Holding Registers Payload
class ReadHoldingRegisters(Packet):
	name= "ReadHoldingRegisters"
	fields_desc = [ShortField("startaddress", 0),
		ShortField("quantityregs", 0)
		]

# Read Holding Registers Response Payload
class ReadHoldingRegistersResponse(Packet):
	name= "ReadHoldingRegistersResponse"
	fields_desc = [ByteField("bytecount", 0),
		StrField("status", "")
		]

# Read Input Registers Payload
class ReadInputRegisters(Packet):
	name= "ReadInputRegisters"
	fields_desc = [ShortField("startaddress", 0),
		ShortField("quantityregs", 0)
		]

# Read Input Registers Response Payload
class ReadInputRegistersResponse(Packet):
	name= "ReadHoldingRegistersResponse"
	fields_desc = [ByteField("bytecount", 0),
		StrField("status", "")
		]

# Write Single Coil Payload
class WriteSingleCoil(Packet):
	name = "WriteSingleCoil"
	fields_desc = [ShortField("coilnumber", 0),
		ByteField("state", 0),
		ByteField("padding", 0)
		]

# Write Single Register Payload
class WriteSingleRegister(Packet):
	name= "WriteSingleRegister"
	fields_desc = [ShortField("regaddress", 0),
		ShortField("regvalue", 0)
		]

# Encapsulated Interface Transport Request Payload
class EncapsulatedInterfaceRequest(Packet):
	name= "EncapsulatedInterfaceRequest"
	fields_desc = [ByteField("meitype", 0),
		ByteField("deviceid", 1),
		ByteField("objectid", 0)
		]

# Encapsulated Interface Transport Response Payload
class EncapsulatedInterfaceResponse(Packet):
	name= "EncapsulatedInterfaceResponse"
	fields_desc = [ByteField("meitype", 0),
		ByteField("deviceid", 1),
		ByteField("conformity", 1),
		ByteField("morefollows", 0),
		ByteField("objectid", 0),
		ByteField("numobjects",0)
		]

# Encapsulated Interface Transport Object Payload
class EncapsulatedInterfaceObject(Packet):
	name= "EncapsulatedInterfaceObject"
	fields_desc = [ByteField("objectid", 0),
		ByteField("objectlength", 0),
		StrField("objectvalue","")
		]

# Error Payload
class Error(Packet):
	name= "Error"
	fields_desc = [ByteField("code", 1)
		]

# Bind Layers
bind_layers(TCP, Modbus, sport = 502)
bind_layers(TCP, Modbus, dport = 502)

# Responding to a SYN request
def responsesyn(packet):
	#Write the connection to the syslog
	syslog.syslog(syslog.LOG_ALERT, 'HP: ID('+HPID+') mod:' +packet[IP].src + ' is connecting to the Modbus Emulator')
	#Build a packet to send back
	response = Ether()/IP()/TCP()
	response[Ether].src = packet[Ether].dst
	response[Ether].dst = packet[Ether].src
	response[IP].src = packet[IP].dst
	response[IP].dst = packet[IP].src
	response[IP].flags = 2	
	response[TCP].sport = packet[TCP].dport
	response[TCP].dport = packet[TCP].sport
	response[TCP].seq = random.randint(1,4294967295)
	response[TCP].ack = packet[TCP].seq + 1
	response[TCP].flags = 'SA'
	response[TCP].options = packet[TCP].options 
	del(response[IP].chksum)
	del(response[TCP].chksum)
	del(response[IP].len)	
	#sendp will recalculate the checksums and IP length before sending the packet.
	sendp(response, loop=0)

# Building a response to Read Coil
def responsereadcoil(packet):
	# Build a packet to send back	
	response = Ether()/IP()/TCP()

	if packet.haslayer(ReadCoil):

		# This checks to see if the request was valid in the number of coils it requested.
		if packet[ReadCoil].quantitycoils > numcoils or packet[ReadCoil].quantitycoils < 1:
			# Write the error to the syslog
			syslog.syslog(syslog.LOG_ALERT, 'HP: ID('+HPID+') mod:' +packet[IP].src + ' sent an INVALID Read Coil request')		
			response = response/Modbus()/Error()
			response[Modbus].transaction = packet[Modbus].transaction	
			response[Modbus].length = 3
			response[Modbus].unit = packet[Modbus].unit
			response[Modbus].function = packet[Modbus].function + 128
			response[Error].code = 3
	
		# This checks to see if the starting address is valid and the starting address + Quatity of Outputs is valid
		elif (packet[ReadCoil].startcoil < 0 or packet[ReadCoil].startcoil > numcoils) or ((packet[ReadCoil].startcoil + packet[ReadCoil].quantitycoils) > numcoils):
			# Write the error to the syslog
			syslog.syslog(syslog.LOG_ALERT, 'HP: ID('+HPID+') mod:' +packet[IP].src + ' sent an INVALID Read Coil request')
	
			response = response/Modbus()/Error()
			response[Modbus].transaction = packet[Modbus].transaction
			response[Modbus].length = 3
			response[Modbus].unit = packet[Modbus].unit
			response[Modbus].function = packet[Modbus].function + 128
			response[Error].code = 2
		
		# Else the Read Coil was a valid command
		else:
			response = response/Modbus()/ReadCoilResponse()
			# Write the valid request to the syslog
			syslog.syslog(syslog.LOG_ALERT, 'HP: ID('+HPID+') mod:' +packet[IP].src + ' sent a valid Read Coil request')
	 
			bytecount = (packet[ReadCoil].quantitycoils/8)
			partbytecount = packet[ReadCoil].quantitycoils%8
			if partbytecount != 0:
				bytecount = bytecount + 1
			response[ReadCoilResponse].bytecount = bytecount
			response[Modbus].unit = packet[Modbus].unit
			response[Modbus].transaction = packet[Modbus].transaction	
			response[Modbus].length = response[ReadCoilResponse].bytecount + 3
			leadzero = (8 - partbytecount)%8
			status = []
			output = ''
	
			# This is + 7 because each high order bit is the highest output address.  
			# (Look at Modbus Application Protocol Specification V1.1b at www.Modbus-IDA.org)
			current = packet[ReadCoil].startcoil + 7
			
			# This will build each byte to be sent back to the master.
			for x in range(0, bytecount):
				if x == (bytecount-1):
					for z in range(current,current-leadzero, -1):
						status.append('0')
					for k in range(current-leadzero, current-8, -1):
						status.append(coil[k])
					current = current + 8
					status = ''.join(status)
					# Below will take the bits and create a byte to be added to the output string.
					temp = BitArray(bin=status)
					value = temp.uint
					string = pack('!h', value)
					output = output + string[1]			
					status = []
				else:
					for y in range(current, current-8, -1):
						status.append(coil[y])
					current = current + 8
					status = ''.join(status)
					# Below will take the bits and create a byte to be added to the output string.
					temp = BitArray(bin=status)
					value = temp.uint
					string = pack('!h', value)
					output = output + string[1]			
					status = []
	
			response[ReadCoilResponse].status = output
		response[Ether].src = packet[Ether].dst
		response[Ether].dst = packet[Ether].src
		response[IP].flags = 0
		response[IP].ttl = 64
		response[IP].src = packet[IP].dst
		response[IP].dst = packet[IP].src
		response[TCP].flags = 'PA'	
		response[TCP].sport = packet[TCP].dport
		response[TCP].dport = packet[TCP].sport
		response[TCP].seq = packet[TCP].ack
		response[TCP].ack = packet[TCP].seq + 12
		response[TCP].window = 4096
		del(response[IP].chksum)
		del(response[TCP].chksum)
		del(response[IP].len)
		#sendp will recalculate the checksums and IP length before sending the packet.
		sendp(response, loop=0)

	else:
		responseerror(packet)
	
# Building a response to Read Discrete Inputs
def responsereaddiscreteinputs(packet):
	# Build a packet to send back
	response = Ether()/IP()/TCP()

	if packet.haslayer(ReadDiscreteInputs):
		# This checks to see if the request was valid in the number of inputs it requested.
		if packet[ReadDiscreteInputs].quantityinputs > numdinputs or packet[ReadDiscreteInputs].quantityinputs < 1:
			# Write the error to the syslog
			syslog.syslog(syslog.LOG_ALERT, 'HP: ID('+HPID+') mod:' +packet[IP].src + ' sent an INVALID Read Discrete Input request')
	
			response = response/Modbus()/Error()
			response[Modbus].transaction = packet[Modbus].transaction
			response[Modbus].length = 3
			response[Modbus].unit = packet[Modbus].unit
			response[Modbus].function = packet[Modbus].function + 128
			response[Error].code = 3
		
		# This checks to see if the starting address is valid and the starting address + Quatity of Outputs is valid
		elif (packet[ReadDiscreteInputs].startinput < 0 or packet[ReadDiscreteInputs].startinput >= numdinputs) or ((packet[ReadDiscreteInputs].startinput + packet[ReadDiscreteInputs].quantityinputs) >= numdinputs):
			# Write the error to the syslog
			syslog.syslog(syslog.LOG_ALERT, 'HP: ID('+HPID+') mod:' +packet[IP].src + ' sent an INVALID Read Discrete Input request')
	
			response = response/Modbus()/Error()
			response[Modbus].transaction = packet[Modbus].transaction
			response[Modbus].length = 3
			response[Modbus].unit = packet[Modbus].unit
			response[Modbus].function = packet[Modbus].function + 128
			response[Error].code = 2
	
		# Else the Read Discrete Inputs was a valid command
		else:
			response = response/Modbus()/ReadDiscreteInputsResponse()
			# Write the valid request to the syslog
			syslog.syslog(syslog.LOG_ALERT, 'HP: ID('+HPID+') mod:' +packet[IP].src + ' sent a valid Read Discrete Input request')
	
			bytecount = (packet[ReadDiscreteInputs].quantityinputs/8)
			partbytecount = packet[ReadDiscreteInputs].quantityinputs%8
			if partbytecount != 0:
				bytecount = bytecount + 1
			response[ReadDiscreteInputsResponse].bytecount = bytecount
			response[Modbus].unit = packet[Modbus].unit
			response[Modbus].transaction = packet[Modbus].transaction	
			response[Modbus].function = packet[Modbus].function	
			response[Modbus].length = response[ReadDiscreteInputsResponse].bytecount + 3
			leadzero = (8 - partbytecount)%8
			status = []
			output = ''
	
			# This is + 7 because each high order bit is the highest output address.  
			# (Look at Modbus Application Protocol Specification V1.1b at www.Modbus-IDA.org)
			current = packet[ReadDiscreteInputs].startinput + 7
	
			# This will build each byte to be sent back to the master.
			for x in range(0, bytecount):
				if x == (bytecount-1):
					for z in range(current,current-leadzero, -1):
						status.append('0')
					for k in range(current-leadzero, current-8, -1):
						status.append(dinputs[k])
					current = current + 8
					# Below will take the bits and create a byte to be added to the output string.
					status = ''.join(status)
					temp = BitArray(bin=status)
					value = temp.uint
					string = pack('!h', value)
					output = output + string[1]			
					status = []
				else:
					for y in range(current, current-8, -1):
						status.append(dinputs[y])
					current = current + 8
					# Below will take the bits and create a byte to be added to the output string.
					status = ''.join(status)
					temp = BitArray(bin=status)
					value = temp.uint
					string = pack('!h', value)
					output = output + string[1]			
					status = []
	
			response[ReadDiscreteInputsResponse].status = output 
	
		response[Ether].src = packet[Ether].dst
		response[Ether].dst = packet[Ether].src
		response[IP].flags = 0
		response[IP].ttl = 64
		response[IP].src = packet[IP].dst
		response[IP].dst = packet[IP].src
		response[TCP].flags = 'PA'	
		response[TCP].sport = packet[TCP].dport
		response[TCP].dport = packet[TCP].sport
		response[TCP].seq = packet[TCP].ack
		response[TCP].ack = packet[TCP].seq + 12
		response[TCP].window = 4096
		del(response[IP].chksum)
		del(response[TCP].chksum)
		del(response[IP].len)
		#sendp will recalculate the checksums and IP length before sending the packet.
		sendp(response, loop=0)

	else:
		responseerror(packet)
	
# Building a response to Read Holding Registers
def responsereadregisters(packet):
	# Build a packet to send back
	response = Ether()/IP()/TCP()
	
	if packet.haslayer(ReadHoldingRegisters):
		# This checks to see if the request was valid in the number of registers it requested.
		if packet[ReadHoldingRegisters].quantityregs < 1 or packet[ReadHoldingRegisters].quantityregs > numholdregisters:
			# Write the error to the syslog
			syslog.syslog(syslog.LOG_ALERT, 'HP: ID('+HPID+') mod:' +packet[IP].src + ' sent an INVALID Read Holding Registers request')
	
			response = response/Modbus()/Error()
			response[Modbus].transaction = packet[Modbus].transaction
			response[Modbus].length = 3
			response[Modbus].unit = packet[Modbus].unit
			response[Modbus].function = packet[Modbus].function + 128
			response[Error].code = 3
	
		# This checks to see if the starting address is valid and the starting address + Quatity of Outputs is valid
		elif (packet[ReadHoldingRegisters].startaddress < 0 or packet[ReadHoldingRegisters].startaddress >= numholdregisters) or ((packet[ReadHoldingRegisters].startaddress + packet[ReadHoldingRegisters].quantityregs) >= numholdregisters):
			# Write the error to the syslog
			syslog.syslog(syslog.LOG_ALERT, 'HP: ID('+HPID+') mod:' +packet[IP].src + ' sent an INVALID Read Holding Registers request')
	
			response = response/Modbus()/Error()
			response[Modbus].transaction = packet[Modbus].transaction
			response[Modbus].length = 3
			response[Modbus].unit = packet[Modbus].unit
			response[Modbus].function = packet[Modbus].function + 128
			response[Error].code = 2
	
		# Else the Read Holding Register Request is valid
		else:
			response = response/Modbus()/ReadHoldingRegistersResponse()
			# Write the valid request to the syslog
			syslog.syslog(syslog.LOG_ALERT, 'HP: ID('+HPID+') mod:' +packet[IP].src + ' sent a valid Read Holding Registers request')
	
			response[ReadHoldingRegistersResponse].bytecount = packet[ReadHoldingRegisters].quantityregs * 2
			response[Modbus].length = response[ReadHoldingRegistersResponse].bytecount + 3
			response[Modbus].function = packet[Modbus].function	
			response[Modbus].unit = packet[Modbus].unit
			response[Modbus].transaction = packet[Modbus].transaction
	
			# This will loop and add all the values requested to the status.
			for x in range(packet[ReadHoldingRegisters].startaddress, (packet[ReadHoldingRegisters].startaddress + packet[ReadHoldingRegisters].quantityregs)):
				response[ReadHoldingRegistersResponse].status = response[ReadHoldingRegistersResponse].status + holdregister[x]
	
		response[Ether].src = packet[Ether].dst
		response[Ether].dst = packet[Ether].src
		response[IP].flags = 0
		response[IP].ttl = 64
		response[IP].src = packet[IP].dst
		response[IP].dst = packet[IP].src
		response[TCP].flags = 'PA'	
		response[TCP].sport = packet[TCP].dport
		response[TCP].dport = packet[TCP].sport
		response[TCP].seq = packet[TCP].ack
		response[TCP].ack = packet[TCP].seq + 12
		response[TCP].window = 4096	
		del(response[IP].chksum)
		del(response[TCP].chksum)
		del(response[IP].len)
		#sendp will recalculate the checksums and IP length before sending the packet.
		sendp(response, loop=0)

	else:
		responseerror(packet)

# Building a response to Read Input Registers
def responsereadinputregisters(packet):
	# Build a packet to send back
	response = Ether()/IP()/TCP()
	
	if packet.haslayer(ReadInputRegisters):
		# This checks to see if the request was valid in the number of registers it requested.
		if packet[ReadInputRegisters].quantityregs < 1 or packet[ReadInputRegisters].quantityregs > numinputregisters:
			# Write the error to the syslog
			syslog.syslog(syslog.LOG_ALERT, 'HP: ID('+HPID+') mod:' +packet[IP].src + ' sent an INVALID Read Input Registers request')
	
			response = response/Modbus()/Error()
			response[Modbus].transaction = packet[Modbus].transaction
			response[Modbus].length = 3
			response[Modbus].unit = packet[Modbus].unit
			response[Modbus].function = packet[Modbus].function + 128
			response[Error].code = 3
	
		# This checks to see if the starting address is valid and the starting address + Quatity of Outputs is valid
		elif (packet[ReadInputRegisters].startaddress < 0 or packet[ReadInputRegisters].startaddress >= numinputregisters) or ((packet[ReadInputRegisters].startaddress + packet[ReadInputRegisters].quantityregs) >= numinputregisters):
			# Write the error to the syslog
			syslog.syslog(syslog.LOG_ALERT, 'HP: ID('+HPID+') mod:' +packet[IP].src + ' sent an INVALID Read Input Registers request')
	
			response = response/Modbus()/Error()
			response[Modbus].transaction = packet[Modbus].transaction
			response[Modbus].length = 3
			response[Modbus].unit = packet[Modbus].unit
			response[Modbus].function = packet[Modbus].function + 128
			response[Error].code = 2
	
		# Else the Read Input Registers is valid
		else:
			response = response/Modbus()/ReadInputRegistersResponse()
			# Write the valid request to the syslog
			syslog.syslog(syslog.LOG_ALERT, 'HP: ID('+HPID+') mod:' +packet[IP].src + ' sent a valid Read Input Registers request')
	
			response[ReadInputRegistersResponse].bytecount = packet[ReadInputRegisters].quantityregs * 2
			response[Modbus].length = response[ReadInputRegistersResponse].bytecount + 3
			response[Modbus].function = packet[Modbus].function	
			response[Modbus].unit = packet[Modbus].unit
			response[Modbus].transaction = packet[Modbus].transaction	
	
			# This will loop and add all the values requested to the status.
			for x in range(packet[ReadInputRegisters].startaddress, (packet[ReadInputRegisters].startaddress + packet[ReadInputRegisters].quantityregs)):
				response[ReadInputRegistersResponse].status = response[ReadInputRegistersResponse].status + inputregister[x]
	
		response[Ether].src = packet[Ether].dst
		response[Ether].dst = packet[Ether].src
		response[IP].flags = 0
		response[IP].ttl = 64
		response[IP].src = packet[IP].dst
		response[IP].dst = packet[IP].src
		response[TCP].flags = 'PA'	
		response[TCP].sport = packet[TCP].dport
		response[TCP].dport = packet[TCP].sport
		response[TCP].seq = packet[TCP].ack
		response[TCP].ack = packet[TCP].seq + 12
		response[TCP].window = 4096
		del(response[IP].chksum)
		del(response[TCP].chksum)
		del(response[IP].len)
		#sendp will recalculate the checksums and IP length before sending the packet.
		sendp(response, loop=0)
	
	else:
		responseerror(packet)

# Building a response to Write Single Coil
def responsewritecoil(packet):
	# Build a packet to send back
	response = Ether()/IP()/TCP()
	
	if packet.haslayer(WriteSingleCoil):
		# This checks to see if the request value was valid.
		if not (packet[WriteSingleCoil].state == 0 or packet[WriteSingleCoil].state == 255):
			# Write the error to the syslog
			syslog.syslog(syslog.LOG_ALERT, 'HP: ID('+HPID+') mod:' +packet[IP].src + ' sent an INVALID Write Single Coil request')
	
			response = response/Modbus()/Error()
			response[Modbus].transaction = packet[Modbus].transaction
			response[Modbus].length = 3
			response[Modbus].unit = packet[Modbus].unit
			response[Modbus].function = packet[Modbus].function + 128
			response[Error].code = 3

		# This checks to see if the coil number is valid 
		elif packet[WriteSingleCoil].coilnumber < 0 or packet[WriteSingleCoil].coilnumber >= numcoils:
			# Write the error to the syslog
			syslog.syslog(syslog.LOG_ALERT, 'HP: ID('+HPID+') mod:' +packet[IP].src + ' sent an INVALID Write Single Coil request')
	
			response = response/Modbus()/Error()
			response[Modbus].transaction = packet[Modbus].transaction
			response[Modbus].length = 3
			response[Modbus].unit = packet[Modbus].unit
			response[Modbus].function = packet[Modbus].function + 128
			response[Error].code = 2

		# Else the Write Single Coil request is valid
		else:
			response = response/Modbus()/WriteSingleCoil()
			# Write the valid request to the syslog
			syslog.syslog(syslog.LOG_ALERT, 'HP: ID('+HPID+') mod:' +packet[IP].src + ' sent a valid Write Single Coil request')		
			response[Modbus].transaction = packet[Modbus].transaction
			response[Modbus].length = packet[Modbus].length
			response[Modbus].unit = packet[Modbus].unit
			response[Modbus].function = packet[Modbus].function
			response[WriteSingleCoil].state = packet[WriteSingleCoil].state
			response[WriteSingleCoil].coilnumber = packet[WriteSingleCoil].coilnumber
			
			# If the value is 255 switch the value to 1 else if it is 0 switch the value to 0.
			if packet[WriteSingleCoil].state == 255:
				coil[packet[WriteSingleCoil].coilnumber] = '1'
			elif packet[WriteSingleCoil].state == 0:
				coil[packet[WriteSingleCoil].coilnumber] = '0'
		response[Ether].src = packet[Ether].dst
		response[Ether].dst = packet[Ether].src
		response[IP].flags = 0
		response[IP].ttl = 64
		response[IP].src = packet[IP].dst
		response[IP].dst = packet[IP].src
		response[TCP].flags = 'PA'	
		response[TCP].sport = packet[TCP].dport
		response[TCP].dport = packet[TCP].sport
		response[TCP].seq = packet[TCP].ack
		response[TCP].ack = packet[TCP].seq + 12
		response[TCP].window = 4096
		del(response[IP].chksum)
		del(response[TCP].chksum)
		del(response[IP].len)
		#sendp will recalculate the checksums and IP length before sending the packet.
		sendp(response, loop=0)

	else:
		responseerror(packet)

# Building a response to Write Single Register
def responsewriteregister(packet):
	# Build a packet to send back
	response = Ether()/IP()/TCP()
	
	if packet.haslayer(WriteSingleRegister):
		# This checks to see if the request value was valid.
		if packet[WriteSingleRegister].regvalue < 0 or packet[WriteSingleRegister].regvalue > 65535:
			# Write the error to the syslog
			syslog.syslog(syslog.LOG_ALERT, 'HP: ID('+HPID+') mod:' +packet[IP].src + ' sent an INVALID Write Single Register request')
	
			response = response/Modbus()/Error()
			response[Modbus].transaction = packet[Modbus].transaction
			response[Modbus].length = 3
			response[Modbus].unit = packet[Modbus].unit
			response[Modbus].function = packet[Modbus].function + 128
			response[Error].code = 3

		# This checks to see if the register address is valid.
		elif packet[WriteSingleRegister].regaddress < 0 or packet[WriteSingleRegister].regaddress >= numholdregisters:
			# Write the error to the syslog
			syslog.syslog(syslog.LOG_ALERT, 'HP: ID('+HPID+') mod:' +packet[IP].src + ' sent an INVALID Write Single Register request')
	
			response = response/Modbus()/Error()
			response[Modbus].transaction = packet[Modbus].transaction
			response[Modbus].length = 3
			response[Modbus].unit = packet[Modbus].unit
			response[Modbus].function = packet[Modbus].function + 128
			response[Error].code = 2
		
		# Else the Write Single Register request is valid
		else:
			response = response/Modbus()/WriteSingleRegister()
			# Write the valid request to the syslog
			syslog.syslog(syslog.LOG_ALERT, 'HP: ID('+HPID+') mod:' +packet[IP].src + ' sent a valid Write Single Register request')		

			response[Modbus].transaction = packet[Modbus].transaction
			response[Modbus].length = packet[Modbus].length
			response[Modbus].unit = packet[Modbus].unit
			response[Modbus].function = packet[Modbus].function	
			response[WriteSingleRegister].regaddress = packet[WriteSingleRegister].regaddress
			response[WriteSingleRegister].regvalue = packet[WriteSingleRegister].regvalue
			# This updates the value of the register to be changed.
			holdregister[packet[WriteSingleRegister].regaddress] = pack('!h', packet[WriteSingleRegister].regvalue)
	
		response[Ether].src = packet[Ether].dst
		response[Ether].dst = packet[Ether].src
		response[IP].flags = 0
		response[IP].ttl = 64
		response[IP].src = packet[IP].dst
		response[IP].dst = packet[IP].src
		response[TCP].flags = 'PA'	
		response[TCP].sport = packet[TCP].dport
		response[TCP].dport = packet[TCP].sport
		response[TCP].seq = packet[TCP].ack
		response[TCP].ack = packet[TCP].seq + 12
		response[TCP].window = 4096
		del(response[IP].chksum)
		del(response[TCP].chksum)
		del(response[IP].len)
		#sendp will recalculate the checksums and IP length before sending the packet.
		sendp(response, loop=0)
	
	else:
		responseerror(packet)

def responseencapsulatedinterface(packet):
	# Build a packet to send back	
	response = Ether()/IP()/TCP()
	if packet.haslayer(EncapsulatedInterfaceRequest):

		# This checks to see if the request was valid MEI type
		if packet[EncapsulatedInterfaceRequest].meitype == 14:
			# Basic Device Identification Stream
			if packet[EncapsulatedInterfaceRequest].deviceid == 1:
				# This must start with a 0
				if packet[EncapsulatedInterfaceRequest].objectid == 0:
					syslog.syslog(syslog.LOG_ALERT, 'HP: ID('+HPID+') mod:' +packet[IP].src + ' sent a VALID Encapsulated Interface Read Basic Information Request')	
					response = response/Modbus()/EncapsulatedInterfaceResponse()
					response[EncapsulatedInterfaceResponse].meitype = packet[EncapsulatedInterfaceRequest].meitype
					response[EncapsulatedInterfaceResponse].deviceid = packet[EncapsulatedInterfaceRequest].deviceid
					response[EncapsulatedInterfaceResponse].conformity = 1
					response[EncapsulatedInterfaceResponse].morefollows = 0
					response[EncapsulatedInterfaceResponse].objectid = 0
					response[EncapsulatedInterfaceResponse].numobjects = 3
					length = len(response[EncapsulatedInterfaceResponse])
	
					#Building the objects to send in the packet
					object1 = EncapsulatedInterfaceObject()
					object1[EncapsulatedInterfaceObject].objectid = 0
					object1[EncapsulatedInterfaceObject].objectvalue = vendorname
					object1[EncapsulatedInterfaceObject].objectlength = len(object1[EncapsulatedInterfaceObject].objectvalue)
					object2 = EncapsulatedInterfaceObject()
					object2[EncapsulatedInterfaceObject].objectid = 1
					object2[EncapsulatedInterfaceObject].objectvalue = productcode
					object2[EncapsulatedInterfaceObject].objectlength = len(object2[EncapsulatedInterfaceObject].objectvalue)
					object3 = EncapsulatedInterfaceObject()
					object3[EncapsulatedInterfaceObject].objectid = 2
					object3[EncapsulatedInterfaceObject].objectvalue = majorminorrevision
					object3[EncapsulatedInterfaceObject].objectlength = len(object3[EncapsulatedInterfaceObject].objectvalue)
					response = response/object1/object2/object3
					length = length + len(object1) + len(object2) + len(object3) + 2	
					response[Modbus].length = length
				else:
					syslog.syslog(syslog.LOG_ALERT, 'HP: ID('+HPID+') mod:' +packet[IP].src + ' sent an INVALID Object ID code ' + str(packet[EncapsulatedInterfaceRequest].objectid))		
	
					response = response/Modbus()/Error()
					response[Modbus].transaction = packet[Modbus].transaction	
					response[Modbus].length = 3
					response[Modbus].unit = packet[Modbus].unit
					response[Modbus].function = packet[Modbus].function + 128
					response[Error].code = 2

			# One Specific Identification Object
			elif packet[EncapsulatedInterfaceRequest].deviceid == 4:
				if packet[EncapsulatedInterfaceRequest].objectid >= 0 and packet[EncapsulatedInterfaceRequest].objectid < 3:
					response = response/Modbus()/EncapsulatedInterfaceResponse()/EncapsulatedInterfaceObject()
					syslog.syslog(syslog.LOG_ALERT, 'HP: ID('+HPID+') mod:' +packet[IP].src + ' sent a VALID Encapsulated Interface Read Single Device Object Request')	
					response[EncapsulatedInterfaceResponse].meitype = packet[EncapsulatedInterfaceRequest].meitype
					response[EncapsulatedInterfaceResponse].deviceid = packet[EncapsulatedInterfaceRequest].deviceid
					response[EncapsulatedInterfaceResponse].conformity = 129
					response[EncapsulatedInterfaceResponse].morefollows = 0
					response[EncapsulatedInterfaceResponse].objectid = 0
					response[EncapsulatedInterfaceResponse].numobjects = 1
					response[EncapsulatedInterfaceObject].objectid = packet[EncapsulatedInterfaceRequest].objectid
					if packet[EncapsulatedInterfaceRequest].objectid == 0:
						response[EncapsulatedInterfaceObject].objectvalue = vendorname
					elif packet[EncapsulatedInterfaceRequest].objectid == 1:
						response[EncapsulatedInterfaceObject].objectvalue = productcode
					elif packet[EncapsulatedInterfaceRequest].objectid == 2:
						response[EncapsulatedInterfaceObject].objectvalue = majorminorrevision
					response[EncapsulatedInterfaceObject].objectlength = len(response[EncapsulatedInterfaceObject].objectvalue)
					response[Modbus].length = len(response[EncapsulatedInterfaceResponse]) + len(response[EncapsulatedInterfaceObject])
					response[EncapsulatedInterfaceObject].objectid = packet[EncapsulatedInterfaceRequest].objectid
	
				else:
					syslog.syslog(syslog.LOG_ALERT, 'HP: ID('+HPID+') mod:' +packet[IP].src + ' sent an INVALID Object ID code ' + str(packet[EncapsulatedInterfaceRequest].objectid))		
	
					response = response/Modbus()/Error()
					response[Modbus].transaction = packet[Modbus].transaction	
					response[Modbus].length = 3
					response[Modbus].unit = packet[Modbus].unit
					response[Modbus].function = packet[Modbus].function + 128
					response[Error].code = 2

			# The Read Device Code is not supported so respond with an error
			else:
				syslog.syslog(syslog.LOG_ALERT, 'HP: ID('+HPID+') mod:' +packet[IP].src + ' sent an INVALID Read Device ID code ' + str(packet[EncapsulatedInterfaceRequest].deviceid))		
	
				response = response/Modbus()/Error()
				response[Modbus].transaction = packet[Modbus].transaction	
				response[Modbus].length = 3
				response[Modbus].unit = packet[Modbus].unit
				response[Modbus].function = packet[Modbus].function + 128
				response[Error].code = 3

		# The MEI Type is not supported
		else:
			syslog.syslog(syslog.LOG_ALERT, 'HP: ID('+HPID+') mod:' +packet[IP].src + ' sent an INVALID MEI Type ' + str(packet[EncapsulatedInterfaceRequest].meitype))
			responseerror(packet)
		
		response[Modbus].transaction = packet[Modbus].transaction
		response[Modbus].unit = packet[Modbus].unit  
		response[Modbus].function = packet[Modbus].function	
		response[Ether].src = packet[Ether].dst
		response[Ether].dst = packet[Ether].src
		response[IP].flags = 0
		response[IP].ttl = 64
		response[IP].src = packet[IP].dst
		response[IP].dst = packet[IP].src
		response[TCP].flags = 'PA'	
		response[TCP].sport = packet[TCP].dport
		response[TCP].dport = packet[TCP].sport
		response[TCP].seq = packet[TCP].ack
		response[TCP].ack = packet[TCP].seq + 11#packet[TCP].len
		response[TCP].window = 4096
		del(response[IP].chksum)
		del(response[TCP].chksum)
		del(response[IP].len)
		#sendp will recalculate the checksums and IP length before sending the packet.
		sendp(response, loop=0)

	else:
		responseerror(packet)

def responseerror(packet):
	# Build a packet to send back
	response = Ether()/IP()/TCP()/Modbus()/Error()
	
	# Write the error to the syslog
	syslog.syslog(syslog.LOG_ALERT, 'HP: ID('+HPID+') mod:' +packet[IP].src + ' sent an INVALID function code ' + str(packet[Modbus].function))

	response[Ether].src = packet[Ether].dst
	response[Ether].dst = packet[Ether].src
	response[IP].flags = 0
	response[IP].ttl = 64
	response[IP].src = packet[IP].dst
	response[IP].dst = packet[IP].src
	response[TCP].flags = 'PA'	
	response[TCP].sport = packet[TCP].dport
	response[TCP].dport = packet[TCP].sport
	response[TCP].seq = packet[TCP].ack
	response[TCP].ack = packet[TCP].seq + packet[TCP].len
	response[TCP].window = 4096
	response[Modbus].transaction = packet[Modbus].transaction
	response[Modbus].length = 3
	response[Modbus].unit = packet[Modbus].unit
	
	# This will change the high order bit to one unless it is already a 1.
	if packet[Modbus].function > 127:
		response[Modbus].function = packet[Modbus].function
	else:
		response[Modbus].function = packet[Modbus].function + 128
	del(response[IP].chksum)
	del(response[TCP].chksum)
	del(response[IP].len)
	#sendp will recalculate the checksums and IP length before sending the packet.
	sendp(response, loop=0)


def response(packet):
	if packet.haslayer(TCP):
		if packet[TCP].dport == 502:
			if packet.haslayer(Modbus) and packet[Modbus].protocol == 0:
				if packet[Modbus].function == function_code['Read Coil']:
					responsereadcoil(packet)
				elif packet[Modbus].function == function_code['Read Discrete Inputs']:
					responsereaddiscreteinputs(packet)
				elif packet[Modbus].function == function_code['Read Holding Registers']:
					responsereadregisters(packet)
				elif packet[Modbus].function == function_code['Read Input Registers']:
					responsereadinputregisters(packet)
				elif packet[Modbus].function == function_code['Write Single Coil']:
					responsewritecoil(packet)
				elif packet[Modbus].function == function_code['Write Single Register']:
					responsewriteregister(packet)
				elif packet[Modbus].function == function_code['Encapsulated Interface Transport']:
					responseencapsulatedinterface(packet)
				else:
					responseerror(packet)
			elif packet[TCP].flags == 2:
				#send a SYN ACK response
				responsesyn(packet)


	return

## This is the Main Script
#sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
#sock.bind(('',502))

print '+++++Modbus Server Started.  Listening on TCP Port 502.'
if(logging==1):
    syslog.syslog(syslog.LOG_ALERT, 'HP: ID('+HPID+') mod: Modbus service started.  Listening on TCP Port 502')

sniff(iface="eth0", filter="tcp and port 502", store = 0, prn=lambda x: response(x))

