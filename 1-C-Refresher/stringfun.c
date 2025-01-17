#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define BUFFER_SZ 50 
#define SPACE_CHAR ' '

//prototypes
void usage(char *);
void print_buff(char *, int);
int  setup_buff(char *, char *, int);

//prototypes for functions to handle required functionality
int  count_words(char *, int, int);
//add additional prototypes here
void  reverse_string(char *, int, int);
void  word_print(char *, int, int);

int setup_buff(char *buff, char *user_str, int len){
    
    int char_count = 0;
	char* source = user_str;		//source of content to be entered into buffer
    char* destination = buff;		//destination where content will be entered
	int space_present = 0;			//integer representation of boolean for if we see a space
    int index = 0;


	//skip over leading space characters
    while (*source == SPACE_CHAR) {
		source++;
	}

	//iterate over the length of the input string
    while (*source != '\0') {
    	if (*source != SPACE_CHAR && *source != '\t') {
    		char_count++;
    		*destination = *source;
    		destination++;
    		space_present = 0;		//indicate that we've not encountered a space
    	} else if (!space_present) {
    		if (char_count >= len) {
    			return -1;
    		}
			*destination = SPACE_CHAR;
			destination++;
			char_count++;
			space_present = 1;		//encountered a space
		}
    	if (*source == '0') {					 			//returns error code if input
    		printf("Error: input cannot contain a 0\n");	//contains a 0 (optional extra error)
    		return -2;	
    	}

    	source++;
    	index++;
    }

	//removing the trailing space characters
    while (char_count > 0 && *(destination - 1) == ' ') {
        destination--;
        char_count--;
    }

	if (char_count > len) {
    	return -1;
	}
	
	//add periods to pad the buffer to 50 characters
	memset(buff + char_count, '.', len - char_count);
	return char_count;
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
	int wc = 0;				//word counter
	int word_start = 0;		//integer representation of a boolean indicating the start of a word
	char cur;				//current character in the string
	
	if (str_len > len) {
		printf("Error: Input must be less than 50 characters\n");
		return -1;
	}
	
	//iterate over the length of the string
	for (int i = 0; i < str_len; i++) {
		cur = *(buff + i);
		if (!word_start) {
			if (cur == SPACE_CHAR) {
				continue;
			}
			
			//starting a new word
			wc++;
			word_start = 1;
		} else {

			//we have encounterd a space
			if (cur == SPACE_CHAR) {
				word_start = 0;
			}
		}
	}

    return wc;
}

void reverse_string(char *buff, int len, int str_len) {
	int end_idx = str_len - 1;	//end index of the string
	int start_idx = 0;			//start index of the string
	char tmp_char;				//temporary character for swapping

	if (str_len > len) {
		printf("Error: Input must be less than 50 characters\n");
		return;
	}
	
	//swap the ending and start characters and pinch until we meet in the middle of the string
	while (end_idx > start_idx) {
		tmp_char = *(buff + start_idx);
		*(buff + start_idx) = *(buff + end_idx);
		*(buff + end_idx) = tmp_char;
		start_idx++;
		end_idx--;
	}
	return;
}

void word_print(char *buff, int len, int str_len) {
	int wc = 0;							//word counter
	int word_start = 0;					//indicating the start of a word
	int last_char_idx = str_len - 1;	//index of the last character in the input string
	int wlen = 0;						//length of the current word
	char cur;							//current element in the string

	
	if (str_len > len) {
        printf("Error: Input must be less than 50 characters\n");
    	return;
    }
	
	//iterate over the length of the string
	for (int i = 0; i < str_len; i++) {
		cur = *(buff + i);				//set the current element to the ith index of the buffer
		if (!word_start) {
			if (cur == SPACE_CHAR) {	
				continue;
			}
			wc++;
			word_start = 1;
			printf("%d. ", wc);			//print the number of the word
			printf("%c", cur);			//print the current letter of the word
			wlen++;
		} else {
			if (cur == SPACE_CHAR) {
				printf(" (%d)\n", wlen);//print the length of the word
				wlen = 0;
				word_start = 0;
			} else {
				printf("%c", cur);
				wlen++;
			}
		}
		if (i == last_char_idx) {		//print the length of the last word
			printf(" (%d)\n", wlen);
			word_start = 0;
			wlen = 0;
		}
	}
	printf("\nNumber of words returned: %d\n", wc);
}

