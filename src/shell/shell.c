#include "shell.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* read_line(void) {
	char* line = NULL;
	size_t bufsize = 0;

	if (getline(&line, &bufsize, stdin) == -1) {
		if (feof(stdin)) {
			free(line);
			exit(EXIT_FAILURE);
		}
		else {
			free(line);
			perror("error while reading the line from stdin");
			exit(EXIT_FAILURE);
		}
	}
	return line;
}

char** split_line(char* line) {
	int bufsize = 64;
	int i = 0;
	char** tokens = malloc(bufsize * sizeof(char*));
	char* token;

	if (!tokens)
	{
		fprintf(stderr, "allocation error in split_line: tokens\n");
		exit(EXIT_FAILURE);
	}
	token = strtok(line, TOK_DELIM);
	while (token != NULL) {
		/* handle comments */
		if (token[0] == '#') {
			break;
		}
		tokens[i] = token;
		i++;
		if (i >= bufsize) {
			bufsize += bufsize;
			tokens = realloc(tokens, bufsize * sizeof(char*));
			if (!tokens) {
				fprintf(stderr, "reallocation error in split_line: tokens");
				exit(EXIT_FAILURE);
			}
		}
		token = strtok(NULL, TOK_DELIM);
	}
	tokens[i] = NULL;
	return (tokens);
}
