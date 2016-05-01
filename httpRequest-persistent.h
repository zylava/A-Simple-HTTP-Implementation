#include <string>
#include <vector>

class HttpRequest {
  public:
  // Sets the url for the request
  void setUrl(std::string url);

  // Sets the url for the request
  void setHost(std::string host);

  // Sets the url for the request
  void setPort(std::string port);

  // Sets the method of the request (GET only)
  void setMethod(std::string method);

  // Encodes the information in this request as a vecotr of chars
  std::string encode();

  // Initializes this request from the data in wire
  void consume(std::string wire);

  // Returns the value of m_invaludRequest
  bool invalidRequest();

  private:
  std::string m_url;
  std::string m_method;
  std::string m_host;
  std::string m_port;
  std::string m_connection = "keep-alive";
  std::int m_alive = 30;
  bool m_invalidRequest;
};
