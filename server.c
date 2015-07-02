#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

void doprocessing (int sock);
void errorCheck(int n, char *message);
char *getCommand(char *message);
char *get_string1(char *string);
char *get_string2(char *string);

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
   char *command = (char *) malloc(32);
   char *message = (char *) malloc(32);
   char *backup = (char *) malloc(32);
   char *file_contents;
   long input_file_size;
   char file_size[20];

   bzero(buffer,256);

   n = read(sock,buffer,255);
   errorCheck(n, "receiving message");

   command = getCommand(buffer);

   if (strcmp(buffer, "LOGIN") != 0) {
     printf("Expecting LOGIN, but got %s\n", buffer);
     shutdown(sock, 2);
   }

   printf("Command from client: %s\n", command);
   n = write(sock, "NEEDAUTH aa1123", 15);

   bzero(buffer, 256);
   n = read(sock, buffer, 48);
   printf("Message from client: %s\n", buffer);
   strcpy(backup, buffer);

   printf("Backup Buffer: %s\n", backup);
   command = get_string1(buffer);
   printf("String1: %s\n", command);

   if (strcmp(command, "AUTH") != 0) {
     printf("Invalid message from client, expecting AUTH\n");
     exit(1);
   }

   message = get_string2(backup);
   printf("hashed password from client: %s\n", message);

   if (strcmp(message, "53c49269ae7f3a1cdf851cf1b4b13593aa1123") != 0){
     printf("Hased password mismatch, closing connection :( \n");
     n = write(sock, "LOGINERROR: Closing connection", 32);
     shutdown(sock, 2);
   }

   /* confirm client that authentication is successful */
   n = write(sock, "LOGINOK", 7);

   /* read the file name from client */
   bzero(buffer, 256);
   n = read(sock, buffer, 48);
   printf("Message from client: %s\n", buffer);
   strcpy(backup, buffer);

   printf("Backup Buffer: %s\n", backup);
   command = get_string1(buffer);
   printf("String1: %s\n", command);

   if (strcmp(command, "GETFILE") != 0) {
     printf("Invalid message from client, expecting GETFILE\n");
     exit(1);
   }

   message = get_string2(backup);
   printf("file name requested from client: %s\n", message);
   if( access( message, F_OK ) != -1 ) {
     // file exists
     printf("Requested file exists :), processing it...\n");
   } else {
     // file doesn't exist
     printf("Requested file doesn't exist :( \n");
     n = write(sock, "ERROR No such file", 18);
     shutdown(sock, 2);
   }

   /* read file contents into string */
   FILE *file_to_send = fopen(message, "rb");
   fseek(file_to_send, 0, SEEK_END);
   input_file_size = ftell(file_to_send);
   rewind(file_to_send);
   file_contents = malloc(input_file_size * (sizeof(char)));
   fread(file_contents, sizeof(char), input_file_size, file_to_send);
   fclose(file_to_send);

   printf("File content read into program: %s\n", file_contents);
   printf("Writing into socket to the client\n");

   /* write file to client through our socket */
   bzero(buffer, strlen(file_contents) + 10);
   strcat(buffer, "DATA ");
   sprintf(file_size, "%ld", input_file_size);
   strcat(buffer, file_size);
   strcat(buffer, file_contents);
   n = write(sock, buffer, input_file_size * (sizeof(char)));
   printf("Writing done to the client\n");

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

char *get_string1(char *string){
  char *substring;
  char *search = " ";
  substring = strtok(string, search);
  return substring;
}

char *get_string2(char *string){
  char *substring;
  char *search = " ";
  substring = strtok(string, search);
  substring = strtok(NULL, search);
  return substring;
}
