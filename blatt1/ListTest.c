#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ListTest.h"

// Fügt einer gegebenen Liste einen neuen Knoten mit dem angegebenen Value hinzu.
// Fügt am Ende an
struct list * addNode(struct list *inList, int inValue)
{	
	if (inList->first == NULL) {
		// Wenn die Liste noch leer ist:
		// Allokiere Speicher und erstelle Knoten mit dem Value		
		struct node *NodeBuff;
		NodeBuff = malloc(sizeof(struct node));
		NodeBuff->val = inValue;
		NodeBuff->next = NULL;
		// Setze dann First und Last der Liste auf den neuen Knoten
		inList->first = NodeBuff;
		inList->last = NodeBuff;

		return inList;
	} else {
		// Enthält die Liste bereits Elemente:
		// Allokiere Speicher und erstelle Knoten mit dem Value	
		struct node *NodeBuff;
		NodeBuff = malloc(sizeof(struct node));
		NodeBuff->val = inValue;
		NodeBuff->next = NULL;
		// Lasse den next Zeiger des letzten Knoten und dann 
		// last der Liste auf den neuen Knoten zeigen
		inList->last->next = NodeBuff;
		inList->last = NodeBuff;

		return inList;
	}
}

// Gibt alle Values einer Liste aus
int printList(struct list *inList) {
	struct node *Counter;
	Counter = inList->first;
	printf("[");
	while (Counter != NULL) {
		printf("%i", Counter->val);
		Counter = Counter->next;
		if (Counter != NULL) {
			printf(" ");
		}
	}
	printf("]");
	return 42;
}

// Löscht eine Liste und gibt den Speicher wieder frei
int clearList(struct list *inList) {
	// 2 temporäre Knoten um noch Zugriff auf den nächsten 
	// Knoten zu haben wenn der jetzige gelöscht wurde.
	struct node *temp, *temp2;
	temp = inList->first;
	while (temp != NULL) {
		temp2 = temp->next;
		// free() Aufruf für jeden Knoten
		free(temp);
		temp = temp2;
	}
	// free() Aufruf für die Liste
	free(inList);
	printf("free memory...\n");
	return 42;

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
	return count;
}

// Testet ob der char eine einzelne Zahl ist oder nicht
int isDigit(char toTest) {
	if (toTest == '0' || toTest == '1' || toTest == '2' || toTest == '3' || toTest == '4' ||
		toTest == '5' || toTest == '6' || toTest == '7' || toTest == '8' || toTest == '9') {
			return 42;
	} 
	return 0;
}

// Testet ob der gegebene String eine ganze (positive oder negative) Zahl ist oder nicht
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

	int i;

	// Erstellen der Liste
	struct list *ListBuff;
	ListBuff = malloc(sizeof(struct list));
	ListBuff->first = NULL;
	ListBuff->last = NULL;

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
			addNode(ListBuff, atoi(argv[i]));
		}

		printf("list input: ");
		printList(ListBuff);
		printf("\n");

		ListBuff = mergesort(ListBuff);

		printf("sorted list: ");
		printList(ListBuff);
		printf("\n");

		clearList(ListBuff);

		printf("\nProgram finished!\n");
	} else {
		printf("Fehler: Bitte gebe nur positive oder negative ganze Zahlen an.\n");
	}
}
