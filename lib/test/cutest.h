/*
 * Copyright (C) 2010  Jan Dvorak <mordae@anilinux.org>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _CUTEST_H
#define _CUTEST_H 1

#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <error.h>
#include <wait.h>

/* Single test unit. */
struct cutest {
	/* Name of the test. */
	const char *name;

	/* Function to call. */
	void (*fn)();

	/* Next test in the linked list. */
	struct cutest *next;
};

/* Linked list of tests. */
static struct cutest *cutests = NULL;

#define decl_init(NAME) \
	static void __attribute__((constructor)) _add_init_##NAME(void)

#define decl_fini(NAME) \
	static void __attribute__((destructor)) _add_fini_##NAME(void)

#define decl_test(NAME) \
	static void test_##NAME(); \
	static struct cutest _test_##NAME = { \
		.name = #NAME, \
		.fn = test_##NAME, \
	}; \
	static void __attribute__((constructor)) _add_test_##NAME(void) \
	{ \
		_test_##NAME.next = cutests; \
		cutests = &_test_##NAME; \
	} \
	static void test_##NAME()

#ifndef CUTEST_WITHOUT_MAIN
/* Main that just executes the tests. */
int main(int argc, char **argv)
{
	pid_t pid;
	int status;

	printf("[%s]\n", program_invocation_short_name);

	for (/**/; cutests; cutests = cutests->next) {
		if (argc > 1 && 0 != strcmp(argv[1], cutests->name))
			continue;

		printf(" * %s: ", cutests->name);
		fflush(stdout);

		if (-1 == (pid = fork()))
			error(1, errno, "fork failed");

		if (pid) {
			if (-1 == wait(&status))
				error(1, errno, "wait failed");

			if (WIFSIGNALED(status)) {
				printf("killed by signal %i\n",
				       WTERMSIG(status));
				continue;
			}

			status = WEXITSTATUS(status);

			if (0 == status)
				puts("ok");
			else if (1 == status)
				puts("failed");
			else if (2 == status)
				puts("skipped");
			else
				puts("unknown");
		} else {
			cutests->fn();
			return 0;
		}
	}

	return 0;
}
#endif

/* Skips the test. */
inline static void skip_test()
{
	exit(2);
}

#endif				/* !_CUTEST_H */
