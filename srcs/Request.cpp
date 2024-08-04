#include "Request.hpp"
#include <sstream>

Request::Request(const std::string& raw_request) {
    parseRequest(raw_request);
}

void Request::parseRequest(const std::string& raw_request) {
    std::istringstream request_stream(raw_request);
    std::string line;
    std::getline(request_stream, line);
    std::istringstream line_stream(line);
    line_stream >> method >> path >> version;

    while (std::getline(request_stream, line) && line != "\r") {
        size_t colon_pos = line.find(":");
        if (colon_pos != std::string::npos) {
            std::string header_name = line.substr(0, colon_pos);
            std::string header_value = line.substr(colon_pos + 1);
            headers[header_name] = header_value;
        }
    }
}

std::string Request::getMethod() const {
    return method;
}

std::string Request::getPath() const {
    return path;
}

std::string Request::getVersion() const {
    return version;
}

std::map<std::string, std::string> Request::getHeaders() const {
    return headers;
}