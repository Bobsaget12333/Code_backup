/* 
 Implementation & bugs:
	1.build-in command : cd, ln, rm. exit. ctrl+D to exit. non built-in command ls echo cat etc.
	2.parse input fine. using command like in real shell.
	3.redirection test case failed:  > gub; cat>bar<README;
	4.This program is compiled using your Makefile file.
	5.must have bugs in using command, but havenot found. time tight` sorry.
	6.Thank you for helping
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#define BUFFERSIZE 1024
#define _GUN_SOURCE

const char *prompt = "cs167Shell $ ";
const char *invalid = "invalid command\n";
const char *incompletecmd = "command needs more argument see man page\n";
const char *formaterror = "format error! check syntax\n";
const char *redirectionerror = "ERROR - cannot have 2 same redirects on one line\n";
const char *commandnotfound = "command not found\n";
const char *openerror = "ERROR - Open failed\n";
const char *redirecterror = "Redirect Error\n";
const char *usage = "usage: ./sh";

char buffer[BUFFERSIZE];
int status = 0;
pid_t pid;

void parseCommand(char *cmd);
int redirect(char* comd, int len);


int main(int argc, char **argv)
{
    char *user;	//login name
    char command[BUFFERSIZE];	//user input
    int fp;	//file descriptor
    if(argc!=1 || argv[0]==NULL)
    {
	write(1, usage, strlen("usage\n"));
	exit(0);
    }
    while (1)
    {
        memset(command, 0, BUFFERSIZE);
        user = getlogin();
        if (write(1, user, strlen(user)) <= 0 || write(1, prompt, strlen(prompt)) <= 0)	//write prompt
            perror(strerror(errno));
        if ((fp = read(0, command, BUFFERSIZE)) < 0)	//get command from keyboard
            perror(strerror(errno));
        else if (fp == 0)	// "ctrl + D" to exit
            exit (1);
        else if (fp == 1)	// process when only type "\n";
            write(1, invalid, strlen(invalid));
        else
            parseCommand(command);
    }
}

void parseCommand(char *cmd)
{
    char *argv[20];	//parse command and store to each argv
    int i = 0, j = 0, k = 0;	//parameters
    int length;
    char path[BUFFERSIZE];	//test path
    pid_t pid;
    length = (int)(strlen(cmd) - 1);
    memset(argv, 0, (sizeof(char*)) * 20);

    for (i = 0; i <= length; i++)
    {
        if (cmd[i] == '>' || cmd[i] == '<')
        {
            redirect(cmd, length);
            return;
        }

        if (cmd[i] == ' ' || cmd[i] == '\t' || cmd[i] == '\n')	//process when met space, TAB, '\n'
        {
            if (j == 0)	//filter extra space or TAB
                continue;
            else
            {
                buffer[j++] = '\0';	//add '\0' to end
                argv[k] = (char *)malloc(sizeof(char) * (unsigned int)j);	// malloc for first command
                strncpy(argv[k], buffer, (size_t)j);	// copy first command
                j = 0;	//reset
                k++;	//command size ++
            }
        }
        else
            buffer[j++] = cmd[i];
    }
    argv[k] = (char *)malloc(sizeof(char));	//to execute 'execve', set last NULL string
    argv[k++] = NULL;
    //printf("%s %s %s %s\n", argv[0], argv[1], argv[2], argv[3]);	//testing
    //printf("%d %d %d\n",strlen(argv[0]), k, strlen(argv[1]));

    if (strcmp(argv[0], "exit") == 0)	// exit when type in "exit"
        exit (0);

    /*********************built-in command***************************************/
    if (strcmp(argv[0], "cd") == 0)	// process built-in "cd"
    {
        if (argv[1] != NULL)
        {
            if (chdir(argv[1]) != 0)
                perror(strerror(errno));
            getcwd(path, sizeof(path));
            write(1, path, strlen(path));	//test
        }
        else
            write(1, incompletecmd, strlen(incompletecmd));
    }
    else if (strcmp(argv[0], "ln") == 0)	//process built-in "ln"
    {
        if (argv[1] != NULL && argv[2] != NULL)
        {
            if (link(argv[1], argv[2]) != 0)
                perror(strerror(errno));
            write(1, path, strlen(path));	//test
        }
        else
            write(1, incompletecmd, strlen(incompletecmd));
    }
    else if (strcmp(argv[0], "rm") == 0)	//process built-in "rm"
    {
        if (argv[1] != NULL)
        {
            if (unlink(argv[1]) != 0)
                perror(strerror(errno));
            write(1, path, strlen(path));	//test
        }
        else
            write(1, incompletecmd, strlen(incompletecmd));
    }

    /*********************non built-in command********************************/
    else
    {
        if ((pid = fork()) == 0)	//child process
        {
            char *envp[] = { NULL };
            char p[30] = "/bin/";
            strcat(p, argv[0]);
            execve(p, argv, envp);
            if (errno == ENOENT)
            {
		write(1, commandnotfound, strlen(commandnotfound));
                exit(1);
            }
            else
            {
                fprintf(stderr, "sh : execution of %s failed: %s\n", p, strerror(errno));
                exit(1);
            }
        }
        else
            waitpid(pid, &status, 0);
    }
    for (i = 0; i < k; i++)	//free malloc
        free(argv[i]);
}

