#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>

void doprocessing (int sock);
void errorCheck(int n, char *message);
char *getCommand(char *message);

int main( int argc, char *argv[] )
{
   int sockfd, newsockfd, portno, clilen;
   char buffer[256];
   struct sockaddr_in serv_addr, cli_addr;
   int  n, pid;
   
   /* First call to socket() function */
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   
   if (sockfd < 0)
      {
      perror("ERROR opening socket");
      exit(1);
      }
   
   /* Initialize socket structure */
   bzero((char *) &serv_addr, sizeof(serv_addr));
   portno = 5001;
   
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = INADDR_ANY;
   serv_addr.sin_port = htons(portno);
   
   /* Now bind the host address using bind() call.*/
   if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
      {
      perror("ERROR on binding");
      exit(1);
      }
   
   /* Now start listening for the clients, here
   * process will go in sleep mode and will wait
   * for the incoming connection
   */
   
   listen(sockfd,5);
   clilen = sizeof(cli_addr);
   
   while (1)
   {
      newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
      if (newsockfd < 0)
         {
         perror("ERROR on accept");
         exit(1);
         }
      
      /* Create child process */
      pid = fork();
      if (pid < 0)
         {
         perror("ERROR on fork");
         exit(1);
         }
      
      if (pid == 0)
         {
         /* This is the client process */
         close(sockfd);
         doprocessing(newsockfd);
         exit(0);
         }
      else
         {
         close(newsockfd);
         }
   } /* end of while */
}

void doprocessing (int sock)
{
   int n;
   char buffer[256];
   char *command;
   char *message;
   
   bzero(buffer,256);
   
   n = read(sock,buffer,255);
   errorCheck(n, "receiving message");

   command = getCommand(buffer);

   if (strcmp(buffer, "LOGIN") == 0) {
     printf("Command from client: %s\n", command);
     n = write(sock, "NEEDAUTH aa1123", 15);
     
   }
   else if(strcmp(buffer, "AUTH") == 0){
     
   }
   else{
     printf("invalid message from client");
   }
   
//   n = write(sock,"I got your message",18);
}

char *getCommand(char *message){
  char *command;
  char *search = " ";
  command = strtok(message, search);
  printf("first substring: %s\n", command);
  return command;
}

void errorCheck(int n, char *message){
   if (n < 0)
   {
      printf("ERROR in %s \n", message);
      exit(1);
   }
   else
   {
      printf("%s message transfer successful\n", message);
   }
}
