# 2014 Deanna Fink
# This file generates a dictionary of input fields from html pages gathered by wget. 
#	It writes this dictionary to a text file called configDict.txt
# Uses python 3.3.3

import os,mimetypes,subprocess
from os.path import join, getsize
		
pathName = input("Path of the html files: ")

configDict = pathName+"/configDict.txt"
configDictfile = open(configDict,"w")

# os.walk will walk the entire directory of files and create a generator object that can be iterated
for root, dirs, files in os.walk(pathName):
 
  for fileName in files:
    filePath = str(root+'/'+fileName)
    # this line runs the "file" bash command. stdout=subprocess.PIPE keeps the output from printing
    #	to the screen and allows you to put the output into a variable
    proc = subprocess.Popen(["file",filePath],stdout=subprocess.PIPE)
    output = proc.stdout.read()
    
    # If the file command identifies the file as html then "HTML" will be somewhere in the output
    if "HTML" in output:
      
      htmlFile = open(filePath, 'r')
      fileText = htmlFile.read()
      
      print "FILE Path: " + filePath
      inputStart = fileText.find("<input")
      if inputStart == -1:
	inputStart = fileText.find("<INPUT") #This is really ugly, find a more elegant solution.
      
      
      #Scan the file looking for form code
      # If find returns -1 (i.e. no more form or a form section never existed) quit
      while inputStart != -1:
	
	inputEnd = fileText[inputStart:].find(">")+inputStart+1
	
	inputSection = fileText[inputStart:inputEnd] 	
	
	#Look for the instance of "name" or "id" in the form section
	# This is the label for a changeable field
	nameIndex=0
	if "name=" in inputSection:
	  nameIndex = inputSection.find("name=")
	elif "id=" in inputSection:
	  nameIndex = inputSection.find("id=")
	      
	nameS = nameIndex+inputSection[nameIndex:].find("\"")+1
	nameE = inputSection[nameS:].find("\"")+nameS
	
	name = inputSection[nameS:nameE]
	
	#NOTE: this format can cause issues. Some files have an equal sign in the name
	#	therefore whe you go to retrieve the value it will look for what is right of the equal
	#	sign, but instead of finding just the value, it will also include part of the name
	configDictfile.write(fileName[:-5]+"_"+name+"=")
		      
	# After finding the name of the variable, look for its current value
	valueIndex = inputSection.find("value=")
	     
	valueS = valueIndex+inputSection[valueIndex:].find("\"")+1
	valueE = inputSection[valueS:].find("\"")+valueS
	
	value = inputSection[valueS:valueE]
	
	configDictfile.write(value+"\n")
      
	inputSection = inputSection[valueE:]
	
	# After finding all of the variable fields, look for any more form sections that
	# may be in the page
	fileText = fileText[inputEnd:]
	inputStart = fileText.find("<input")
	if inputStart == -1:
	  inputStart = fileText.find("<INPUT") #This is really ugly, find a more elegant solution.
                

configDictfile.close()
        
#TODO: This code currently makes a lot of assumptions about how each html file will be formatted.
#	More testing must be done to ensure that this will work on any web site

