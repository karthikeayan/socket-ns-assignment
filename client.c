#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>

void errorCheck(int n, char *message);

int main(int argc, char *argv[])
{
   int sockfd, portno, n;
   char downloadDir[256];
   char username[25];
   char currentMessage[10];

   struct sockaddr_in serv_addr;
   struct hostent *server;
   
   char buffer[256];
   
   if (argc <4) {
      fprintf(stderr,"usage %s hostname port\n", argv[0]);
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
   
   if (n < 0)
   {
      perror("ERROR reading from socket");
      exit(1);
   }
   printf("%s\n", buffer);
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
