#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>      
#include <sys/wait.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

typedef struct {
	char comm[1024];
	pid_t pid;													//History entry holds pid,timestamps,command
	struct timeval start;
	struct timeval end;
}history_logs;

history_logs logs[100];
int n=0;                   // buffer for last 100 commands history.


//print history for terminating
void terminate(int sig){
	printf("\n\n===== Command History =====\n");
	for (int i=0; i<n; i++){
		long st=logs[i].start.tv_sec*1000+logs[i].start.tv_usec/1000;
		long end=logs[i].end.tv_sec * 1000 + logs[i].end.tv_usec / 1000;
		long tim=end-st;

		printf("[%d] PID=%d | \"%s\" | Start=%ld ms | Duration=%ld ms\n",i + 1,logs[i].pid,logs[i].comm,st,tim);
    }
	if (sig == SIGINT){
        printf(" Bye Byee.. (;;)\n");   // check and exiting program.
        exit(0);
    } else {
        printf("===========================\n");
    }
}

int main(){
	char* line=NULL;
	size_t line_len=0;

	signal(SIGINT,terminate);



	while(1){
		printf("arzl@donkey:??");  //print username.
		
		if (getline(&line, &line_len, stdin) == -1) {  // read one line of input
			if (feof(stdin)) {                 // Ctrl-D: print history and exit cleanly
    	    	terminate(0);
        		exit(0);
    		}
	        perror("getline");
	        exit(1);
	    }

	    // save command in history
	    history_logs h;
	    char line_og[1024];
		strncpy(line_og, line, sizeof(line_og));
		line_og[sizeof(line_og)-1] = '\0';
		strcpy(h.comm, line_og); 

		char* coms[10];
		int ncom=0;
		//split input by pipe
		char* part= strtok(line,"|");
		while (part!=NULL){

			//store each pipeline stage
			coms[ncom++]=part;
			if (ncom >= 10) {                      // max number of commands
    			fprintf(stderr, "error: too many pipeline stages (max 10)\n");
    			ncom = 0;                          // invalidate this line
    		break;
			}
			part=strtok(NULL,"|");
		}

		char* args[10][100];
		int argc[10];

		for (int i=0;i<ncom;i++){
			argc[i]=0;
			char* comm=strtok(coms[i], " \t\n");
			while (comm!=NULL){
				args[i][argc[i]++]=comm;
				if (argc[i] >= 100) {                  // max number of arguments
    				fprintf(stderr, "error: too many arguments in stage %d (max 100)\n", i+1);
    				ncom = 0;                          // abort this pipeline
    				break;
				}

				comm=strtok(NULL, " \t\n");
			}
			args[i][argc[i]]=NULL;  //NULL-terminate argv for execvp
		}
		if (ncom == 0) continue; 
		if (args[0][0] == NULL) continue; //ignores blank line
		if (strcmp(args[0][0], "history") == 0) {  //handling history command
		    terminate(0);
		    continue;
		}

		int pipes[ncom-1][2]; //creates n-1 pipes for n commands in pipeline
		pid_t pids[ncom];
		for(int i=0;i<ncom-1;i++){
			if(pipe(pipes[i]) == -1){
				perror("pipe");
				exit(1);
			}
		}
		//timestamping
		if(gettimeofday(&h.start,NULL)==-1)perror("gettimeofday-start");

		//fork and exec for each pipeline stage. Making child processes to execute each command.
		for(int i=0;i<ncom;i++){
    		if (ncom ==0) continue;
			pids[i] = fork();
			if (pids[i] == -1) {
		    perror("fork");
		    // close all pipes
		    for (int k=0; k<ncom-1; k++) {
		        close(pipes[k][0]);
		        close(pipes[k][1]);
		    }
		    // reap any children already started
		    for (int k=0; k<i; k++) {
		        int st; (void)waitpid(pids[k], &st, 0);
		    }
		    ncom = 0;              // abort this pipeline
		    break;
			}
			if (ncom == 0) break;      // if we aborted due to fork error


			if (pids[i] == 0){
				if (i>0){
					//child takes input from previous stage if not first stage
				
					if (dup2(pipes[i-1][0],STDIN_FILENO) == -1) {
					        perror("dup2- stdin"); 
					        exit(1);
					    }
				}

				if(i<ncom-1){
					if (dup2(pipes[i][1],STDOUT_FILENO) == -1) {
					    perror("dup2-stdout");
					    exit(1);
					}
				//child output to next stage if not last stage
				}

				for(int j=0;j<ncom-1;j++){
					close(pipes[j][0]);
					close(pipes[j][1]);   // child close all pipe fds
				}
				if(args[i][0]!=NULL){
					execvp(args[i][0],args[i]);
					perror("execvp");   // exec the command
					exit(1);
				}	
			}

		}

		for (int i = 0; i < ncom - 1; i++) {
	    	close(pipes[i][0]);
	        close(pipes[i][1]);  //parent close fds
	    }

	    int last_status = 0;
	    for (int i = 0; i < ncom; i++) {  // parent wait for child process to finish
	        int st = 0;
	        if (waitpid(pids[i], &st, 0) == -1) perror("waitpid");
	        if (i == ncom - 1) last_status = st;
	    }

	    if(gettimeofday(&h.end,NULL)==-1) perror("gettimeofday-end");
	    h.pid = pids[ncom - 1];
	    if(n<100){
	    logs[n++]=h;  //record to history 
	    }else{
	    	printf("Error: history exceeded" );
	    	exit(1);
	    }
	}

	free(line); //clean buffer
	return 0;
}