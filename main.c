#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <readline/readline.h>

#include <creed.h>

void help() {
	fprintf(stderr,
"creed - a concatenative text editing language\n"
"  creed [ -s script ] [ -c command ] [ -b buffer ]\n");
}

int main(int argc, char *argv[]) {
	setlocale(LC_ALL, "");

	int i, opt;
	char *buf = NULL;
	char *command = NULL;
	char *script = NULL;
	bool repl = false;
	while ((opt = getopt(argc, argv, "rb:c:s:")) != -1) {
		switch (opt) {
		case 's':
			if (command != NULL) {
				fprintf(stderr, "Can't use both -c and -s\n");
				exit(1);
			}
			script = optarg;
			break;

		case 'c':
			if (script != NULL) {
				fprintf(stderr, "Can't use both -c and -s\n");
				exit(1);
			}
			command = optarg;
			break;

		case 'b':
			buf = optarg;
			break;

		default:
			help();
			return 1;
		}
	}

	if (command == NULL && script == NULL)
		repl = true;

	if (strcmp(argv[0], "screed") == 0)
		buf = "-";

	char *bd = "";

	if (buf != NULL) {
		if (strcmp(buf, "-") == 0) {
			char c;
			bd = calloc(1, BUFSIZ);
			i = 0;
			for (int len = 0; len < BUFSIZ && (c = fgetc(stdin)) != EOF; i++)
				bd[len++] = c;
		} else {
			bd = crReadAll(buf);
		}
	}

	/*while (optind < argc)
		crStatePush(&s, (struct CrVal) {
			.str = crUTF8ToSlice(argv[optind], strlen(argv[optind])),
			.kind = CrValStr
		});*/

	if (!repl) {
		if (command == NULL) {
			if (script == NULL) {
				fprintf(stderr, "no script was passed\n");
				goto fail;
			}

			command = crReadAll(script);
		}

		struct CrState s;
		int res = crRunStr(&s, bd, command);

		printf("%.*ls\n", (int)s.buf.s, s.buf.p);

		crFreeState(&s);

		if (res)
			goto fail;
		else
			goto cleanup;
	}

	struct CrState s;
	crStateInit(&s);
	crStateSetBuf(&s, bd);

	for (;;) {
		char *c = readline(">> ");
		if (!c)
			break;

		struct CrGroup g;
		struct CrErr err = crParseStr(c, &g);
		if (err.kind) {
			crErrPrint(stderr, err);
			goto fail;
		}

		err = crEval(&s, &g);
		if (err.kind) {
			crErrPrint(stderr, err);
			goto fail;
		}

		free(c);
	}

	crFreeState(&s);

cleanup:
	if (strlen(bd))
		free(bd);

	return 0;

fail:
	if (strlen(bd))
		free(bd);
	crFreeState(&s);
	if (script && command)
		free(command);

	return 1;
}
