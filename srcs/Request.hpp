#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <map>

class Request {
public:
    Request(const std::string& raw_request);
    std::string getMethod() const;
    std::string getPath() const;
    std::string getVersion() const;
    std::map<std::string, std::string> getHeaders() const;

private:
    void parseRequest(const std::string& raw_request);

    std::string method;
    std::string path;
    std::string version;
    std::map<std::string, std::string> headers;
};

#endif 