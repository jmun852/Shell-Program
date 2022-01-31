#ifndef PROCESS_H
#define PROCESS_H


extern int N;
extern pid_t *bg_pcs;

extern int bg_running( pid_t bg_pcs[] );
extern int pid_running( pid_t pid, pid_t bg_pcs[]);
extern void clean_bgpcs();
extern void handle_tilde( struct commandType *com, int builtInCommand);
extern int launch_ps( parseInfo *info);

#endif
