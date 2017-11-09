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

	//printf("\n You inserted following numbers: \n");
	while (Counter != NULL) {
		printf("%i ", Counter->val);
		Counter = Counter->next;
	}
	//printf("\n printing finished! \n");

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
	return count;
}

//Nimmt eine Liste und sortiert diese mit Hilfe des mergesort Algorithmus
struct list * mergesort(struct list *inList) {
	//Initialisiere nötige Variablen
	int listLength, i;
	struct list *tmp_list, *finalList;
	tmp_list = malloc(sizeof(struct list));
	tmp_list->first = inList->first;
	tmp_list->last = inList->last;
	listLength = getListLength(inList);

	printf("Mergesort: ");
	printList(inList);
	printf("\n");

	//Wenn die Liste mehr als ein Element enthält, Liste aufsplitten
	if (listLength > 1) {
		
		printf("split ");
		printList(inList);
		printf(" into ");

		//Eine Hälfte der Elemente in eine neue Liste schreiben
		for (i = 0; i < (listLength/2); i++) {	

			if (i + 2 >= (listLength / 2)) {
				inList->last = tmp_list->first;
			}

		tmp_list->first = tmp_list->first->next;
		}

		//setze die Enden beider Listen auf NULL
		inList->last->next = NULL;
		tmp_list->last->next = NULL;

		printList(inList);
		printf(" and ");
		printList(tmp_list);
		printf("\n");

		//rekursiver Aufruf von mergesort für beide Listen.
		inList = mergesort(inList);
		tmp_list = mergesort(tmp_list);

	//Hat die Liste nur ein Element wird die Rekursion abgebrochen.
	} else {
		free(tmp_list);
		return inList;
	}

	//Dritte Liste erstellen, in der die sortierte Liste zwischengespeichert wird.
	finalList = malloc(sizeof(struct list));
	finalList->last = NULL;
	finalList->first =NULL;

	printf("merge ");
	printList(tmp_list);
	printf(" and ");
	printList(inList);
	printf("\n");

	//solange in beiden Listen noch Elemente stehen das jeweils kleinere in die neue Liste schreiben.
	while (tmp_list->first != NULL && inList->first != NULL) {
		if (inList->first != NULL && inList->first->val <= tmp_list->first->val) {
			//wenn noch kein Element in der Ergebnisliste initialisiere fürs erste Element
			if (finalList->last == NULL) {
				finalList->first = inList->first;
				finalList->last = finalList->first;
			//ansonsten Element hinten anfügen
			} else {
				finalList->last->next = inList->first;
				finalList->last = finalList->last->next;
			}

			//Ursprungsliste weiterrücken und next für das eingefügte element auf NULL setzen
			inList->first = inList->first->next;
			finalList->last->next = NULL;
			
		} else if (tmp_list->first != NULL && tmp_list->first->val <= inList->first->val) {
			//wenn noch kein Element in der Ergebnisliste initialisiere fürs erste Element
			if (finalList->last == NULL) {
				finalList->first = tmp_list->first;
				finalList->last = finalList->first;
			//ansonsten Element hinten anfügen
			} else {
				finalList->last->next = tmp_list->first;
				finalList->last = finalList->last->next;
			}

			//Ursprungsliste weiterrücken und next für das eingefügte element auf NULL setzen
			tmp_list->first = tmp_list->first->next;
			finalList->last->next = NULL;
		}
	}

	//wenn eine Liste leer ist den Rest der zweiten Liste an die Ergebnisliste anfügen
	if (tmp_list->first == NULL) {
		finalList->last->next = inList->first;
		finalList->last = inList->last;
	} else {
		finalList->last->next = tmp_list->first;
		finalList->last = tmp_list->last;
	}

	free(inList);
	free(tmp_list);
	return finalList;
}

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

		ListBuff = mergesort(ListBuff);

		printf("Sorted List: ");
		printList(ListBuff);
		printf("\n");

		clearList(ListBuff);

		printf("\nProgram finished!\n");
	} else {
		printf("Fehler: Bitte gebe nur positive oder negative ganze Zahlen an.\n");
	}
}
