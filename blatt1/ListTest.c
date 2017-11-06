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

struct list * addNode(struct list *inList, double inValue)
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
		printf("%f ", Counter->val);
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
		addNode(ListBuff, atof(argv[i]));
		//printf("%s\n", "main for 2");
	}
	printList(ListBuff);

	clearList(ListBuff);

	printf("\nProgram finished!\n");
}

