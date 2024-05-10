CC := gcc
CFLAGS := -Wall -g

BINS := librarytest
all: $(BINS)

# Command to link with shared library and assign rpath.
librarytest: librarytest.c libtest_shared.so
	$(CC) $(CFLAGS) -o $@ $< -L. -ltest_shared -Wl,-rpath=.

libtest_static.a: libtest.o
	ar rcs $@ $^

libtest_shared.so: libtest.c libtest.h
	$(CC) $(CFLAGS) -fPIC -shared -o $@ $< -lc

libtest.o: libtest.c libtest.h
	$(CC) $(CFLAGS) -c $<

.PHONY: clean
clean:
	rm *.o *.a *.so $(BINS)