#include "../include/helpers2.h"
#include "../include/hw2.h"
#include "../include/hw2_ec.h"
#include "../include/constants2.h"

int main(int argc, char* argv[]) {
    char* keyphrase = NULL;
    char* mapping_file = NULL;
    char* output_file = NULL;
    int mode_flag = -1;    // 0 for encrypt, 1 for decrypt

    // Use basic getopt to parse flags and respective arguments
    int c;
    while ((c = getopt(argc, argv, "hk:m:O:ED")) >= 0) {
        switch (c) {
            case 'h':
                fprintf(stdout,USAGE_MSG);
            case 'k':
                keyphrase = optarg;
                break;
            case 'm':
                mapping_file = optarg;
                break;
            case 'O':
                output_file = optarg;
                break;
            case 'E':
                mode_flag = 0;
                break;
            case 'D':
                mode_flag = 1;
                break;
            default:
                
                fprintf(stderr, USAGE_MSG);
                return 2;
                //return EXIT_FAILURE;
        }
    }

    

    // validate mode was specified
    if (mode_flag == -1)
    {
        fprintf(stderr, "Encryption or Decryption not specified.\n\n" USAGE_MSG);
        return EXIT_FAILURE;
    }
     
    // INSERT YOUR IMPLEMENTATION HERE
    int status;
    list_t *list = NULL;
    FILE *out = stdout;
    if(mode_flag == 0){
        if(mapping_file == NULL){
            list = GenerateDefaultMorseMappings();
        }
        else{
            FILE *map = fopen(mapping_file, "r");
            if(map == NULL){ fclose(map); return 1;}
            fclose(map);
            list = GenerateFMCMappings(mapping_file);
        }
       

        if(output_file != NULL){
            out = fopen(output_file, "w+");
        }
        
        
        if(keyphrase == NULL){
            keyphrase = "";
        }

        status = encrypt(list, DEFAULT_FMC2KEY, keyphrase , out);
        fclose(out);

        DestroyDefaultMorseMappings(list);
    }
    else{
        if(mapping_file == NULL){
            list = GenerateDefaultMorseMappings();
        }
        else{
            return 1;
            FILE *map = fopen(mapping_file, "r");
            if(map == NULL){ fclose(map); return 1;}
            fclose(map);
            list = GenerateFMCMappings(mapping_file);
        }
        DestroyDefaultMorseMappings(list);
    }
    

    return 0;
}
