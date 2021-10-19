// Raul Cervantes
// rvcervan

#include "hw1.h"
#include <stdio.h>
#include <string.h>
// You may define any helper functions you want. Place them in helpers.c/.h


// reformats characters read from standard input (STDIN) and prints the converted
// input to standard out (STDOUT). 

// statistics about the input are printed to standard error (STDERR) on successful
// execution (exit status is 0).
// the following statistics are based on the input txt not output(reformatted) and 
// are printed to STDERR in the following order.
// - total number of characters
// - total number of lines
// - total number of words. A word is a nonzero length sequence of characters delimited by
//   one or more whitespace
// - length of the longest line (in characters)
// - [only printed when -W is specified] total number of occurences of string WORD (WORD may
//   contain any characters except whitespace)


// Main program
int main(int argc, char *argv[]) {
    
    int option_W = 0; //0 if no -W
    char *W_Word = NULL;
    int i;

    char *pattern;

    int num_of_commands = 0; //must be 1
    for(i = 0; i < argc; ++i){
        if(strcmp(argv[i], "-W")==0){
            
            option_W = 1;
            W_Word = argv[i+1];
            if(W_Word == NULL){
                option_W = 0;
                num_of_commands++;
            }
        }

        if(strcmp(argv[i], "-R")==0){
            pattern = argv[i+1];
            num_of_commands++;
        }

        if(strcmp(argv[i], "-t")==0){
     
            num_of_commands++;
        }
        if(strcmp(argv[i], "-T")==0){
    
            num_of_commands++;
        }
        if(strcmp(argv[i], "-U")==0){
        
            num_of_commands++;
        }
        if(strcmp(argv[i], "-L")==0){
       
            num_of_commands++;
        }
        

        
    }
    
    int char_number = 0; //number of characters
    int line_number = 0; //number of lines
    int word_number = 0; //number of words
    int longest_line = 0; //length of longest line;
    int word_occurence = 0; //number of word occurences, only if -W is used.

    //./bin/formattxt -U < [txt file]
    // -U convert all alphabet characters to uppercase
    if(strcmp(argv[1], "-U")==0 && num_of_commands == 1){
        int current_char = 0;
        int running = 1;
        int in_word = 0;
        int line_length = 0;

        char current_word[100] = "";
        int word_progress = 0;
        char dummy[100] = "";
        while(running){
            current_char = getchar(); 
            if(current_char == EOF){break;}
            if(option_W == 1){
                if(current_char == 32 || current_char == 10 || current_char == 9 || current_char == 11){
                    
                    if(strcmp(current_word, W_Word)==0){
                        
                        word_occurence++;
   
                    }
                    memcpy(current_word, dummy, sizeof dummy);
                    word_progress = 0;
                }
                else{
                    current_word[word_progress] = current_char;
                    word_progress++;
                }
            }

            

           

            if(current_char == 32 || current_char == 10){in_word = 0;}
            else if(in_word == 0){in_word = 1; word_number++;}
            char_number++;
            if(current_char == 10){line_number++;}
            
            line_length++;
            if(current_char == 10){
                line_length--;
                if(line_length > longest_line){
                    longest_line = line_length;
                }
                line_length = 0;
            }

            if(current_char > 96 && current_char < 123){
                current_char = current_char - 32;
            }
            
            putchar(current_char);
        }
        if(char_number == 0){
            
            fprintf(stderr, "%s\n", "ERROR! Usage: ./formattxt -L | -U | -T | -t | -R SYMBOLS [-W WORD]\n");
            return 1;
        }
        if(option_W == 0){
            fprintf(stderr, "%i\n%i\n%i\n%i\n", char_number, line_number, word_number, longest_line);
        }
        else{
            fprintf(stderr, "%i\n%i\n%i\n%i\n%i\n", char_number, line_number, word_number, longest_line, word_occurence);
        }
       
    }
    else if(strcmp(argv[1], "-L")==0 && num_of_commands == 1){
        int current_char = 0;
        int running = 1;
        int in_word = 0;
        int line_length = 0;

        char current_word[100] = "";
        int word_progress = 0;
        char dummy[100] = "";
        while(running){
            current_char = getchar();
            
            if(option_W == 1){
                if(current_char == 32 || current_char == 10 || current_char == 12 || current_char == 13 ||
                current_char == 9 || current_char == 11 || current_char == EOF){
                    if(strcmp(current_word, W_Word)==0){
                        word_occurence++;
   
                   }
                    memcpy(current_word, dummy, sizeof dummy);
                    word_progress = 0;
                }
                else{
                    current_word[word_progress] = current_char;
                    word_progress++;
                }
            }
            
            line_length++;
            if(current_char == 10 || current_char == EOF){
                line_length--;
                if(line_length > longest_line){
                    longest_line = line_length;
                }
                line_length = 0;
            }
            if(current_char == EOF){break;}
            if(current_char == 32 || current_char == 10 || current_char == 12 || current_char == 13 ||
                current_char == 9 || current_char == 11){in_word = 0;}
            else if(in_word == 0){in_word = 1; word_number++;}
            
            char_number++;
            if(current_char == 10){line_number++;}
            
/*            line_length++;
            if(current_char == 10 || current_char == EOF){
                line_length--;
                if(line_length > longest_line){
                    longest_line = line_length;
                }
                line_length = 0;
            }*/

            if(current_char > 64 && current_char < 91){
                current_char = current_char + 32;
            }
            
            putchar(current_char);
        }
     
        if(char_number == 0){
            
            fprintf(stderr, "%s\n", "ERROR! Usage: ./formattxt -L | -U | -T | -t | -R SYMBOLS [-W WORD]\n");
            return 1;
        }

        if(option_W == 0){
            fprintf(stderr, "%i\n%i\n%i\n%i\n", char_number, line_number, word_number, longest_line);
        }
        else{
            fprintf(stderr, "%i\n%i\n%i\n%i\n%i\n", char_number, line_number, word_number, longest_line, word_occurence);
        }
    }
    else if(strcmp(argv[1], "-T")==0 && num_of_commands == 1){
        int current_char = 0;
        int running = 1;
        int in_word = 0;
        int firstletter = 1;
        
        int line_length = 0;
        char current_word[100] = "";
        int word_progress = 0;
        char dummy[100] = "";
        while(running){
            current_char = getchar();
            if(current_char == EOF){break;}
            if(option_W == 1){
                if(current_char == 32 || current_char == 10){
                    if(strcmp(current_word, W_Word)==0){
                        word_occurence++;
   
                    }
                    memcpy(current_word, dummy, sizeof dummy);
                    word_progress = 0;
                }
                else{
                    current_word[word_progress] = current_char;
                    word_progress++;
                }
            }

            if(current_char == 32 || current_char == 10){in_word = 0;}
            else if(in_word == 0){in_word = 1; word_number++;}
            char_number++;
            if(current_char == 10){line_number++;}
                
            line_length++;
            if(current_char == 10){
                line_length--;
                if(line_length > longest_line){
                    longest_line = line_length;
                }
                line_length = 0;
            }
                
            //Change below to modify text
            //first detect if we are first letter in word
            if(current_char == 32 || current_char == 10){
                firstletter = 1;
            }
            else if(firstletter == 0){
                if(current_char > 64 && current_char < 91){
                    current_char = current_char + 32;
                }
                else{
                    putchar(current_char);
                    continue;
                }
            }
            else if(firstletter == 1){
                if(current_char > 96 && current_char < 123){
                    current_char = current_char -32;
                    firstletter = 0;
                        
                }
                else if(current_char > 64 && current_char < 91){
                    firstletter = 0;
                }

                
            }
               
            putchar(current_char);
        }
      
        
        if(char_number == 0){
            
            fprintf(stderr, "%s\n", "ERROR! Usage: ./formattxt -L | -U | -T | -t | -R SYMBOLS [-W WORD]\n");
            return 1;
        }
        if(option_W == 0){
            fprintf(stderr, "%i\n%i\n%i\n%i\n", char_number, line_number, word_number, longest_line);
        }
        else{
            fprintf(stderr, "%i\n%i\n%i\n%i\n%i\n", char_number, line_number, word_number, longest_line, word_occurence);
        }
    } 
    else if(strcmp(argv[1], "-t")==0 && num_of_commands == 1){
        int current_char = 0;
        int running = 1;
        int in_word = 0;
        int maxlength = 0;
        int line_length = 0;
        char current_word[100] = "";
        int word_progress = 0;
        char dummy[100] = "";
        while(running){
            current_char = getchar();
            if(current_char == EOF){break;}
            if(option_W == 1){
                if(current_char == 32 || current_char == 10){
                    if(strcmp(current_word, W_Word)==0){
                        word_occurence++;
   
                    }
                    memcpy(current_word, dummy, sizeof dummy);
                    word_progress = 0;
                }
                else{
                    current_word[word_progress] = current_char;
                    word_progress++;
                }
            }

            if(current_char == 32 || current_char == 10){in_word = 0;}
            else if(in_word == 0){in_word = 1; word_number++;}
            char_number++;
            if(current_char == 10){line_number++;}
            
            line_length++;
            if(current_char == 10){
                line_length--;
                if(line_length > longest_line){
                    longest_line = line_length;
                }
                line_length = 0;
            }
            
            //Change below to modify text
            //count characters and if characters reach 80 begin newline
            if(current_char != 10){maxlength++;putchar(current_char); }
             
            if(maxlength == 80){
                putchar(10);
                
                maxlength = 0;
            }
            else if(current_char == 10 && maxlength != 0){
            
                putchar(current_char);
                maxlength = 0;
            }
      
            //if(current_char != 10){putchar(current_char);}        
            //putchar(current_char);
        }
       
    
        if(char_number == 0){
            
            fprintf(stderr, "%s\n", "ERROR! Usage: ./formattxt -L | -U | -T | -t | -R SYMBOLS [-W WORD]\n");
            return 1;
        }
        if(option_W == 0){
            fprintf(stderr, "%i\n%i\n%i\n%i\n", char_number, line_number, word_number, longest_line);
        }
        else{
            fprintf(stderr, "%i\n%i\n%i\n%i\n%i\n", char_number, line_number, word_number, longest_line, word_occurence);
        }
    }
    else if(strcmp(argv[1], "-R")==0 && num_of_commands == 1){
        int current_char = 0;
        int running = 1;
        int in_word = 0;
        int maxlength = 0;
        int line_length = 0;
        char current_word[100] = "";
        int word_progress = 0;
        char dummy[100] = "";
        while(running){
            current_char = getchar();
            if(current_char == EOF){break;}
            if(option_W == 1){
                if(current_char == 32 || current_char == 10){
                    if(strcmp(current_word, W_Word)==0){
                        word_occurence++;
   
                    }
                    memcpy(current_word, dummy, sizeof dummy);
                    word_progress = 0;
                }
                else{
                    current_word[word_progress] = current_char;
                    word_progress++;
                }
            }

            if(current_char == 32 || current_char == 10){in_word = 0;}
            else if(in_word == 0){in_word = 1; word_number++;}
            char_number++;
            if(current_char == 10){line_number++;}
                
            line_length++;
            if(current_char == 10){
                line_length--;
                if(line_length > longest_line){
                    longest_line = line_length;
                }
                line_length = 0;
            }
                
            //Change below to modify text
            //if char from getchar is in pattern, then don't call putchar
                
            char *present = strchr(pattern, current_char);
           
            if(present != NULL){
                
          
                current_char = getchar();
         
                char *present2 = strchr(pattern, current_char);
                if(present2 != NULL){
                
                    if(current_char == EOF){break;}

                    if(current_char == 32 || current_char == 10){in_word = 0;}
                    else if(in_word == 0){in_word = 1; word_number++;}
                    char_number++;
                    if(current_char == 10){line_number++;}
                
                    line_length++;
                    if(current_char == 10){
                        line_length--;
                        if(line_length > longest_line){
                            longest_line = line_length;
                        }
                    line_length = 0;
                    }
                    continue;
                }
           
                if(current_char == EOF){break;}

                if(current_char == 32 || current_char == 10){in_word = 0;}
                else if(in_word == 0){in_word = 1; word_number++;}
                char_number++;
                if(current_char == 10){line_number++;}
                
                line_length++;
                if(current_char == 10){
                    line_length--;
                    if(line_length > longest_line){
                        longest_line = line_length;
                }
                line_length = 0;
                }
            }
            putchar(current_char);

        }

        if(char_number == 0){
            fprintf(stderr, "%s\n", "ERROR! Usage: ./formattxt -L | -U | -T | -t | -R SYMBOLS [-W WORD]\n");
            return 1;
        }
        fprintf(stderr, "%i\n%i\n%i\n%i\n", char_number, line_number, word_number, longest_line);
        
        
    }
    else{
        fprintf(stderr, "%s\n", "ERROR! Usage: ./formattxt -L | -U | -T | -t | -R SYMBOLS [-W WORD]\n");
        return 1;
    }
    return 0;
}
