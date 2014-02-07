all:
	gcc -Wall -o outfilter.o out_filter.c checksum.c -lnfnetlink  -lnetfilter_queue  
	gcc -Wall -o infilter.o in_filter.c checksum.c -lnfnetlink  -lnetfilter_queue
	gcc -Wall -o logging.o logging.c checksum.c -lnfnetlink  -lnetfilter_queue 

run:	
	gcc -Wall -o outfilter.o out_filter.c checksum.c -lnfnetlink  -lnetfilter_queue  
	gcc -Wall -o infilter.o in_filter.c checksum.c -lnfnetlink  -lnetfilter_queue
	gcc -Wall -o logging.o logging.c checksum.c -lnfnetlink  -lnetfilter_queue 

	sudo ./iptables_rules.sh	
	sudo ./outfilter.o
	sudo ./infilter.o

clean:
	rm -rf *.o
	gcc -Wall -o outfilter.o out_filter.c checksum.c -lnfnetlink  -lnetfilter_queue  
	gcc -Wall -o infilter.o in_filter.c checksum.c -lnfnetlink  -lnetfilter_queue


