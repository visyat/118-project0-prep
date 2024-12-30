#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>

// server takes port number argument ...
int main(int argc, char *argv[]) {
   // set up socket ...
   int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
                  // use IPv4  use UDP
   
   // make socket non-blocking ...
   int flags_socket = fcntl(sockfd, F_GETFL);
   flags_socket |= O_NONBLOCK;
   fcntl(sockfd, F_SETFL, flags_socket);

   // make standard input non-blocking ...
   int flags_stdin = fcntl(STDIN_FILENO, F_GETFL);
   flags_stdin |= O_NONBLOCK;
   fcntl(STDIN_FILENO, F_SETFL, flags_stdin);

   struct sockaddr_in servaddr;
   servaddr.sin_family = AF_INET; // use IPv4
   servaddr.sin_addr.s_addr = INADDR_ANY; // accept all connections
                           // same as inet_addr("0.0.0.0") 
                                    // "Address string to network bytes"
   // read server port from args ...
   int PORT = atoi(argv[1]);
   servaddr.sin_port = htons(PORT); // Big endian

   int did_bind = bind(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr));
   if (did_bind < 0) return errno;

   int BUF_SIZE = 1024;
   char client_buf[BUF_SIZE];
   char server_buf[BUF_SIZE];

   struct sockaddr_in clientaddr; // Same information, but about client
   socklen_t clientsize = sizeof(clientaddr);
   
   int CONNECTED = 0;

   for (;;) {
      // recv from clients ...
      memset(client_buf, 0, BUF_SIZE);
      int bytes_recvd = recvfrom(sockfd, client_buf, BUF_SIZE, 
                              // socket  store data  how much
                                 0, (struct sockaddr*) &clientaddr, 
                                 &clientsize);
      if (bytes_recvd <= 0 && CONNECTED == 0) {
         if (errno != EAGAIN && errno != EWOULDBLOCK) return errno;
         usleep(1000);
         continue;
      } else if (bytes_recvd > 0) {
         // set client connected as true ...
         CONNECTED = 1;

         char* client_ip = inet_ntoa(clientaddr.sin_addr); // "Network bytes to address string"
         int client_port = ntohs(clientaddr.sin_port); // Little endian
         
         // if data recieved, write to stdout ...
         write(1, client_buf, bytes_recvd);
      }

      memset(server_buf, 0, BUF_SIZE);
      ssize_t read_bytes = read(STDIN_FILENO, server_buf, BUF_SIZE);
      if (read_bytes > 0) {
         int did_send = sendto(sockfd, server_buf, read_bytes, 
                           // socket  send data   how much to send
                              0, (struct sockaddr*) &clientaddr, 
                           // flags   where to send
                              sizeof(clientaddr));
         if (did_send < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) return errno;
         }
      } else if (read_bytes < 0) {
         if (errno != EAGAIN && errno != EWOULDBLOCK) return errno;
      }
   }
  
   close(sockfd);
   return 0;
}