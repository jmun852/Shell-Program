
/* -----------------------------------------------------------------------------
FILE: yosh.c

NAME: yosh

DESCRIPTION: A SIMPLE SHELL
-------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <fcntl.h> //for open/read/

#include "parse.h"   // local file include declarations for parse-related structs
#include "printing.h"
#include "process.h"



#define PATH_MAX 4096


enum BUILTIN_COMMANDS { NO_SUCH_BUILTIN=0, EXIT, JOBS, CD, KILL, HELP, HISTORY, EXCLAMATION };


int is_bg = 0;

/* -----------------------------------------------------------------------------
FUNCTION: buildPrompt()
DESCRIPTION:

Builds and returns a prompt as a string

char *path: a string that the prompt will be stored in. the function returns the same
string address.

returns: the altered char *path.

-------------------------------------------------------------------------------*/
char * buildPrompt( char* path)
{

	char *cwd;

	int i = 0;

	cwd = malloc(PATH_MAX * sizeof(char));

	path[0] = '{';

	getcwd(cwd, PATH_MAX);

	for (; cwd[i] != '\0'; i++)
	{
		path[i + 1] = cwd[i];

	}

	path[i + 1] = '}';

	path[i + 2] = '\0';

	free(cwd);

	return path;

} //build prompt

/* -----------------------------------------------------------------------------
   FUNCTION: isBuiltInCommand( char *cmd)

   DESCRIPTION:

   determines whether a string is a built in command for yosh.

   char *cmd: the command to be interpreted

   returns: an enum BuiltInCommands
   -------------------------------------------------------------------------------*/
int isBuiltInCommand( char * cmd )
{
	if( strncmp(cmd, "exit", strlen( "exit" ) ) == 0 )
	{
		return EXIT;
  	} else if( strncmp(cmd, "cd", strlen( "cd" ) ) == 0 )
	{
		return CD;
	} else if( strncmp(cmd, "jobs", strlen( "jobs" ) ) == 0 )
	{
		return JOBS;
	} else	if( strncmp(cmd, "kill", strlen( "kill" ) ) == 0 )
	{
		return KILL;
	} else if ( strncmp(cmd, "help", strlen( "help" ) ) == 0 )
	{
		return HELP;
	} else if ( strncmp(cmd, "history", strlen( "history" ) ) == 0 )
	{
		return HISTORY;
	} else if ( cmd[0] == '!' )
	{
		return EXCLAMATION;
	}
	return NO_SUCH_BUILTIN;
}


/* -----------------------------------------------------------------------------
FUNCTION: handler( int sign )
DESCRIPTION:

    handles sigchild signal for background processes

-------------------------------------------------------------------------------*/
void handler( int sign )
{

	if (sign == SIGCHLD)
	{

		while( waitpid(0, NULL, WNOHANG) > 0);
		clean_bgpcs();
	}

} //handler

/* -----------------------------------------------------------------------------
FUNCTION: main()
DESCRIPTION:

    main function of the yosh program.
-------------------------------------------------------------------------------*/
int main( int argc, char **argv )
{
	char * cmdLine;
	char * path;
  parseInfo *info; 		// info stores all the information returned by parser.
  struct commandType *com; 	// com stores command name and Arg list for one command.

  pid_t pid;

  int j;
  int in_fd;
  int out_fd;
  register int num_commands = 0;


  signal( SIGCHLD, handler);

  register HIST_ENTRY **hist_list;
  register int i;

  enum BUILTIN_COMMANDS builtInCommand;

  bg_pcs = calloc(N, sizeof(pid_t) );

  fprintf( stdout, "This is the SHELL version 0.1\n" ) ;

  using_history();
  stifle_history(10);

  while(1)
  {
	  path = malloc(PATH_MAX * sizeof(char));
    	// insert your code here

	  cmdLine = readline( buildPrompt(path) ) ;
	  if( cmdLine == NULL )
	  {
		  fprintf(stderr, "Unable to read command\n");
		  continue;
	  }

	  // insert your code about history and !x !-x here


	  add_history( cmdLine );
	  num_commands++;


	  // calls the parser
	  info = parse( cmdLine );
	  if( info == NULL )
	  {
		  free(cmdLine);
		  continue;
	  }

//	  print_info( info );

	  //com contains the info. of the command before the first "|"
	  com = &info->CommArray[0];
	  if( (com == NULL)  || (com->command == NULL))
	  {
		  free_info(info);
		  free(cmdLine);
		  continue;
	  }

	  // com contains the commands
	  // determine whether command is builtin and executes it if it is.
	  if( (builtInCommand = isBuiltInCommand( com->command )) != NO_SUCH_BUILTIN )
	  { //built in command given

		  if ( builtInCommand == EXIT)
		  {
			  if ( bg_running( bg_pcs ) == 1)
			  {
				  write(STDOUT_FILENO, "please terminate running processes before exit\n", 47);
			  } else {
				  free_info(info);
				  free(cmdLine);
				  free(path);
				  free(bg_pcs);
				  write(STDOUT_FILENO, "Terminated successfully\n", 24);
				  exit(1);
			  }
		  } else if ( builtInCommand == CD)
		  {
			  if ( com->VarNum >= 2 && com->VarList[1][0] == '~')
			  {

				  handle_tilde(com, CD);
			  } else
			  {
				  chdir(com->VarList[1]);
			  }
		  } else if (builtInCommand == JOBS )
		  {
			  print_jobs( bg_pcs );
		  } else if ( builtInCommand == KILL )
		  {

			  if ( com->VarList[1][0] == '%')
			  {
				  char *arg = com->VarList[1] + 1;
				  int i_arg = atoi( arg );



				  if ( bg_pcs[i_arg] > 0 && i_arg <= num_bg)
				  {
					  kill( bg_pcs[i_arg], SIGKILL);
					  clean_bgpcs();

				  } else
				  {
					  write( STDOUT_FILENO, "-yosh: kill: ", 13);
					  write( STDOUT_FILENO, arg, count_str(arg));
					  write( STDOUT_FILENO, ": no such job\n", 14);
				  } //if else

			  } else
			  {
				  int pid = atoi(com->VarList[1]);
				  if ( pid_running( pid, bg_pcs) )
				  {
					  kill( pid, SIGKILL);
				  }else
				  {
					  write( STDOUT_FILENO, "-yosh: kill: ", 13);
					  write( STDOUT_FILENO, com->VarList[1] , count_str( com->VarList[1] ) );
					  write( STDOUT_FILENO, ": no such job\n", 14);
				  }
			  }

		  } else if ( builtInCommand == HELP)
		  {
			  print_help();
		  } else if ( builtInCommand == HISTORY)
		  {

			  hist_list = history_list();
			  for (i = 0; hist_list[i]; ++i)
			  {
				  printf("%d: %s\n", i + history_base, hist_list[i]->line );
			  }

		  } else if ( builtInCommand == EXCLAMATION )
		  {
			  char *num;
			  register int k;

			  num = &com->command[1];

			  hist_list = history_list();
			  k = atoi(num) ;

			  if (k < 0)
			  {
				  k = history_length + k;
			  }
			  k--;

			  info = parse( hist_list[k]->line);
			  launch_ps( info );
		  }//if else

	  } else
	  { // not built in
		  launch_ps( info );
	  }

	  //insert your code here / commands etc.
	  free_info(info);
	  free(cmdLine);
	  free(path);

  }/* while(1) */

	  free(bg_pcs);

}
