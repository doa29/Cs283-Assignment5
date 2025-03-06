# Questions

1. Your shell forks multiple child processes when executing piped commands. How does your implementation ensure that all child processes complete before the shell continues accepting user input? What would happen if you forgot to call waitpid() on all child processes?

Answer: The implementation stores each child process’s PID in an array and then uses waitpid() on each PID in a loop. This ensures that the parent waits until every child has finished execution before prompting the user for new input. If waitpid() were omitted for some or all children, those child processes might become zombies—lingering in the process table—and the shell could continue running before all processes have completed, potentially causing resource exhaustion.

2. The dup2() function is used to redirect input and output file descriptors. Explain why it is necessary to close unused pipe ends after calling dup2(). What could go wrong if you leave pipes open?

Answer: After using dup2() to duplicate a file descriptor onto a standard I/O descriptor, the original pipe file descriptors are no longer needed and should be closed. If these file descriptors are left open, they can lead to descriptor leaks and may prevent the correct detection of an end-of-file (EOF) condition on the reading side of a pipe. This can cause processes waiting for input to hang indefinitely.

3. Your shell recognizes built-in commands (cd, exit, dragon). Unlike external commands, built-in commands do not require execvp(). Why is cd implemented as a built-in rather than an external command? What challenges would arise if cd were implemented as an external process?

Answer: The cd command must change the working directory of the shell process itself. If cd were implemented as an external command, it would run in a child process, and any directory change would affect only that child process, leaving the parent shell’s working directory unchanged. This would render the cd command ineffective in the interactive shell environment.

4. Currently, your shell supports a fixed number of piped commands (CMD_MAX). How would you modify your implementation to allow an arbitrary number of piped commands while still handling memory allocation efficiently? What trade-offs would you need to consider?

Answer: To support an arbitrary number of piped commands, the fixed-size array could be replaced with a dynamically allocated array or a linked list that grows as needed. This approach requires careful memory management to avoid leaks and ensure efficient use of resources. The trade-offs include added complexity in dynamic memory allocation and deallocation, potential performance overhead, and the need to implement robust error handling for memory operations.
