#include <stdio.h>
#include <stdlib.h>

#define ASCII_SIZE 256
#define MAX_HEAP_SIZE 256

struct CharCount {
	char character;
	int count;
};

typedef struct HuffNode {
	char c;
	int weight;
	struct HuffNode *l;
	struct HuffNode *r;
} HuffNode;

typedef struct {
	int size;
	HuffNode *arr[MAX_HEAP_SIZE];
} MinHeap;

void swap(HuffNode **a, HuffNode **b)
{
	HuffNode *tmp = *a;
	*a = *b;
	*b = tmp;
}

HuffNode *create_huff_node(char c, int weight)
{
	HuffNode *new = malloc(sizeof(HuffNode));
	new->c = c;
	new->weight = weight;
	new->l = new->r = NULL;

	return new;
}

MinHeap *create_min_heap()
{
	MinHeap *min_heap = malloc(sizeof(MinHeap));
	min_heap->size = 0;

	return min_heap;
}

void heapify(MinHeap *min_heap, int idx)
{
	int smallest = idx;

	// Given a node at index i,
	// its children are at indices 2i + 1 and 2i + 2
	int l = 2 * idx + 1;
	int r = 2 * idx + 2;

	// We want to sort the array such that the smallest node is at the root
	// So first we have to find the smallest element between the current set of
	// nodes.
	
	// Check if the node is at an index currently outside the heap's registered size
	// Then compare the weights of the node's left child and itself
	// Pick the smallest value
	if (l < min_heap->size && min_heap->arr[l]->weight < min_heap->arr[smallest]->weight) {
		smallest = l;
	}

	// Check if the node is at an index currently outside the heap's registered size
	// Then compare the weights of the node's right child and itself
	// Pick the smallest value
	if (r < min_heap->size && min_heap->arr[r]->weight < min_heap->arr[smallest]->weight) {
		smallest = r;
	}

	// If there is a smaller value than the node we are currently indexed at
	// We swap the positions of that node and the indexed node
	// We take the smallest node and run the algorithm again
	//
	// In this way, the smallest value will get pushed upwards in the array
	if (smallest != idx) {
		swap(&min_heap->arr[smallest], &min_heap->arr[idx]);
		heapify(min_heap, smallest);
	}
}

void insert_min_heap(MinHeap *min_heap, HuffNode *node)
{
	// Increase the registered heap size
	min_heap->size++;

	// The index of the new node
	int i = min_heap->size - 1;

	min_heap->arr[i] = node;

	// Loop over the parents of each node, starting by comparing the new
	// node and its parent (For a given node i, its parent is
	// located at (i - 1) / 2)
	while (i && min_heap->arr[i]->weight < min_heap->arr[(i - 1) / 2]->weight) {
		swap(&min_heap->arr[i], &min_heap->arr[(i - 1) / 2]);
		i = (i - 1) / 2;
	}
}

HuffNode *extract_min(MinHeap *min_heap)
{
	if (min_heap->size == 0) return NULL;

	HuffNode *tmp = min_heap->arr[0];

	// Min-Heap node deletion
	// switch the root with its highest node
	// and re-run the heapify function
	min_heap->arr[0] = min_heap->arr[min_heap->size - 1];
	min_heap->size--;
	heapify(min_heap, 0);

	return tmp;
}

HuffNode *build_huff_tree(struct CharCount **count_arr, int size)
{
	HuffNode *left, *right, *top;

	MinHeap *min_heap = create_min_heap();

	for (int i = 0; i < size; i++) {
		insert_min_heap(
			min_heap,
			create_huff_node(count_arr[i]->character, count_arr[i]->count)
		);
	}

	while (min_heap->size > 1) {
		left = extract_min(min_heap);
		right = extract_min(min_heap);

		top = create_huff_node('$', left->weight + right->weight);
		top->l = left;
		top->r = right;

		insert_min_heap(min_heap, top);
	}

	return extract_min(min_heap);
}

static int read_from_file(char *filename, int *char_count)
{
	FILE *fp;
	char ch;

	fp = fopen(filename, "rb");
	if (fp == NULL) {
		printf("Error opening file.\n");
		return 1;
	}

	while ((ch = fgetc(fp)) != EOF) {
		char_count[ch]++;
	}

	fclose(fp);

	return 0;
}

static int create_char_array(int *char_count, struct CharCount *count_arr)
{
	int index = 0;

	for (int i = 0; i < ASCII_SIZE; i++) {
		if (char_count[i] > 0) {
			count_arr[index].character = (char)i;
			count_arr[index].count = char_count[i];
			index++;
		}
	}

	return index;
}

int compare(const void *a, const void *b)
{
	struct CharCount *char_count_a = (struct CharCount *)a;
	struct CharCount *char_count_b = (struct CharCount *)b;
	return char_count_a->count - char_count_b->count;
}

static void sort_char_count_array(struct CharCount *count_arr, int size)
{
	qsort(count_arr, size, sizeof(struct CharCount), compare);
}

int main(int argc, char **argv)
{
	char *text_data;
	int char_count[ASCII_SIZE] = {0};

	if (argc != 2) {
		text_data = "ABCBCDCDDEEEDEE\0";
		for (char *c = text_data; *c != '\0'; c++) {
			char_count[*c]++;
		}
	} else {
		char *filename = argv[1];
		read_from_file(filename, char_count);
	}

	struct CharCount count_arr[ASCII_SIZE];
	int valid_counts = create_char_array(char_count, count_arr);

	sort_char_count_array(count_arr, valid_counts);

	for (int i = 0; i < valid_counts; i++) {
		printf("Character: '%c' Count: %d\n", count_arr[i].character, count_arr[i].count);
	}

	// HUFFMAN CODING TREE ALGORITHM
	//
	// 1. Take the lowest two nodes, get the sum of their weights
	// 2. Set left node to the lower w val, right node to the higher w val
	// 3. Re-sort list with the new tree node introduced
	// 4. Continue with the next two lowest nodes

	return 0;
}

