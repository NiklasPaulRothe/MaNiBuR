#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ListTest.h"

// Knotentyp. Speichert einen Verweis auf den nächsten Knoten und einen int Value
struct node {
	struct node *next;
	int val;
};

// Listentyp. Merkt sich den ersten und letzten Knoten
struct list {
	struct node *first;
	struct node *last;
};

// Fügt einer gegebenen Liste einen neuen Knoten mit dem angegebenen Value hinzu.
// Fügt am Ende an
struct list * addNode(struct list *inList, int inValue)
{	
	if (inList->first == NULL) {
		struct node *NodeBuff;
		NodeBuff = malloc(sizeof(struct node));
		//printf("%s\n", "if 1");
		NodeBuff->val = inValue;
		//printf("%s\n", "if 2");
		inList->first = NodeBuff;
		//printf("%s\n", "if 3");
		inList->last = NodeBuff;

		//printf("addNote hat  %f als erstes Element eingefügt.\n", inList->first->val);
	} else {
		struct node *NodeBuff;
		NodeBuff = malloc(sizeof(struct node));
		//printf("%s\n", "else 1");
		NodeBuff->val = inValue;
		//printf("%s\n", "else 2");
		inList->last->next = NodeBuff;
		//printf("%s\n", "else 3");
		inList->last = NodeBuff;

		//printf("first value der Liste ist %f.\n", inList->first->val);
		//printf("addNote hat %f als weiteres Element eingefügt.\n", inList->last->val);
		return inList;
	}
}

// Gibt alle Values einer Liste aus
int printList(struct list *inList) {
	struct node *Counter;
	Counter = inList->first;

	printf("\n You inserted following numbers: \n");
	while (Counter != NULL) {
		printf("%i ", Counter->val);
		Counter = Counter->next;
	}
	printf("\n printing finished! \n");

}

// Löscht eine Liste und gibt den Speicher wieder frei
int clearList(struct list *inList) {
	struct node *temp, *temp2;
	temp = inList->first;
	while (temp != NULL) {
		temp2 = temp->next;
		free(temp);
		temp = temp2;
	}
	free(inList);
	printf("free memory...\n");

}

// Gibt die Anzahl der Elemente einer Liste zurück
int getListLength(struct list *inList) {
	struct node *temp;
	temp = inList->first;
	int count;
	count = 0;
	while (temp != NULL) {
		count++;
		temp = temp->next;
	}
	printf("Anzahl der Elemente in der Liste: %i \n", count);
	return count;
}

/*struct list* split(struct list *inList, int length) {
	if (1 == length) {
		printf("split if\n");
		return inList;
	} else {
		printf("split else 1\n");
		struct list *secHalf;
		struct node *nFirst, *nLast;
		
		printf("split else 2\n");
		nFirst = inList->first;
		printf("split else 3\n");
		nLast = secHalf->last;
		printf("split else 4\n");
		int newlength, i;
		printf("split else 5\n");
		newlength = length / 2;
		printf("split else 6\n");
		for (i = 1; i < newlength; i++) {
			printf("split else for 1\n");
			nFirst = nFirst->next;
			printf("split else for 2\n");
		}
		secHalf->first = nFirst;
		secHalf->last = nLast;
	}
}

int mergesort(struct list *inList) {
	int listLength;
	printf("mergesort 1\n");
	listLength = getListLength(inList);
	printf("mergesort 2\n");
	split(inList, listLength);
	printf("mergesort 3\n");
}*/

// Testet ob der char eine einzelne Zahl ist oder nicht
int isDigit(char toTest) {
	if (toTest == '0' || toTest == '1' || toTest == '2' || toTest == '3' || toTest == '4' ||
		toTest == '5' || toTest == '6' || toTest == '7' || toTest == '8' || toTest == '9') {
			return 42;
	} 
	return 0;
}

// Testet ob der gegebene String eine ganze Zahl(inkl negativ) ist oder nicht
int isNumber(char *input) {	
	int i, isNumber;
	isNumber = 42;
	if (!isDigit(input[0]) && input[0] != '-') {
		isNumber = 0;
	}
	for (i = 1; i < strlen(input); i++) {
		if (!isDigit(input[i])) {
			isNumber = 0;
		}
	}
	return isNumber;	 
}


int main(int argc, char **argv)
{	
	printf("\n");

	int i, j;
	//printf("%s\n", "main 1");

	// Erstellen der Liste
	struct list *ListBuff;
	ListBuff = malloc(sizeof(struct list));
	
	// Testet ob die Übergabeparameter den Vorschriften entsprechen
	int areNumbers;
	areNumbers = 42;
	for (i = 1; i < argc; i++) {
		if (!isNumber(argv[i])) {
			areNumbers = 0;
		}
	}

	// Eine Liste wird nur erstellt wenn die Übergabeparameter korrekt waren.
	if (areNumbers) {
		for (i = 1; i < argc; i++) {	
			//printf("%s\n", "main for 1");
			addNode(ListBuff, atoi(argv[i]));
			//printf("%s\n", "main for 2");
		}
		printList(ListBuff);

		//mergesort(ListBuff);

		clearList(ListBuff);

		printf("\nProgram finished!\n");
	} else {
		printf("Fehler: Bitte gebe nur positive oder negative ganze Zahlen an.\n");
	}
}
