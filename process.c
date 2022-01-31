#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <fcntl.h> //for open/read/
#include <wordexp.h>
#include "parse.h"   // local file include declarations for parse-related structs
#include "printing.h"


pid_t *bg_pcs;
int N = 10;

/* -----------------------------------------------------------------------------
   FUNCTION: bg_running( pid_t bg_pcs[] )

   DESCRIPTION:

   determines whether there are any backgroung processes running.

   pid_t gb_pcs[]: an array that contains all running background processes

   returns: an enum BuiltInCommands
   -------------------------------------------------------------------------------*/
int bg_running( pid_t bg_pcs[] )
{
	int i;
	for (i = 0; i < N; ++i)
	{
		if ( bg_pcs[i] > 0)
		{
			return 1;
		} //if
	} //for

	return 0;


}

/* -----------------------------------------------------------------------------
   FUNCTION: pid_running( pid_t pid, pid_t bg_pcs[] )

   DESCRIPTION:

   determines whether a specific pid is running in an array of
   background processes

   pid_t gb_pcs[]: an array that contains all running background processes

   pid_t pid: The specific pid to look for.

   returns: 1 if the pid is running; 0 otherwise.
   -------------------------------------------------------------------------------*/
int pid_running( pid_t pid, pid_t bg_pcs[])
{
	int i;
	for (i = 0; i < N; ++i )
	{
		if ( pid == bg_pcs[i])
		{
			return 1;
		}
	}

	return 0;
}

/* -----------------------------------------------------------------------------
   FUNCTION: clean_bgpcs()

   DESCRIPTION:

   goes through global variable pid_t bg_pcs[] and determines whether
   a process is running.


   returns: void
   -------------------------------------------------------------------------------*/
void clean_bgpcs()
{
	int i;
	for (i = 0; i < N; ++i)
	{
		if ( ( kill( bg_pcs[i], 0 ) ) != 0)
		{
			bg_pcs[i] = -1;
			--num_bg;
		} //if
	} //for
} //clean_bgpcs

/* -----------------------------------------------------------------------------
   FUNCTION: handle_tilde( struct commandType *com, int builtinCommand)

   DESCRIPTION:

       This function should be called if a tilde character is found
	   as an argument while trying to execute a command. It will expand
	   the tilde and execute the command.


   returns: void
   -------------------------------------------------------------------------------*/
void handle_tilde( struct commandType *com, int builtInCommand)
{

	wordexp_t p;
	char **w;
	char *tilde;

	wordexp(com->VarList[1], &p, 0);
	w = p.we_wordv;

	com->VarList[1] = w[0];

	if ( builtInCommand == 0 )
	{
		if (execvp(com->command, com->VarList) < 0)
		{
			perror("execvp error");
		}
		wordfree(&p);
	} else if ( builtInCommand == 3)
	{
		chdir(com->VarList[1]);
	}



}

void handle_asterisk( struct commandType *com )
{

	wordexp_t p;
	char **w;
	char *tilde;

	wordexp(com->VarList[1], &p, 0);
	w = p.we_wordv;

	if (execvp(com->command, w) < 0)
	{
		perror("execvp error");
	}

	wordfree(&p);
}

/* -----------------------------------------------------------------------------
   FUNCTION: get_in_fd( parseInfo *info )

   DESCRIPTION:

       This function determines the correct file description for info->inFile
	   if a redirection is detected/required.

   returns: int fd.
   -------------------------------------------------------------------------------*/
int get_in_fd( parseInfo *info )
{
	wordexp_t p;
	char **w;
	char *tilde;
	int in_fd;

	if ( info->inFile[0] == '~')
	{
		wordexp(info->inFile, &p, 0);
		w = p.we_wordv;

		in_fd = open(w[0], O_RDONLY);
		wordfree(&p);
	} else
	{
		in_fd = open( info->inFile, O_RDONLY );
	} //if else

	return in_fd;
}

/* -----------------------------------------------------------------------------
   FUNCTION: get_out_fd( parseInfo *info )

   DESCRIPTION:

       This function determines the correct file description for info->outFile
	   if a redirection is detected/required.

   returns: int fd.
   -------------------------------------------------------------------------------*/
int get_out_fd( parseInfo *info )
{
	wordexp_t p;
	char **w;
	char *tilde;
	int out_fd;

	if ( info->outFile[0] == '~')
	{
		wordexp(info->outFile, &p, 0);
		w = p.we_wordv;

		out_fd = open(w[0],  O_WRONLY | O_APPEND | O_CREAT, 0766 );
		wordfree(&p);
	} else
	{
		out_fd = open( info->outFile,  O_WRONLY | O_APPEND | O_CREAT, 0766 );
	} //if else

	return out_fd;
}

/* -----------------------------------------------------------------------------
FUNCTION: launch_pc( parseInfo *info)
DESCRIPTION:

    Backbone of yosh. The plumbing and redirection is handled in this function.

returns: 0 for success; -1 otherwise.
-------------------------------------------------------------------------------*/
int launch_ps( parseInfo *info)
{

	struct commandType *com; 	// com stores command name and Arg list for one command.

	int tmp_in  = dup(STDIN_FILENO); //temporarily saving STDIN
	int tmp_out = dup(STDOUT_FILENO); //temporarily saving STDOUT
	int status;
	int in_fd;
	int out_fd;
	int pid;
	int i;

	// check if redirection
	if ( info->boolInfile )
	{
		if ( strcmp(info->inFile, "") == 0 )
		{
			write(STDOUT_FILENO, "Missing name for redirect.\n", 27);
			return -1;
		}

		in_fd = get_in_fd( info );

	} else
	{
		in_fd = dup(tmp_in);
	} // if else


	for (i = 0; i <= info->pipeNum; ++i)
	{
		int pipe_fd[2];

		com = &(info->CommArray[i]);

		dup2(in_fd, STDIN_FILENO); // read from in_fd
		close(in_fd);

		if ( i == (info->pipeNum))
		{ // last iteration

			if ( info->boolOutfile )
			{
				if ( strcmp(info->outFile, "") == 0 )
				{
					write(STDOUT_FILENO, "Missing name for redirect.\n", 27);
					return -1;
				}

				out_fd = get_out_fd( info );
			} else
			{
				out_fd = dup(tmp_out);
			}


		} else
		{
			pipe( pipe_fd );
			in_fd  = pipe_fd[0];
			out_fd = pipe_fd[1];

		} //if else

		dup2( out_fd, STDOUT_FILENO); //write from out_fd
		close(out_fd);

		if ( (pid = fork()) == 0 )
		{ //child

			if ( com->VarNum >= 2 &&
			(strstr( com->VarList[1], "~" ) ||
			strstr( com->VarList[1], "\'")) )
			{
				handle_tilde(com, 0);
			} else if ( com->VarNum >= 2 && strstr(com->VarList[1], "*") )
			{
				handle_asterisk(com);
			}else
			{

				if (execvp(com->command, com->VarList) < 0)
				{
					perror("execvp error");
				}

			}// if else



			exit(0);
		}

	} //for

	//duplicate STDIN_FILENO and STDOUT_FILENO back to default
	dup2(tmp_in, STDIN_FILENO);
	dup2(tmp_out, STDOUT_FILENO);
	close(tmp_in);
	close(tmp_out);

	if( info->boolBackground )
	{ //if background process

		bg_pcs[++num_bg] = pid;

		print_bg( pid );

		waitpid(pid, &status, WNOHANG);

	} else
	{
		waitpid(pid, &status, 0);
	}

	return 0;

} //launch_ps
