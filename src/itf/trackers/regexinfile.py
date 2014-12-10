import os
import re
import sys
if sys.argv<2:
	exit()
argidx=1
rootdir = str(sys.argv[argidx])
argidx+=1
targetregex=str(sys.argv[argidx])
argidx+=1
maxshow=int(str(sys.argv[argidx]))
argidx+=1
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
				searchstr=line.rstrip()
				print "\033[40;0;37m"+str(linecounter)+"\t\033[0;0;36m",
				matchobj = re.search(targetregex,searchstr) 
				while (matchobj is not None) and len(searchstr)>0:
					print  searchstr[0:matchobj.start(0)]+"",
					print "\033[0;1;31m" + searchstr[matchobj.start(0):matchobj.end(0)] +"\033[0;0;36m",
					searchstr=searchstr[matchobj.end(0):]
					matchobj = re.search(targetregex,searchstr)
				print  searchstr[0:]+"",
				print "\033[0m"
				isprinting -= 1
				if isprinting==0:
					print "\033[47;8;37m\t\t\t\t\033[0m"
