#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wordexp.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "errno.h"
//#include "io.h"
#include <fcntl.h>
#include <regex.h>
//export res_path=${PWD}
//extern void copy(int fdin, int fdout);

struct State{
    char *in_redirect_regex;
    char *out_redirect_regex;
    char *err_redirect_regex;
    char path[200];
    char *prompt;
    short max_line_length;
    int current_line;
    int current_line_length;
    char *command;
    int fatal_error;
}ShellState;
struct Command{
    int line;
    char *command;
    int  argc;
    char *argv[6];
    char stdin_file[8];
    char stdout_file[8];
    char *stdout_overwrite;
    char *stderr_file;
    char *stderr_overwrite;
    int exit_code;
}ShellCommand;

void handle_run_error(char *cmd,int errNo) {

    if (errNo == ENOENT){
        if (strcmp(cmd, "cd")!=0) {
            printf("command:%s  not found\n", cmd);
        }
        else {
            printf("<dir>:%s does not exist\n", ShellCommand.argv[1]);
        }
    }
    else if(errNo==ENOTDIR) printf("<dir>:%s is not a directory\n", ShellCommand.argv[1]);
    else printf("error\n");
    printf("%d\n",errNo);
//    return errNo;

}
void handle_error(){}
void destroy_state(){
//    free
    free(ShellState.command);
    free(ShellCommand.command);
    for(int i=0;i<ShellCommand.argc+1;i++){
        free(ShellCommand.argv[i]);
    }
//    free(ShellCommand.argv);
    printf("lzh@BCIT -Simple Unix Shell END\n");
    ShellCommand.exit_code=1;
}
void do_reset_state(){
    destroy_state();
}

void do_exit(){
//    do_reset_state();
    destroy_state();
}

int init_state(){
    ShellState.max_line_length= sysconf(_SC_ARG_MAX);
    ShellState.in_redirect_regex="[ \t\f\v]<.*";
    ShellState.out_redirect_regex="[ \t\f\v][1^2]?>[>]?.*";
    ShellState.err_redirect_regex="[ \t\f\v]2>[>]?.*";

//    ShellState.path=getenv("RESOURCE_DIR");
    getcwd(ShellState.path,200);
//    printf("path:%s\n",ShellState.path);
    ShellState.prompt= getenv("PS1");
//    printf("prompt:%s\n",ShellState.prompt);//ShellState.prompt);
    if(ShellState.prompt==NULL)
        ShellState.prompt="$";
    printf("[%s] ",ShellState.path);
    printf("%s ",ShellState.prompt);
    ShellState.current_line=0;
    ShellState.current_line_length=0;
    ShellState.command=NULL;
    ShellCommand.stdin_file[0]='\0';
    ShellCommand.stdout_file[0]='\0';
    ShellCommand.exit_code=0;
    if(ShellState.max_line_length<0||ShellState.path<0){
        ShellState.fatal_error=1;
        printf("error");
        return 0;
    }
    else{
        ShellState.fatal_error=0;
    }
    return 1;
}
int read_commands(){//char *cmd
    int buffer_size = 1024;
    int check=0;
    int pos = 0;
    ShellState.command= malloc(sizeof(char) * buffer_size);

    for (int i = 0; i<1000; i++) {
        check = getchar();
        if (check == '\n') {
            ShellState.command[pos]='\0';
            i=10000;
        }
        else
            ShellState.command[pos]=check;
        pos++;

    }
//    printf("输入的命令：%s\n",ShellState.command);
    if(ShellState.command[0]=='\0'){
        ShellState.fatal_error=1;
//        printf("no comdand!\n");
        printf("[%s] ",ShellState.path);
        printf("%s ",ShellState.prompt);
        return 0;
    }
    else {
        return 1;
    }
}
/*
int separate_commands(){//char *cmd
    ShellState.command=ShellState.command;//*cmd;
    ShellCommand.line=ShellState.current_line;
}
int parse_commands(){}
 */
