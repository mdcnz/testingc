#include <dirent.h>
#include <dlfcn.h>	//dlopen and other shared object functions
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef char *error;

const char *testrunner = "_generated_test_runner.c";

//cleanupAtExit is called on normal program exit (even if tests exit).
void cleanupAtExit() {
	system("rm -f ./_generated_test_runner.c");
	system("rm -f ./_generated_test_runner.so");
}

const int TEST_NAME_LENGTH_MAX = 100;

const char *TEST_TEMPLATE =
	"testfunc = \"Test%s()\";\n\t"
	"Test%s();\n\t"
	"nTests++;\n\t";

void printTest(FILE *out, char *method) {
	fprintf(out, TEST_TEMPLATE, method, method);
}

typedef struct Tester {
	FILE *in;
	char *test;
} Tester;

//caller responsible for closing intput file.
Tester *tester_create(FILE *in) {
	Tester *t = (Tester *)calloc(1, sizeof(Tester));
	t->in = in;
	t->test = (char *)calloc(TEST_NAME_LENGTH_MAX, sizeof(char));
	return t;
}

void tester_delete(Tester *t) {
	free(t->test);
	free(t);
}

const char TEST_PATTERN[] = "void Test";

//parsing C functions has many special cases, so instead consider using the gcc parser itself:
//gcc -I. -S -o /dev/stdout testing_test.c | grep '@function'

//next returns the next test method or NULL
char *next(Tester *t) {
	int last = strlen(TEST_PATTERN);
	int i = 0;
	int pos = 0;
	char c;
	char skipto = 0;
	while ((c = fgetc(t->in)) != EOF) {
		if (skipto) {
			if (c != skipto) {
				continue;
			}
			skipto = 0;
		}

		//skip blocks
		if (c == '{') {
			skipto = '}';
			continue;
		}

		//skip includes
		if (c == '#') {
			skipto = '\n';
			continue;
		}

		//skip comments
		if (c == '/') {
			skipto = '\n';
			continue;
		}

		if (pos == last) {
			if (c == '(') {
				t->test[i] = '\0';	// terminate string
				return t->test;
			}

			t->test[i] = c;
			i++;
		} else if (c == TEST_PATTERN[pos]) {
			pos++;
		} else {
			pos = 0;
		}
	}
	return NULL;
}

// printTestCalls finds test signatures in files and prints their calls directly
// to the output file.
void printTestCalls(FILE *out, FILE *input) {
	Tester *t = tester_create(input);

	char *method;
	while ((method = next(t))) {
		printTest(out, method);
	}

	tester_delete(t);
}

error test(void) {
	void *lib = dlopen("./_generated_test_runner.so", RTLD_NOW);
	if (!lib) {
		return "did not open shared library";
	}

	dlerror();	//clear any existing error

	int (*RunTests)() = dlsym(lib, "RunTests");
	error err = dlerror();
	if (err != NULL) {
		dlclose(lib);
		return err;
	}
	RunTests();
	dlclose(lib);
	return NULL;
}

error compile(const char *dir) {
	char *cmdfmt = "gcc -fpic -std=c11 -Wall -Werror -fmax-errors=1 -I%s -shared -o _generated_test_runner.so %s";
	int len = strlen(cmdfmt) + strlen(dir) - 2 + strlen(testrunner) - 2;
	char *cmd = (char *)calloc(len, sizeof(char));
	sprintf(cmd, cmdfmt, dir, testrunner);
	int status = system(cmd);
	free(cmd);
	if (status != 0) {
		return "did not compile";
	}
	return NULL;
}

//IsTestFile returns true if the name matches the test file pattern.
bool IsTestFile(char *name) {
	return strstr(name, "_test.c") != NULL;
}

//caller MUST free the result
char *filepath(const char *dir, const char *file) {
	int len = strlen(dir) + 1 + strlen(file);
	char *filename = (char *)calloc(len, sizeof(char));
	sprintf(filename, "%s/%s", dir, file);
	return filename;
}

//generateTestHarness searches for test files in a directory and writes a c file to call their tests
error generateTestHarness(const char *dirname) {
	DIR *dir;
	dir = opendir(dirname);
	if (!dir) {
		fprintf(stderr, "not a directory: %s\n", dirname);
		return "not a directory";
	}

	FILE *out;
	out = fopen(testrunner, "w+");
	if (out == NULL) {
		fprintf(stderr, "did not open file: %s\n", testrunner);
		return "did not open test harness file";
	}

	int nTests = 0;
	struct dirent *dp;
	while ((dp = readdir(dir)) != NULL) {
		if (IsTestFile(dp->d_name)) {
			fprintf(out, "#include <%s>\n", dp->d_name);  // include the file containing Test functions
			nTests++;
		}
	}
	closedir(dir);

	if (nTests < 1) {
		fprintf(stderr, "no test files ending _test.c in %s\n", dirname);
		fclose(out);
		return "no test files";
	}

	//entry point
	fprintf(out, "extern void RunTests(void) { \n\t");
	fprintf(out, "int nTests = 0;\n\t");

	dir = opendir(dirname);
	while ((dp = readdir(dir)) != NULL) {
		if (!IsTestFile(dp->d_name)) {
			continue;
		}

		char *filename = filepath(dirname, dp->d_name);
		FILE *testfile = fopen(filename, "r");
		free(filename);
		if (testfile == NULL) {
			fprintf(stderr, "did not open test file: %s\n", filename);
			fclose(out);
			closedir(dir);
			return "did not open test file";
		}
		printTestCalls(out, testfile);
		fclose(testfile);
	}
	closedir(dir);

	// close main method
	fprintf(out, "printf(\"ok (%%d tests)\\n\", nTests); \n");
	fprintf(out, "}\n");
	fclose(out);
	return NULL;
}

// testFilesInDir searches for test files and tests them
error testFilesInDir(const char *dirname) {
	error err = generateTestHarness(dirname);
	if (err) {
		return err;
	}
	err = compile(dirname);
	if (err) {
		return err;
	}
	return test();
}

// main runs the tests on a given dir, or the current dir by default
int main(int argc, char *argv[]) {
	char *dir = ".";
	if (argc > 1) {
		dir = argv[1];
	}

	atexit(cleanupAtExit);

	error err = testFilesInDir(dir);
	if (err) {
		fprintf(stderr, "Fail: %s\n", err);
	}
}
