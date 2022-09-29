#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_STR_SIZE 64

int main(int argc, char* argv[]){

    if (argc == 1) {
		printf("usage: phrases [-l] file.\n");
		return 0;
	} 

    if (argc > 3) {
		printf("Wrong usage: phrases [-l] file.\n");
		return 1;
	} 

    int flagOn = 0;

    if (argc == 3){
        if (strcmp(argv[1], "-l") == 0){
            flagOn = 1;
        }
        else{
            printf("Invalid flag: phrases [-l] file.\n");
            return 1;
        }
    }

    //OPENS FILE - FIRST ARGUMENT
    //*fp - file pointer
    //*txt - text pointer
    //N - number of chars
    int fileNumber = flagOn + 1;
    FILE *fp;
    char txt;
    fp = fopen(argv[fileNumber], "r");
	
	int phraseSize = 0, count = 0;
    char* phrase = (char*)malloc(sizeof(char) * 1);
    phrase[0] = '\0';

	txt = fgetc(fp);
	while(txt != EOF){
        phrase = realloc(phrase, sizeof(char) * phraseSize + 2);
		phrase[phraseSize] = txt;
        phrase[phraseSize + 1] = '\0';
        phraseSize++;

        if (txt == '.' || txt == '?' || txt == '!') {
            count++;
            if (flagOn) {
                printf("[%d] %s\n", count, phrase);
            }
            txt = fgetc(fp);
            while ((txt == ' ') || (txt == '\n') || (txt == '\t') || (txt == '\r')){
                txt = fgetc(fp); //consume withespaces
            }
            phrase = (char*)malloc(sizeof(char) * 1);
            phrase[0] = '\0';
			phraseSize = 0;
            continue;
		}
        else{
            txt = fgetc(fp);
        }
	}

    fclose(fp);
    
    if (phraseSize != 0) {
        count++;
        if (flagOn) {
            printf("[%d] %s\n", count, phrase);
        }
    } 

    free(phrase);

    if (!flagOn) printf("%d \n",count);
	

    return EXIT_SUCCESS;
}