int parse_command(){
    wordexp_t p;
    char **w;
    int i;
//    ShellState.command="cat '>' 3.txt";
//    printf("全命令：%s\n",ShellState.command);
//重定向
    regex_t reg;
//    char *reg_str=ShellState.in_redirect_regex;
//    char *test_str=ShellState.command;
//    in_redirect_regex
//    char *infile="in";
    regmatch_t pm[10];
    size_t nmatch = 10;
    ShellCommand.stdin_file[0]='\0';
    ShellCommand.stdout_file[0]='\0';
//in_redirect
    if(regcomp(&reg,ShellState.in_redirect_regex,REG_EXTENDED|REG_ICASE|REG_NOSUB))
    {
        do_exit();
    }
    if(regexec(&reg,ShellState.command,nmatch,pm,0)==0)
    {
        int lenCmd=strlen(ShellState.command);
//        char infilename[8];
        int k=0;
        int j=0;
//        printf("%d",lenCmd);
        while (k<lenCmd){
            if(ShellState.command[k]=='<'){
                k=k+2;
//                printf("%d\n",k);
                while ((ShellState.command[k]!=' ')&&(ShellState.command[k]!='\0')) {
//                    printf("%d\n",k);
                    ShellCommand.stdin_file[j] =ShellState.command[k];
//                    if(ShellState.command[k]==' ') break;
                    k++;
                    j++;
//                    printf("%c.%d",ShellCommand.stdin_file[j],k);
                }
                ShellCommand.stdin_file[j]='\0';
//                k=lenCmd;
            }
            k++;
        }
//    printf("ok\n");
//    printf("--%s \n",ShellCommand.stdin_file);

        regfree(&reg);
    }
    //out_redirect
    if(regcomp(&reg,ShellState.out_redirect_regex,REG_EXTENDED|REG_ICASE|REG_NOSUB))
    {
        do_exit();
    }
    if(regexec(&reg,ShellState.command,nmatch,pm,0)==0)
    {
        int lenCmd=strlen(ShellState.command);
//        char infilename[8];
        int k=0;
        int j=0;
//        printf("%d",lenCmd);
        while (k<lenCmd){
            if(ShellState.command[k]=='<'){
                k=k+2;
//                printf("%d\n",k);
                while ((ShellState.command[k]!=' ')&&(ShellState.command[k]!='\0')) {
//                    printf("%d\n",k);
                    ShellCommand.stdout_file[j] =ShellState.command[k];
//                    if(ShellState.command[k]==' ') break;
                    k++;
                    j++;
//                    printf("%c.%d",ShellCommand.stdin_file[j],k);
                }
                ShellCommand.stdout_file[j]='\0';
//                k=lenCmd;
            }
            k++;
        }
//    printf("ok\n");
//    printf("--%s \n",ShellCommand.stdin_file);

        regfree(&reg);
    }
    if((ShellCommand.stdin_file[0]=='\0')&&(ShellCommand.stdout_file[0]=='\0')){
        wordexp(ShellState.command, &p, 0);
        ShellCommand.argc = p.we_wordc;
        w = p.we_wordv;
//    printf("解析后的命令：%s\n",w[0]);
        ShellCommand.command = (char *) malloc(sizeof(char[strlen(p.we_wordv[0] + 1) + 1]));
//    ShellCommand.argv=malloc(sizeof(char) * 1024);
        strcpy(ShellCommand.command, p.we_wordv[0]);
//    printf("结构体保留的解析后的命令:%s\n",ShellCommand.command);

//    ShellCommand.argv= malloc(sizeof(char )*8*(ShellCommand.argc+1));
        for (i = 0; i < p.we_wordc; i++) {
            ShellCommand.argv[i] = malloc(sizeof(char) * 8);
            strcpy(ShellCommand.argv[i], p.we_wordv[i]);
//        ShellCommand.argv[i]=p.we_wordv[i];
//        printf("%s\n", ShellCommand.argv[i]);

        }
        ShellCommand.argv[p.we_wordc] = malloc(sizeof(char) * 8);
        ShellCommand.argv[p.we_wordc] = NULL;
        wordfree(&p);
    }
    else{
        ShellCommand.command="cat";
        ShellCommand.argv[0]="cat";
        strcpy(ShellCommand.argv[1],ShellCommand.stdin_file);
        ShellCommand.argv[2]=NULL;
    }
}
void builtin_cd(){
    char *path;
    int backNo;
//    printf("%s\n",ShellCommand.argv[1]);
    if (ShellCommand.argv[1] == NULL) {
        path=getenv("HOME");
    }
    else {
        path=ShellCommand.argv[1];
//        printf("%c  %c %s\n",ShellCommand.argv[1][0],ShellCommand.argv[1][1],path);
    }
    backNo=chdir(path);
//    printf("b:%d\n",backNo);
    if(backNo!=0)
        handle_run_error(ShellCommand.command,errno);
    else
        printf("%d\n",ShellState.fatal_error);
//    getcwd(ShellState.path,200);
//    printf("[%s] ",ShellState.path);
//    printf("%s",ShellState.prompt);
//    printf("path:%s\n",path);
}

void redirect(){
    if(ShellCommand.stdin_file[0]!='\0'){
        int fd;
        fd = open(ShellCommand.stdin_file,O_RDONLY,0);
        if (fd<0)
        {perror("Could not open input file"); exit(0);}

        dup2(fd,0);
        close(fd);
    }
    if(ShellCommand.stdout_file[0]!='\0'){
        int fd;
        fd = open(ShellCommand.stdout_file,O_WRONLY,0);
        if (fd<0)
        {perror("Could not open input file"); exit(0);}

        dup2(fd,0);
        close(fd);
    }
}

