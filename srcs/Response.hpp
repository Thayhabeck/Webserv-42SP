#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <map>

class Response {
public:
    Response(int status_code, const std::string& body);
    std::string toString() const;

private:
    int status_code;
    std::string body;
    std::map<int, std::string> status_codes;
};

#endif