**Documentation of the code:**


**Contribution By members:**

Both the Members have equal contribution towards the functioning of the code. Every line of code was discussed equally by both the members before imbibing it into the c file, we have also attached our resources for certain snippets along with the code.
Team Members: Muthuraj & Noel



**Working of the code:**


The functioning of the code is split into several different functions as follows:

1.int launch(char *array[]);

This code has the ability to handle the background process command and as well as the new line command and send the tokenized input to the create_process_and_run function

2. void handle_CtrlC();

This code handles the explicit case of ending the program once Ctrl+ C key is pressed.

3.int create_process_and_run(char *array[], int bg);

This function serves the purpose of starting the child process (and the implementation by using execvp function) along with the error handling, and also additionally waiting for the exceptional condition incase if there is any background process to be adhered to.

4.void pipe_creation(char *array[], int count, int bg);

This code handles the condition if there is pipeline method being specified in the terminal. This code creates a pipe function to handle the case of pipes then once the pipe function is created we are looping the function for n number of times based on number of ‘|’,the command respectively writes into the STDOUT_FILENO ,it checks the fork process status using the child_pid() and calls the execvp condition to perform the function it also has the conditions to utilize the variable bg to make the code work for ‘&’,(background process) by having the separate conditions to check it and make it work. It also takes care of the error handling process occurring during the course of the code.


5.void shell_process(); & 6.void terminal_process();

We have distinguished the main code to adhere to the need of the user the void_shell_process deals with the commands obtained from the .sh file whereas the terminal process (the terminal line interface) are handled by the void_terminal_process function

6.Usage of Linked List:

Our code uses the concept of linked list to store the acquired data by each command and as well for the effective calculation of time



**Commands not able to function by our code:**

1.	cd and cd..:
we are trying to utilize cd and cd.. both of which are in built in shell commands having the power to change the directory but the problem is we are using them in the child creation which by default doesn’t have the permission or the ability to change the working of the parent process hence the cd command doesn’t provide a viable output for this command.

2.	exit():
It is a in built shell command and not a command which can be executed in this code owing to the main reason that it is called in child process and even though the child process is able to exit the functioning it doesn’t have the ability nor the permission to change the state of parent and hence the working of the parent state continues.

3.	In general, any command which tries to alter the state of the working parent through the child and as well as any command that tries to redirect the current state of the program will not be executable by the code.



GitHub link:
https://github.com/Muthuraj-Vairamuthu/Assignments_Operating_System/tree/cba3960223764b86106b1dcb7e1f651be1a24651/assignment-2
