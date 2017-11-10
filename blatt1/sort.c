#include <stdio.h>
#include <stdlib.h>
#include "ListTest.h"

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