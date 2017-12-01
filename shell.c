#include <stdio.h>
#include <string.h>

int max(int a, int b) {
	return a > b ? a : b;
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

    close(pipefd[1]);
    execvp(commands[numCommands][0],commands[numCommands]);
}

int main() {
    char* a = "ls";
    char* b = "|";
    char* c = "strings";
    char* d = "|";
    char* e = "wc";
    char** argv = (char**)malloc(5*sizeof(char*));
    argv[0] = a;
    argv[1] = b;
    argv[2] = c;
    argv[3] = d;
    argv[4] = e;
    int argc = 5;

    int numPipes = countPipes(argv,argc);
    int maxNumParams = maxParams(argv,argc);
    char*** individCommands = (char***)malloc((numPipes+1)*sizeof(char**));
    for(int i = 0; i <= numPipes; ++i) {
        individCommands[i] = (char**)malloc((maxNumParams+1)*sizeof(char**));
    }

    parsePipes(argv, argc,individCommands,numPipes);
    execCommands(individCommands,numPipes);
}