static void parent_process(pid_t child_pid)
{
    pid_t wpid;
    int status;
/*
#define READ 0
#define WRITE 1
    int p[2];
    close(p[READ]);//close child pipe write

    dup2(p[WRITE],1);
    close(p[WRITE]);//close child pipe read
    char *argv[] = {"cat", ">", "3.txt", NULL};
//    execvp(argv[0], argv);
*/
    do
    {
        wpid = waitpid(child_pid, &status, WUNTRACED
                                           #ifdef WCONTINUED
                                           | WCONTINUED
#endif
        );

        if(wpid == -1)
        {
            perror("waitpid");
            exit(EXIT_FAILURE);
        }

        if(WIFEXITED(status))
        {
//            printf("child exited, status=%d\n", WEXITSTATUS(status));
        }
        else if(WIFSIGNALED(status))
        {
//            printf("child killed (signal %d)\n", WTERMSIG(status));
        }
        else if(WIFSTOPPED(status))
        {
//            printf("child stopped (signal %d)\n", WSTOPSIG(status));
#ifdef WIFCONTINUED
        }
        else if(WIFCONTINUED(status))
        {
//            printf("child continued\n");
#endif
        }
        else
        {
//            printf("Unexpected status (0x%x)\n", status);
        }
    }
    while (!WIFEXITED(status) && !WIFSIGNALED(status));
}

void run(void)
{
//    #define READ 0
//    #define WRITE 1
//    int p[2];
/*    if(ShellCommand.stdin_file[0]!='\0') {
        int fd_in = STDIN_FILENO;
        int fd_out = STDOUT_FILENO;

//    char *argv1[] = {"cat", ">", "3.txt", NULL};
        fd_in = open(ShellCommand.stdin_file, O_RDONLY);
        dup2(fd_in, STDIN_FILENO);
        close(fd_in);
        ShellState.fatal_error = 0;
//    char *argv2[]={"copy","STDIN_FILENO","STDOUT_FILENO"};
//    execvp("copy",argv1);

//    close(p[WRITE]);//close child pipe write
//    dup2(p[READ],0);
//    close(p[READ]);//close child pipe read
//    char *argv[] = {"cat", ">", "3.txt", NULL};
//    execvp(argv[0], argv);


//    copy(STDIN_FILENO, STDOUT_FILENO);
//    execvp("cat",argv1);
//    printf("%s\n",ShellCommand.argv[0]);
    }*/
//    else
    {
        int backNo;
        backNo = execvp(ShellCommand.command, ShellCommand.argv);//"/Users/mac"  argv);
        if (backNo != 0) {
//        printf("err:%d\n",errno);
            handle_run_error(ShellCommand.command, errno);
        } else
            printf("%d\n", ShellState.fatal_error);
//    perror("execv");
    }
}

void execute(){
    pid_t pid;
    pid = fork();
    if (pid == -1)
    {
        perror("fork");
        exit(1);
    }
    if (pid == 0)
    {
        redirect();
        run();
    }
    else
    {
        parent_process(pid);
    }

//    return EXIT_SUCCESS;
}
void reset_state(){
    ShellState.fatal_error=0;
    free(ShellState.command);
    free(ShellCommand.command);
    for(int i=0;i<ShellCommand.argc+1;i++){
        free(ShellCommand.argv[i]);
    }
}






int execute_commands(){
//    printf("excu.:%s\n",ShellCommand.command);
    if(strcmp(ShellCommand.command,"cd")==0){
//        printf("run cd\n");
        builtin_cd();
        getcwd(ShellState.path,200);
        printf("[%s] ",ShellState.path);
        printf("%s ",ShellState.prompt);
    }
    else if(strcmp(ShellCommand.command,"exit")==0){
//        printf("exit!\n");
        do_exit();
//        return EXIT_SUCCESS;
    }
//    else if(strcmp(ShellCommand.command,"")){
//        printf("[%s] ",ShellState.path);
//        printf("%s ",ShellState.prompt);
//    }
    else{
        execute();
        getcwd(ShellState.path,200);
        printf("[%s] ",ShellState.path);
        printf("%s ",ShellState.prompt);
    }

}



//run(){}


int main() {
    init_state();
    while(ShellCommand.exit_code==0) {
        if(read_commands()==0)reset_state();
        else {
            parse_command();
//            printf("要执行的命令:%s   argv[0]:%s\n", ShellCommand.command, ShellCommand.argv[0]);
            execute_commands();
        }
    }
//    printf("Hello, World!\n");
    return 0;
}
