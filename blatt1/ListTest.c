#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ListTest.h"

struct node {
	struct node *next;
	int val;
};

struct list {
	struct node *first;
	struct node *last;
};

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


int main(int argc, char **argv)
{
	printf("\n");

	int i, j;
	//printf("%s\n", "main 1");
	struct list *ListBuff;
	ListBuff = malloc(sizeof(struct list));

	for (i = 1; i < argc; i++)
	{
		//printf("%s\n", "main for 1");
		addNode(ListBuff, atoi(argv[i]));
		//printf("%s\n", "main for 2");
	}
	printList(ListBuff);

	mergesort(ListBuff);

	clearList(ListBuff);

	printf("\nProgram finished!\n");
}

// ToDo- Bei dem addNode in der for schleife der Main muss noch eine Abfrage,
// ob der übergebene wert ein int ist
