#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>

// server takes port number argument ...
int main(int argc, char *argv[]) {
   int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

   // make socket non-blocking ...
   int flags_socket = fcntl(sockfd, F_GETFL);
   flags_socket |= O_NONBLOCK;
   fcntl(sockfd, F_SETFL, flags_socket);

   // make standard input non-blocking ...
   int flags_stdin = fcntl(STDIN_FILENO, F_GETFL);
   flags_stdin |= O_NONBLOCK;
   fcntl(STDIN_FILENO, F_SETFL, flags_stdin);

   struct sockaddr_in servaddr;
   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr = INADDR_ANY;

   // read port from argument ...
   servaddr.sin_port = htons(atoi(argv[1])); // Big endian

   int did_bind = bind(sockfd, (struct sockaddr*) &servaddr, 
                     sizeof(servaddr));
   if (did_bind < 0) return errno;

   int BUF_SIZE = 1024;
   char client_buf[BUF_SIZE];
   struct sockaddr_in clientaddr; // Same information, but about client
   socklen_t clientsize = sizeof(clientaddr);

   // infinite loop to create an ever-listening server ...
   for (;;) {
      int bytes_recvd = recvfrom(sockfd, client_buf, BUF_SIZE, 
                              // socket  store data  how much
                                 0, (struct sockaddr*) &clientaddr, 
                                 &clientsize);
      // check if client is connected ...
      if (bytes_recvd <= 0) {
         if (errno != EAGAIN && errno != EWOULDBLOCK) {
            return errno;
         } else {
            continue;
         }
      }
      
      // if data has been recieved (client connected) ...
      char* client_ip = inet_ntoa(clientaddr.sin_addr); // "Network bytes to address string"
      int client_port = ntohs(clientaddr.sin_port); // Little endian
      // write to stdout ...
      write(1, client_buf, bytes_recvd);

      // read from stdin ...
      char server_buf[BUF_SIZE];
      ssize_t bytes_read = read(STDIN_FILENO, server_buf, BUF_SIZE);
      // if data available from stdn, send to client ...
      int did_send = sendto(sockfd, server_buf, bytes_read, 
                        // socket  send data   how much to send
                           0, (struct sockaddr*) &clientaddr, 
                        // flags   where to send
                           sizeof(clientaddr));
      if (did_send < 0) return errno; 
   }
 
   close(sockfd);
   return 0;
}