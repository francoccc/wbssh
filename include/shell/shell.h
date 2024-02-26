#ifndef __SHELL_H__
#define __SHELL_H__

#define TOK_DELIM " \t\r\n\a\""

#ifdef __cplusplus
extern "C" {
#endif
	void shell_interactive();

	void shell_no_interactive();

	char* read_line(void);
	char** split_line(char *line);

#ifdef __cplusplus
}
#endif
#endif