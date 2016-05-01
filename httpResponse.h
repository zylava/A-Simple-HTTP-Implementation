#include <string.h>
#include <fstream>
#include <streambuf>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>

class HttpResponse {
  public:
  // Constructs a response from a wire
  HttpResponse();

  void consume(std::string wire);
  void setUrl(std::string url);
  void setMessage();
  void setStatus(std::string status);
  std::string encode();
  std::string message(); 
  std::string status(); 
  size_t contentLength(); 

  private:

  std::string m_url;
  std::string m_message;
  std::string m_status;
  size_t m_length;
};