int redirect(char* comd, int len)
{
    char *argv[20];
    char *filename[5];	//store the filename 
    int fp_in, fp_out;	//file descripter for in and out
    int flag_in = -1, flag_out = -1;	
    int i, l, j = 0, k = 0;
    int count = 0;
    int append = O_TRUNC;
    memset(argv, 0, (sizeof(char*)) * 20);
/****************parse the command********************************/
    for (i = 0; i <= len; i++)
    {
        if (comd[i] == ' ' || comd[i] == '	' || comd[i] == '\n' || comd[i] == '>' || comd[i] == '<')
        {
            if (comd[i] == '>' || comd[i] == '<')
            {
                count++; //count 
                if (comd[i] == '<')
                {
                    flag_in = count - 1;
                    for (l = i + 1; l < len; l++)
                    {
                        if (comd[l] == '<') //cannot have two input redirects
			{
                            write(1, redirectionerror, strlen(redirectionerror));
			    return -1;
			}
                    }
                }
                else
                {
                    flag_out = count - 1;
                    for (l = i + 2; l < len; l++)
                    {
                        if (comd[i + 1] == '>')// check '>>' 
			    append = O_APPEND;
			else
			    continue;
                    }
                }
                if (j > 0 && count == 1)
                {
                    buffer[j++] = '\0';
                    argv[k] = (char *)malloc(sizeof(char) * (unsigned int)j);
                    strcpy(argv[k], buffer);
                    j = 0;
                    k++;
                }
                if (count >= 3)
                {
                    write(1, formaterror, strlen(formaterror));
                    return -1;
                }
            }
            if (j == 0)
                continue;
            else
            {
                buffer[j++] = '\0';
                if (count == 0)
                {
                    argv[k] = (char *)malloc(sizeof(char) * (unsigned int)j);
                    strncpy(argv[k], buffer, (size_t)j);
                    k++;
                    j = 0;
                }
                else
                {
                    filename[status] = (char *)malloc(sizeof(char) * (unsigned int)j);
                    strcpy(filename[status++], buffer);
                    j = 0;
                }
            }
        }
        else
            buffer[j++] = comd[i];
    }
    argv[k] = (char *)malloc(sizeof(char));
    argv[k ++] = (char *)0;
    //printf("%s %s %s\n", argv[0], argv[1], filename[0]);	//testing
    //printf("%d %d %d\n",strlen(argv[0]), k, strlen(argv[1]));
    if ((pid = fork()) == 0)
    {
        char *envp[] = { NULL };
        char p[30] = "/bin/";
        strcat(p, argv[0]);

        if (flag_out != -1)
        {
            if ((fp_out = open(filename[flag_out], O_WRONLY|O_CREAT|append, S_IRUSR|S_IWUSR )) == -1)
            {
                write(1, openerror, strlen(openerror));
                return -1;
            }            
            if (dup2(fp_out, STDOUT_FILENO) == -1)
            {
                write(1, redirecterror, strlen(redirecterror));
                exit(0);
            }
        }
        if (flag_in != -1)
        {
            if ((fp_in = open(filename[flag_in], O_RDONLY, S_IRUSR|S_IWUSR)) == -1)
            {
                write(1, openerror, strlen(openerror));
                return -1;
            }            
            if (dup2(fp_in, STDIN_FILENO) == -1)
            {
                write(1, redirecterror, strlen(redirecterror));
                exit(0);
            }
        }
        execve(p, argv, envp);
    }
    else
        waitpid(pid, &status, 0);

    for (i = 0; i <= k; i++)
        free(argv[i]);
    if (flag_in != -1)
    {
        free(filename[flag_in]);
        close(fp_in);
    }
    if (flag_out != -1)
    {
        free(filename[flag_out]);
        close(fp_out);
    }
    return 0;
}