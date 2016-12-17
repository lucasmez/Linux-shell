#ifndef BUILTIN_H
#define BUILTIN_h

#define CMD_LEN 200 //Max number of characters per command
#define FNC_NUM 3 //Number of built in commands
#define MAX_JOBS 4 //Max number of jobs

typedef int(*funcadd)(int argc, char** argv); //Format for builtin functions

typedef struct {
	char name[20];	//Name ofbuilt in command
	funcadd fnc;	//Function to execute 
} BuiltList;

typedef struct Jobs {	//Linked list that holds each process JID and PID info
	pid_t PID; //Process ID
	short valid; //Is this Job array element already being used
} Jobs;

extern BuiltList builtFnc[]; //Array of builtin function addresses and their associated names
extern Jobs jobs[]; //array containing all jobs executed by shell
extern curJID;

/*---Built in functions definitions-----*/
int quit(int argc, char** argv);
int func1(int argc, char** argv);
int func2(int argc, char** argv);

#endif
