#include "httpRequest.h"
// Sets the url for the request
void HttpRequest::setUrl(std::string url) {
  this->m_url = url;
}

void HttpRequest::setHost(std::string host) {
  this->m_host = host;
}

void HttpRequest::setPort(std::string port) {
  this->m_port = port;
}

// Sets the method of the request (GET only)
void HttpRequest::setMethod(std::string method) {
  this->m_method = method;
}

// Encodes the information in this request as a vector of chars
std::string HttpRequest::encode() {
  std::string blob = this->m_method + " " + this->m_url + " " + "HTTP/1.0\r\n";
  blob += "User-Agent: 118-client\r\n";
  blob += "Accept: */*\r\n";
  blob += "Host: " + this->m_host + ":" + this->m_port + "\r\n\r\n";
  return blob;
}

// Initializes this request from the data in wire
void HttpRequest::consume(std::string wire) {
  size_t len = wire.length();
  for(size_t i = 0; i < len; i++) {
    if('\r' == wire[i]) {
      wire = wire.substr(0, i - 1) + wire.substr(i + 1, -1);
      len--;
      i--;
    }
  }
  std::vector<std::string> strs;
  int start = 0;
  for(size_t i = 0; i < len; i++) {
    if('\n' == wire[i]) {
      strs.push_back(wire.substr(start, i - start - 1));
      start = i + 1;
    }
  }
  std::vector<std::string> first;
  size_t firstlen = strs[0].length();
  start = 0;
  for(size_t i = 0; i < firstlen; i++) {
    if(' ' == strs[0][i]) {
      first.push_back(strs[0].substr(start, i - start - 1));
      start = i + 1;
    }
  }
  if(first.size() != 2) {
    this->m_invalidRequest = true;
    return;
  }
  this->m_method = first[0];
  this->m_url = first[1];
  // Look for an empty string in strs
  len = strs.size();
  //size_t i = 0;
  //for(; i < len; i++) {
    //if(strs[i] != "") {
      //continue;
    //}
    //break;
  //}
  //// Get message if needed
  this->m_invalidRequest = false;
}

bool HttpRequest::invalidRequest() {
  return this->m_invalidRequest;
}
