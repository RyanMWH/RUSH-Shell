#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>


//Ryan Hanner
//U69082659
//This is a simple installment of a rush unix system this program using simple commands as well 
//it also uses some redirection and parallel commands.

//Definitions for the maximum input and the specific delimiters to separate by
#define MAX_INPUT 255
#define TOKENDELIM " \t\r\n\a"

//Path for the path command
char * path[MAX_INPUT];

//Stand error message for the RUSH system
char error_message[30] = "An error has occurred\n";

//Simple function to print the error message and flush it out
void ErrorPrint(){
    write(STDERR_FILENO, error_message, strlen(error_message));
    fflush(stderr);
}

//This function grabs the data from the input and stores it
int ReadLine(char **line, size_t *buffsize){
    int read = getline(line, buffsize, stdin);
    if (read == -1){
        exit(0);
    }
    return read;
}

//Separates the command line input by tokens and stores it in the cmd line array
int ParseLine(char *line, char * cmd[]){
    int i;
    line[strcspn(line, "\n")] = '\0';
    for(i = 0; (cmd[i] = strsep(&line, TOKENDELIM)) != NULL; i++);
    return i;
}

//This function trys to act like the path commands main function but I don't think it works the way it is supposed too properly played
//with it for three days can't seem to figure out the for sure answer
void SetPathCommand(char * cmd[], int lineLength) {
    for(int i = 0; i < lineLength; i++){
        path[i] = (char *)malloc(strlen(cmd[i]) + 1);
        strcpy(path[i], cmd[i]);
    }
}

//This is where the command looks for the executable
int FindCommand(char * cmd[]){
    char tempPath[MAX_INPUT + 1];
    for (int i = 0; path[i] != NULL; i++){
        strcpy(tempPath, path[i]);
        strcat(tempPath, "/");
        strcat(tempPath, cmd[0]);
        //printf("Attempting to execute: %s\n", tempPath);
        if (access(tempPath, X_OK) == 0) {
            execv(tempPath, cmd);
            return 0;
        }
    }
    return -1;
}
        
//This function checks to see what type of execution we are doing normal, redirection or parallel
int ExecutionTypeHandler(char * cmd[], int * execType) {
    for (int i = 0; cmd[i] != NULL; i++){
        //Redirection Execution
        if (strcmp(cmd[i], ">") == 0) {
            *execType = 1;
            return i;
        }
        //Parallel Execution
        if (strcmp(cmd[i], "&") == 0) {
            *execType = 2;
            return -1;
        }
    }
    //Normal execution
    *execType = 0;
    return -1;
}

//This function runs the redirection execution
void HandleRedirectionAndExecution(char * cmd[], int execType, int redirectIndex, int lineLength) {
    char *redirectFile = NULL;

    if ((execType == 1) && (lineLength > redirectIndex + 2)){
        ErrorPrint();
    } else {
        if (execType == 1){
            redirectFile = cmd[redirectIndex + 1];
            if (redirectFile == NULL){
                ErrorPrint();
                return;
            }
            cmd[redirectIndex] = NULL;
        }
        int rc = fork();
        if (rc < 0){
            ErrorPrint();
        } else if (rc == 0) {
            if (execType == 1) {
                int fileDirection = open(redirectFile, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                if (fileDirection == -1){
                    ErrorPrint();
                }
                dup2(fileDirection, STDOUT_FILENO);
                close(fileDirection);
            }
            if (FindCommand(cmd) == -1) {
                ErrorPrint();
            }
            exit(0);
        } else {
            wait(NULL);
        }
    }
}

//This function is what handles the parallel commands
void HandleParallel(char *cmd[], int execType, int redirectIndex){
    int processCount = 1, i;
    int parallelIndex[10];
    char *redirectFile = NULL;

    parallelIndex[0] = 0;
    for(i = 0; cmd[i] != NULL; i++){
        if (strcmp(cmd[i], "&") == 0){
            cmd[i] = NULL;
            parallelIndex[processCount] = i + 1;
            processCount++;
        }
    }
    for(i = 0; i < processCount; i++){
        redirectIndex = ExecutionTypeHandler(cmd + parallelIndex[i], &execType);
        if (((execType == 0) || (execType == 1)) && ((execType == 1) && (cmd[parallelIndex[i] + redirectIndex + 2] == NULL))){
            ErrorPrint();
        } else {
            if (execType == 1){
                redirectFile = cmd[parallelIndex[i] + redirectIndex + 1];
                cmd[parallelIndex[i] + redirectIndex] = NULL;
            }
            int rc = fork();
            if (rc < 0){
                ErrorPrint();
            } else if (rc == 0) {
                if (execType == 1){
                    int fileDirection = open(redirectFile, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                    if (fileDirection == -1){
                        ErrorPrint();
                    }
                    dup2(fileDirection, STDOUT_FILENO);
                    close(fileDirection);
                }    
                if (FindCommand(cmd + parallelIndex[i]) == -1) {
                    ErrorPrint();
                }
                exit(0);
            }
        }
    }
    for (i = 0; i < processCount; i++) wait(NULL);
}

int main(int argc, char * argv[]){
    if (argc > 1){
        ErrorPrint();
        exit (1);
    }
    //Standard path /bin
    path[0] = "/bin";

    char * line;
    line = NULL;
    size_t buffer = 0;
    char * cmd [MAX_INPUT];
    int stat = 1;

    //Need these to handle redirection and parallel
    int execType;

    while (stat){
        printf("rush> ");
        fflush(stdout);

        //Grabs the input and parses it
        int read = ReadLine(&line, &buffer);
        int lineLength = ParseLine(line, cmd);
        for (int i = 0; cmd[i] != NULL; i++) {
            //printf("cmd[%d]: %s\n", i, cmd[i]);
        }
        //this is the return of a good getline read
        if (read == 1) {
            continue;
        } else {
            //This is the exit command to exit the program which drops an error if more input than just exit
            if (strcmp(cmd[0], "exit") == 0){
                if (lineLength > 1){
                    ErrorPrint();
                    continue;
                }
                exit(0);
            }
            //This is the change directory command it checks to make sure there is atleast two arguments
            if (strcmp(cmd[0], "cd") == 0){
                if (lineLength != 2) {
                    ErrorPrint();
                    continue;
                }
                if (chdir(cmd[1]) != 0) {
                    ErrorPrint();
                }
                continue;
            }
            //This is supposed to be my path command but does have an issue some where that I cannot find
            if (strcmp(cmd[0], "path") == 0){
                if (lineLength > 1){
                    SetPathCommand(cmd + 1, lineLength - 1);
                } else { 
                    path[0] = NULL;
                }
                continue;
            }

            if (path[0] == NULL){
                continue;
            }
            //Index of the & or the > for redirection and parallelism
            int redirectIndex = ExecutionTypeHandler(cmd, &execType);
            if (redirectIndex == 0){
                ErrorPrint();
                continue;
            }
            //This if/else checks the execution type and which function to run
            if (execType == 0 || execType == 1) {
                HandleRedirectionAndExecution(cmd, execType, redirectIndex, lineLength);

            } else if (execType == 2) {
                HandleParallel(cmd, execType, redirectIndex);
            }
        }           
    }
    //Frees the input space that was saved.
    free(line);
    return 0;
}