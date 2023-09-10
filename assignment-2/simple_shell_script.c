#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>

//Defining time methods
struct timeval start, end;

//Defining true and false
#define true 1
#define false 0

// Error handlings:
// for wrong file type
// Pipe command != null

//Typecasting the functions:
int launch(char *array[]);
void handle_CtrlC();
int create_process_and_run(char *array[], int bg);
void pipe_creation(char *array[], int count, int bg);
void shell_process();
void terminal_process();

typedef struct node {
    char *s;
    struct node *next;
    pid_t pid;
    struct timeval start, end;
    double duration;
} node;

//Creates a node - Noel
node *create_node() {
    node *obj = (struct node *)malloc(sizeof(struct node));
    obj->s = NULL;
    obj->next = NULL;
    obj->pid = 0;
    return obj;
}

void add_node(node *head, char *s, struct timeval start, struct timeval end, pid_t pid, double duration) {
    node *obj = create_node();
    obj->s = strdup(s);
    obj->start = start;
    obj->end = end;
    obj->pid = pid;
    obj->duration = duration;

    while (head->next != NULL) {
        head = head->next;
    }

    head->next = obj;
}

void print_history(node *head) {
    int count = 0;
    head = head->next;
    printf("\n");
    while (head != NULL) {
        printf("Command %d: %s\n", ++count, head->s);
        head = head->next;
    }
}

void print_during_exit(node *head) {
    int count = 0;
    head = head->next;
    printf("\n");
    while (head != NULL) {
        printf("Command %d: %s\n", ++count, head->s);
        char startTime[9];
        strftime(startTime, sizeof(startTime), "%T", localtime(&(head->start.tv_sec)));
        printf("\tStart Time: %s\n", startTime);
        printf("\tDuration: %f seconds\n", head->duration);
        printf("\tPID: %d\n\n", head->pid);
        head = head->next;
    }
}

//Initializing the head pointer and the child pid.
node *head = NULL;
pid_t child_pid;

//Main function calls the terminal process and then asks for user input.
int main() {
    head = create_node();
    signal(SIGINT, handle_CtrlC);
    int status;
    printf("\nShell execution begins!\n");
    shell_process();

    printf("\nTerminal execution begins!\n");
    terminal_process();

}

void pipe_creation(char *array[], int count, int bg) {
    int fd[2];
    int prev = STDIN_FILENO;
    for (int i = 0; i < count; i++) {
        pipe(fd);
        
        //Error handling: Checking if pipes are created.
        if (pipe(fd) == -1){
            perror("Pipe not created!");
        }
        child_pid = fork();
        //Error handling: Checking pipe output
        if (child_pid == -1){
            perror("Execution stopped!");
        }
        if (child_pid == 0) {
            close(fd[0]);
            if (i == 0) {
                dup2(prev, STDIN_FILENO);
            } else {
                dup2(prev, STDIN_FILENO);
                close(prev);
            }

            if (i == count - 1) {
                if (!bg){
                    dup2(STDOUT_FILENO, STDOUT_FILENO);
                }
                
            } else {
                dup2(fd[1], STDOUT_FILENO);
            }

            //Tokenizing the input and sending to execvp
            char *tokens[1024];
            char *command = strtok(array[i], " ");
            int count = 0;

            while (command != NULL) {
                tokens[count++] = command;
                command = strtok(NULL, " ");
            }
            tokens[count] = NULL;
            
            //Exception handling: Execvp not executable
            execvp(tokens[0], tokens);
            perror("execvp");
            exit(1);
        } else {
            close(fd[1]);

            if (!bg){
                wait(NULL);
            }
    
            if (i != 0) {
                close(prev);
            }
        }
        prev = fd[0];
        gettimeofday(&end, NULL);
    }
}

//Handle Ctrl C routine
void handle_CtrlC() {
    printf("\n\nEnding the program! \n");
    print_during_exit(head);

    exit(0);
}

int launch(char *array[]) {
    int status;
    int count = 0;
    char *token; int bg = false;
    for (count = 0 ; array[count]!= NULL; count++){};

    //Tokenizing input for removing "" in the first word
    token = strtok(array[1],"\"");
    if (token != NULL){
        array[1] = token;
    }

    //Tokenizing input for removeing "" in the last word
    token = strtok(array[count - 1],"\"");
    if (token != NULL){
        array[count-1] = token;
    }
    
    //Checks if the last word is & if so do this.
    if (strcmp(array[count - 1], "&") == 0) {
        bg = true;
        array[count - 1] = '\0';
    }
    
    status = create_process_and_run(array, bg);

    return status;
}

int create_process_and_run(char *array[], int bg) {
    char *command = array[0];

    child_pid = fork(); //Starts a child process

    //Error handling: Checking if child process is created.
    if (child_pid < 0) {
        printf("Error happened here\n");
    } else if (child_pid == 0) {
        //Error handling: Checking if the exec method is completed.
        if (execvp(command, array) == -1) {
            perror("execvp");
            exit(1);
        }
    } else {
        int status;
        if (!bg){
            //Waits for child to finish executing
            wait(&status);
            gettimeofday(&end,NULL);
        }
        

        if (WIFEXITED(status)) {
            //If bg = 1, start next process
            return WEXITSTATUS(status);
        } else {
            return -1;
        }
    }
    return 0;
}

