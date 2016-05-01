#include <string>
#include <thread>
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <algorithm>
#include "httpRequest.h"
#include "httpResponse.h"
#include <sys/signal.h>

int _Fd;
volatile sig_atomic_t timeout = 0;
std::string directory;

void doStuff(int socketFd);

// Web server script
// Initiates a server
// Arguments: ./webserver hostname  port directory
// Defaults:              localhost 4000 .
int main(int argc, char* argv[]) {
  std::vector<std::thread> threads;

  if(4 != argc && 1 != argc) {
    std::cerr << "Wrong number of arguments";
    return -1;
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
  if(listenFd < 0) {
    std::cerr << "Cannot open socket" << std::endl;
    return -1;
  }
  int res = bind(listenFd, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
  if(res < 0) {
    std::cerr << "Cannot bind" << std::endl;
    return -1;    
  }
  if(listen(listenFd, 1) < 0) {
    std::cerr << "Cannot listen" << std::endl;
    return -1;
  }
  struct sockaddr_in clientAddress;
  while(true) {
    int numthread = 0;
    while (numthread < 8) {
      socklen_t len = sizeof(clientAddress);
      int newFd = accept(listenFd, (struct sockaddr*)&clientAddress, &len);
      // Do stuff
      threads.push_back(std::thread(std::bind(&doStuff, newFd)));
      numthread++;
    }
    for (int i = 0; i < numthread; i++) {
      threads[i].join();
    }
    threads.clear();
  }
  return 0;
}

void signalrm_handler(int sig) {
  std::cerr << "Timeout occurring" << std::endl;
  timeout = 1;
  close(_Fd);
}

// Handle a connection from a client
void doStuff(int socketFd) {
  timeout = 0;
  // Get the request
  _Fd = socketFd;
  signal(SIGALRM, &signalrm_handler);
  alarm(60);
  std::string request = "";
  char buf[8193] = {0};
  if(recv(socketFd, buf, 8192, 0) > 0) {
    request.append(buf);
  }
  if(timeout) {
    return;
  }
  // Parse the request
  HttpResponse response;
  // Get the url from request
  size_t start = request.find(' ');
  size_t end = request.find(' ', start + 1);
  std::string version = request.substr(end + 1, 8);
  HttpRequest req;
  req.consume(request);
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
  close(socketFd);
}
