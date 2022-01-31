#ifndef PRINTING_H
#define PRINTING_H

extern int num_bg; //number of background processes

extern int count_str( char *str );
extern int num_digits( int num );
extern void convert( int num, char *str);
extern void my_print(char *str, ...);
extern void print_bg( int pid );
extern void print_jobs( pid_t bg_pcs[]);
extern void print_help( void );

#endif
