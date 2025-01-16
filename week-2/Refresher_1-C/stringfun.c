#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define BUFFER_SZ 50

//prototypes
void usage(char *);
void print_buff(char *, int);
int setup_buff(char *, char *, int);

//prototypes for functions to handle required functionality
int count_words(char *, int);
void reverse_string(char*, int);
void reverse_print(char*, int);
void word_print(char*, int);
void selection_print(char*, int, int);

// sets up the buffer by copying over user string and returns string length
int setup_buff(char *buff, char *user_str, int len){
    
    int userStringLength = 0;
    int bufferIndex = 0;
    int lastCharWhiteSpace = 0;

    // while we haven't reached the null terminator in our user string
    while (*user_str) {

        // we exit program if the user's string is greater than 50 characters
        if (userStringLength >= len) {
            exit(-1);
        }

        // combinations of last previous character and current character in terms of white space
        // this is to see if we should be coping over whitespace or not
        if (lastCharWhiteSpace && (*(user_str) == ' ' || *(user_str) == '\t')) {
            user_str++;
            continue;
        }  else if (lastCharWhiteSpace && !(*(user_str) == ' ' || *(user_str) == '\t')) {
            lastCharWhiteSpace = 0;
            *(buff + (sizeof(char) * userStringLength)) = *(user_str);
            userStringLength++;
            user_str++;
        } else if (!lastCharWhiteSpace && (*(user_str) == ' ' || *(user_str) == '\t')) {
            lastCharWhiteSpace = 1;
            *(buff + (sizeof(char) * userStringLength)) = *(user_str);
            userStringLength++;
            user_str++;
        } else if (!lastCharWhiteSpace && !(*(user_str) == ' ' || *(user_str) == '\t')) {
            lastCharWhiteSpace = 0;
            *(buff + (sizeof(char) * userStringLength)) = *(user_str);
            userStringLength++;
            user_str++;
        }
    }

    bufferIndex = userStringLength;

    // while we haven't reached the end of our buffer
    while (bufferIndex < len) {
        *(buff + (sizeof(char) * bufferIndex)) = '.';
        bufferIndex++;
    }

    return userStringLength; 
}

void print_buff(char *buff, int len){
    printf("Buffer:  ");
    for (int i=0; i<len; i++){
        putchar(*(buff+i));
    }
    putchar('\n');
}

void usage(char *exename){
    printf("usage: %s [-h|c|r|w|x] \"string\" [other args]\n", exename);

}

// counts words, returns the number of words in user string from buffer
int count_words(char *buff, int str_len){
    
    int words = 0;
    int buffIndex = 0;

    // if we know the string length was zero, we can immediately return to make other logic easier
    if (str_len == 0) {
        printf("Word Count: %d\n", words);
        return 0;
    }

    // while we are in range of the user string, check for white space and increment a counter every white space
    for (int i = 0; i < str_len; i++) {
        if (*(buff + (sizeof(char) * i)) == ' ' && i != 0 && i != str_len-1) {
            words++;
        }
        buffIndex = i;
    }

    // increment one last time at the end of the word
    if (*(buff + (sizeof(char) * (buffIndex + 1))) == '.') {
        words++;
    }

    return words;
}

// reverses the string in place using a two pointer approach on the buffer
void reverse_string(char* buff, int str_len) {
    char* tail = buff + str_len - 1;
    char temp;

    // iterate until the pointers meet or pass one another
    while (buff < tail) {
        temp = *buff;
        *buff = *tail;
        *tail = temp;
        buff++;
        tail--;
    }
}

// special print function for reverse print
// buff is not null terminated so we need to print up to a back limit
void reverse_print(char* buff, int print_len) {
    for (int i = 0; i < print_len; i++) {
        printf("%c", *(buff + (sizeof(char) * i)));
    }
    printf("\n");
}

// similar to reverse print except this prints a portion of a string from
// one part to another so we don't need to modify buff to print individual words
void selection_print(char* buff, int start_index, int end_index) {
    for (int i = start_index; i < end_index; i++) {
        printf("%c", *(buff + (sizeof(char) * i)));
    }
}

