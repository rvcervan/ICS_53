#include "icssh.h"
#include <readline/readline.h>
#include "linkedList.h"


int flag = 0; //if flag is 1, a child has been terminated

void sigchld_handler(int sig){
    // if signal is recieved then set flag to 1
    flag = 1;
    //printf("child terminated with signal %d\n", sig);
    return;
}

void sigusr_handler(int sig){
    printf("Hi User! I am process %d\n", getpid());
    return;
}

int main(int argc, char* argv[]) {
	int exec_result;
	int exit_status;
	pid_t pid;
	pid_t wait_result;
	char* line;
    //int flag = 0; //if flag is not 0, a child has been terminated

    List_t* myList = malloc(sizeof(List_t)); //Linked list holding bg processes
    myList->head = NULL;
    myList->length = 0;
    myList->comparator = NULL; // not sure if I need a comparator

#ifdef GS
    rl_outstream = fopen("/dev/null", "w");
#endif

	// Setup segmentation fault handler
	if (signal(SIGSEGV, sigsegv_handler) == SIG_ERR) {
		perror("Failed to set signal handler");
		exit(EXIT_FAILURE);
	}

    // sigchld handler here?
    if(signal(SIGCHLD, sigchld_handler) == SIG_ERR){
        exit(EXIT_FAILURE);
    }

    // SIGUSR2 here?
    if(signal(SIGUSR2, sigusr_handler) == SIG_ERR){
        exit(EXIT_FAILURE);
    }

    // print the prompt & wait for the user to enter commands string
	while ((line = readline(SHELL_PROMPT)) != NULL) {
        // MAGIC HAPPENS! Command string is parsed into a job struct
        // Will print out error message if command string is invalid
		job_info* job = validate_input(line);
        if (job == NULL) { // Command was empty string or invalid
			free(line);
			continue;
		}

        //Prints out the job linked list struture for debugging
        #ifdef DEBUG   // If DEBUG flag removed in makefile, this will not longer print
            debug_print_job(job);
        #endif
        
        //for part2. Do I need to execve()?--------------------------------------------------------------
        //check for flag, if flag is 1. wait() then remove bg process from bgentry linked list
        if(flag == 1){
            pid_t bg_wait_result;
            //waitpid with WNOHANG on each of the process of the linked list
            //bg_wait_result = waitpid(pid_of_each_pid_in_bgentry, wut, WNOHANG); returns 0 if no child(ren) specified by pid has 
                                                                                 //yet to change state
                                                                                //otherwise returns pid of child terminated
       
            while((bg_wait_result = waitpid(-1, &exit_status, WNOHANG)) > 0){
                //iterate through linked lis and find pid of bg process that matches bg_wait_result
                //if found remove bgentry_t and node_t
                
                node_t* head = myList->head;
                int index = 0; //index of linked list
                while(head != NULL){
                    //printf("%d\n", bg_wait_result);
                    node_t* temp = head->next;
                    //check pid in head->value->pid
                    pid_t the_pid = ((bgentry_t*)head->value)->pid;
                    if(bg_wait_result == the_pid){
                        printf(BG_TERM, the_pid, ((bgentry_t*)head->value)->job->line);
                        free_job(((bgentry_t*)head->value)->job);
                        free(head->value);
                        removeByIndex(myList, index);
                    }
                    head = temp;
                    ++index;
                }
                
            }
            flag = 0;
            

        }
        


		// example built-in: exit------------------------------------------------------------------
		if (strcmp(job->procs->cmd, "exit") == 0) {
            //should I free the linked list and all the stuff for bg?
            if(myList->length != 0){
                node_t* head = myList->head;
                printf("%d\n", myList->length);
                while(head != NULL){
                    node_t* temp = head->next;
                    pid_t bg_pid = ((bgentry_t*)head->value)->pid;
                    kill(bg_pid, SIGKILL);
                    waitpid(bg_pid, &exit_status, 0);
                    printf(BG_TERM, bg_pid, ((bgentry_t*)head->value)->job->line);
                    free_job(((bgentry_t*)head->value)->job);
                    free(head->value);
                    removeFront(myList);

                    head = temp;
                    
                }
            }


            deleteList(myList);
            free(myList);

			// Terminating the shell
			free(line);
			free_job(job);
            validate_input(NULL);
            return 0;
		}
        else if(strcmp(job->procs->cmd, "cd") == 0) {
            //chdir to an existing directory
            int result;
            //argc should be at most 2, 3 or more are ignored
            if(*((job->procs->argv)+1) == NULL){
                //cd into home
                result = chdir(getenv("HOME"));
                
            }
            else if(strcmp(*((job->procs->argv)+1), ".")==0){
                //cd into current directory .
                result = chdir(".");
            }
            else if(strcmp(*((job->procs->argv)+1), "..")==0){
                //go up 1 directory
                result = chdir("..");
            }
            else{
                result = chdir(*((job->procs->argv)+1));
            }
            char cwd[100];
            getcwd(cwd, 100);
            
            
            //check if result is 0
            if(result == 0){//change is successful
                fprintf(stdout, "%s\n", cwd);
            }
            else{ //change failed, print to stderr
                fprintf(stderr, "%s", DIR_ERR);
            }
            //should I free line and job before continue?
            free(line);
			free_job(job);
            continue;
        }
        else if(strcmp(job->procs->cmd, "estatus")==0){
            pid_t child;
            child = wait(&exit_status);
            if(WIFEXITED(exit_status)){
                printf("%d\n", WEXITSTATUS(exit_status));
            }
            else if (WIFSIGNALED(exit_status)){
                psignal(WTERMSIG(exit_status), "EXIT SIGNAL");
            }
            free(line);
			free_job(job);
            continue;
        }
        else if(strcmp(job->procs->cmd, "bglist") == 0){
            node_t* head = myList->head;
            while(head != NULL){
                node_t* temp = head->next;
                print_bgentry((bgentry_t*)head->value);
                head = temp;
            }
            free(line);
            free_job(job);
            continue;
        }
        else if(strcmp(job->procs->cmd, "ascii53")==0){
            puts(
                    
                                                             
"8 8888      88               ,o888888o.               8 8888\n"
"8 8888      88              8888     `88.             8 8888\n" 
"8 8888      88           ,8 8888       `8.            8 8888\n"
"8 8888      88           88 8888                      8 8888\n"
"8 8888      88           88 8888                      8 8888\n"
"8 8888      88           88 8888                      8 8888\n"
"8 8888      88           88 8888                      8 8888\n"
"` 8888     ,8P           `8 8888       .8'            8 8888\n"
"  8888   ,d8P               8888     ,88'             8 8888\n"
"   `Y88888P'                 `8888888P'               8 8888\n"

                                                 
                );
            free(line);
            free_job(job);
            continue;
        }

        //part 4 stuff
        int in = 0;
        int out = 0;
        int err = 0;
        int both = 0;

        int fd_in;
        if(job->procs->in_file != NULL){
            if((fd_in = open(job->procs->in_file, O_RDONLY)) < 0){
                fprintf(stderr, "%s", RD_ERR);
                exit(1);
            }
            in = 1;
        
        }
        
        int fd_out;
        if(job->procs->out_file != NULL){
            if((fd_out = open(job->procs->out_file, O_CREAT | O_WRONLY, 0666)) < 0){
                fprintf(stderr, "%s", RD_ERR);
                exit(1);
            }
            out = 1;
        }

        int fd_err;
        if(job->procs->err_file != NULL){
            if((fd_err = open(job->procs->err_file, O_CREAT | O_WRONLY, 0666)) < 0){
                
                fprintf(stderr, "%s", RD_ERR);
                exit(1);
            }
            err = 1;
        }

        int fd_both;
        if(job->procs->outerr_file != NULL){
            if((fd_both = open(job->procs->outerr_file, O_CREAT | O_WRONLY, 0666)) < 0){
                
                fprintf(stderr, "%s", RD_ERR);
                exit(1);
            }
            both = 1;
        }


        int p1[2] = { 0, 0 }; //for 1 pipe

        int p2[2] = { 0, 0 }; //for 2 pipes

        if(job->nproc == 2){ //there is 1 pipe
            pipe(p1);
        }

        if(job->nproc == 3){ //there is 2 pipes
            pipe(p1);
            pipe(p2);
        }

        



		// example of good error handling!
		if ((pid = fork()) < 0) {
			perror("fork error");
			exit(EXIT_FAILURE);
		}
        //part 2 stuff
        if(job->bg == true){ // it is an background job
            if(pid == 0){// if zero, then its the child
                //or signal here?
                if(in == 1){ dup2(fd_in, 0); close(fd_in); }
                if(out == 1) { dup2(fd_out, 1); close(fd_out); }
                if(err == 1) { dup2(fd_err, 2); close(fd_err); }
                if(both == 1) { dup2(fd_both, 2); dup2(fd_both, 1); close(fd_both); }
                
                proc_info* proc = NULL;
                if(job->nproc == 1){
		            proc = job->procs;
		    	    exec_result = execvp(proc->cmd, proc->argv);
                }
                else if(job->nproc == 2){ //there is 1 pipe
                    close(p1[0]);
                    dup2(p1[1], 1);
                    close(p1[1]);

                    proc = job->procs;
                    //fprintf(stderr, "%s\n", proc->cmd);
                    exec_result = execvp(proc->cmd, proc->argv);

                }
                else if(job->nproc == 3){ //there is 2 pipes
                    
                    kill(getpid(), SIGTERM);
                }

                if(exec_result < 0) { //error check, should I also free up bgentry as well?
                    printf(EXEC_ERR, proc->cmd);

                    free_job(job);
                    free(line);
                    validate_input(NULL);



                    exit(EXIT_FAILURE);
                }
                //handle signal here?
                

            }
            else{//process is a parent but doesn't wait for child to finish executing, so continue with the shell.
                //what do I do in here?
                //add the job to bgentry
                bgentry_t *bg_job = malloc(sizeof(bgentry_t));//bgentry struct to hold bg process (child process);
                
                bg_job->job = job;
                bg_job->pid = pid;
                bg_job->seconds = time(NULL);
                insertRear(myList, (void*)bg_job); //insert bg_entry to linked list

                if(job->nproc == 2){ //there is 1 pipe, 2nd half
                    int sec_pid = fork();
                    if(sec_pid  == 0){
                        close(p1[1]);
                        dup2(p1[0], 0);
                        close(p1[0]);
                        
                        proc_info* proc2 = job->procs->next_proc;
                        //fprintf(stderr, "%s\n", proc2->cmd);
                        exec_result = execvp(proc2->cmd, proc2->argv);

                        if(exec_result < 0){ 
                            printf(EXEC_ERR, proc2->cmd);
                            free_job(job);
                            free(line);
                            validate_input(NULL);
                            exit(EXIT_FAILURE);
                        }
                    }
                    else{
                        close(p1[1]);
                        wait_result = waitpid(sec_pid, &exit_status, 0);
                        if(wait_result < 0) {
                            printf(WAIT_ERR);
                            exit(EXIT_FAILURE);
                        }
                    }
                } 
                else if(job->nproc == 3){
                    int fir_pid = fork();
                    if(fir_pid == 0){
                        dup2(p1[1], 1);
                        close(p1[0]);
                        close(p1[1]);
                        close(p2[0]);
                        close(p2[1]);

                        proc_info* proc = job->procs;
                        exec_result = execvp(proc->cmd, proc->argv);
                        if(exec_result < 0){ 
                            printf(EXEC_ERR, proc->cmd);
                            free_job(job);
                            free(line);
                            validate_input(NULL);
                            exit(EXIT_FAILURE);
                        }

                    }

                    int sec_pid = fork();
                    if(sec_pid == 0){
                        dup2(p1[0], 0);
                        dup2(p2[1], 1);
                        close(p1[0]);
                        close(p1[1]);
                        close(p2[0]);
                        close(p2[1]);

                        proc_info* proc2 = job->procs->next_proc;
                        exec_result = execvp(proc2->cmd, proc2->argv);
                        if(exec_result < 0){ 
                            printf(EXEC_ERR, proc2->cmd);
                            free_job(job);
                            free(line);
                            validate_input(NULL);
                            exit(EXIT_FAILURE);
                        }

                    }
               

                    int thr_pid = fork();
                    if(thr_pid == 0){
                        dup2(p2[0], 0);
                        close(p1[0]);
                        close(p1[1]);
                        close(p2[0]);
                        close(p2[1]);

                        proc_info* proc3 = job->procs->next_proc->next_proc;
                        
                        exec_result = execvp(proc3->cmd, proc3->argv);
                        if(exec_result < 0){ 
                            printf(EXEC_ERR, proc3->cmd);
                            free_job(job);
                            free(line);
                            validate_input(NULL);
                            exit(EXIT_FAILURE);
                        }

                    }

                    close(p1[0]);
                    close(p1[1]);
                    close(p2[0]);
                    close(p2[1]);
                    waitpid(fir_pid, &exit_status, 0);
                    waitpid(sec_pid, &exit_status, 0);
                    waitpid(thr_pid, &exit_status, 0);
                        
                        
                }
                
                // As the parent, wait for the foreground job to finish
                
                if(job->nproc == 2){
                    close(p1[0]);
                }

                if(job->nproc == 3){
                    close(p1[0]);
                    close(p1[1]);
                    close(p2[0]);
                    close(p2[1]);
                }


                //free_job(job);
                free(line);
                continue;
            }
        }
        else{//it is a foreground job
    		if (pid == 0) {  //If zero, then it's the child process
                
                //fprintf(stderr, "before everyting and stuff ------------------------------------------------------"); 
                //get the first command in the job list
                if(in == 1){ dup2(fd_in, 0); close(fd_in); }
                if(out == 1) { dup2(fd_out, 1); close(fd_out); }
                if(err == 1) { dup2(fd_err, 2); close(fd_err); }
                if(both == 1) { dup2(fd_both, 2); dup2(fd_both, 1); close(fd_both); }

                

                
                
                proc_info* proc = NULL;
                if(job->nproc == 1){
		            proc = job->procs;
		    	    exec_result = execvp(proc->cmd, proc->argv);
                }
                else if(job->nproc == 2){ //there is 1 pipe
                    
                    close(p1[0]);
                    dup2(p1[1], 1);
                    close(p1[1]);

                    proc = job->procs;
                    //fprintf(stderr, "%s\n", proc->cmd);
                    exec_result = execvp(proc->cmd, proc->argv);

                    
                }
                else if(job->nproc == 3){ //there is 3 pipes
                    
                    kill(getpid(), SIGTERM);
                }
                   
		    	if (exec_result < 0) {  //Error checking
			    	printf(EXEC_ERR, proc->cmd);
				
		    		// Cleaning up to make Valgrind happy 
		    		// (not necessary because child will exit. Resources will be reaped by parent or init)
		    		free_job(job);  
		    		free(line);
    	    		validate_input(NULL);

			    	exit(EXIT_FAILURE);
		    	}
                
	    	} 
            else {

                if(job->nproc == 2){ //there is 1 pipe, 2nd half
                    int sec_pid = fork();
                    if(sec_pid  == 0){
                        close(p1[1]);
                        dup2(p1[0], 0);
                        close(p1[0]);
                        
                        proc_info* proc2 = job->procs->next_proc;
                        //fprintf(stderr, "%s\n", proc2->cmd);
                        exec_result = execvp(proc2->cmd, proc2->argv);

                        if(exec_result < 0){ 
                            printf(EXEC_ERR, proc2->cmd);
                            free_job(job);
                            free(line);
                            validate_input(NULL);
                            exit(EXIT_FAILURE);
                        }
                    }
                    else{
                        close(p1[1]);
                        
                        wait_result = waitpid(sec_pid, &exit_status, 0);
                        
                        if(wait_result < 0) {
                            printf(WAIT_ERR);
                            exit(EXIT_FAILURE);
                        }
                    }
                }
                else if(job->nproc == 3){
                    int fir_pid = fork();
                    if(fir_pid == 0){
                        dup2(p1[1], 1);
                        close(p1[0]);
                        close(p1[1]);
                        close(p2[0]);
                        close(p2[1]);

                        proc_info* proc = job->procs;
                        exec_result = execvp(proc->cmd, proc->argv);
                        if(exec_result < 0){ 
                            printf(EXEC_ERR, proc->cmd);
                            free_job(job);
                            free(line);
                            validate_input(NULL);
                            exit(EXIT_FAILURE);
                        }

                    }

                    int sec_pid = fork();
                    if(sec_pid == 0){
                        dup2(p1[0], 0);
                        dup2(p2[1], 1);
                        close(p1[0]);
                        close(p1[1]);
                        close(p2[0]);
                        close(p2[1]);

                        proc_info* proc2 = job->procs->next_proc;
                        exec_result = execvp(proc2->cmd, proc2->argv);
                        if(exec_result < 0){ 
                            printf(EXEC_ERR, proc2->cmd);
                            free_job(job);
                            free(line);
                            validate_input(NULL);
                            exit(EXIT_FAILURE);
                        }

                    }
               

                    int thr_pid = fork();
                    if(thr_pid == 0){
                        dup2(p2[0], 0);
                        close(p1[0]);
                        close(p1[1]);
                        close(p2[0]);
                        close(p2[1]);

                        proc_info* proc3 = job->procs->next_proc->next_proc;
                        
                        exec_result = execvp(proc3->cmd, proc3->argv);
                        if(exec_result < 0){ 
                            printf(EXEC_ERR, proc3->cmd);
                            free_job(job);
                            free(line);
                            validate_input(NULL);
                            exit(EXIT_FAILURE);
                        }

                    }

                    close(p1[0]);
                    close(p1[1]);
                    close(p2[0]);
                    close(p2[1]);
                    waitpid(fir_pid, &exit_status, 0);
                    waitpid(sec_pid, &exit_status, 0);
                    waitpid(thr_pid, &exit_status, 0);
                    
                }

              

                
                // As the parent, wait for the foreground job to finish
                
                if(job->nproc == 2){
                    close(p1[0]);
                }
                if(job->nproc == 3){
                    close(p1[0]);
                    close(p1[1]);
                    close(p2[0]);
                    close(p2[1]);
                }
                
		    	wait_result = waitpid(pid, &exit_status, 0);
                
		    	if (wait_result < 0) {
				printf(WAIT_ERR);
				exit(EXIT_FAILURE);
		    	}
                
	    	}
        }

		free_job(job);  // if a foreground job, we no longer need the data
		free(line);
	}

    // calling validate_input with NULL will free the memory it has allocated
    validate_input(NULL);

#ifndef GS
	fclose(rl_outstream);
#endif
	return 0;
}
