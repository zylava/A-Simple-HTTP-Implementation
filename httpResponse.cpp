#include "httpResponse.h"

HttpResponse::HttpResponse() {
  this->m_url = "";
  this->m_message = "";
  this->m_status = "";
  this->m_length = 0;
}
void HttpResponse::consume(std::string wire) {
  // Get the status and message body
  // First 9 chars are 'HTTP/1.0 '
  this->m_status = wire.substr(9, 3);
  size_t cstart = wire.find("Content-Length:");
  size_t cend = wire.find("\r\n", cstart);
  // content-length: len\r\n
  // 01234567890123456789 0
  std::string len = wire.substr(cstart + 16, cend - cstart - 16);
  this->m_length = std::stoi(len);
  this->m_message = "";
  if(this->m_status == "200") {
    int pos = wire.find("\r\n\r\n") + 4;
    this->m_message = wire.substr(pos, -1);
  }
}

void HttpResponse::setUrl(std::string url) {
  this->m_url = url;
}

void HttpResponse::setMessage() {
  if(this->m_status != "") {
    return;
  }
  // Check existence
  if(access(this->m_url.c_str(), F_OK) != 0) {
    this->setStatus("404 Not Found");
    return;
  }
  // Check permissions
  struct stat results;
  stat(this->m_url.c_str(), &results);
  if(!(results.st_mode & S_IROTH)) {
    this->setStatus("403 Forbidden");
    return;
  }
  this->setStatus("200 OK");
  std::ifstream stream(this->m_url);
  std::string file((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
  this->m_message = file;
  this->m_length = file.length();
}

void HttpResponse::setStatus(std::string status) {
  this->m_status = status;
}

std::string HttpResponse::encode() {
  std::string blob = "HTTP/1.0 " + this->m_status + "\r\n";
  blob += "Content-Type: text/html\r\n";
  blob += "Connection: close\r\n";
  // Copy file over to body
  // Content-Length: length
  char buffer[33];
  sprintf(buffer, "%zu", this->m_length);
  std::string length(buffer);
  blob += "Content-Length: " + length + "\r\n\r\n";
  blob += this->m_message;
  return blob;
}

std::string HttpResponse::message() {
  return this->m_message;
}

std::string HttpResponse::status() {
  return this->m_status;
}

size_t HttpResponse::contentLength() {
  return this->m_length;
}
