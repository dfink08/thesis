import os

os.system("cd ~/Desktop/wget-1.11.4")

# -m recursively mirrors the site. It has infinite recursion
# -e Execute command as if it were a part of .wgetrc. This is like exec and is
#    required to properly turn off robots.txt
os.system("wget -m -e --user=Administrator --password= robots=off http://10.1.0.117")
