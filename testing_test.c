// test the testing package: inception...!
#include <testing.h>

#include <array.c>
#include <testing.c>

void TestIsTest() {}

void TestIsTestFile() {
	Array *a = array_create(0);
	a = append(a, "file_test.c");
	a = append(a, "another_test.c");
	a = append(a, "this_should_be_a_test.c");
	for (int i = 0; i < array_len(a); i++) {
		char *name = array_at(a, i);
		if (!IsTestFile(name)) {
			Failf("%s should be a test file\n", name);
		}
	}
	array_delete(a);
}

void TestNonTestFiles() {
	Array *bad = array_create(0);
	bad = append(bad, "filetest.c");
	bad = append(bad, ".c");
	bad = append(bad, ".");
	bad = append(bad, "..");
	bad = append(bad, "");
	bad = append(bad, "/");
	bad = append(bad, "@#$%^&*");
	for (int i = 0; i < array_len(bad); i++) {
		char *name = array_at(bad, i);
		if (IsTestFile(name)) {
			Failf("%s should NOT be a test file\n", name);
		}
	}
	array_delete(bad);
}

void testTestCall(char *input, char *want) {
	char *inname = "in_print_test_calls.txt";
	FILE *in = fopen(inname, "w+");
	fprintf(in, "%s", input);
	rewind(in);

	int MAXBUFLEN = 1024;
	char got[MAXBUFLEN];
	char *outname = "out_print_test_cals.txt";
	FILE *out = fopen(outname, "w+");

	printTestCalls(out, in);

	fclose(in);
	remove(inname);

	rewind(out);

	size_t newLen = fread(got, sizeof(char), MAXBUFLEN, out);

	if (ferror(out) != 0) {
		fprintf(stderr, "Error reading file");
	} else {
		got[newLen] = '\0';
	}
	fclose(out);
	remove(outname);

	if (strcmp(got, want) != 0) {
		Failf("got:\n%s\n\nwant:\n%s\n", got, want);
	}
}

void TestPrintTestCalls() {
	char *input = "void TestXXX() {}";
	char *want =
		"testfunc = \"TestXXX()\";\n\t"
		"TestXXX();\n\t"
		"nTests++;\n\t";

	testTestCall(input, want);
}

void TestComment() {
	char *input = "// void TestXXX() {}";
	char *want = "";

	testTestCall(input, want);
}

void check(char *want, char *got) {
	if (strcmp(got, want) != 0) {
		Failf("want '%s' but got '%s'\n", want, got);
	}
}

void TestFilepath() {
	check("some/dir/file", filepath("some/dir", "file"));
}

void TestSingleDir() {
	check("some/file", filepath("some", "file"));
}

void TestDot() {
	check("./f.c", filepath(".", "f.c"));
}
