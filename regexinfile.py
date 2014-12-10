import os
import re
import sys
#find string in files in directory 
#usage
#python regexinfile DIRPATH regex followLineNumb [file subfix(.txt .cpp .c ...)]
if sys.argv<2:
	exit()
argidx=1
rootdir = str(sys.argv[argidx])
argidx+=1
targetregex=str(sys.argv[argidx])
argidx+=1
maxshow=int(str(sys.argv[argidx]))
argidx+=1

#shell color code
colorMatch="\033[0;1;31m"#red
colorNonMatch="\033[0;0;36m"#dark blue
colorLineNumb="\033[40;0;37m"#white
colorSplit="\033[47;8;37m"#white stripe
colorNormal="\033[0m"#clear

for afile in os.listdir(rootdir):
	isInterested = False;
	for argidx in range(argidx,len(sys.argv)):
		if afile.endswith(str(sys.argv[argidx])):
			isInterested=True
			break
	if isInterested:
		isprinting=0
		fullfname=os.path.join(rootdir, afile)
		fptr=open(fullfname)
		firstime = True
		linecounter=0
		for line in fptr:
			linecounter=linecounter+1
			m = re.search(targetregex,line);
			if m:
				if firstime:
					print fullfname
					firstime=False
				isprinting=maxshow
			if isprinting>0:
				strbuff=""
				searchstr=line.rstrip()
				strbuff+= colorLineNumb+str(linecounter)+colorNonMatch
				matchobj = re.search(targetregex,searchstr) 
				while (matchobj is not None) and len(searchstr)>0:
					strbuff+=  searchstr[0:matchobj.start(0)]
					strbuff+= colorMatch + searchstr[matchobj.start(0):matchobj.end(0)] +colorNonMatch
					searchstr=searchstr[matchobj.end(0):]
					matchobj = re.search(targetregex,searchstr)
				strbuff+=  searchstr[0:]
				print strbuff
				isprinting -= 1
				if isprinting==0:
					print colorSplit+"\t\t\t\t"+colorNormal