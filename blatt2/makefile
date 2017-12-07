default:sender_udp.c receiver_udp.c sender_tcp.c receiver_tcp.c Aufgabe2.h Aufgabe2_additional.h
	gcc -o sender_udp sender_udp.c -lssl -lcrypto
	gcc -o receiver_udp receiver_udp.c -lssl -lcrypto
	gcc -o sender_tcp sender_tcp.c -lssl -lcrypto
	gcc -o receiver_tcp receiver_tcp.c -lssl -lcrypto
	mkdir received
clean:
	rm sender_udp
	rm receiver_udp
	rm sender_tcp
	rm receiver_tcp
	rm -rf received

udp:sender_udp.c receiver_udp.c Aufgabe2.h Aufgabe2_additional.h
	gcc -o sender_udp sender_udp.c -lssl -lcrypto
	gcc -o receiver_udp receiver_udp.c -lssl -lcrypto
	mkdir received
clean_udp:
	rm sender_udp
	rm receiver_udp
	rm -rf received


tcp:sender_tcp.c receiver_tcp.c Aufgabe2.h Aufgabe2_additional.h
	gcc -o sender_tcp sender_tcp.c -lssl -lcrypto
	gcc -o receiver_tcp receiver_tcp.c -lssl -lcrypto
	mkdir received
clean_tcp:
	rm sender_tcp
	rm receiver_tcp
	rm -rf received

sender_udp: sender_udp.c Aufgabe2.h Aufgabe2_additional.h
	gcc -o sender_udp sender_udp.c -lssl -lcrypto
receiver_udp: receiver_udp.c Aufgabe2.h Aufgabe2_additional.h
	gcc -o receiver_udp receiver_udp.c -lssl -lcrypto

sender_tcp: sender_tcp.c Aufgabe2.h Aufgabe2_additional.h
	gcc -o sender_tcp sender_tcp.c -lssl -lcrypto
receiver_tcp: receiver_tcp.c Aufgabe2.h Aufgabe2_additional.h
	gcc -o receiver_tcp receiver_tcp.c -lssl -lcrypto

