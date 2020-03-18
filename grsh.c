//Zachary Hansen
//Assignment 2
//March 19th,2019
//Prof.Ngo


#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

char error_message[30]= "An error has occured \n";
char binpath[999] = "/bin/ls";

//Declarations for builtin commands
int grsh_cd(char **args);
int grsh_help(char **args);
int grsh_exit(char **args);
int grsh_path(char **args);


//Listing of individual built-in commands, with their functions. 
char *builtin_str[] = {
  "cd",
  "exit",
  "path"
};

int (*builtin_func[]) (char **) = {
  &grsh_cd,
  &grsh_exit,
  &grsh_path

};

int grsh_redirect (){
int file = open("output.txt", O_APPEND | O_WRONLY);
    if(file < 0)    return 1;
    if(dup2(file,1) < 0)    {
        close(file);
        return 1; 
    }
     
    close(file);
 
    return 0;
} 



int grsh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}



//Builtin path command
//needs to append binpath
int grsh_path(char **args){
  if(args[1]==NULL){
    write(STDERR_FILENO, error_message, strlen(error_message)); 
  }else{
    for(int x = 1; args[x]!= NULL;x++){
    strcat(binpath, args[x]);
    }
  }
  return 1;
}


//Builtin cd command
int grsh_cd(char **args)
{
  if (args[1] == NULL) {
    write(STDERR_FILENO, error_message, strlen(error_message));
  } else {
    if (chdir(args[1]) != 0) {
    write(STDERR_FILENO, error_message, strlen(error_message));
    }
  }
  return 1;
}




//Builtin exit command
int grsh_exit(char **args)
{
  return 0;
}



//grsh launcher
int grsh_launch(char **args)
{
  pid_t pid;
  int status;
  pid = fork();
  if (pid == 0) {
    // Child process
    if (execv(binpath,args) == -1) {
      write(STDERR_FILENO, error_message, strlen(error_message));
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
      write(STDERR_FILENO, error_message, strlen(error_message));
  } else {
    do {
      waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}





// GRSH shell commands executer
int grsh_execute(char **args)
{
  int i;

  if (args[0] == NULL) {
    write(STDERR_FILENO, error_message, strlen(error_message));
    return 1;
  }

  for (i = 0; i < grsh_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return grsh_launch(args);
}



//reads line from standard input
#define GRSH_RL_BUFSIZE 1024
char *grsh_read_line(void)
{
  int bufsize = GRSH_RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
    write(STDERR_FILENO, error_message, strlen(error_message));
    exit(EXIT_FAILURE);
  }

  while (1) {
    c = getchar();
    if (c == EOF) {
      exit(EXIT_SUCCESS);
    } else if (c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;

    // Reallocate.
    if (position >= bufsize) {
      bufsize += GRSH_RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(EXIT_FAILURE);
      }
    }
  }
}



//splits up input into tokens 
#define GRSH_TOK_BUFSIZE 64
#define GRSH_TOK_DELIM " \t\r\n\a"
char **grsh_split_line(char *line)
{
  int bufsize = GRSH_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token, **tokens_backup;

  if (!tokens) {
    write(STDERR_FILENO, error_message, strlen(error_message));
    exit(EXIT_FAILURE);
  }

  token = strtok(line, GRSH_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    if(strstr(token, ">")!= NULL){
    grsh_redirect();
    }
    position++;

    if (position >= bufsize) {
      bufsize += GRSH_TOK_BUFSIZE;
      tokens_backup = tokens;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
		free(tokens_backup);
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, GRSH_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}




//loops through user input looking for commands and args 
void grsh_loop(void)
{
  char *line;
  char **args;
  int status;
  do {
    printf("grsh> ");
    line = grsh_read_line();
   
     args = grsh_split_line(line);
    status = grsh_execute(args);
    
    free(line);
    free(args);
  } while (status);
}


//main class
int main(int argc, char **argv)
{
  grsh_loop();
  return EXIT_SUCCESS;
}


