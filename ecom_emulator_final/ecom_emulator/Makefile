all:
	gcc -g -c minIni.c -o minIni.o
	gcc -g -Wall -c packet_send.c -lpcap -o packet_send.o	
	gcc -g -Wall packet_rcv.c packet_send.o minIni.o -lnet -lpcap -o ecom_svr.o

run:	
	gcc -g -c minIni.c -o minIni.o
	gcc -g -Wall -c packet_send.c -lpcap -o packet_send.o	
	gcc -g -Wall packet_rcv.c packet_send.o minIni.o -lnet -lpcap -o ecom_svr.o
	sudo ./ecom_svr.o 0 "dst port 28784"

clean:
	rm -rf *.o
	gcc -g -c minIni.c -o minIni.o
	gcc -g -Wall -c packet_send.c -lpcap -o packet_send.o	
	gcc -g -Wall packet_rcv.c packet_send.o minIni.o -lnet -lpcap -o ecom_svr.o

