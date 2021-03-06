# 2014 Deanna Fink
# This file generates a dictionary of input fields from html pages gathered by wget. 
#	It writes this dictionary to a text file called configDict.txt
# Uses python 3.3.3

import os,mimetypes,subprocess, sys
from os.path import join, getsize
import httplib
import time

ipaddress = sys.argv[1]
print "IP: ",ipaddress
#pathName = input("Path of the html files: ")
pathName = '/root/Desktop/ab_emulator'
htmlPath = pathName+'/'+ipaddress
newHtmlPath = pathName+'/web_server/'
configDict = pathName+"/config.txt"
configDictfile = open(configDict,"w")
dictionary = {}

responses = {}
h1 = httplib.HTTPConnection(ipaddress)
h1.request("GET", "/")
r1 = h1.getresponse() 
allHeaders = r1.getheaders()
headerString = ""
for header in allHeaders:
  headerString = headerString+header[0]+':'+header[1]+'\r\n'
		  
toWrite = ("HTTP/1.0" if r1.version == 10 else "HTTP/1.1")+" "
toWrite += str(r1.status)+" "
toWrite += str(r1.reason)+'\r\n'
toWrite += headerString+'\r\n'
if ipaddress in toWrite:
  toWrite = toWrite.replace(ipaddress,"127.0.0.1")

responses['/'] = toWrite

# os.walk will walk the entire directory of files and create a generator object that can be iterated
for root, dirs, files in os.walk(htmlPath):
  
  for fileName in files:
    time.sleep(.001)
    #print "File name: ", fileName
    filePath = str(root+'/'+fileName)
    preamble, realPath = root.split('117',1)
    
    # this line runs the "file" bash command. stdout=subprocess.PIPE keeps the output from printing
    #	to the screen and allows you to put the output into a variable
    proc = subprocess.Popen(["file",filePath],stdout=subprocess.PIPE)
    output = proc.stdout.read()
    #print "File Path: ", filePath
    #print "File Name: ", fileName
    #h1.request("GET", "/"+fileName)
    h1.request("GET", realPath+'/'+fileName)
    #print "Get request: ", realPath+'/'+fileName
    r1 = h1.getresponse() 

    allHeaders = r1.getheaders()
    headerString = ""
    for header in allHeaders:
	headerString = headerString+header[0]+':'+header[1]+'\r\n'
		  
    toWrite = ("HTTP/1.0" if r1.version == 10 else "HTTP/1.1")+" "
    toWrite += str(r1.status)+" "
    toWrite += str(r1.reason)+'\r\n'
    toWrite += headerString+'\r\n'
    if ipaddress in toWrite:
	toWrite = toWrite.replace(ipaddress,"127.0.0.1")
    #if fileName.endswith(".gif"):
      #print "Gif response: ", toWrite
    responses[realPath+'/'+fileName] = toWrite
    #responses[realPath] = toWrite
    # If the file command identifies the file as html then "HTML" will be somewhere in the output
    if ("HTML" in output) or ("SGML" in output) :
      #if "home" in fileName:
        #print "File name: ", fileName
        #print "real path: ", realPath
      
      
      htmlFile = open(filePath, 'r+')
      fileText = htmlFile.read()
      
      newFileText = fileText
      
      inputStart = fileText.find("<input")
      if inputStart == -1:
	inputStart = fileText.find("<INPUT") #This is really ugly, find a more elegant solution.
      
      
      #Scan the file looking for form code
      # If find returns -1 (i.e. no more form or a form section never existed) quit
      while inputStart != -1:
	
	inputEnd = fileText[inputStart:].find(">")+inputStart+1
	
	inputSection = fileText[inputStart:inputEnd] 	
	newInputSection = inputSection
		
	typeIndex = inputSection.find("type=")
	typeS = typeIndex+inputSection[typeIndex:].find("\"")+1
	typeE = inputSection[typeS:].find("\"")+typeS
	
	varType = inputSection[typeS:typeE]
	
	#Look for the instance of "name" or "id" in the form section
	# This is the label for a changeable field	
	nameIndex=0
	if "name=" in inputSection:
	  nameIndex = inputSection.find("name=")
	  nameS = nameIndex+inputSection[nameIndex:].find("\"")+1
	  nameE = inputSection[nameS:].find("\"")+nameS
	
	  name = inputSection[nameS:nameE]
	  newName = fileName[:-5]+"_"+name
	
	elif "id=" in inputSection:
	  nameIndex = inputSection.find("id=")
	  nameS = nameIndex+inputSection[nameIndex:].find("\"")+1
	  nameE = inputSection[nameS:].find("\"")+nameS
	
	  name = inputSection[nameS:nameE]
	  newName = fileName[:-5]+"_"+name
	      
	#NOTE: this format can cause issues. Some files have an equal sign in the name
	#	therefore whe you go to retrieve the value it will look for what is right of the equal
	#	sign, but instead of finding just the value, it will also include part of the name
	if nameIndex != 0:
	  configDictfile.write(newName+'@')
	
	if (varType == "checkbox") or (varType == "radio"):
      
	  valueS = typeE+2
	  valueE = inputSection[valueS:].find("name")+valueS-1
	  
	  value = inputSection[valueS:valueE]
	
	elif (varType != "checkbox") and (varType != "radio"):
	  # After finding the name of the variable, look for its current value
	  valueIndex = inputSection.find("value=")
	      
	  valueS = valueIndex+inputSection[valueIndex:].find("\"")+1
	  valueE = inputSection[valueS:].find("\"")+valueS
	  
	  value = inputSection[valueS:valueE]

	if nameIndex != 0:
	  newInputSection = inputSection
	  newInputSection = inputSection[:valueS]+'${'+fileName[:-5]+"_"+name+'}'+inputSection[valueE:]
	  
	  newFileText = newFileText.replace(fileText[inputStart:inputEnd],newInputSection)
	  
	  configDictfile.write(value+"\n")
	  dictionary[newName] = value
	  
	inputSection = inputSection[valueE:]
	  
	# After finding all of the variable fields, look for any more form sections that
	  # may be in the page
	fileText = fileText[inputEnd:]
	inputStart = fileText.find("<input")
	if inputStart == -1:
	  inputStart = fileText.find("<INPUT") #This is really ugly, find a more elegant solution.      
     
      if not os.path.exists(newHtmlPath+realPath+'/'):
	os.makedirs(newHtmlPath+realPath+'/')
      
      #if fileName.endswith('.html') or fileName.endswith('.asp'):
      newHtmlFile = open(newHtmlPath+realPath+'/'+fileName, 'w')
      newHtmlFile.write(newFileText)
      newHtmlFile.close()
      htmlFile.close()
    else:
      if not os.path.exists(newHtmlPath+realPath+'/'):
	os.makedirs(newHtmlPath+realPath+'/')
      htmlFile = open(filePath, 'r+')
      fileText = htmlFile.read()
      newFileText = fileText
      newHtmlFile = open(newHtmlPath+realPath+'/'+fileName, 'w')
      newHtmlFile.write(newFileText)
      newHtmlFile.close()
      htmlFile.close()
#print dictionary
configDictfile.close()

varFile = open(pathName+"/variableFile.py", "a")
varFile.write("dictionary = ")
varFile.write(str(dictionary))
varFile.write("\n")

varFile.write("responses = ")
varFile.write(str(responses))
varFile.write("\n")
varFile.close()

#TODO: This code currently makes a lot of assumptions about how each html file will be formatted.
#	More testing must be done to ensure that this will work on any web site

