#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>


// Defining true and false
#define true 1
#define false 0

// Making a set of global variables
int NCPU;
int TSLICE;
int switchp = -1; //-1 is default status
int ctrl_status = 0;


// Queue structure
typedef struct qnode {
    char cmd[100];
    pid_t pid;
    struct qnode *next;
    int priority;
    struct timeval start;
    struct timeval wait;
    double wait_time;
    double execution_time;
} qnode;

// Shared memory object
typedef struct shm_t {
    qnode *qhead;
    qnode *qtail;
    sem_t mutex;
    qnode *running;
    pid_t scheduler_pid;
    sem_t scheduler;
    qnode *array_of_completed_jobs[100];
} shm_t;

//The main shared memory
shm_t *shm_global;

// Queue functionalities
qnode *make_qnode(char cmd[], pid_t pid, int priority) {
    qnode *obj = (qnode *)malloc(sizeof(qnode));
    gettimeofday(&obj->wait,NULL);
    if(obj==NULL){
        printf("The node you're trying to initialize doesn't exist");
    }
    strncpy(obj->cmd, cmd, sizeof(obj->cmd) - 1);
    obj->cmd[sizeof(obj->cmd) - 1] = '\0';
    obj->pid = pid;
    obj->priority = priority;
    obj->wait_time = 0;
    obj->execution_time = 0;
    obj->next = NULL;
    gettimeofday(&obj->start, NULL);

    return obj;
}


// Insert a new node at the end of the queue
void insert_qnode(qnode **qhead, qnode **qtail, qnode *obj) {
    if (*qhead == NULL || obj->priority > (*qhead)->priority) {
        obj->next = *qhead;
        *qhead = obj;
        if (*qtail == NULL) {
            *qtail = obj;
        }
    } else {
        qnode *current = *qhead;
        while (current->next != NULL && obj->priority <= current->next->priority) {
            current = current->next;
        }
        
        obj->next = current->next;
        current->next = obj;
        
        if (current == *qtail) {
            *qtail = obj;
        }
    }
}


// Dequeue and remove the first node from the queue
qnode *dequeue(qnode **qhead) {
    if (*qhead == NULL) {
        return NULL; // Queue is empty
    }
    qnode *spare = *qhead;
    *qhead = (*qhead)->next;
    spare->next = NULL;
    return spare;
}


// Traverse and print the contents of the queue
void traversing(qnode **qhead) {
    qnode *current = *qhead;
    while (current != NULL) {
        printf("%s\n", current->cmd);
        current = current->next;
    }
}

//For all the nodes in the queue except "node" wait time is calculated
void wait_time_calculator(qnode **qhead, qnode *node) {
    struct timeval end;
    gettimeofday(&end, NULL);
    qnode *current = *qhead;
    while (current != NULL) {
        if (current != node){
            double duration = (end.tv_sec - current->wait.tv_sec) +
                             (end.tv_usec - current->wait.tv_usec) / 1000000.0;
            current->wait_time = duration;
        }
        current->wait = end;
        current = current->next;
    }
}

// Calculates the length of queue
int length_of_queue(qnode **qhead) {
    qnode *current = *qhead;
    int count = 0;
    while (current != NULL) {
        count++;
        current = current->next;
    }
    return count;
}

//Cleans up and exits
void cleanup_and_exit() {
    if(munmap(shm_global, sizeof(shm_t))==-1){
        printf("munmap is not initialized");
    }
  
    if(sem_destroy(&shm_global->mutex)==-1){
        printf("sem_destroy is not initialized");
    }
    exit(0);
}

//Clean up routine
void cleanup() {

    if(sem_destroy(&shm_global->mutex)==-1){
        printf("sem_destroy is not initialized");
    }

    if(munmap(shm_global, sizeof(shm_t))==-1){
        printf("munmap is not initialized");
    }

    if(shm_unlink("abc")==-1){
        printf("shm_unlink is not initialized");

    }
}

// Typecasting the functions
void terminal_process();
void scheduler_process();

