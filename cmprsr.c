#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ASCII_SIZE      256
#define MAX_HEAP_SIZE   256
#define MAX_CODE_LENGTH 256

char *lookup_table[MAX_CODE_LENGTH];

struct CharCount {
	unsigned char character;
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

/*
 *	HUFFMAN TREE IMPLEMENTATIONS
 */
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

HuffNode *build_huff_tree(struct CharCount *count_arr, int size)
{
	HuffNode *left, *right, *top;

	MinHeap *min_heap = create_min_heap();

	for (int i = 0; i < size; i++) {
		insert_min_heap(
			min_heap,
			create_huff_node(count_arr[i].character, count_arr[i].count)
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

void free_huff_tree(HuffNode *root)
{
	if (root) {
		free_huff_tree(root->l);
		free_huff_tree(root->r);
		free(root);
	}
}

/*
 *	ENCODING AND DECODING
 */
void encode(HuffNode *root, char *code, int len)
{
	if (!root) return;

	// if node is a leaf, store it in the lookup table
	if (!root->l && !root->r) {
		code[len] = '\0';
		lookup_table[(unsigned char)root->c] = strdup(code);
		return;
	}

	// append '0' to the code and traverse the left subtree
	code[len] = '0';
	encode(root->l, code, len + 1);

	// append '1' to the code and traverse the right subtree
	code[len] = '1';
	encode(root->r, code, len + 1);
}

/*
 *	CHAR ARRAYS AND SORTING
 */
static int create_char_array(int *char_count, struct CharCount *count_arr)
{
	int index = 0;

	for (int i = 0; i < ASCII_SIZE; i++) {
		if (char_count[i] > 0) {
			count_arr[index].character = (unsigned char)i;
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

/*
 *	MAIN
 */
int main(int argc, char **argv)
{
	char *filename;
	char *buf;
	size_t buf_size = 0;
	size_t buf_capacity = 1;

	buf = (char *)malloc(buf_capacity * sizeof(char));
	if (!buf) {
		perror("Failed to allocated memory");
		return EXIT_FAILURE;
	}
	
	int char_count[ASCII_SIZE] = {0};

	if (argc != 2) {
		perror("Incorrect number of arguments");
		return EXIT_FAILURE;
	} else {
		filename = argv[1];
	}

	FILE *fp;
	char c;

	fp = fopen(filename, "r");
	if (fp == NULL) {
		perror("Failed to open file");
		return 1;
	}

	while ((c = fgetc(fp)) != EOF) {
		char_count[(unsigned char)c]++;

		if (buf_size + 1 >= buf_capacity) {
			buf_capacity *= 2;
			buf = (char *)realloc(buf, buf_capacity * sizeof(char));
			if (!buf) {
				perror("Failed to reallocate memory");
				fclose(fp);
				return 1;
			}
		}

		buf[buf_size++] = c;
	}

	buf[buf_size] = '\0';

	fclose(fp);

	printf("%s\n\n", buf);

	int non_zero = 0;
	for (int i = 0; i < ASCII_SIZE; i++) {
		if (char_count[i] > 0) non_zero++;
	}

	struct CharCount *char_count_arr = (struct CharCount *)malloc(non_zero * sizeof(struct CharCount));
	int valid_counts = create_char_array(char_count, char_count_arr);

	sort_char_count_array(char_count_arr, valid_counts);

	// HUFFMAN CODING TREE ALGORITHM
	//
	// 1. Take the lowest two nodes, get the sum of their weights
	// 2. Set left node to the lower w val, right node to the higher w val
	// 3. Re-sort list with the new tree node introduced
	// 4. Continue with the next two lowest nodes
	
	for (int i = 0; i < ASCII_SIZE; i++) {
		if (char_count_arr[i].count > 0) {
			printf(
				"CHAR: %c (0x%02x): COUNT: %d\n",
				char_count_arr[i].character,
				char_count_arr[i].character,
				char_count_arr[i].count
			);
		}
	}
	
	HuffNode *root = build_huff_tree(char_count_arr, valid_counts);

	for (int i = 0; i < MAX_CODE_LENGTH; i++) {
		lookup_table[i] = NULL;
	}

	char code[MAX_CODE_LENGTH];
	encode(root, code, 0);

	FILE *out_fp = fopen("out.cmpr", "wb");
	if (!out_fp) {
		perror("Failed to open file.");
		return EXIT_FAILURE;
	}

	for (int i = 0; i < strlen(buf); i++) {
		printf("%s", lookup_table[(unsigned char)buf[i]]);

		char *huff_code = lookup_table[(unsigned char)buf[i]];
		if (huff_code) fputs(huff_code, out_fp);
	}
	printf("\n");

	fclose(out_fp);

	for (int i = 0; i < MAX_CODE_LENGTH; i++) {
		free(lookup_table[i]);
	}

	free_huff_tree(root);
	free(buf);

	return EXIT_SUCCESS;
}

