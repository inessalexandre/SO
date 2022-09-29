#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>

int main(int argc, char* argv[]){

    if (argc == 1) {
		printf("usage: addmx file1 file2\n");
		return 0;
	} 

    if (argc != 3) {
		printf("Wrong usage: addmx file1 file2.\n");
		return 1;
	} 

    //read file
    FILE *matrix1 = fopen(argv[1], "r");
    FILE *matrix2 = fopen(argv[2], "r");

    if (matrix1 == NULL){                   //test file1
    	perror("cannot open file1");
		exit(EXIT_FAILURE);
    }
    if (matrix2 == NULL){                   //test file2
        perror("cannot open file2");
		exit(EXIT_FAILURE);    
    }
    

    int n_row1, n_col1, n_row2, n_col2; 

    /*read matrices size*/
    fscanf(matrix1, "%dx%d", &n_row1, &n_col1);
    fscanf(matrix2, "%dx%d", &n_row2, &n_col2);
    
    fgetc(matrix1);
    fgetc(matrix2);

    if (n_row1 != n_row2 || n_col1 != n_col2){
        perror("matrices sizes doesn't match with each other");
        exit(EXIT_FAILURE);
    }

    /*set shared memory*/
    int *mtx1  = mmap(NULL, sizeof(int) * n_row1 * n_col1, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0);
    int *mtx2  = mmap(NULL, sizeof(int) * n_row2 * n_col2, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0);
    int *mtx_res=mmap(NULL, sizeof(int) * n_row1 * n_col1, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0);

    char * line = NULL;
    size_t line_size = 0;

    /*read matrix1*/
    int i = 0;
    while (getline(&line,&line_size, matrix1) > 0)
    {
        char *token = strtok(line, " ");
		while (token != NULL)
		{
			*(mtx1 + i) = atoi(token);
			token = strtok(NULL, " ");
            i++;
		}
    }
    
    /*read matrix2*/
    i = 0;
    while (getline(&line,&line_size, matrix2) > 0)
    {
        char *token = strtok(line, " ");
		while (token != NULL)
		{
			*(mtx2 + i) = atoi(token);
			token = strtok(NULL, " ");
            i++;
		}
    }

    /*close files*/
    fclose(matrix1);
    fclose(matrix2);
    
    /*initialize result matrix*/
    for (int x = 0; x < n_row1 * n_col1; x++){
        *(mtx_res + x) = 0;
    }

    /*parent generate process to calculate each collum*/
    for (int c = 0; c < n_col1; c++){
        pid_t pid = fork();
        

        if (pid < 0) //fork
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        if (pid == 0) //children process
        {
            for (int r = 0; r < n_row1; r++)
            {
                *(mtx_res + r * n_col1 + c) = *(mtx1 + r * n_col1 + c) + *(mtx2 + r * n_col2 + c);
            }
            exit(EXIT_SUCCESS);
        }
    }

    
    //wait for all childs
    for (i = 0; i < n_col1; i++)
	{
		if (waitpid(-1, NULL, 0) < 0)
		{
			perror("waitpid");
			exit(EXIT_FAILURE);
		}
	} 
    
    /*print result matrix*/
    for (int r = 0; r < n_row1; r++)
    {
        for(int c = 0; c < n_col1; c++)
        {
            printf("%d\t", *(mtx_res + r * n_col1 + c));
        }
        printf("\n");
    }

    //released shared memory
    if (munmap(mtx1, sizeof(mtx1)) < 0)
	{
		perror("munmap");
		exit(EXIT_FAILURE);
	}
        if (munmap(mtx2, sizeof(mtx2)) < 0)
	{
		perror("munmap");
		exit(EXIT_FAILURE);
	}
        if (munmap(mtx_res, sizeof(mtx_res)) < 0)
	{
		perror("munmap");
		exit(EXIT_FAILURE);
	}

    return 0;

}