/*
* Modified from "The C Programming Language" by Kernighan & Ritchie, 1978.
* page 18.
*/

#include <stdio.h>

#define IN	1	/* inside a word */
#define OUT	0	/* outside a word */

/* count lines, words and characters in input */
count(file, p_nl, p_nw, p_nc)
FILE	*file;
int	*p_nl, *p_nw, *p_nc;
{
	int c, nl, nw, nc, state;

	state = OUT;
	nl = nw = nc = 0;
	while (EOF != (c = getc(file))) {
		++nc;
		if (c == '\n')
			++nl;
		if (c == ' ' || c == '\n' || c == '\t')
			state = OUT;
		else if (state == OUT) {
			state = IN;
			++nw;
		}
	}
	*p_nl = nl;
	*p_nw = nw;
	*p_nc = nc;
}


