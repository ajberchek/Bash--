#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>


int max(int a, int b) {
	return a > b ? a : b;
}

void parseIn(char *datIn, char **datOut, char* argCount) {
    int datOutCount = 0, i = 0;
    datOut[datOutCount++] = datIn;
    for(; datIn[i] != '\n'; ++i) {
        if(datIn[i] == ' ' && datIn[i+1] != 0) {
            datOut[datOutCount++] = &datIn[i+1];
            datIn[i] = 0;
        }
    }
    datIn[i] = 0;
    datOut[datOutCount] = 0;
    *argCount = datOutCount;
}

void parsePipes(char** params, int numParams, char*** individCommands, int numIndivid) {
    int place = 0;
    for(int i = 0; i < numParams; ++i) {
        if(params[i][0] == '|') {
            individCommands[numIndivid][place] = 0;
            --numIndivid;
            place = 0;
        } else {
            individCommands[numIndivid][place++] = params[i];
        }
    }
}

int countPipes(char** params, int numParams) {
    int count = 0;
    for(int i = 0; i < numParams; ++i) {
        if(params[i][0] == '|') {
            ++count;
        }
    }
    return count;
}

int maxParams(char** params, int numParams) {
    int count = 0;
    int paramCount = 0;
    for(int i = 0; i < numParams; ++i) {
        if(params[i][0] == '|') {
            count = max(count,paramCount);
            paramCount = 0;
        } else {
            ++paramCount;
        }
    }
    count = max(count,paramCount);
    return count;
}

void execCommands(char*** commands, int numCommands) {
    int pipefd[2];

    for(int i = 0; i < numCommands; ++i) {
        pipe(pipefd);
        dup2(pipefd[0],0);          //Map read end of pipe to stdin
        close(pipefd[0]);

        if(fork() == 0) {
            //Child
            dup2(pipefd[1],1);      //Map stdout to write end of pipe
        } else {
            //Parent
            close(pipefd[1]);
            execvp(commands[i][0],commands[i]);
        }
    }

    if(numCommands != 0) {
        close(pipefd[1]);
    }
    execvp(commands[numCommands][0],commands[numCommands]);
}

int main()
{
    int status;
    char datIn[100];
    char* args[100];

    while(1)
    {
        if(read(0,datIn,100) > 0)
        {
            int argCount;
            parseIn(datIn,args,&argCount);

            int pid = fork();
            if(pid == 0)
            {
                int numPipes = countPipes(args,argCount);
                int maxNumParams = maxParams(args,argCount);
                char*** individCommands = (char***)malloc((numPipes+1)*sizeof(char**));
                for(int i = 0; i <= numPipes; ++i) {
                    individCommands[i] = (char**)malloc((maxNumParams+1)*sizeof(char**));
                }

                parsePipes(args, argCount,individCommands,numPipes);
                execCommands(individCommands,numPipes);

            }
            else if(pid > 0)
            {
                if((pid = waitpid(pid,&status,0)) < 0)
                {
                    write(2,"waitpid error",13);
                }
                write(1,">",1);
            }
            else
            {
                printf("ERROR!");
            }
        }
    }
}
