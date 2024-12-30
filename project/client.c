#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>

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

   /* 2. Construct server address */
   struct sockaddr_in serveraddr;
   serveraddr.sin_family = AF_INET; // use IPv4
   serveraddr.sin_addr.s_addr = INADDR_ANY;
   
   // Set sending port
   int SEND_PORT = atoi(argv[2]);
   serveraddr.sin_port = htons(SEND_PORT); // Big endian

   int BUF_SIZE = 1024;
   char server_buf[BUF_SIZE];
   char client_buf[BUF_SIZE];

   socklen_t serversize = sizeof(serveraddr); // Temp buffer for recvfrom API

   for (;;) {
      int bytes_recvd = recvfrom(sockfd, server_buf, BUF_SIZE, 
                              // socket  store data  how much
                                 0, (struct sockaddr*) &serveraddr, 
                                 &serversize);
      if (bytes_recvd <= 0) {
         if (errno != EAGAIN && errno != EWOULDBLOCK) return errno;
      } else {
         write(1, server_buf, bytes_recvd);
      }

      memset(client_buf, 0, BUF_SIZE);
      ssize_t read_bytes = read(STDIN_FILENO, client_buf, BUF_SIZE);
      if (read_bytes > 0) {
         int did_send = sendto(sockfd, client_buf, read_bytes, 
                           // socket  send data   how much to send
                              0, (struct sockaddr*) &serveraddr, 
                           // flags   where to send
                              sizeof(serveraddr));
         if (did_send < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) return errno;
         }
      }
   }

   /* 6. You're done! Terminate the connection */     
   close(sockfd);
   return 0;
}
