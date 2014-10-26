
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>

void term_all_processes()
{
    kill(-1, SIGKILL);
    signal(SIGALRM, SIG_IGN);
}

void kill_all_processes(int signo)
{
    kill(-1, SIGTERM);
    signal(SIGALRM, term_all_processes);
    alarm(5);

    while (waitpid(-1, NULL, 0)) {
        if (errno == ECHILD) {
            break;
        }
    }
}

int main ( int argc, char *argv[] )
{
    if (argc < 2)
    {
        printf( "usage: %s <arguments>", argv[0] );
    }


    signal(SIGALRM, SIG_IGN);
    signal(SIGTERM, kill_all_processes);
    signal(SIGPWR, kill_all_processes);
    signal(SIGINT, kill_all_processes);

    const char * init_dir = "/etc/initrc.d";
    if (access(init_dir, F_OK) == 0) {
        struct dirent **namelist;
        int n, m;
        n = scandir(init_dir, &namelist, 0, alphasort);
        if ( n < 0 ) {
            printf("Error scanning directory %s", init_dir);
            return 11;
        } else {
            for(m = 0; m < n; m++) {
                char * file;
                int len;
                len = snprintf(NULL, 0, "%s/%s", init_dir, namelist[m]->d_name);
                if (!(file = malloc((len + 1) * sizeof (char)))) {
                    printf("Could not allocate memory. Exiting.");
                    return 15;
                }
                snprintf(file, len+1, "%s/%s", init_dir, namelist[m]->d_name);
                struct stat s;
                if( stat(file,&s) == 0 && s.st_mode & S_IFREG && access(file, F_OK|X_OK) == 0) {
                    int i = ((system(file) & 0xff00) >> 8);
                    if(i != 0) {
                        printf("An error occured running initrc.d (%s:%d).", file, i);
                        return i;
                    }
                }
                free(file);
                free(namelist[m]);
            }
        }
        free(namelist);
    }

    char * args[argc];

    int i;
    for (i=0; i<argc-1; i++) 
        args[i] = argv[i+1];
    args[argc-1] = NULL;

    int pid;
    pid = fork();
    if(!pid)
    {
        if ( execvp(args[0], args) == -1)
            fprintf(stderr, "Fatal Error: failed to run %s.\n", args[0]);

        _exit(0);
    }else{
        int status;
        if (waitpid(pid, &status, WNOHANG) == pid)
        {
            fprintf(stderr, "Fatal Error: %s exited unexpectedly with code %d\n", args[0], status);
            exit(1);
        }
        while (waitpid(-1, NULL, 0)) {
            if (errno == ECHILD) {
                break;
            }
        }

    }
}
