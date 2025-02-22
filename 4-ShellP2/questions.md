1. Can you think of why we use `fork/execvp` instead of just calling `execvp` directly? What value do you think the `fork` provides?

    > **Answer**: 
Using `fork/execvp` creates a new child process to execute the command so the shell can maintain its state and manage multiple commands. The parent process continues to read commands and create additional child processes for other commands which in turn will foster a multi-tasking environment.

2. What happens if the fork() system call fails? How does your implementation handle this scenario?

    > **Answer**: if the fork() system call fails, it will return -1 meaning that it wasn't able to create a new process. My implementation checks the return value of fork by seeing if it returns a value < 1, and it will print an error message. 

3. How does execvp() find the command to execute? What system environment variable plays a role in this process?

    > **Answer**:  It searches for the command in the directories listed in the PATH variable which contains a colon-separated list of the directories. It searches through each directory to find the file that matches the command name

4. What is the purpose of calling wait() in the parent process after forking? What would happen if we didn’t call it?

    > **Answer**:  It waits for the child process explained in question 1 to finish executing before it can continue the parent process. If it doesn't wait, the parent can't see the child's exit status and woluldn't be able to full determine if we successfully executed the command.

5. In the referenced demo code we used WEXITSTATUS(). What information does this provide, and why is it important?

    > **Answer**:  It provides the exit code that got returned by the child process. It's important because it informs the parent process on whether the command was successful or not. 

6. Describe how your implementation of build_cmd_buff() handles quoted arguments. Why is this necessary?

    > **Answer**: I handles the quoted arguments by detecting any encounter of a quote and captured all characters until the program reached the closing quote. This turns arguments with spaces to be treated as a single argument since spaces are used a delimiter by default. If we didn't handle the quotes, commands with spaces wouldn't be interpreted correctly.

7. What changes did you make to your parsing logic compared to the previous assignment? Were there any unexpected challenges in refactoring your old code?

    > **Answer**:  I improved my parsing logic by ensuring that spaces within the quotes were preserved which was something I didn't have in the previous assignment. I also refactored the string tokenization logic for clarity which made it cleaner. This brought up a couple challenges when trying to ensure proper boundary checks and trying to maintain functionality within the nested quotes, but I figured it out.

8. For this quesiton, you need to do some research on Linux signals. You can use [this google search](https://www.google.com/search?q=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&oq=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&gs_lcrp=EgZjaHJvbWUyBggAEEUYOdIBBzc2MGowajeoAgCwAgA&sourceid=chrome&ie=UTF-8) to get started.

- What is the purpose of signals in a Linux system, and how do they differ from other forms of interprocess communication (IPC)?

    > **Answer**: The purpose of signals in a linux system are to allow processes to communicate with each other and respond to events. Unlike other IPC mechanisms, the signals are asynchronous and won't require direct communication between the processes like being sent by the kernel. 

- Find and describe three commonly used signals (e.g., SIGKILL, SIGTERM, SIGINT). What are their typical use cases?

    > **Answer**:  SIGKILL is used to immediately terminate a process and cannot be caught or ignored. SIGTERM requests the process to terminate, but it can be caught and handled. This allows the process to confirm termination or clean up before it exits. SIGINT interrupts a process like when we use CTRL+C in the terminal, and it can be caught like SIGTERM.

- What happens when a process receives SIGSTOP? Can it be caught or ignored like SIGINT? Why or why not?

    > **Answer**: The process is suspended until it receives a SIGCONT signal, and it cannot be caught or ignored. This is because the operating system needs to be able to control process execution and suspend processes when it needs to.
