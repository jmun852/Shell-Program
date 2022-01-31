#include <unistd.h>
#include <fcntl.h> //for open/read/
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>


int num_bg = 0;

/* -----------------------------------------------------------------------------
FUNCTION: count_str( int num )
DESCRIPTION:

Counts the number of characters in a string

str: the string to be examined

returns: int the number of characters in char *str
-------------------------------------------------------------------------------*/
int count_str( char *str )
{

	int i;
	int count;

	for (i = 0; str[i]; ++i);

	return i;

}


/* -----------------------------------------------------------------------------
FUNCTION: num_digits ( int num )
DESCRIPTION:

Counts the number of digits of an int num.

num: the integer to be examined.

returns: int the number of digits of int num
-------------------------------------------------------------------------------*/
int num_digits( int num )
{
	int len = 0;

	while (num != 0)
    {
        len++;
        num /= 10;
    }

	return len;
}

/* -----------------------------------------------------------------------------
FUNCTION: conver( int num, char *str)
DESCRIPTION:

converts an integer, num to a string, str.

num: the integer to be converted.
str: string conversion result will be stored in this string.

returns: void
-------------------------------------------------------------------------------*/
void convert( int num, char *str)
{
	static char representation[] = "0123456789";


	int i, rem, len = 0, n;

	len = num_digits(num);

	for (i = 0; i < len; i++)
    {
		str[len - (i + 1)] = representation[num % 10];
		num = num / 10;
    }
    str[len + 1] = '\0';

}

void print_bg( int pid )
{
	char pid_str[10];
	char i_str[10];
//	char *str = "";

	convert(pid, pid_str);
	convert(num_bg, i_str);

	write(STDOUT_FILENO, "[", 1);
	write(STDOUT_FILENO, i_str, count_str(i_str) );
	write(STDOUT_FILENO, "]    ", 5);
	write(STDOUT_FILENO, pid_str, count_str(pid_str) );
	write(STDOUT_FILENO, "\n", 1);

} //print_bg

/* -----------------------------------------------------------------------------
FUNCTION: print_jobs( pid_t bg_pcs[] )
DESCRIPTION:

prints the background processes to standard output.

bg_pcs: an array of pid_t that are back ground processes

returns: void
-------------------------------------------------------------------------------*/
void print_jobs( pid_t bg_pcs[])
{
	int i;
	char *size;

	for (i = 0 ; i <= num_bg ; ++i)
	{

		if ( bg_pcs[i] > 0 && i != 0)
		{
			char buf[num_digits(i) + 1];
			convert(i, buf);

			size = malloc( 12 * sizeof(char) );
			sprintf(size, "%11d", bg_pcs[i] );
			write(1, "[", 2);
			write(1, buf, num_digits(i));
			write(1, "]- running ", 11);
			write(1, size, 12 );
			write(1, "\n", 1);
			free(size);


		}


	} //for
}

/* -----------------------------------------------------------------------------
FUNCTION: print_help()
DESCRIPTION:

prints the help menu to standard output.

returns: void
-------------------------------------------------------------------------------*/
void print_help()
{

	char *str = "cd [dir]\nexit\nhelp\nhistory\njob\n";

	write(STDOUT_FILENO, str, count_str(str));


} //print_help
