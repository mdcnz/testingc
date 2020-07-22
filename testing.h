#pragma once

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

char *testfunc = "";

void Failf(const char *fmt, ...) {
	fprintf(stderr, "%s:\n\t", testfunc);
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	fprintf(stderr, "\n");
	exit(EXIT_FAILURE);
}
