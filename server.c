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
char *find_password(char *username);
char *get_hash(char *string);

int main( int argc, char *argv[] )
{
   int sockfd, newsockfd, portno, clilen;
   char buffer[256];
   struct sockaddr_in serv_addr, cli_addr;
   int  n, pid;

   /* First call to socket() function */
   printf("--> Creating socket file descriptor \n");
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
   printf("--> Binding the host address \n");
   if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
      {
      perror("ERROR on binding");
      exit(1);
      }

   /* Now start listening for the clients, here
   * process will go in sleep mode and will wait
   * for the incoming connection
   */

   printf("--> Socker server starting listening to localhost with port %d\n", portno);
   listen(sockfd,5);
   clilen = sizeof(cli_addr);

   while (1)
   {
      printf("--> Waiting for connection from client\n");
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
   char *password = (char *) malloc(48);
   char *hash = (char *) malloc(256);

   bzero(buffer,256);

   n = read(sock,buffer,255);
   errorCheck(n, "Initial message from client\n");
   printf("--> Message received from client: %s\n", buffer);

   strcpy(backup, buffer);
   command = getCommand(buffer);

   if (strcmp(buffer, "LOGIN") != 0) {
     printf("ERROR: Expecting LOGIN, but got %s\n", buffer);
     shutdown(sock, 2);
   }

   message = get_string2(backup);
/*   
   printf("DEBUG: message: %s\n", message);
   password = find_password(message);
   printf("DEBUG: password: %s\n", password);

   hash = get_hash(password);
   printf("DEBUG: hash: %s\n", hash);

   hash = get_string1(hash);
   printf("DEBUG: hash: %s\n", hash);
   strcat(hash, password);

   bzero(buffer, 256);
   strcpy(buffer, "NEEDAUTH ");
   strcat(buffer, password);
*/
   n = write(sock, "NEEDAUTH aa1123", 15);
   errorCheck(n, "Writing NEEDAUTH to client\n");
   printf("--> Message sent to client: %s\n", buffer);

   bzero(buffer, 256);
   n = read(sock, buffer, 48);
   errorCheck(n, "Reading hashed password to client\n");
   printf("--> Message received from client: %s\n", buffer);

   strcpy(backup, buffer);
   command = get_string1(buffer);

   if (strcmp(command, "AUTH") != 0) {
     printf("ERROR: Invalid message from client, expecting AUTH\n");
     exit(1);
   }

   message = get_string2(backup);

   if (strcmp(message, hash) != 0){
     printf("ERROR: Hased password mismatch, closing connection :( \n");
     n = write(sock, "LOGINERROR: Closing connection", 32);
     shutdown(sock, 2);
   }

   /* confirm client that authentication is successful */
   n = write(sock, "LOGINOK", 7);
   errorCheck(n, "Writing LOGINOK to client\n");
   printf("--> Message sent to client: LOGINOK\n");

   /* read the file name from client */
   bzero(buffer, 256);
   n = read(sock, buffer, 48);
   errorCheck(n, "Reading file name from client\n");
   printf("--> Message received from client: %s\n", buffer);

   strcpy(backup, buffer);
   command = get_string1(buffer);

   if (strcmp(command, "GETFILE") != 0) {
     printf("ERROR: Invalid message from client, expecting GETFILE\n");
     exit(1);
   }

   message = get_string2(backup);
   printf("--> File name requested from client: %s\n", message);
   if( access( message, F_OK ) != -1 ) {
     // file exists
     printf("--> Requested file exists :), processing it...\n");
   } else {
     // file doesn't exist
     printf("ERROR: Requested file doesn't exist :( \n");
     n = write(sock, "ERROR No such file", 18);
     shutdown(sock, 2);
     exit(1);
   }

   /* read file contents into string */
   FILE *file_to_send = fopen(message, "rb");
   fseek(file_to_send, 0, SEEK_END);
   input_file_size = ftell(file_to_send);
   rewind(file_to_send);
   file_contents = malloc(input_file_size * (sizeof(char)));
   fread(file_contents, sizeof(char), input_file_size, file_to_send);
   fclose(file_to_send);

   printf("--> File content read into program: %s\n", file_contents);
   printf("--> Writing into socket to the client\n");

   /* write file to client through our socket */
   bzero(buffer, input_file_size * sizeof(char) + 10);
   strcat(buffer, "DATA ");
   sprintf(file_size, "%ld", input_file_size);
   strcat(buffer, file_size);
   strcat(buffer, " ");
   strcat(buffer, file_contents);
   input_file_size = input_file_size + 10;
   n = write(sock, buffer, input_file_size * (sizeof(char)));
   printf("--> Writing done to the client\n");
   printf("<--------------------------------------------------->\n");

   close(sock);
}

char *getCommand(char *message){
  char *command;
  char *search = " ";
  command = strtok(message, search);
  return command;
}

void errorCheck(int n, char *message){
   if (n < 0)
   {
      printf("ERROR in %s \n", message);
      exit(1);
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

char *find_password(char *username)
{
   char command[50];
   FILE *fp;
   char *s = (char *) malloc(20);
   char *output = (char *) malloc(20);

   strcpy(command, "awk '/");
   strcat(command, username);
   strcat(command, "/ {printf $2}' password.db");
   printf("--> Command is : %s\n", command);

   fp = popen(command, "r");
   while(fgets(s, sizeof(s), fp) != 0)
   {
     strcat(output, s);
   }
   pclose(fp);
   printf("--> User password: %s\n", output);
   return output;
}

char *get_hash(char *string){
  char command[256];
  char s[256];
  FILE *fp;

  strcpy(command, "echo -n ");
  strcat(command, string);
  strcat(command, " | md5sum");
  fp = popen(command, "r");
  printf("--> Command is : %s\n", command);
  while(fgets(s, sizeof(s), fp) != 0)
  {
    strcat(string, s);
  }
  pclose(fp);
  printf("--> converted hash: %s\n", string);
  return string;
}
