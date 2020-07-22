# Testing C

A toy program to explore a Go like testing process in C.

## Usage

1. Run `make` to create a `testing` binary
2. Copy the `testing` binary and `testing.h` header to your C project
3. Create test files and add test functions
4. Run `./testing` to test

Test files are files with suffix `_test.c`.

Test functions are functions matching `void Test...()`.

Call `Failf(pattern, args...)` to fail a test function. `Failf` is the only function provided. It's defined in testing.h.

### Example

A directory contains the `testing` binary, the `testing.h` header and a test file called `sample_test.c`:

```
$ ls
sample_test.c  testing  testing.h
```

`sample_test.c`:

```c
#include <testing.h>

void TestCompare() {
	int want = 2;
	int got = 1 + 1;
	if (got != want) {
		Failf("want %d but got %d", want, got);
	}
}
```

The `testing` binary is executed to compile and run the tests, passing successfully:

```
$ ./testing
ok (1 tests)
```

### Test Failure

If a test function fails, it prints it's name and the message, for example:

```c
void TestFailure() {
    Failf("you shall not pass");
}
```

```
$ ./testing
TestFailure():
	you shall not pass
```