//Main control box of the program
static void my_handler(int signum) {

    //Ctrl - C routine
    if (signum == SIGINT) {

        sem_wait(&shm_global->scheduler);
        printf("\nExiting the program! \n");
        cleanup();
        printf("Memory cleaned up\n");
        abort();
        
    } 
    
    //when schedule is passed in the shell
    else if (signum == SIGUSR1) {
        switchp = 0;
        shm_global->scheduler_pid = fork();
        if (shm_global->scheduler_pid == 0){
            shm_global->scheduler_pid = getpid();
            scheduler_process();
        }else{
            waitpid(shm_global->scheduler_pid,NULL,0);
        }
    } 
    
    //when the queue is empty or Ctrl-Z is pressed
    else if (signum == SIGUSR2 || signum == SIGTSTP) {
        struct timeval start;
        signal(SIGINT, my_handler);
        char input[1024];char s[1024];
        if (fgets(input, sizeof(input), stdin) != NULL) {
            char *command = strtok(input, " \n");
            strcpy(s,command);
        }
        sem_wait(&shm_global->mutex);
        qnode *node = make_qnode(s, 0, 1);
        gettimeofday(&start, NULL);
        node->start = start;
        insert_qnode(&shm_global->qhead,&shm_global->qtail,node);
        sem_post(&shm_global->mutex);

    }
}

//Main shell of the program
void terminal_process() {
    char input[1024];

    do {
        printf("\nSimple shell!: ");
        char *command;


        if (fgets(input, sizeof(input), stdin) != NULL) {
            for (int i = 0; input[i] != '\0'; i++) {
                if (input[i] == '\n') {
                    input[i] = '\0';
                    break;
                }
            }

            char *command = strtok(input, " \n");
            if (command != NULL && strcmp(command, "submit") == 0) {
                char *tokens[1024];

                int count = 0;

                while (command != NULL) {
                    command = strtok(NULL, " ");
                    tokens[count++] = command;
                }
                tokens[count] = NULL;

                char s[100];
                strcpy(s,tokens[0]);

                
                if (tokens[1]!= NULL){
                    int priority = atoi(tokens[1]);
                    qnode *node = make_qnode(s,0,priority);
        
                    struct timeval start;
                    gettimeofday(&start, NULL);
                    node->start = start;
                    sem_wait(&shm_global->mutex);
                    insert_qnode(&shm_global->qhead,&shm_global->qtail,node);
                    sem_post(&shm_global->mutex);
                    continue;
                }


                qnode *node = make_qnode(s, 0, 1);
                struct timeval start;
                gettimeofday(&start, NULL);
                node->start = start;
                sem_wait(&shm_global->mutex);
                insert_qnode(&shm_global->qhead,&shm_global->qtail,node);
                sem_post(&shm_global->mutex);

            
            } else if (command != NULL && strcmp(command, "ps") == 0) {
                execvp("ps", NULL);
            } else if (command != NULL && strcmp(command, "schedule") == 0) {
                kill(getpid(), SIGUSR1);
            }
        } else {
            printf("Input invalid");
            break;
        }
    } while (switchp == -1);

    while (switchp != -1){} //Terminal process rests here
    
}