void replace_word(char* buff, int len, int str_len, char* target, char* new) {
	int target_len = 0;							//length of the target word
	int new_len = 0;							//length of the new word to replace the target
	if (str_len > len) {
		printf("Error: Input must be less than 50 characters\n");
	}

	//count the length of the target word
	while (*(target + target_len) != '\0') {
		target_len++;
	}
    
    //count the length of the replacement word
    while (*(new + new_len) != '\0') {
    	new_len++;
    }
	
	//iterate until we are at a point where the target word would be too long to be found
    for (int i = 0; i < str_len - target_len; i++) {
        int j = 0;		//create a new variable to test if we've matched the entire target
        while (j < target_len && *(buff + i + j) == *(target + j)) {
        	j++;
        }

        //check if the target word appears by itself in the stribg
        if (j == target_len && *(buff + i + j) == SPACE_CHAR|| *(buff + i + j) == '\0') {
			int difference = new_len - target_len;						//calculate the difference of new and target
			int max_replacement_len = len - str_len - target_len;		//calcaulte the max length of new word
			if (new_len > max_replacement_len) {
				printf("Error: Replacement word must be at most %d characters long", max_replacement_len);
        	}

        	//make space for the larger new word replacing the target
			if (difference > 0) {
				for (int k = str_len; k >= i + target_len; k--) {
					*(buff + k + difference) = *(buff + k);
				}

			//reduce space for the smaller word replacing 
			} else if (difference < 0) {
				for (int k = i + target_len; k <=str_len; k++) {
					*(buff + k + difference) = *(buff + k);
				}
			}

			//replace the target with the new word
			for (int k = 0; k < new_len; k++) {
				*(buff + i + k) = *(new + k);
			}
			
			printf("Modified String: ");
			for (int k = 0; k < str_len + difference; k++) {
				printf("%c", *(buff + k));
			}
			printf("\n");
			return;
		}
	}
	printf("You did not enter a word that was in the string.\n");
	return;
}

			
		
int main(int argc, char *argv[]){

    char *buff;             //placehoder for the internal buffer
    char *input_string;     //holds the string provided by the user on cmd line
    char opt;               //used to capture user option from cmd line
    int  rc;                //used for return codes
    int  user_str_len;      //length of user supplied string

    //TODO:  #1. WHY IS THIS SAFE, aka what if arv[1] does not exist?

    //		 It's safe because the first condition of the or statement
    //		 is the edge case for argv[1] not exisiting.
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

     			
	//			The purpose of the if statement below is to handle
    //			any case where the user doesn't include enough
    //			commands, hence why it will execute if there are
    //			less than three input arguments. Without all three
    //			input arguments, the program does not have enough 
    //			information to run

    if (argc < 3){
        usage(argv[0]);
        exit(1);
    }

    input_string = argv[2]; //capture the user input string

    //TODO:  #3 Allocate space for the buffer using malloc and
    //          handle error if malloc fails by exiting with a 
    //          return code of 99
    
	buff = malloc(BUFFER_SZ);
	
	if (buff == NULL) {
		printf("Error : Memory allocation failure\n");
		exit(99);
	}

    user_str_len = setup_buff(buff, input_string, BUFFER_SZ);     //see todos
    if (user_str_len < 0){
        printf("Error setting up buffer, error = %d", user_str_len);
        free(buff);
        exit(2);
    }

    switch (opt){
        case 'c':
            rc = count_words(buff, BUFFER_SZ, user_str_len);
            if (rc < 0){
                printf("Error counting words, rc = %d", rc);
                exit(2);
            }

			printf("Word Count: %d\n", rc);
            break;

		default:
            usage(argv[0]);
            exit(1);
    	
    	case 'r':
    		reverse_string(buff, BUFFER_SZ, user_str_len);
			printf("Reversed String: ");
            for (int i = 0; i < user_str_len; i++) {
                printf("%c", *(buff + i));
            }
            printf("\n");
			break;    

		case 'w':
			printf("Word Print\n----------\n");
			word_print(input_string, BUFFER_SZ, user_str_len);
            break;

		case 'x':
			replace_word(buff, BUFFER_SZ, user_str_len, argv[3], argv[4]);
			break;

    }

    print_buff(buff,BUFFER_SZ);
    free(buff);
    exit(0);
}

//			Notice all of the helper functions provided in the 
//          starter take both the buffer as well as the length.  Why
//          do you think providing both the pointer and the length
//          is a good practice, after all we know from main() that 
//          the buff variable will have exactly 50 bytes?
//
//  		It's important to use both to ensure that the buffer will
//			not overflow, and it would help in some case where we 
//			change the buffer size from 50 to something else. It also
//			important to include both the pointer and its length to
//			avoid any hidden assumptions, and it also aligns with other
//			system calls and library functions as they have similar
//			arguments.
