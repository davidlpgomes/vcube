#include <stdio.h>

typedef struct node_set {
	int* nodes;
	ssize_t size;
	ssize_t offset;
} node_set;

extern node_set *cis(int i, int s);
extern void set_free(node_set* nodes);
