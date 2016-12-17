#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include "builtin.h"

#define DEBUG 0

typedef char flag;

//TODO consider the case where multiple child processes are active
char cmd[CMD_LEN]; //Hold entered line. Also functions as argv array for commands

static void interpret(char* command);
static int isBuiltIn(char* command);
static char** parsecmd(char* command, flag* background, int* argc, int* builtin);
static int createJob(pid_t chdpid);
static void deleteJob(int JID);

//Signal handlers functions
void setSignals(void);
void SIGCHLD_handler(int sig);


int main(int argc, char** argv, char** envp)
{
	setSignals(); //Initialize signal handlers


	//TODO use $PS1 as promptChar ?
	char promptChar = '>';
	memset((void*)jobs, 1, sizeof(struct Jobs) * MAX_JOBS); //Initialize jobs array to 0

	putchar(promptChar);
	while(fgets(cmd, CMD_LEN, stdin) != NULL)
	{
		interpret(cmd);
		putchar(promptChar);
	}
}


static void interpret(char* command)
{
	flag background; //Command is executed on the background
	int builtin, builtret; //is command a built in. return value for builtin
	char** argv; //Arguments for command
	int argc; //Number of arguments in command

	pid_t childpid;
	int jobID, status;
	
	if(command[0] == '\n') //User typed ENTER
		return;

	argv = parsecmd(command, &background, &argc, &builtin);
	
	//TODO Maybe create error codes later
	if(argv == NULL) {
		printf("Background process flag '&' should be added at the end only\n");
		return;
	}

	if(builtin >= 0) { //Built in command 
		builtret = ((builtFnc[builtin]).fnc)(argc, argv);
	}
		
	else { //Not a built in command 
		//TODO Create createJob() and deleteJob() functions
		childpid = fork();
		if(childpid == -1) {
			free(argv);
			perror("fork() error");
			return;
		}

		jobID = createJob(childpid); //Register process as new job
		if(jobID < 0) {
			free(argv);
			fprintf(stderr,"Could not create job\n");		
			return;
		}

		if(childpid == 0) { //Child code
			//Execute command (search in $PATH first) using same enviroment as shell
			if(execvp(*argv, argv) == -1) {
				free(argv);
				deleteJob(jobID); //Unregister job
				perror("execve() error");
				kill(getpid(), SIGKILL); //Kill itself
			}
		}	
		else { //Parent (shell) code
			#ifdef DEBUG
			int i;
			printf("\n----job ID table information:---\n");
			for(i=0; i<MAX_JOBS; i++)
				if(!jobs[i].valid)
					printf("JID:%d\tPID:%d\t\n", i, jobs[i].PID);
			#endif
			if(!background)	
			//wait for child to terminate and reap it. Leave waitpid() if job is stopped
				waitpid(childpid, &status, WUNTRACED);
		}
	}

	free(argv);
}
	

static int createJob(pid_t chdpid)
{
	//Find next avaliable free position in jobs array
	int searched = 0; //Number of JID entries already searched
	int index = curJID; //Position to start from
	while((jobs[index].valid == 0) && (searched < MAX_JOBS)) {
		searched++;
		index = (index+1) % MAX_JOBS; //Go back to 0 if MAX_JOBS was reached
	}

	if (searched == MAX_JOBS) //No space was found
		return -1;

	curJID = index;

	//Initiate structure fields
	jobs[index].PID = chdpid;	
	jobs[index].valid = 0;
	
	return index;
}

static void deleteJob(int JID) {
	jobs[JID].valid = 1;
}


static int isBuiltIn(char* command) {
	int num = 0;
	while(num < FNC_NUM)
		if(strcmp(builtFnc[num++].name, command) == 0)
			return num-1;
	return -1;
}

/*------------------------------------------------------------------
 *Parse command, extracting arguments into argv array, setting bg if
 *command should execute on background (determined by '&' character,
 *returns argv or NULL on error
 *------------------------------------------------------------------
 */
static char** parsecmd(char* command, flag* bg, int* argc, int* builtIndex)
{
	char** argv;		//Return value
	int cmdLen, optLen;
	char *curOpt;		//Holds address of current option
	int numArgs = 0;	//Number of arguments in command
	
	argv = NULL;
	*builtIndex = -1;
	*argc = 0;
	*bg = 0;

	cmdLen = strlen(command);
	command[cmdLen-1] = '\0';	//Get rid of newline
	while(*command == ' ') command++;	//Get rid of trailing spaces

	while(cmdLen > 0)
	{
		while(*command == ' ') { //Get rid of extra white spaces between options
			command++;
			cmdLen--;
		}
		curOpt = command;

		//Split spaces separated words into different option strings
		while((*command != ' ') && (*command != '\0')) {
			command++;
			cmdLen--;	
		}
		*command = '\0';		
		
		//------Parse current command option----------------------

		if(*curOpt == '\0') //No more options
			break;

		else if(strcmp(curOpt, "&") == 0) { //Set Background flag	
			//'&' flag should be last command line option 
			//If curOpt is not last option, return error
			if(cmdLen != 0) {
				int i;
				for(i = 1; i <= cmdLen; i++) //search for non-space characters until EOL
					if(isalnum(*(command+i)))
						break;
				if(i < cmdLen)
					return NULL;
			}

			*bg = 1;
		}

		else { //Check for built in command and populate argv
			if(numArgs == 0) //Check if first argument is a built in
				*builtIndex = isBuiltIn(curOpt);

			numArgs++;
			//Allocate memory for argv
			//TODO Maybe reallocate realloc outside the while loop, because
			//TODO it could use a system call
			argv = realloc(argv, ((numArgs+1) * sizeof(char*)));
			if(argv == NULL) {
				fprintf(stderr, "Error: No more memory\n");
				return NULL;
			}
			argv[numArgs-1] = curOpt; //Last address in argv points to this option in cmd array
		}
			
		//-----------Prepare to read next option------------------------
		command++;
		cmdLen--;

	}
	
	argv[numArgs] = NULL;
	*argc = numArgs;

	return argv;
}

void setSignals(void) {
	signal(SIGCHLD, SIGCHLD_handler);	
}

//TODO finish this function
void SIGCHLD_handler(int sig) {
	return;
	/*
	pid_t chdpid;
	int status;
	//Reap all zombie children
	while ((chdpid = waitpid(-1, &status, 0)) > 0) {
		deleteJob(pidToJid(chdpid));
	}
	*/
}



