#include <sys/wait.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

#define MSGSIZE 256
#define READ_END 0
#define WRITE_END 1


int isChar(char x){
    return !(((x < 65) || (x > 122)) || ( x > 90 && x < 97));
}

int main(int argc, char *argv[])
{

    if (argc != 1)
    {
        printf("Wrong usage: cypher < file1 ");
        exit(EXIT_FAILURE);
    }


    int ptoc[2]; // parent to children
    int ctop[2]; // children to parent

    pid_t pid;
    char buffer[MSGSIZE];

    if (pipe(ptoc) < 0 || pipe(ctop) < 0)
    {
        // pipe failed
        exit(EXIT_FAILURE);
    }

    memset(buffer, '\0', MSGSIZE); //initialize all values on the array

    if ((pid = fork()) < 0) // fork
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    else if (pid == 0) // child process
    {
        char end[2];
        memset(end, '\0', 2);
        end[1] = '1';

        close(ptoc[WRITE_END]);
        close(ctop[READ_END]);


        /* read cypher.txt */

        FILE * cypher_file;
        char file_name[11] = "cypher.txt";
        char * line = NULL;
        size_t len = 0;

        char **strings1;
        char **strings2;
        int numberOfElements = 0;

        cypher_file = fopen(file_name, "r");
        if(cypher_file == NULL){
            exit (EXIT_FAILURE);
        }

        while (getline(&line, &len, cypher_file) > 0)
        {
            numberOfElements++;
            strings1 = realloc(strings1, numberOfElements * sizeof(char*));
            strings2 = realloc(strings2, numberOfElements * sizeof(char*));

            char *token = strtok(line, " ");
            strings1[numberOfElements - 1] = malloc(strlen(token) * sizeof(char));
            strcpy(strings1[numberOfElements - 1], token);
            
            token = strtok(NULL, " ");
            token[strcspn(token, "\r\n")] = '\0'; 
            strings2[numberOfElements - 1] = malloc(strlen(token) * sizeof(char));
            strcpy(strings2[numberOfElements - 1], token);
        }
        fclose(cypher_file);


        /* read pipe -processs word - write pipe */

        while (1)
        {
            read(ptoc[READ_END], buffer, MSGSIZE);

            if(buffer[1] == end[1] && buffer[0] == end[0]) break; //end of the message

            for(int i = 0; i < numberOfElements; i++){
                if(strcmp(buffer, strings1[i]) == 0){
                    strcpy(buffer, strings2[i]);
                    break;
                }
                if(strcmp(buffer, strings2[i]) == 0){
                    strcpy(buffer, strings1[i]);
                    break;
                }
            }

            write(ctop[WRITE_END], buffer, MSGSIZE);
        }
        
        close(ptoc[READ_END]);
        close(ctop[WRITE_END]);

        while (numberOfElements-- >= 0)
        {
            free(strings1[numberOfElements]);
            free(strings2[numberOfElements]);
        }
        
        free(strings1);
        free(strings2);

        exit(EXIT_SUCCESS);

        /* end of child process */

    }
    else if (pid > 0) // parent process
    {
        char x;

        close(ptoc[0]);
        close(ctop[1]);

        int it_counter = 0; //iteration counter

        /*read words of stdin - write on pipe */
        x = getc(stdin);
        int i = 0;

        while (x != EOF)
        {
            i = 0;
            buffer[i++] = x;

            //Pontuation
            if(!isChar(x))
            {
                while(((x = getc(stdin)) != EOF) && (!isChar(x))){
                    buffer[i++] = x;
                }
            }
            //Words
            else
            {
                while(((x = getc(stdin)) != EOF) && isChar(x)){
                    buffer[i++] = x; 
                }

            }
            buffer[i] = '\0';

            write(ptoc[1], buffer, MSGSIZE);
            //printf("w1: %d\n", w1);

            it_counter++;
        }

        buffer[0] = '\0';
        buffer[1] = '1';
        write(ptoc[1], buffer, MSGSIZE); //messag to exit childs process


        /* read pipe - write on stdout */
        while(it_counter--){
            read(ctop[0],buffer , MSGSIZE);
            fprintf(stdout, "%s", &buffer[0]);
        }
        close(ptoc[1]);
        close(ctop[0]);
        
        if (waitpid(pid, NULL, 0) == -1) {
            perror("wait");
            return EXIT_FAILURE;
        }

    }

    return 0;
}
