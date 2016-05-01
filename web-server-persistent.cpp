#include <string>
#include <thread>
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <algorithm>
#include <time.h>
#include "httpRequest.h"
#include "httpResponse.h"

std::string directory;

void doStuff(int socketFd);
int timeout[1024];

// Web server script
// Initiates a server
// Arguments: ./webserver hostname  port directory
// Defaults:              localhost 4000 .
int main(int argc, char* argv[])
{
  fd_set readFds;
  fd_set errFds;
  fd_set watchFds;
  FD_ZERO(&readFds);
  FD_ZERO(&errFds);
  FD_ZERO(&watchFds);

  for (int i = 0; i < 1024; i++){
    timeout[i] = 0;
  }
  struct sockaddr_in serverAddress;
  bzero((char*) &serverAddress, sizeof(serverAddress));
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
  serverAddress.sin_port = htons(4000);
  directory = ".";
  if(4 == argc) {
    // store this IP address in sa:
    inet_pton(AF_INET, argv[1], &(serverAddress.sin_addr));
    serverAddress.sin_port = htons(atoi(argv[2]));
    directory = std::string(argv[3]);
  }

  // Create a webserver on hostname:port
  // that serves files from directory
  int listenFd = socket(AF_INET, SOCK_STREAM, 0);
  int maxFd = listenFd;
  if(listenFd < 0)
  {
    std::cerr << "Cannot open socket" << std::endl;
    return -1;
  }
  FD_SET(listenFd, &watchFds);

  int yes = 1;
  if (setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
    perror("setsockopt");
    return 1;
  }

  int res = bind(listenFd, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
  if(res < 0) {
    std::cerr << "Cannot bind" << std::endl;
    return -1;    
  }
  if(listen(listenFd, 10) < 0) {
    std::cerr << "Cannot listen" << std::endl;
    return -1;
  }

  struct timeval tv;
  while (true) {
    int nReadyFds = 0;
    readFds = watchFds;
    errFds = watchFds;
    tv.tv_sec = 3;
    tv.tv_usec = 0;
      
      time_t curr;
      time(&curr);
      for (int i = 0; i <= maxFd; i++){
          if (i != listenFd && timeout[i] != 0 && timeout[i] <= curr){
              timeout[i] = 0;
              close(i);
              FD_CLR(i, &watchFds);
          }
      }
    if ((nReadyFds = select(maxFd + 1, &readFds, NULL, &errFds, &tv)) == -1)
    {
      perror ("Cannot select");
      return -1;
    }
    if (nReadyFds != 0){
      for (int fd = 0; fd <= maxFd; fd++){
        if (FD_ISSET(fd, &readFds)){//get one for reading
          if(fd == listenFd) {
            struct sockaddr_in client_addr;
            socklen_t client_addr_size = sizeof(client_addr);
            int clientFd = accept(fd, (struct sockaddr*)&client_addr, &client_addr_size);
            time_t current;
            time(&current);
            timeout[clientFd] = current + 30;
            if (clientFd == -1){
              perror("Cannot accept");
              return -1;
            }
            char ipstr[INET_ADDRSTRLEN] = {'\0'};
            inet_ntop(client_addr.sin_family, &client_addr.sin_addr, ipstr, sizeof(ipstr));
            time(&current);
            
            timeout[clientFd] = current + 30;

            if (maxFd < clientFd){
              maxFd = clientFd;
            }
            FD_SET (clientFd, &watchFds);
          }
          else { //normal socket
              time_t now;
              time(&now);
            timeout[fd] = now + 30;
            doStuff(fd);
            FD_CLR(fd, &watchFds);
          }
        }
      }
    }
    time_t cur;
    time(&cur);
    for (int i = 0; i < maxFd; i++){
      if (i != listenFd && timeout[i] <= cur){
        close(i);
        FD_CLR(i, &watchFds);
      }
    }
  }
  return 0;
}

// Handle a connection from a client
void doStuff(int socketFd) {
  // Get the request
  std::string request = "";
  char buf[8193] = {0};
  if(recv(socketFd, buf, 8192, 0) > 0) {
    request.append(buf);
  }
  // Parse the request
  HttpResponse response;
  // Get the url from request
  size_t start = request.find(' ');
  size_t end = request.find(' ', start + 1);
  std::string version = request.substr(end + 1, 8);
  HttpRequest req;
  if (request != "") {
    req.consume(request);
  }
  if(req.invalidRequest() || version.substr(0, 4) != "HTTP") {
    response.setStatus("400 Invalid Request");
  }
  else if ("HTTP/1.0" != version) {
    response.setStatus("505 Version not supported");
  }
  else {
    std::string url = request.substr(start + 1, end - start - 1);
    response.setUrl(directory + "/" + url);
    response.setMessage();
  }
  std::string blob = response.encode();
  // Form a response
  // Send back the response
  send(socketFd, blob.c_str(), blob.length() + 1, 0);
}

