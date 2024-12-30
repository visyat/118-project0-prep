#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>

// client takes port number and hostname args ...
// hostname = argv[1], port = argv[2]
int main(int argc, char *argv[]) {
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

   struct sockaddr_in serveraddr;
   serveraddr.sin_family = AF_INET; // use IPv4
   serveraddr.sin_addr.s_addr = INADDR_ANY;
   
   // set sending port ...
   if (strcmp(argv[1], "localhost") == 0) {
        serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    } else {
        serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
    }
   int SEND_PORT = atoi(argv[2]);
   serveraddr.sin_port = htons(SEND_PORT); // Big endian

   int BUF_SIZE = 1024;
   char server_buf[BUF_SIZE];
   socklen_t serversize = sizeof(socklen_t); // Temp buffer for recvfrom API

   for (;;) {
      // listen to response from server ...
      int bytes_recvd = recvfrom(sockfd, server_buf, BUF_SIZE, 
                              // socket  store data  how much
                                 0, (struct sockaddr*) &serveraddr, 
                                 &serversize);
      if (bytes_recvd <= 0) {
         if (errno != EAGAIN && errno != EWOULDBLOCK) return errno;
      }
      // Print out data
      write(1, server_buf, bytes_recvd);

      // send messages from stdin ...
      char client_buf[BUF_SIZE];
      ssize_t bytes_read = read(STDIN_FILENO, client_buf, BUF_SIZE);
      int did_send = sendto(sockfd, client_buf, bytes_read, 
                        // socket  send data   how much to send
                           0, (struct sockaddr*) &serveraddr, 
                        // flags   where to send
                           sizeof(serveraddr));
      if (did_send < 0) return errno;
   }

   /* 6. You're done! Terminate the connection */     
   close(sockfd);
   return 0;
}
