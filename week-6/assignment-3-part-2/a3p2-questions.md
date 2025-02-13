1. Can you think of why we use `fork/execvp` instead of just calling `execvp` directly? What value do you think the `fork` provides?

    > **Answer**:  When we call `execvp` directly this causes our currently process to morph into this other executable that is started by the syscall. This means when we are finished executin and need to return to the "parent" program, we would have nothing left to return to. By using fork we are making a new child process that can morph into running the executable and then when we are done we can go back to using the parent program (the shell), because it is left untouched.

2. What happens if the fork() system call fails? How does your implementation handle this scenario?

    > **Answer**: is the `fork()` system call fails then we know the return code for fork (which would be in the parent process) would be not equal to zero. In the case of my implementation i make sure to check if the return code is less than zero. If that is the case we simply return from the function that called the system call with the return code of `fork()`. Otherwise I continue execution as normal.

3. How does execvp() find the command to execute? What system environment variable plays a role in this process?

    > **Answer**:  `execvp()` finds the command to execute by searching through values stores in an environment variable. Specifically one called PATH. This is a list of paths that lead to binaries that are named and can be executed. In general a lot of these binaries can be found in `/bin` and that path is on the PATH environment variable. If the name of the executable we are trying to start is not present in any of the paths listed in PATH, then we return an error code and continue with shell execution.

4. What is the purpose of calling wait() in the parent process after forking? What would happen if we didn’t call it?

    > **Answer**:  Calling `wait()` in the parent process ensures that we are waiting for the child process to completely finish out its execution before we continue as the parent. This means the child process must return with a return code before continuing. This is done to make sure that the parent doesn't continue execution which could result in "weird things" happening. For instance, if we were waiting to see the return code of the child process in the parent but we didn't wait, we could use the wrong value or have a memory error in our code that could go unnoticed or cause a crash. In the case of my program that is certainly the case. However, in general, one could imagine forking and then booting an executable in the child process such as a file copying process. If we didn't wait for the child to be done we might be limited in resources or we could have mixed output on our screen which could be very confusing for the user.

5. In the referenced demo code we used WEXITSTATUS(). What information does this provide, and why is it important?

    > **Answer**: `WEXITSTATUS()` allows us to extract the return code of a child process via `wait()` or `waitpid()`. This is important because it makes finding out if the child process was successful or not much easier.

6. Describe how your implementation of build_cmd_buff() handles quoted arguments. Why is this necessary?

    > **Answer**:  In my implementation of `build_cmd_buff()`, I iterate through the entire _cmd_buff string character by character. At each character I will ask "is this a quote?" and if so, then I will toggle into quote mode and continued coping *every* character I see, not omitting whitespace. Then I toggle back off quote mode when I see another quote and continue to the end. This is necessary because by default we split on spaces, but we might want to pass an argument that contains spaces in it to our program. In the example of echo, we could use `echo "  hello    world  "` and our quote mode will preserve those whitespace characters.

7. What changes did you make to your parsing logic compared to the previous assignment? Were there any unexpected challenges in refactoring your old code?

    > **Answer**:  There were no unexpected challenges, but I did have to refactor more than I originally thought. The first assignment I broke down the entire command buffer into truncated chunks of sub-commands, then would strtok() each sub-command into its exe and args. I accomplished this with speicialized functions I wrote. Now we are much more robust for sub-command parsing as I can iterate once in a single function since our data structure is now different. Now I simply replace white space with null bytes and assign pointers to substrings in a character array that is stored in the same data structure rather than manually allocating the memory. I suspect in future assignments I could combine the logic so I could quickly and efficiently split command buffers into sub-commands and then use my new tactic to parse the sub-commands.

8. For this quesiton, you need to do some research on Linux signals. You can use [this google search](https://www.google.com/search?q=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&oq=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&gs_lcrp=EgZjaHJvbWUyBggAEEUYOdIBBzc2MGowajeoAgCwAgA&sourceid=chrome&ie=UTF-8) to get started.

- What is the purpose of signals in a Linux system, and how do they differ from other forms of interprocess communication (IPC)?

    > **Answer**:  The purpose of a signal in a Linux system is to send notifications via the kernel to signal events such as termination or interruption. Signals are different than interprocess communication because signals are sent asynchronously and are lightweight. They also do not send or transfer data, just notifications.

- Find and describe three commonly used signals (e.g., SIGKILL, SIGTERM, SIGINT). What are their typical use cases?

    > **Answer**:  Three commonly used signals are SIGKILL (9), SIGNTERM (15) and SIGINT (2). SIGKILL is used to immediately terminate a process and cannot be ignored until later. This is good for forcibly stopping a program in an emergency. SIGTERM is a more polite version of SIGKILL and is preferred over it generally since SIGTERM allos the process to terminate itself allowing things like cleanup and freeing memory and stopping other subprocesses. SIGINT is known as a process interruption. This is what happens when the user presses ctrl+c. It can be handled or ignored by the program as well.

- What happens when a process receives SIGSTOP? Can it be caught or ignored like SIGINT? Why or why not?

    > **Answer**:  SIGSTOP is used to pause a process immediately. It doesn't force quit, but will stop it in place. Unlike SIGINT, which can be ignored, SIGSTOP is enforced by the kernel so it cannot be ignored and the process receiving the notification must immediately comply. 