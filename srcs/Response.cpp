#include "Response.hpp"
#include <sstream>

Response::Response(int status_code, const std::string& body) 
    : status_code(status_code), body(body) {
    status_codes[200] = "OK";
    status_codes[404] = "Not Found";
    status_codes[500] = "Internal Server Error";
}

std::string Response::toString() const {
    std::ostringstream response_stream;
    response_stream << "HTTP/1.1 " << status_code << " " << status_codes.at(status_code) << "\r\n";
    response_stream << "Content-Length: " << body.size() << "\r\n";
    response_stream << "\r\n";
    response_stream << body;
    return response_stream.str();
}