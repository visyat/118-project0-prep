#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

// server takes port number argument from stdin ...
int main(int argc, char *argv[]) {
   int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
                  // use IPv4  use UDP

   // make socket non-blocking ...
   int flags_socket = fcntl(sockfd, F_GETFL);
   flags_socket |= O_NONBLOCK;
   fcntl(sockfd, F_SETFL, flags_socket);

   // make standard input non-blocking ...
   int flags_stdin = fcntl(0, F_GETFL);
   flags_stdin |= O_NONBLOCK;
   fcntl(0, F_SETFL, flags_stdin);

   struct sockaddr_in servaddr;
   servaddr.sin_family = AF_INET; // use IPv4
   servaddr.sin_addr.s_addr = INADDR_ANY; // accept all connections
                           // same as inet_addr("0.0.0.0") 
                                    // "Address string to network bytes"

   // read port from standard input ...
   servaddr.sin_port = htons(atoi(argv[1])); // Big endian

   /*
      connection has been established as soon as the client has sent over data. 
      the server does not have to accept any other connections. 
      once this connection is established, it will:
      1. start reading from the socket and output its contents to standard output. 
      2. start reading from standard input and forwarding its contents to the newly connected client.
   */

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
      if (bytes_recvd < 0) return errno;

      char* client_ip = inet_ntoa(clientaddr.sin_addr);
                     // "Network bytes to address string"
      int client_port = ntohs(clientaddr.sin_port); // Little endian
      write(1, client_buf, bytes_recvd);

      /* 7. Send data back to client */
      char server_buf[] = "Hello world!";
      int did_send = sendto(sockfd, server_buf, strlen(server_buf), 
                        // socket  send data   how much to send
                           0, (struct sockaddr*) &clientaddr, 
                        // flags   where to send
                           sizeof(clientaddr));
      if (did_send < 0) return errno;  
   }
 
   close(sockfd);
   return 0;
}