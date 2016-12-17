#include <stdlib.h>
#include <stdio.h>
#include "builtin.h"

//Array of builtin function addresses and their associated names
BuiltList builtFnc[FNC_NUM] = 
{
	{"quit", quit},
	{"func1", func1},
	{"com2", func2}
};	

Jobs jobs[MAX_JOBS];
int curJID = 0;

//Built in functions definitions

int quit(int argc, char** argv)
{
	free(argv);
	exit(0);
}

int func1(int argc, char** argv)
{
	printf("...INSIDE FUNC1...\n");
	printf("numargs: %d\n", argc);
	while(argc-- > 0)
		printf("%s\n", argv[argc]);
	return 2+2;
}

int func2(int argc, char** argv)
{
	return 3;
}

