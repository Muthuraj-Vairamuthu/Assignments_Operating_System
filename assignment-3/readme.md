**Documentation of the code:**


**Contribution By members:**

Both the Members have equal contribution towards the functioning of the code. Every line of code was discussed equally by both the members before imbibing it into the c file, we have also attached our resources for certain snippets along with the code.
Team Members: Muthuraj & Noel



**Working of the code:**


The functioning of the code is split into several different functions as follows:

1.	make_qnode: creates a node called obj to handle the wait time,execution time,priority initializations.

2.	insert_qnode: Insert a new node at the end of the queue and also handles priority handling as explained in heuristics section.

3.	dequeue: Dequeue and remove the first node from the queue

4.	 traversing: Traverse and print the contents of the queue

5.	Wait time calculator: For all the nodes in the queue except "node" wait time is calculated

6.	 length_of_queue: Calculates the length of queue

7.	cleanup_and_exit: Cleans up and exits with munmap,sem_destory and exit

8.	cleanup: Clean up routine with sem-destory,munmap,shm_unlink
9.	 my_handler: Main control box of the program with commands for signals and their execution
10.	 terminal_process: Main shell of the program with inputs and signal switches

11.	 daemon_process: Executes a given node at a time

12.	scheduler_process: The scheduler process runs n processes parallely with help of fork and for loops with the NCPU’s
13.	 main:shared memory intializations,forking to start the terminal_pid and semaphores intialiations




**Heuristic for Priority Handling:**
We have taken 4 to be the highest priority and 1 to be of least priority


Statistics for priority jobs(Advanced Functionalities):
For the priority jobs we are using the concept of queue to give the output, we are basically inserting elements into the queue one by one ,when we are doing that for example, priority ele with ele with priority 3,so while adding the element with Priority 3 it checks whether the first element in the node has a smaller value than it or not if it has then the node goes in front of the node , if it is of the same value it will add right after the node with the same value.


As you can see the one with higher priority was prioritised first and completed in earlier time.
Priority 4>Priority 3 >Priority 2 >Priority 1.
Here are the statistics graphs:

