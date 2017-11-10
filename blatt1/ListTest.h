#ifndef _LISTTEST_H
	#define _LISTTEST_H
	#define BUFFER_SIZE 32
#endif

// Listentyp. Merkt sich den ersten und letzten Knoten
struct list {
	struct node *first;
	struct node *last;
};

// Knotentyp. Speichert einen Verweis auf den nächsten Knoten und einen int Value
struct node {
	struct node *next;
	int val;
};

int printList(struct list *inList);

int getListLength(struct list *inList);

struct list * mergesort(struct list *inList);