// prints words and their lengths on new lines
void word_print(char* buff, int str_len) {
    int words = 0;
    int buffIndex = 0;
    int wordStart = 0;
    int wordEnd = 0;

    printf("Word Print\n----------\n");

    // same logic as word count, except we keep track of the beginning of the word and end
    // based on index so we can use selection print each time we reach a new word
    for (int i = 0; i < str_len; i++) {
        if (*(buff + (sizeof(char) * i)) == ' ' && i != 0 && i != str_len-1) {
            words++;

            printf("%d. ", words);
            selection_print(buff, wordStart, wordEnd);
            printf(" (%d)\n", wordEnd - wordStart);

            wordStart = wordEnd + 1;
        }
        buffIndex = i;
        wordEnd++;
    }

    // print one more time for last word in the string
    if (*(buff + (sizeof(char) * (buffIndex + 1))) == '.') {
        words++;
        printf("%d. ", words);
        selection_print(buff, wordStart, wordEnd);
        printf(" (%d)\n", wordEnd - wordStart);
    }
}

int main(int argc, char *argv[]){

    char *buff;             
    char *input_string;     
    char opt;               
    int  rc;                
    int  user_str_len;      

    //TODO:  #1. WHY IS THIS SAFE, aka what if arv[1] does not exist?
    /* ANSWER: this is safe beecause if argv[1] doesn't exist then we won't know what option the
                user is trying to perform so we quit the program but not before telling the user
                what the usage is. We are also making sure that the second argument IS a flag with
                the dash we are checking for. This means the program will work as intended (i.e.
                it makes the program "safe").
    */
    if ((argc < 2) || (*argv[1] != '-')){
        usage(argv[0]);
        exit(1);
    }

    opt = (char)*(argv[1]+1); 

    //handle the help flag and then exit normally
    if (opt == 'h'){
        usage(argv[0]);
        exit(0);
    }


    //TODO:  #2 Document the purpose of the if statement below
    /* ANSWER: This is to check for the amount of arguments passed into the program at run time.
                we want to make sure that at least three are passed in (program name, flag/option and string).
                If we have less than three then the program will not be able to function as intended. So we print
                the usage as good practice to make sure the user knows what the program expects.
    */
    if (argc < 3){
        usage(argv[0]);
        exit(1);
    }

    input_string = argv[2]; 

    //TODO:  #3 Allocate space for the buffer using malloc and
    //          handle error if malloc fails by exiting with a 
    //          return code of 99
    // CODE GOES HERE FOR #3

    buff = malloc(BUFFER_SZ * sizeof(char)); // allocated the space

    if (buff == NULL) { // making sure that the buffer isn't NULL (i.e. we got the memory we wanted)
        exit(99);
    }

    user_str_len = setup_buff(buff, input_string, BUFFER_SZ);
    if (user_str_len < 0){
        printf("Error setting up buffer, error = %d", user_str_len);
        exit(2);
    }

    switch (opt){
        case 'c':
            rc = count_words(buff, user_str_len); 
            if (rc < 0){
                printf("Error counting words, rc = %d", rc);
                exit(2);
            }
            printf("Word Count: %d\n", rc);
            break;

        case 'r':
            reverse_string(buff, user_str_len);
            printf("Reversed String: ");
            reverse_print(buff, user_str_len);
            break;

        case 'w':
            word_print(buff, user_str_len);
            break;

        default:
            usage(argv[0]);
            exit(1);
    }

    print_buff(buff, BUFFER_SZ);
    free(buff);
    exit(0);
}

//TODO:  #7  Notice all of the helper functions provided in the 
//          starter take both the buffer as well as the length.  Why
//          do you think providing both the pointer and the length
//          is a good practice, after all we know from main() that 
//          the buff variable will have exactly 50 bytes?
//  
/* ANSWER: This is good practice because we never know if we need to change the size
            of the buffer. In general for the first version of this application we can assume it 
            will always be 50 bytes, but if we pass in the alias for the size then we can simply change
            how the size is defined and we don't need to refactor much. This is also good practice because we
            never know when we may need the size rather than guess we don't and then have to go back and modify prototypes
            later. NOTE: in my solutions, I originally passed in the BUFFER_SZ variable as an integer to every
            function but got rid of it once I CONFIRMED that everything worked without it in some functions.
            This was to make the code cleaner but also to eliminate default compilier warnings (which can be turned off, but
            it was sanity of mind).
*/