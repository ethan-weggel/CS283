1. In this assignment I suggested you use `fgets()` to get user input in the main while loop. Why is `fgets()` a good choice for this application?

    > **Answer**:  `fgets()` is a good command to use here because a shell is a line reader that will take in a line and parse it to see its arguments. `fgets()` also stops reading input at the newline or EOF which is good because it is standard to input a command and press `enter` to add a newline and execute the command. EOF is also good because it allows us to pipe or redirect stdin to the program to terminate our input and start execution. In summation, this function helps us replication common functionality among all existing shell programs.

2. You needed to use `malloc()` to allocte memory for `cmd_buff` in `dsh_cli.c`. Can you explain why you needed to do that, instead of allocating a fixed-size array?

    > **Answer**:  `malloc()` is used here rather than a fixed-size array because of a few reasons. The first is due to scoping; if we allocate a fixed sized array it would exist in the scope of the function but would be freed once we exit the scope. Additionally, these varaibles are stored on the stack which means if we were to have too much input or mishandle it, it could result in a stack overflow. `malloc()`, unlike a fixed-size array grabs memory from the heap. This means we can maintain it and keep track of it until we explicitly free it. It also allows us to change our code with relative ease if we wanted to accept a larger or smaller input later.


3. In `dshlib.c`, the function `build_cmd_list()` must trim leading and trailing spaces from each command before storing it. Why is this necessary? If we didn't trim spaces, what kind of issues might arise when executing commands in our shell?

    > **Answer**:  If we didn't trim leading and trailing spaces then, in extreme cases, we might find that our command buffer is full of white space before we even read a single useful character specifying information about the executable or its arguments. It also allows us to normalize our input, allowing for a more algorithmic parsing of the line once we can assume a few facts about what input we are receiving (i.e. "we can handle the string this way because we know there are no leading or trailing whitespace characters).

4. For this question you need to do some research on STDIN, STDOUT, and STDERR in Linux. We've learned this week that shells are "robust brokers of input and output". Google _"linux shell stdin stdout stderr explained"_ to get started.

- One topic you should have found information on is "redirection". Please provide at least 3 redirection examples that we should implement in our custom shell, and explain what challenges we might have implementing them.

    > **Answer**:  One example of redirection is the redirection of STDOUT, denoted with the `>` operator. This will take the output from a process, specifically from STDOUT, and will redirect it to a file. For instance: `ls > output.txt`. A second example is redirecting STDERR, denoted with `>>`. This acts the exact same way as STDOUT, the key difference being this will speicifcally redirect things written to STDERR to a different location. An example being `echo "test" >> output.txt`. Thirdly is the way to redirect STDIN. This is done with the `<` operator. This will allow us to read input from a file rather than the keyboard. An example is `sort < input.txt`. 

    The biggest challenge with redirecting STDOUT and STDERR implementation is that we need to properly opena nd managae files with the open() sys call. This means we need to ensure the proper flags, modes and permissions are present to accomodate the users needs. This may prove difficult to do in a low level programming language as there are a lot of different angles to cover. Similarly, for STDIN, the main challenge would be ensuring file files *exists* and that permissions are set to allow us to read from it.

- You should have also learned about "pipes". Redirection and piping both involve controlling input and output in the shell, but they serve different purposes. Explain the key differences between redirection and piping.

    > **Answer**:  Pipes and redirection do both involve controlling input and output, but from what to what varies. Standard redirection noramlly entails the reading and writing to *files* whereas pipes redirection input and output from *processes or commands*. This allows us to chain together commands using ones output as anothers input and vice versa. This is performed in temporary and more dynamic memory rather than static files.

- STDERR is often used for error messages, while STDOUT is for regular output. Why is it important to keep these separate in a shell?

    > **Answer**:  In general, debugging is made easier with the separation of output from error because it makes deliberate and system messages easier to identify. In regards to redirection, it also allows us to keep these two separate. This means we can redirect from and to STDOUT and STDERR separately. This also prevents STDERR from corrupting an expected STDOUT data steam which can be critical in certain situations. Additionally, older styles of interfaces would read from these files separately giving the user an enhanced 'GUI' experience.

- How should our custom shell handle errors from commands that fail? Consider cases where a command outputs both STDOUT and STDERR. Should we provide a way to merge them, and if so, how?

    > **Answer**:  By default, our shell should print the STDOUT and STDERR files separately to make it easier for users to discern normal output and error messages. However, we should also allow users to merge them when needed using redirection (which we would also have to implement). This ensures that merging is a deliberate choice rather than a default functionality that forces users to separate them after the fact. Additionally, we should properly handle command failures by capturing and displaying their exit status, making debugging easier.