void terminal_process(){
    //Executes user input
    char input[1024];
    char input2[1024];
    signal(SIGINT, handle_CtrlC);
    int status;
    int bg = false;

    do {

        printf("\nSimple shell!: ");
        char *command;
            
        if (fgets(input, sizeof(input), stdin) != NULL) {
            // Calculating the start time
            gettimeofday(&start, NULL);
            for (int i = 0; input[i] != '\0'; i++) {
                if (input[i] == '\n') {
                    input[i] = '\0';
                    break;
                }
            }
            strcpy(input2, input);
            if (strcmp(input2, "") == 0) {
                continue;
            }
            int presence = false;

            for (int i = 0; i < 1024; i++) {
                if (input[i] == '|') {
                    presence = true;
                }
            }
            if (!presence) {
                command = strtok(input, " ");
                if (strcmp(input, "history") == 0) {
                    gettimeofday(&end, NULL);
                    add_node(head, input2, start, end, 0, 0);
                    
                    print_history(head);
                    
                    continue;
                } else {
                    char *tokens[1024];

                    int count = 0;

                    while (command != NULL) {
                        tokens[count++] = command;
                        command = strtok(NULL, " ");
                    }
                    tokens[count] = NULL;
                    launch(tokens);
                }
            } else {
                char *command = strtok(input, "|");
                int count = 0;
                char *tokens[1024];
                while (command != NULL) {
                    tokens[count++] = command;
                    command = strtok(NULL, "|");
                }

                int bg = false;

                if (count > 0 && strcmp(tokens[count - 1], "&") == 0) {
                    bg = true;
                    tokens[count - 1] = NULL;
                }
                
                char *pipe[count];
                for (int i = 0; i < count; i++) {
                    pipe[i] = tokens[i];
                }
                pipe_creation(pipe, count, bg);
            }
            double duration = (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_usec - start.tv_usec) / 1000000;
            add_node(head, input2, start, end, child_pid, duration);
        }else{
            //Error handling: Continue only if the input != NULL
            printf("Input invalid");
        }

    } while (true);
}

void shell_process(){
    char input[1024];
    char input2[1024];
    //Error handling: Checking input file
    FILE *scriptFile = fopen("shellscript.sh","r");
    if (scriptFile == NULL){
        perror("Error opening script file");
        return;
    }
    do {
        char *command;
        
        //Error handling: Reading the file until empty
        if (scriptFile != NULL) {
            if (fgets(input, sizeof(input), scriptFile) == NULL) {
                fclose(scriptFile);
                scriptFile = NULL;
                return;
            }
        }else{
            //Error handling: Message if the file is empty
            printf("Script file is empty");
        }

        // Calculating the start time
        gettimeofday(&start, NULL);
        
        //Error handling: Making "\n" as "\0"
        for (int i = 0; input[i] != '\0'; i++) {
            if (input[i] == '\n') {
                input[i] = '\0';
                break;
            }
        }
        //Copying into a new string
        strcpy(input2, input);
        if (strcmp(input2, "") == 0) {
            continue;
        }
        int presence = false;

        for (int i = 0; i < 1024; i++) {
            if (input[i] == '|') {
                presence = true;
            }
        }
        //Runs this routine if | is not found
        if (!presence) {
            command = strtok(input, " ");

            if (strcmp(input, "history") == 0) {
                gettimeofday(&end, NULL);
                add_node(head, input2, start, end, 0, 0);
                
                print_history(head);
                
                continue;
            } else {
                char *tokens[1024];

                int count = 0;

                while (command != NULL) {
                    tokens[count++] = command;
                    command = strtok(NULL, " ");
                }
                tokens[count] = NULL;

                launch(tokens);
            }
        } else {
            char *command = strtok(input, "|");
            int count = 0;
            char *tokens[1024];
            while (command != NULL) {
                tokens[count++] = command;
                command = strtok(NULL, "|");
            }
            char *pipe[count];
            for (int i = 0; i < count; i++) {
                pipe[i] = tokens[i];
            }
            pipe_creation(tokens, count, false);
        }
        //Calculates the duration
        double duration = (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_usec - start.tv_usec) / 1000000;
        add_node(head, input2, start, end, child_pid, duration);
        

    } while (true);
}

//Code reference is referred from:
//1. Lecture slides 5 - 7
//2. Stack over flow for measuring execution time: https://stackoverflow.com/questions/12722904/how-to-use-struct-timeval-to-get-the-execution-time
//3. fgets method: https://www.geeksforgeeks.org/fgets-gets-c-language/
//4. Wall clock time: https://www.tutorialspoint.com/c_standard_library/c_function_localtime.htm#:~:text=The%20C%20library%20function%20struct,in%20the%20local%20time%20zone