//Executes a given node at a time
void daemon_process() {
    
    sem_wait(&shm_global->mutex);
    while (true) {
        
        //Removes the first element
        qnode *cmd2 = dequeue(&shm_global->qhead);

        int status; int result;
        struct timeval end;
        gettimeofday(&end,NULL);
    
        pid_t child = cmd2->pid;
        sem_post(&shm_global->mutex);
        
        //If node is empty
        if (cmd2 == NULL){
            break;
        }

        //If first time the node is running
        if (child == 0) {
            child = fork();
            if (child == 0) {
                execvp(cmd2->cmd, NULL);
                exit(1);signal(SIGINT, my_handler);
            }else if (child > 0) {
                usleep(TSLICE * 1000);
                
            }else {
                printf("Fork failed!\n");
            }
        
        
        }else{
            kill(child, SIGCONT);
            usleep(TSLICE * 1000);
        }

        cmd2->pid = child;
        result = waitpid(child,&status,WNOHANG);
        if (result == 0){
            //If child process is not completed its added back to the queue
            kill(child, SIGSTOP);
            sem_wait(&shm_global->mutex);
            insert_qnode(&shm_global->qhead, &shm_global->qtail, cmd2);
            sem_post(&shm_global->mutex);
        }

        else if (result == -1){
            perror("wait pid issue");
        }else {

             //If the process has been completed
            double duration = (double)(end.tv_sec - cmd2->start.tv_sec) + (double)(end.tv_usec - cmd2->start.tv_usec) / 1000000;
            sem_wait(&shm_global->mutex);
            cmd2->execution_time = duration;
            wait_time_calculator(&shm_global->qhead,cmd2);
            printf("---------------------------------\n");
            printf("Name of command: %s\n", cmd2->cmd);
            printf("\tPID: %d\n", cmd2->pid);
            printf("\tExecution time: %.10f\n", cmd2->execution_time);
            printf("\tWait time: %.10f\n", cmd2->wait_time);
            printf("\tPriority: %d\n", cmd2->priority);
            printf("---------------------------------\n");

            // append_to_array(cmd2,shm_global->array_of_completed_jobs);
            sem_post(&shm_global->mutex);

        }
        sem_post(&shm_global->mutex);
       
    }
}

//The scheduler process runs n processes parallely
void scheduler_process() {
    while (true) {
        //Check for Ctrl-C as an input
        signal(SIGINT, my_handler);
        pid_t array_of_pids[NCPU];
        pid_t main_forks[NCPU];
        for (int np = 0; np < NCPU; np++) {
            pid_t main = fork();
            main_forks[np] = main;
            if (main == 0) {
                daemon_process();
                exit(0);
            }else{
                ;
            }
        }

        //Waits for all the children to complete 
        for (int np = 0; np < NCPU; np++) {
            waitpid(main_forks[np], NULL, 0);
        }

        //Checks if the queue is empty
        if (shm_global->qhead == NULL) {
            kill(getpid(), SIGUSR2);
        }
    }

}

int main(int argc, char *argv[]) {
    signal(SIGINT, my_handler);
    signal(SIGUSR1, my_handler);
    signal(SIGUSR2, my_handler);
    signal(SIGTSTP, my_handler);

    if (argc != 3) {
        printf("Invalid arguments!");
    }

    NCPU = atoi(argv[1]);
    if(NCPU <=0){
        printf("invalid no of cpu's is typed");
    }
    TSLICE = atoi(argv[2]);
    if(TSLICE<=0){
        printf("Invalid no of TSLICE us typed");
    }

    int shm_fd = shm_open("abc", O_CREAT | O_RDWR, 0777);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }

    ftruncate(shm_fd, sizeof(shm_t));
    shm_global = (shm_t *)mmap(NULL, sizeof(shm_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_global == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    if(sem_init(&shm_global->mutex, 1, 1)==-1){
        printf("The sem_init function is not initialized properly");
    }
    if(sem_init(&shm_global->scheduler, 1, 1)==-1){
        printf("The sem_init function is not initialized properly");
    }

    shm_global->qhead = NULL;
    shm_global->qtail = NULL;
    shm_global->scheduler_pid = 0;
    for (int i = 0 ; i < 100 ; i++){
        shm_global->array_of_completed_jobs[i] = NULL;
    }
    pid_t terminal_pid = fork();
    if (terminal_pid == 0){
        terminal_process();
        exit(0);
    }else{
        waitpid(terminal_pid,NULL,0);
    }
    
}


//WNOHANG: https://man7.org/linux/man-pages/man3/wait.3p.html#:~:text=WNOHANG%20The%20waitpid()%20function,child%20processes%20specified%20by%20pid.
//SHM_Declarations: https://man7.org/linux/man-pages/man7/shm_overview.7.html
//Semaphore: https://man7.org/linux/man-pages/man7/sem_overview.7.html
//Kill method: https://man7.org/linux/man-pages/man2/kill.2.html
//Priority queue inspired from GFG: https://www.geeksforgeeks.org/priority-queue-set-1-introduction/