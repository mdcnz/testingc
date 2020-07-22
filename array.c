#pragma once

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int ARRAY_DEFAULT_SIZE = 10;
const bool DEBUG = false;

typedef struct Arrays {
	int pos;
	int cap;
	char **words;
} Array;

//array creates and initializes an empty array of length len
Array *array_create(uint64_t n) {
	if (n < 0) {
		fprintf(stderr, "attempt to create array with negative length: %ld\n", n);
		exit(EXIT_FAILURE);
	}
	if (n == 0) {
		n = ARRAY_DEFAULT_SIZE;
	}
	Array *arr = (Array *)malloc(sizeof(Array) + sizeof(char *) * n);
	if (!arr) {
		fprintf(stderr, "did not allocate new pointer for array\n");
		exit(EXIT_FAILURE);
	}
	arr->pos = 0;
	arr->cap = n;
	arr->words = (char **)malloc(sizeof(char *) * n);
	if (!arr->words) {
		fprintf(stderr, "did not allocate words array of size %ld\n", n);
		exit(EXIT_FAILURE);
	}
	return arr;
}

//array_len returns number of elements in Array
int array_len(Array *arr) {
	return arr->pos;
}

//array_bytes returns the total size in bytes of the Array
int array_bytes(Array *arr) {
	int total = sizeof(Array);
	for (int i = 0; i < arr->pos; i++) {
		total += sizeof(arr->words[i]);
	}
	return total;
}

void array_dump(Array *arr) {
	fprintf(stderr, "Array %p pos: %d, cap: %d, %d bytes", arr, arr->pos, arr->cap, array_bytes(arr));
	fprintf(stderr, " [");
	for (int i = 0; i < arr->pos; i++) {
		if (i > 0) fprintf(stderr, ",");
		fprintf(stderr, "\"%s\"", arr->words[i]);
	}
	fprintf(stderr, "]\n");
}

//array_delete frees the memory used by each word in the Array, and the Array itself
void array_delete(Array *arr) {
	for (int i = 0; i < arr->pos; i++) {
		free(arr->words[i]);
	}
	free(arr->words);
	free(arr);
}

//grow increases the capacity of the array, if necessary creating a larger array
Array *grow(Array *arr) {
	int cap = arr->cap;
	if (cap == 0) {
		cap = ARRAY_DEFAULT_SIZE;
	}
	cap = (2 * cap) + (cap / 2);
	if (DEBUG) fprintf(stderr, "pos %d, growing to %d\n", arr->pos, cap);

	Array *new = array_create(cap);

	//copy
	for (int i = 0; i < arr->pos; i++) {
		new->words[i] = arr->words[i];
	}

	array_delete(arr);

	return new;
}

//append adds the word to the array and returns a new array
Array *append(Array *arr, char *word) {
	if (DEBUG) array_dump(arr);
	if (arr == NULL) {
		fprintf(stderr, "did not append: array is null");
		exit(EXIT_FAILURE);
	}
	if (arr->pos >= arr->cap) {
		arr = grow(arr);
	}
	char *new = (char *)calloc(strlen(word) + 1, sizeof(char *));
	strcpy(new, word);
	arr->words[arr->pos] = new;
	arr->pos++;
	if (DEBUG) array_dump(arr);
	return arr;
}

//array_at returns element at index
char *array_at(Array *arr, int index) {
	if (index >= arr->pos) {
		if (DEBUG) fprintf(stderr, "did not get element at index %d: out of bounds\n", index);
		return NULL;  // index out of bounds
	}
	return arr->words[index];
}

//binsearch returns the index of the word in words
int binsearch(Array *arr, char *word) {
	int lo, hi, mid, cmp;

	lo = 0;
	hi = arr->pos - 1;

	while (lo <= hi) {
		mid = (lo + hi) / 2;
		cmp = strcmp(word, arr->words[mid]);
		if (DEBUG) fprintf(stderr, "lo %d, hi %d, mid %d, cmp %d\n", lo, hi, mid, cmp);
		if (cmp < 0) {
			hi = mid - 1;
		} else if (cmp > 0) {
			lo = mid + 1;
		} else {
			return mid;
		}
	}

	return -1;
}

//array_find returns the index of the word, or -1 if not found
int array_find(Array *arr, char *word) {
	return binsearch(arr, word);
}

//array_load loads words from the file into the Array
Array *array_load(Array *a, FILE *fd) {
	char buf[1024];
	char b;
	int i = 0;

	rewind(fd);

	for (;;) {
		b = fgetc(fd);	//read byte

		if (b != '\n' && b != EOF) {
			buf[i] = b;
			i++;
			continue;
		}

		if (i > 0) {
			buf[i] = '\0';	//terminate string
			a = append(a, (char *)&buf);
			i = 0;
		}

		if (b == EOF) {
			break;
		}
	}
	return a;
}
