/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thabeck- <thabeck-@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/07 18:17:58 by thabeck-          #+#    #+#             */
/*   Updated: 2024/08/10 00:55:35 by thabeck-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
#define UTILS_HPP

/* Defines */
# define CONNECTION_TIMEOUT 60
# define MESSAGE_BUFFER 40000
# define MAX_URI_LENGTH 4096
# define MAX_CONTENT_LENGTH 30000000

# define RED "\033[1;31m"
# define GREEN "\033[1;32m"
# define YELLOW "\033[1;33m"
# define BLUE "\033[1;34m"
# define MAGENTA "\033[1;35m"
# define CYAN "\033[1;36m"
# define RESET "\033[0m"

/* Default */
# include <iostream>
# include <fcntl.h>
# include <cstring>
# include <string> 
# include <unistd.h>
# include <dirent.h>
# include <sstream>
# include <cstdlib>
# include <fstream>
# include <sstream>
# include <cctype>
# include <ctime>
# include <cstdarg>

/* STL Containers */
# include <map>
# include <set>
# include <vector>
# include <algorithm>
# include <iterator>
# include <list>

/* System */
# include <sys/types.h>
# include <sys/wait.h>
# include <sys/stat.h>
# include <sys/time.h>
# include <unistd.h>
# include <signal.h>

/* Network */
# include <sys/socket.h>
# include <netinet/in.h>
# include <sys/select.h>
# include <arpa/inet.h>

// /* Includes */
// # include "ConfFile.hpp"
// # include "ServerConf.hpp"
// # include "Server.hpp"
// # include "Location.hpp"
// # include "Cgi.hpp"
// # include "Request.hpp"
// # include "Response.hpp"
// # include "Client.hpp"
// # include "Webserv.hpp"

/* Utils */
std::string                     getStrSatusCode(short);
std::string                     getErrorPage(short);
int                             ft_strtoi(std::string str);
unsigned int                    ft_hexToDec(const std::string& nb);
int                             getAutoIndexPage(std::string &, std::vector<uint8_t> &, size_t &);
bool                            allowedCharURI(uint8_t ch);
void                            trimStr(std::string &str);
void                            toLower(std::string &str);
bool                            checkUriPos(std::string path);
bool                            fileExists (const std::string& f);
bool                            isDirectory(std::string path);
std::string                     combinePaths(std::string p1, std::string p2, std::string p3);

/* Enums */
enum HttpMethod
{
    GET,
    POST,
    DELETE,
    NONE
};

enum RequestParsingState
{
    Request_Line,
    Request_Line_Method,
    Request_Line_First_Space,
    Request_Line_URI_Path_Slash,
    Request_Line_URI_Path,
    Request_Line_URI_Query,
    Request_Line_URI_Fragment,
    Request_Line_Ver,
    Request_Line_HTTP_1,
    Request_Line_HTTP_2,
    Request_Line_HTTP_3,
    Request_Line_HTTP_Slash,
    Request_Line_Major,
    Request_Line_Dot,
    Request_Line_Minor,
    Request_Line_CR,
    Request_Line_LF,
    Field_Name_Start,
    Fields_End,
    Field_Name,
    Field_Value,
    Field_Value_End,
    Chunked_Length_Begin,
    Chunked_Length,
    Chunked_Ignore,
    Chunked_Length_CR,
    Chunked_Length_LF,
    Chunked_Data,
    Chunked_Data_CR,
    Chunked_Data_LF,
    Chunked_End_CR,
    Chunked_End_LF,
    Message_Body,
    Parsing_Done
};

/* Template */
template <typename T>
std::string toString(const T val)
{
    std::stringstream stream;
    stream << val;
    return stream.str();
}

#endif