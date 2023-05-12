// The MIT License (MIT)
// 
// Copyright (c) 2016 Trevor Bakker 
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 5     // Mav shell only supports four arguments

//int childPid = 0; //global variable to store the child pid

//it will move the history array contents
void moveHistoryContents(char newCommand[], char history[][MAX_COMMAND_SIZE])
{
  for(int i = 0; i <=13; i++)
  {
    strcpy(history[i],history[i+1]); //will copy the i+1 index
  }
  strcpy(history[14],newCommand);
}

//this will move the pid array contents
void movePidContents(int pid[],int parentPid)
{
  for(int i = 0; i <=13; i++)
  {
   pid[i] = pid[i+1]; //copying the contents of the next index's pid
  }
  pid[14] = parentPid;
}
int main()
{

  char * command_string = (char*) malloc( MAX_COMMAND_SIZE ); 
  char history[15][MAX_COMMAND_SIZE] = {}; //will be used to track command history
  int total_index = 0; //will be used to keep track of the index being used
  int pids[15] = {};

  while( 1 )
  {
    // Print out the msh prompt
    printf ("msh> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (command_string, MAX_COMMAND_SIZE, stdin) );

    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    for( int i = 0; i < MAX_NUM_ARGUMENTS; i++ )
    {
      token[i] = NULL;
    }

    int token_count = 0;                                 
                                                           
    // Pointer to point to the token
    // parsed by strsep
    char *argument_ptr = NULL;                                         

    if(command_string[0] == '!')
    {
      // int num = atoi(token[0]);
      if(total_index < 15)
      {
        strtok(command_string,"\n"); //werid way to remove \n and replacing with \0
        pids[total_index] = -1;
        strcpy(history[total_index],command_string); //history for the change directory
        total_index++;
      }
      else
      {
        strtok(command_string,"\n"); //werid way to remove \n and replacing with \0
        moveHistoryContents(command_string,history); //will move contents one index up
        movePidContents(pids,-1);
      }
      int number = atoi ( &command_string[1]);
      if(number < 0 || number >= total_index)
      {
        printf("The given number is out of bounds.\n");
      }
      else
      {
        strcpy(command_string, history[number]);
      }
    }

    char *working_string  = strdup( command_string );                

    // we are going to move the working_string pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *head_ptr = working_string;

    // Tokenize the input strings with whitespace used as the delimiter
    while ( ( (argument_ptr = strsep(&working_string, WHITESPACE ) ) != NULL) && 
              (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup( argument_ptr, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
        token_count++;
    }

    // Now print the tokenized input as a debug check
    // \TODO Remove this for loop and replace with your shell functionality
    /*
    int token_index  = 0;
    for( token_index = 0; token_index < token_count; token_index ++ ) 
    {
      printf("token[%d] = %s\n", token_index, token[token_index] );  
    }
    */
    if(token[0] == NULL)
    {
      continue;
    }
    strtok(command_string,"\n"); //werid way to remove \n and replacing with \0

  
    //checking the built-in functions
    
    
    if(!strcmp("quit",token[0]) || !strcmp("exit",token[0]))
    {
      exit(0);
    }
    else if(!strcmp("cd",token[0]))
    {
      if(total_index < 15)
      {
        pids[total_index] = -1;
        strcpy(history[total_index],command_string); //history for the change directory
        total_index++;
      }
      else
      {
        moveHistoryContents(command_string,history); //will move contents one index up
        movePidContents(pids,-1);
      }
      if(token[1] != NULL)
      {
        int checkdir = chdir(token[1]); //working
        if(checkdir == -1)
        {
          printf("%s: path not found\n",token[1]);
        }
      }
    }
    else if(!strcmp(token[0],"history")) //comparing with history
    {
      if(total_index < 15)
      {
        pids[total_index] = -1;
        strcpy(history[total_index],command_string); //for the history and history -p command
        total_index++;
      }
      else
      {
        moveHistoryContents(command_string,history);
        movePidContents(pids,-1);
      }
      if(token[1] == NULL)
      {
        if(total_index != 0)
        {
          for(int i = 0; i < total_index; i++)
          {
            printf("%d: %s\n",i,history[i]); //for the history command
          }
        }
      }
      else if(strcmp(token[1],"-p") == 0)
      {
        if(total_index != 0)
        {
          for(int i = 0; i < total_index; i++)
          {
            printf("%d: [%d] %s\n",i,pids[i],history[i]); //for the history -p command
          }
        }
      }
    } 
    else //NON BUILT IN COMMANDS
    {
      pid_t pid = fork();

      //save pid into your array
      if(total_index < 15) //inserting into history
      {
        pids[total_index] = pid;
        strcpy(history[total_index],command_string); //for the non built in commands
        total_index++;
      }
      else
      {
        moveHistoryContents(command_string,history); //will move the previous commands
        movePidContents(pids,pid);
      }
      if(pid == 0)
      {
        int check_execvp = execvp(token[0],&token[0]); //child is becoming the command 
        if(check_execvp == -1) //exec fails
        {
          printf("%s: Command not found.\n",command_string);
          return 0;
        }
      }
      else
      {
        int status;
        wait(&status); //making the parent wait
      }
    }
    // Cleanup allocated memory
    for( int i = 0; i < MAX_NUM_ARGUMENTS; i++ )
    {
      if( token[i] != NULL )
      {
        free( token[i] );
      }
    }

    free( head_ptr );

  }

  free( command_string );

  return 0;
  // e2520ca2-76f3-90d6-0242ac120003
}
