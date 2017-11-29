default:sender_udp.c receiver_udp.c Aufgabe2.h
	gcc -o sender_udp sender_udp.c
	gcc -o receiver_udp receiver_udp.c
	mkdir received

sender_udp: sender_udp.c Aufgabe2.h
	gcc -o sender_udp sender_udp.c
receiver_udp: receiver_udp.c Aufgabe2.h
	gcc -o receiver_udp receiver_udp.c

clean:
	rm sender_udp
	rm receiver_udp
	rm -rf received