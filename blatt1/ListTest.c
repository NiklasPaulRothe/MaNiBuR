#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ListTest.h"

struct node {
	struct node *next;
	double val;
};

struct list {
	struct node *first;
	struct node *last;
};

struct list Liste;

struct list addNode(struct list inList, double inValue)
{	
	if (inList.first == NULL) {
		struct node knoten;
		printf("%s\n", "if 1");
		knoten.val = inValue;
		printf("%s\n", "if 2");
		inList.first = &knoten;
		printf("%s\n", "if 3");
		inList.last = &knoten;

		printf("addNote hat  %f als erstes Element eingefügt.\n", inList.first->val);
	} else {
		struct node knoten;
		printf("%s\n", "else 1");
		knoten.val = inValue;
		printf("%s\n", "else 2");
		inList.last->next = &knoten;
		printf("%s\n", "else 3");
		inList.last = &knoten;

		printf("first value der Liste ist %f.\n", inList.first->val);
		printf("addNote hat %f als weiteres Element eingefügt.\n", inList.last->val);
		return inList;
	}
}

int main(int argc, char **argv)
{	
	printf("\n");

	int i, j;
	printf("%s\n", "main 1");

	for (i = 1; i < argc; i++)
	{	
		printf("%s\n", "main for 1");		
		Liste = addNode(Liste, atof(argv[i]));
		printf("%s\n", "main for 2");
	}

	printf("\nProgram finished!\n");
}


/* 
Datengrößen: 
list = 8
node = 12
*/

/*
Zeile 35 verursacht Fehler: Segmentation fault
Auskommentiert steht im first value eine random zahl drin
Vermutliche Lösung: Speicher allokieren. Die Knoten auf die die Liste
zeigt werden überschrieben sobald die Funktion verlassen wird.
*/