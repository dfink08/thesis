#~2014 Deanna Fink
#~This file generates a collection of .html files based on packets read from
#   a wireshark text file
# Uses python 3.3.3
import os,mimetypes,subprocess
from os.path import join, getsize
		
pathName = input("Path of the html files: ")
pathwalk = os.walk(pathName)
for root, dirs, files in os.walk(pathName):
  print root
  
  for name in files:
    filePath = str(root+'/'+name)
    proc = subprocess.Popen(["file",filePath],stdout=subprocess.PIPE)
    output = proc.stdout.read()
    
    if "HTML" in output:
      print(name)

                
# packetFile = open(fileName, 'r')

# fileText = htmlFile.read()
# formStart = fileText.find("<form>" or "<FORM>")

# #Scan the file looking for form code
# #   If find returns -1 (i.e. no more form or a form section never existed) quit
# while formStart != -1:
    # formEnd = fileText.find("</form>" or "</FORM>")+7

    # formText = fileText[formStart:formEnd]
    # print(formText)
    
    # htmlFileName = formText[nameIndexS:nameIndexE]
    # print(htmlFileName,'\n')
    # fileText = fileText[formEnd:]
    
    # formStart = fileText.find("<form>" or "<FORM>")

    # # Check if file already exists
    # # if it doesn't, make it and make it generic
    # try:
        # # Open file for exclusive creation
        # htmlFile = open(htmlFileName,"x")
        # htmlFile.write(formText)
        # print("\nFile creation successful")

        # htmlFile.close()

    # except FileExistsError:
        # pass


# packetFile.close()

