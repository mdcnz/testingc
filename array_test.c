#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <testing.h>

#include <array.c>

void assert(char *want, char *got) {
	if (strcmp(got, want) != 0) {
		Failf("want \"%s\", got \"%s\"\n", want, got);
	}
}

void TestCreate() {
	Array *a = array_create(0);
	int len = array_len(a);
	if (len != 0) {
		Failf("want 0 length for new array but was %d", len);
	}
	array_delete(a);
}

void TestIndex() {
	Array *names = array_create(0);
	names = append(names, "Malaysia");
	names = append(names, "Indonesia");
	names = append(names, "Singapore");
	names = append(names, "Thailand");
	names = append(names, "Phillipines");
	names = append(names, "Vietnam");
	names = append(names, "Cambodia");

	int want = 2;
	int got = array_find(names, "Singapore");
	if (got != want) {
		Failf("want %d but got %d", want, got);
	}
	array_delete(names);
}

void TestNotFound() {
	Array *names = array_create(0);
	names = append(names, "Malaysia");
	names = append(names, "Indonesia");
	names = append(names, "Singapore");
	names = append(names, "Thailand");
	names = append(names, "Phillipines");
	int want = -1;
	int got = array_find(names, "New Zealand");
	if (got != want) {
		Failf("want %d but got %d\n", want, got);
	}
	array_delete(names);
}

void TestCapacity() {
	Array *a = array_create(0);
	for (uint64_t i = 0; i < 1e3; i++) {
		a = append(a, "x");
	}
	array_delete(a);
}

void TestLoadFromFile() {
	Array *a = array_create(0);
	char *filename = "lines.txt";
	FILE *f = fopen(filename, "w+");

	fprintf(f, "one line\n");
	fprintf(f, "line 2\n");
	fprintf(f, "third line");  //note no new line

	a = array_load(a, f);

	char *want = "third line";
	char *got = array_at(a, 2);
	if (strcmp(want, got) != 0) {
		Failf("want %s, got %s\n", want, got);
	}

	fclose(f);
	remove(filename);
	array_delete(a);
}

void TestNoElement() {
	Array *a = array_create(5);
	if (array_at(a, 0) != NULL) {
		Failf("array_at should be NULL if no element\n");
	}
	array_delete(a);
}

void TestString() {
	char *want = "hello";
	char *heap = (char *)calloc(strlen(want) + 1, sizeof(char *));
	strcpy(heap, want);

	Array *a = array_create(1);
	a = append(a, heap);

	free(heap);

	char *got = array_at(a, 0);
	if (strcmp(got, want) != 0) {
		Failf("want %s, got %s\n", want, got);
	}
	array_delete(a);
}

void TestUTF8() {
	Array *a = array_create(2);

	char *japanese = u8"確認するためのテスト";
	a = append(a, japanese);
	assert(array_at(a, 0), japanese);

	char *arabic = u8"العربية";
	a = append(a, arabic);
	assert(array_at(a, 1), arabic);

	array_delete(a);
}
