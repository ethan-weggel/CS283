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
void search_and_replace(char*, int, char*, char*);
int string_eq(char*, char*, int, int, int);
int size_check(int, int, int, int); 
int size_of_null_terminated_string(char*);
void overwrite_buff_with_replacement(char*, int, int, char*, int);

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

// calculate if a section of buffer equals a null terminated string
int string_eq(char* buff, char* string2, int buffStart, int len1, int len2) {
    if (len1 != len2) {
        return 0;
    }

    // exit as soon as a character doesn't match
    for (int i = 0; i < len1; i++) {
        if (*(buff + (sizeof(char) * (i + buffStart))) != *(string2 + (sizeof(char) * i))) {
            return 0;
        }
    }

    // otherwise return 1 or true
    return 1;
}

// get size by iterating through character array with char pointer, must be null termrmianted (used for getting length of args)
int size_of_null_terminated_string(char* string) {
    int count = 0;
    while (*string) {
        count++;
        string++;
    }
    return count;
}

// ensures that our replacement won't exceed our buffer size
int size_check(int bufferSize, int targetSize, int replacementSize, int str_len) {

    int newLength = str_len - targetSize + replacementSize;
    if (newLength <= bufferSize) {
        return 1;
    } else {
        return 0;
    }
}

// creates temp buff with the original buff up to replacement, replacement gets added, then the rest of original buffer. 
// original buffer is then overwritten with temp buff to have replaced word
void overwrite_buff_with_replacement(char* buff, int startIndex, int endIndex, char* replacement, int replacementLength) {
    char* tempBuff = malloc(BUFFER_SZ * sizeof(char)); // Allocate memory for temp buffer

    int buffIndex = 0;      // tracks position in the original buffer
    int tempBuffIndex = 0;  // tracks position in the temp buffer

    // start by loading up the temp buffer with buffer values before target word
    while (buffIndex < startIndex && tempBuffIndex < BUFFER_SZ) {
        *(tempBuff + tempBuffIndex) = *(buff + buffIndex);
        tempBuffIndex++;
        buffIndex++;
    }

    // load replacement values from replacement into temp buff 
    for (int i = 0; i < replacementLength && tempBuffIndex < BUFFER_SZ; i++) {
        *(tempBuff + tempBuffIndex) = *(replacement + i);
        tempBuffIndex++;
    }

    buffIndex = endIndex;

    // add the rest of the buffer after replacement to temp buffer
    while (buffIndex < BUFFER_SZ && tempBuffIndex < BUFFER_SZ) {
        *(tempBuff + tempBuffIndex) = *(buff + buffIndex);
        tempBuffIndex++;
        buffIndex++;
    }

    // fill the rest of tempBuff with null '.' to maintain the buffer size if the replacement was smaller
    while (tempBuffIndex < BUFFER_SZ) {
        *(tempBuff + tempBuffIndex) = '.';
        tempBuffIndex++;
    }

    // overwrite the original buffer in place with our temporary one
    for (int i = 0; i < BUFFER_SZ; i++) {
        *(buff + i) = *(tempBuff + i);
    }

    // free the memory cause that is good 'n stuff 'n yeah :D
    free(tempBuff);
}


// replaces first occurrence of target word with replacement assuming buffer size is not exceeded by operation
// otherwise operation is voided. does nothing is no matches were found.
void search_and_replace(char* buff, int str_len, char* target, char* replacement) {
    int words = 0;
    int buffIndex = 0;
    int wordStart = 0;
    int wordEnd = 0;

    printf("Word Search and Replace\n-----------------------\n");

    // same functionality as word print except we do not use selection_print
    // instead we use helpers to make sure we found a word equal to our target.
    // and that we can replace without exceeding buffer. then we call the overwrite function
    // the structure is otherwise identical
    for (int i = 0; i < str_len; i++) {
        if (*(buff + (sizeof(char) * i)) == ' ' && i != 0 && i != str_len-1) {
            words++;
            int targetSize = size_of_null_terminated_string(target);
            int replacementSize = size_of_null_terminated_string(replacement);

            if (size_check(BUFFER_SZ, targetSize, replacementSize, str_len)) {
                if (string_eq(buff, target, wordStart, (wordEnd - wordStart), targetSize)) {
                    overwrite_buff_with_replacement(buff, wordStart, wordEnd, replacement, replacementSize);
                }
            } 

            wordStart = wordEnd + 1;
        }
        buffIndex = i;
        wordEnd++;
    }

    if (*(buff + (sizeof(char) * (buffIndex + 1))) == '.') {
        words++;
        int targetSize = size_of_null_terminated_string(target);
        int replacementSize = size_of_null_terminated_string(replacement);
        if (size_check(BUFFER_SZ, targetSize, replacementSize, str_len)) {
            if (string_eq(buff, target, wordStart, (wordEnd - wordStart), targetSize)) {
                overwrite_buff_with_replacement(buff, wordStart, wordEnd, replacement, replacementSize);
            }
        } 
    }

    int newUserStringLength = str_len - size_of_null_terminated_string(target) + size_of_null_terminated_string(replacement);

    printf("Modified String: ");
    for (int i = 0; i < newUserStringLength; i++) {
        printf("%c", *(buff + i));
    }
    printf("\n");

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

        case 'x':
            if (argc != 5) {
                usage(argv[0]);
                exit(-1);
            }
            search_and_replace(buff, user_str_len, *(argv + 3), *(argv + 4));
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