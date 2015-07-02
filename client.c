#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>

void errorCheck(int n, char *message);
char *get_hash(char *string);
char *get_string1(char *hashstring);
char *get_string2(char *hashstring);

int main(int argc, char *argv[])
{
   int sockfd, portno, n;
   char downloadDir[256];
   char username[25];
   char currentMessage[10];
   char *command = (char *) malloc(100);
   char *message = (char *) malloc(100);
   char *hash = (char *) malloc(100);
   char *backup = (char *) malloc(100);
   char filepath[256];

   struct sockaddr_in serv_addr;
   struct hostent *server;

   char buffer[256];

   if (argc <4) {
      fprintf(stderr,"usage %s hostname port work_directory\n", argv[0]);
      exit(0);
   }
   portno = atoi(argv[2]);

   /* Create a socket point */
   sockfd = socket(AF_INET, SOCK_STREAM, 0);

   if (sockfd < 0)
   {
      perror("ERROR opening socket");
      exit(1);
   }
   server = gethostbyname(argv[1]);

   if (server == NULL) {
      fprintf(stderr,"ERROR, no such host\n");
      exit(0);
   }

   bzero((char *) &serv_addr, sizeof(serv_addr));
   serv_addr.sin_family = AF_INET;
   bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
   serv_addr.sin_port = htons(portno);

   /* Now connect to the server */
   if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
   {
      perror("ERROR connecting");
      exit(1);
   }

   /* Now LOGIN <username> to server
   */
   bzero(buffer,256);
   strcpy(username, "dhoni");
   strcpy(buffer, "LOGIN ");
   strcpy(currentMessage, buffer);
   strcat(buffer, username);

   /* Send message to the server */
   n = write(sockfd, buffer, strlen(buffer));
   printf(" LOGIN Status: %d \n", n);
   errorCheck(n, currentMessage);

   /* Now read server response */
   bzero(buffer,256);
   n = read(sockfd, buffer, 255);
   errorCheck(n, buffer);
   printf("Message from Server: %s\n", buffer);
   strcpy(backup, buffer);

   printf("Backup Buffer: %s\n", backup);
   command = get_string1(buffer);
   printf("String1: %s\n", command);

   if (strcmp(command, "NEEDAUTH") != 0) {
     printf("Invalid message from server, expecting NEEDAUTH\n");
     exit(1);
   }

   printf("inside if, buffer is: %s\n", backup);
   message = get_string2(backup);
   printf("String2: %s\n", message);
   strcpy(backup, message);
   hash = get_hash(message);
   hash = get_string1(hash);
   printf("converted hash: %s, for password: %s\n", hash, backup);

   /* Writing back to server */
   bzero(buffer, 256);
   strcpy(buffer, "AUTH ");
   strcat(hash, backup);
   strcat(buffer, hash);
   n = write(sockfd, buffer, strlen(buffer));

   /* Get confirmation from server */
   bzero(buffer, 256);
   n = read(sockfd, buffer, 32);
   printf("Login confirmation from server: %s\n", buffer);

   /* Request file to server */
   n = write(sockfd, "GETFILE detail.txt", 32);

   /* Receive file from server */
   bzero(buffer, 256);
   n = read(sockfd, buffer, sizeof(buffer));
   printf("file content received form server: %s\n", buffer);

   /* checking if the file present in server using server response*/
   bzero(message, 32);
   bzero(backup, strlen(buffer));
   strcpy(backup, buffer);
   message = get_string1(buffer);
   if (strcmp(command, "DATA") != 0) {
     printf("ERROR message from server, expecting DATA\n");
     exit(1);
   }

   /* write data into file */
   strcpy(filepath, argv[3]);
   strcat(filepath, "/");
   strcat(filepath, "details.txt");
   FILE *fp = fopen(filepath, "ab");
   printf("Writing the received data into file...\n");
   fwrite(backup, sizeof(char), strlen(backup), fp);
   printf("Write done into %s file\n", filepath);
   fclose(fp);

   return 0;
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

char *get_hash(char *string){
  char command[50];
  char *s = (char *) malloc(10);
  FILE *fp;

  strcpy(command, "echo -n ");
  strcat(command, string);
  strcat(command, " | md5sum");
  printf("command to execute: %s\n", command);
  fp = popen(command, "r");
  printf("after popen\n");
  bzero(string, 32);
  while(fgets(s, sizeof(s), fp) != 0)
  {
    strcat(string, s);
  }
  pclose(fp);
  printf("hash value: %s", string);
  return string;
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
