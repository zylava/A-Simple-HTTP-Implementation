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
#include <netdb.h>
#include "httpRequest.h"
#include "httpResponse.h"
#include <iostream>
#include <fstream>

int main(int argc, char* argv[])
{
  // Parse the argument
  if(2 > argc) {
    std::cerr << "Wrong number of arguments!" << std::endl;
    return -1;
  }
  std::string hostname;
  std::string port_str;
  int port;
  int pos1;
  int pos2;

  for(int i = 1; i < argc; i++) {
    std::string url = argv[i];
    pos1 = url.find(":", 7);
    // No port specified default to 80
    if(-1 == pos1) {
      port = 80;
      port_str = "80";
      pos2 = url.find("/", 7);
      if(-1 == pos2) {
        continue;
      }
      hostname = url.substr(7, pos2 - 7);
    }
    else {
      // Get all the characters after http:// up to the next colon
      hostname = url.substr(7, pos1 - 7);
      pos2 = url.find("/", pos1);
      if(-1 == pos2) {
        continue;
      }
      // Get the string after the hostname until the next /
      port_str = url.substr(pos1 + 1, pos2 - pos1 - 1);
      port = atoi(port_str.c_str());
    }
    // Get the rest of the strings after the port number
    std::string filename = url.substr(pos2, -1);

    // Initialize connection
    struct hostent* server = gethostbyname(hostname.c_str());
    if(0 == server) {
      std::cerr << "Server not found" << std::endl;
      return -1;
    }

    struct sockaddr_in serverAddress;
    bzero((char *)&serverAddress, sizeof(serverAddress));
    bcopy((char *)server->h_addr, (char *)&serverAddress.sin_addr.s_addr, server->h_length);
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    int socketFd = socket(AF_INET, SOCK_STREAM, 0);
    int ret = connect(socketFd, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if(ret < 0) {
      std::cerr << "Error connecting" << std::endl;
      return -1;
    }

    // Construct the request
    HttpRequest request;
    request.setUrl(filename);
    request.setMethod("GET");
    request.setHost(hostname);
    request.setPort(port_str);
    std::string blob = request.encode();
    // set hostname and port
    // set filename to get
    // make a string
    
    // Send the request
    send(socketFd, blob.c_str(), blob.length() + 1, 0);
      
    // Get the response
    std::string response = "";
    char buf[4097] = {0};
    while(recv(socketFd, buf, 4096, 0) > 0) {
      response.append(buf);
      memset(buf, 0, sizeof(buf));
    }
    
    // Interpret the response
    HttpResponse resp;
    if (response != "") {
      resp.consume(response);
    }
    // find the last '/' character in the filename
    int lastslash = filename.find_last_of('/');
    filename = filename.substr(lastslash + 1, -1);
    // If success, save the file in the current directory
    if('2' == resp.status()[0] && resp.contentLength() == resp.message().length()) {
      std::ofstream file;
      file.open (filename);
      file << resp.message();
      file.close();
    }
  }
  return 0;
}
