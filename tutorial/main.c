#include <stdio.h>

main(argc,argv)
	int argc;
	char **argv;
{
	char	*p;
	long	tlinect = 0, twordct = 0, tcharct = 0;
	int	linect, wordct, charct;
	int	doline = 0, doword = 0, dochar = 0;
	FILE	*file;

	if (argc > 1 && argv[1][0] == '-') {
		for (p = argv[1] + 1; *p; ++p)
			switch(*p) {
			case 'l':
				doline = 1;
				break;
			case 'w':
				doword = 1;
				break;
			case 'c':
				dochar = 1;
				break;
			default:
				fprintf(stderr, "invalid option: -%c\n",
					*p);
			case '?':
				fputs("usage: wc [-lwc] [files]\n",stderr);
				return 1;
			}
		argv += 2;
	}
	else {
		++argv;
		doline = doword = dochar = 1;
	}

	do {
		if (!*argv) {
			count(stdin, &linect, &wordct, &charct);
			print(doline, doword, dochar, linect, wordct, charct,
			      "");
			return;
		}
		else {
		   	file = fopen(*argv, "r");
			if (file == NULL) {
			    	perror(*argv);
				return 1;
			}
			count(file, &linect, &wordct, &charct);
			fclose(file);
			print(doline, doword, dochar, linect, wordct, charct,
			      *argv);
		}
		tlinect += linect;
		twordct += wordct;
		tcharct += charct;
	} while(*++argv);

	print(doline, doword, dochar, tlinect, twordct, tcharct, "total");
	return 0;
}

static print(doline, doword, dochar, linect, wordct, charct, file)
	int	doline,	doword, dochar;
	int	linect, wordct, charct;
	char	*file;    
{
	if (doline)
		printf(" %7ld", linect);
	if (doword)
		printf(" %7ld", wordct);
	if (dochar)
		printf(" %7ld", charct);
	printf(" %s\n", file);
}
