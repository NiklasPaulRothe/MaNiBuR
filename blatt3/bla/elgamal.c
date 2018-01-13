#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "brpa3_959042_959218_header.h"
#include "mod_exp.h"

int main (int argc, char **argv) {
	// Parameter des Algorithmus
	
	unsigned short secret = 5; // a
	unsigned short openkey = 32; // A
	unsigned short secret_sender = 4; // b	
	unsigned short order = 59; // p
	unsigned short generator = 2; // g

	// Öffnen des Moduls
	int fd = open("/dev/brpa3_959042_959218", O_RDWR);

	// secret(und openkey) auf abeichenden Wert ändern
	secret = 9;
	openkey = mod_exp(generator, secret, order);
	// Erstellen des Objekts für den ioctl()-Befehl SET_SECRET
	brpa3_args io_secret;
	io_secret.value = secret;
	// Setzen des Secret Values
	if (ioctl(fd, BRPA3_SET_SECRET, &io_secret) == -1) {
		perror("ioctl()-Error at SET_SECRET");
	}
	
	// Erstellen des Objekts für den ioctl()-Befehl GET_OPENKEY
	brpa3_args io_open;

	// Anfragen des Openkey
	if (ioctl(fd, BRPA3_GET_OPENKEY, &io_open) == -1) {
		perror("ioctl()-Error at GET_OPENKEY");
	}

	// Ausgabe des Openkey
	printf("Openkey: %hu\n", io_open.value);

	// For-Schleife um Zahlen von 1 bis 58 zu verarbeiten
	printf("Ursprung - Verschlüsselt - Entschlüsselt\n");
	int m;	
	for(m = 1; m <= 58; m++) {
		printf("%i\t - ", m);

		// Verschlüsseln der Zahl
		unsigned short c;
		int tmp;
		unsigned long long first = openkey;
		for(tmp = 1; tmp < secret_sender; tmp++) {
			first = first * openkey;
		}
		first = first * m;
		c = first % order;

		printf("\t%lu\t - ", c);

		// Erstellen eines Buffers und Konvertierung von c in einen String
		const int n = snprintf(NULL, 0, "%lu", c);
		char c_string[n+1];
		snprintf(c_string, n+1, "%lu", c);
		c_string[n] = '\0';

		// Übergeben des Strings an das Modul und lesen der entschlüsselten Zahl
		write(fd, c_string, strlen(c_string));
		read(fd, c_string, strlen(c_string) + 1);

		// Ausgabe der entschlüsselten Zahl
		printf("\t%s", c_string);
		printf("\n");

	}
	// Schließen der Geräte Datei
	close(fd);
}