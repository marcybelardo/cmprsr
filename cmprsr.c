#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ASCII_SIZE 256
#define MAX_HEAP_SIZE 256
#define INTERNAL_NODE -1

typedef struct HuffNode {
	int c;
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
HuffNode *create_huff_node(int c, int weight)
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

	for (int i = 0; i < MAX_HEAP_SIZE; i++) {
		min_heap->arr[i] = malloc(sizeof(HuffNode *));
	}

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
	if (l < min_heap->size &&
	    min_heap->arr[l]->weight < min_heap->arr[smallest]->weight) {
		smallest = l;
	}

	// Check if the node is at an index currently outside the heap's registered size
	// Then compare the weights of the node's right child and itself
	// Pick the smallest value
	if (r < min_heap->size &&
	    min_heap->arr[r]->weight < min_heap->arr[smallest]->weight) {
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
	while (i &&
	       min_heap->arr[i]->weight < min_heap->arr[(i - 1) / 2]->weight) {
		swap(&min_heap->arr[i], &min_heap->arr[(i - 1) / 2]);
		i = (i - 1) / 2;
	}
}

HuffNode *extract_min(MinHeap *min_heap)
{
	if (min_heap->size == 0)
		return NULL;

	HuffNode *tmp = min_heap->arr[0];

	// Min-Heap node deletion
	// switch the root with its highest node
	// and re-run the heapify function
	min_heap->arr[0] = min_heap->arr[min_heap->size - 1];
	min_heap->size--;
	heapify(min_heap, 0);

	return tmp;
}

HuffNode *build_huff_tree(int *char_count)
{
	HuffNode *left, *right, *top;

	MinHeap *min_heap = create_min_heap();

	for (int i = 0; i < ASCII_SIZE; i++) {
		if (char_count[i] > 0)
			insert_min_heap(min_heap, create_huff_node(i, char_count[i]));
	}

	while (min_heap->size > 1) {
		left = extract_min(min_heap);
		right = extract_min(min_heap);

		top = create_huff_node(-1, left->weight + right->weight);
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

void generate_huffman_codes(HuffNode *node, int idx, char code[MAX_HEAP_SIZE], char *code_arr[MAX_HEAP_SIZE])
{
	if (node->c != INTERNAL_NODE) {
		code[idx] = '\0';
		code_arr[node->c] = strdup(code);
	} else {
		if (node->l) {
			code[idx] = '0';
			generate_huffman_codes(node->l, idx + 1, strdup(code), code_arr);
		}
		if (node->r) {
			code[idx] = '1';
			generate_huffman_codes(node->r, idx + 1, strdup(code), code_arr);
		}
	}
}

void read_from_file(const char *filename, int char_count[ASCII_SIZE],
		    unsigned char **buf)
{
	FILE *fp = fopen(filename, "r");
	if (fp == NULL) {
		perror("Failed to open file");
		exit(EXIT_FAILURE);
	}

	size_t capacity = 1024;
	*buf = (unsigned char *)malloc(capacity);
	if (!*buf) {
		perror("Failed to allocate memory");
		fclose(fp);
		exit(EXIT_FAILURE);
	}

	size_t buf_size = 0;
	int c;
	while ((c = fgetc(fp)) != EOF) {
		if (buf_size >= capacity) {
			capacity *= 2;
			*buf = (unsigned char *)realloc(*buf, capacity);
			if (!*buf) {
				perror("Failed to reallocate memory");
				fclose(fp);
				exit(EXIT_FAILURE);
			}
		}
		(*buf)[buf_size++] = (unsigned char)c;

		if (c >= 0 && c < ASCII_SIZE)
			char_count[(unsigned char)c]++;
	}
	(*buf)[buf_size] = '\0';

	fclose(fp);

	printf("%s\n\n", *buf);
}

/*
 *	MAIN
 */

static int encode_f;

int main(int argc, char **argv)
{
	char *infile = NULL;
	char *outfile = NULL;
	int c;

	while (1) {
		static struct option long_options[] =
		{
			{"encode", no_argument, &encode_f, 1},
			{"decode", no_argument, &encode_f, 0},
			{"file", required_argument, 0, 'f'},
			{"out", required_argument, 0, 'o'},
			{0, 0, 0, 0}
		};

		int opt_idx = 0;
		c = getopt_long(argc, argv, "f:o:", long_options, &opt_idx);

		if (c == -1) break;

		switch (c) {
			case 0:
				if (long_options[opt_idx].flag != 0) break;
				printf("option %s", long_options[opt_idx].name);
				if (optarg)
					printf(" with arg %s", optarg);
				printf("\n");
				break;
			case 'f':
				infile = optarg;
				break;
			case 'o':
				outfile = optarg;
				break;
			case '?':
				break;
			default:
				abort();
		}
	}

	if (encode_f) {
		// ENCODE
		printf("Encoding...\n");

		int char_count[ASCII_SIZE] = { 0 };
		char code[MAX_HEAP_SIZE] = { 0 };
		char *code_arr[MAX_HEAP_SIZE];
		
		unsigned char *buf;
		read_from_file(infile, char_count, &buf);

		HuffNode *root = build_huff_tree(char_count);

		for (int i = 0; i < MAX_HEAP_SIZE; i++) {
			code_arr[i] = malloc(MAX_HEAP_SIZE);
		}

		generate_huffman_codes(root, 0, code, code_arr);

		for (int i = 0; i < MAX_HEAP_SIZE; i++) {
			if (strlen(code_arr[i]) > 0)
				printf("%c (0x%x): %s\n", (char)i, i, code_arr[i]);
		}

		free_huff_tree(root);
		free(buf);
	} else {
		// DECODE
		printf("Decode flag set!\n");
	}

	
	return EXIT_SUCCESS;
}
