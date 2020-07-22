//#include <testing.h>

void TestCompare() {
	int want = 2;
	int got = 1 + 1;
	if (got != want) {
		Failf("got %d but want %d", got, want);
	}
}