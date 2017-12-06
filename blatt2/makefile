default:sender_udp.c receiver_udp.c Aufgabe2.h Aufgabe2_additional.h
	gcc -o sender_udp sender_udp.c -lssl -lcrypto
	gcc -o receiver_udp receiver_udp.c -lssl -lcrypto
	mkdir received

sender_udp: sender_udp.c Aufgabe2.h Aufgabe2_additional.h
	gcc -o sender_udp sender_udp.c
receiver_udp: receiver_udp.c Aufgabe2.h Aufgabe2_additional.h
	gcc -o receiver_udp receiver_udp.c

clean:
	rm sender_udp
	rm receiver_udp
	rm -rf received