#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define BUFFER_SZ 50

//prototypes
void usage(char *);
void print_buff(char *, int);
int setup_buff(char *, char *, int);

//prototypes for functions to handle required functionality
int count_words(char *, int, int);
void reverse_string(char*, int);
void reverse_print(char*, int);
void word_print(char*, int);
void selection_print(char*, int, int);


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

int count_words(char *buff, int len, int str_len){
    
    int words = 0;
    int buffIndex = 0;

    if (str_len == 0) {
        printf("Word Count: %d\n", words);
        return 0;
    }

    for (int i = 0; i < str_len; i++) {
        if (*(buff + (sizeof(char) * i)) == ' ' && i != 0 && i != str_len-1) {
            words++;
        }
        buffIndex = i;
    }

    if (*(buff + (sizeof(char) * (buffIndex + 1))) == '.') {
        words++;
    }

    return words;
}

void reverse_string(char* buff, int str_len) {
    char* tail = buff + str_len - 1;
    char temp;

    while (buff < tail) {
        temp = *buff;
        *buff = *tail;
        *tail = temp;
        buff++;
        tail--;
    }
}

void reverse_print(char* buff, int print_len) {
    for (int i = 0; i < print_len; i++) {
        printf("%c", *(buff + (sizeof(char) * i)));
    }
    printf("\n");
}

void selection_print(char* buff, int start_index, int end_index) {
    for (int i = start_index; i < end_index; i++) {
        printf("%c", *(buff + (sizeof(char) * i)));
    }
}

void word_print(char* buff, int str_len) {
    int words = 0;
    int buffIndex = 0;
    int wordStart = 0;
    int wordEnd = 0;

    printf("Word Print\n----------\n");

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

    if (*(buff + (sizeof(char) * (buffIndex + 1))) == '.') {
        words++;
        printf("%d. ", words);
        selection_print(buff, wordStart, wordEnd);
        printf(" (%d)\n", wordEnd - wordStart);
    }
}

int main(int argc, char *argv[]){

    char *buff;             //placehoder for the internal buffer
    char *input_string;     //holds the string provided by the user on cmd line
    char opt;               //used to capture user option from cmd line
    int  rc;                //used for return codes
    int  user_str_len;      //length of user supplied string

    //TODO:  #1. WHY IS THIS SAFE, aka what if arv[1] does not exist?
    //      PLACE A COMMENT BLOCK HERE EXPLAINING
    if ((argc < 2) || (*argv[1] != '-')){
        usage(argv[0]);
        exit(1);
    }

    opt = (char)*(argv[1]+1);   //get the option flag

    //handle the help flag and then exit normally
    if (opt == 'h'){
        usage(argv[0]);
        exit(0);
    }

    //WE NOW WILL HANDLE THE REQUIRED OPERATIONS

    //TODO:  #2 Document the purpose of the if statement below
    //      PLACE A COMMENT BLOCK HERE EXPLAINING
    if (argc < 3){
        usage(argv[0]);
        exit(1);
    }

    input_string = argv[2]; //capture the user input string

    //TODO:  #3 Allocate space for the buffer using malloc and
    //          handle error if malloc fails by exiting with a 
    //          return code of 99
    // CODE GOES HERE FOR #3

    buff = malloc(BUFFER_SZ * sizeof(char));

    if (buff == NULL) {
        exit(99);
    }

    user_str_len = setup_buff(buff, input_string, BUFFER_SZ);     //see todos
    if (user_str_len < 0){
        printf("Error setting up buffer, error = %d", user_str_len);
        exit(2);
    }

    switch (opt){
        case 'c':
            rc = count_words(buff, BUFFER_SZ, user_str_len);  //you need to implement
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

    //TODO:  #6 Dont forget to free your buffer before exiting
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
//          PLACE YOUR ANSWER HERE