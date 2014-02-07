#~2014 Deanna Fink
#~This file generates a collection of .html files based on packets read from
#   a wireshark text file
# Uses python 3.3.3
import os,mimetypes,subprocess
from os.path import join, getsize
		
pathName = input("Path of the html files: ")

htmlFiles = []

variableNames = []
variableValues = []
configDict = pathName+"/configDict.txt"
configDictfile = open(configDict,"w")

for root, dirs, files in os.walk(pathName):
  print root
  
  for fileName in files:
    filePath = str(root+'/'+fileName)
    proc = subprocess.Popen(["file",filePath],stdout=subprocess.PIPE)
    output = proc.stdout.read()
    
    if "HTML" in output:
      #htmlFiles.append(filePath)
      
      htmlFile = open(filePath, 'r')
      fileText = htmlFile.read()
      
      print "FILE Path: " + filePath
      inputStart = fileText.find("<input")
      if inputStart == -1:
	inputStart = fileText.find("<INPUT") #This is really ugly, find a more elegant solution.
      
      
      #Scan the file looking for form code
      # If find returns -1 (i.e. no more form or a form section never existed) quit
      while inputStart != -1:
	#print "Input start: ",inputStart
	inputEnd = fileText[inputStart:].find(">")+inputStart+1
	#print "Input End: ", inputEnd
	#if inputEnd == 7:
	  #inputEnd = fileText.find(">")+8
	  #print "Input End: ", inputEnd
	inputSection = fileText[inputStart:inputEnd] 
	#print "Input Text: ", inputSection
	
	
	#Look for the instance of "name" or "id" in the form section
	# This is the label for a changeable field
	nameIndex=0
	if "name=" in inputSection:
	  nameIndex = inputSection.find("name=")
	elif "id=" in inputSection:
	  nameIndex = inputSection.find("id=")
	#print "Name Index: ",nameIndex	      
	nameS = nameIndex+inputSection[nameIndex:].find("\"")+1
	nameE = inputSection[nameS:].find("\"")+nameS
	
	name = inputSection[nameS:nameE]
	
	variableNames.append(name)
	configDictfile.write(fileName[:-5]+"_"+name+"=")
		      
	# After finding the name of the variable, look for its current value
	valueIndex = inputSection.find("value=")
	#print "Value Index: ", valueIndex	      
	valueS = valueIndex+inputSection[valueIndex:].find("\"")+1
	#print "Value start: ", valueS+valueIndex
	valueE = inputSection[valueS:].find("\"")+valueS
	#print "Value end: " , valueE+valueS
	value = inputSection[valueS:valueE]
	
	variableValues.append(value)
	configDictfile.write(value+"\n")
      
	inputSection = inputSection[valueE:]
	
	# After finding all of the variable fields, look for any more form sections that
	# may be in the page
	fileText = fileText[inputEnd:]
	inputStart = fileText.find("<input")
	if inputStart == -1:
	  inputStart = fileText.find("<INPUT") #This is really ugly, find a more elegant solution.
                

#print variableNames
#print variableValues
configDictfile.close()
        
# packetFile = open(fileName, 'r')

# fileText = htmlFile.read()
# inputStart = fileText.find("<form>" or "<FORM>")

# #Scan the file looking for form code
# #   If find returns -1 (i.e. no more form or a form section never existed) quit
# while inputStart != -1:
    # inputEnd = fileText.find("</form>" or "</FORM>")+7

    # inputSection = fileText[inputStart:inputEnd]
    # print(inputSection)
    
    # htmlFileName = inputSection[nameIndexS:nameIndexE]
    # print(htmlFileName,'\n')
    # fileText = fileText[inputEnd:]
    
    # inputStart = fileText.find("<form>" or "<FORM>")

    # # Check if file already exists
    # # if it doesn't, make it and make it generic
    # try:
        # # Open file for exclusive creation
        # htmlFile = open(htmlFileName,"x")
        # htmlFile.write(inputSection)
        # print("\nFile creation successful")

        # htmlFile.close()

    # except FileExistsError:
        # pass


# packetFile.close()

