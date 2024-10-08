/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils-teste.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: matcardo <matcardo@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/07 18:17:55 by thabeck-          #+#    #+#             */
/*   Updated: 2024/08/13 04:21:56 by matcardo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Utils.hpp"

/* Converte string para inteiro */
int ft_strtoi(std::string str)
{
    std::stringstream ss(str);
    if (str.length() > 10)
        throw std::exception();
    for (size_t i = 0; i < str.length(); ++i)
    {
        if(!isdigit(str[i]))
            throw std::exception();
    }
    int res;
    ss >> res;
    return (res);
}

/* Converte hexadecimal para decimal */
unsigned int ft_hexToDec(const std::string& nb)
{
	unsigned int x;
	std::stringstream ss;
	ss << nb;
	ss >> std::hex >> x;
	return (x);
}

/* Obtém o http status code de acordo com o código de status */
std::string getStrSatusCode(short statusCode)
{
    switch (statusCode)
    {
        case 100:
            return "Continue";
        case 101:
            return "Switching Protocol";
        case 200:
            return "OK";
        case 201:
            return "Created";
        case 202:
            return "Accepted";
        case 203:
            return "Non-Authoritative Information";
        case 204:
            return "No Content";
        case 205:
            return "Reset Content";
        case 206:
            return "Partial Content";
        case 300:
            return "Multiple Choice";
        case 301:
            return "Moved Permanently";
        case 302:
            return "Moved Temporarily";
        case 303:
            return "See Other";
        case 304:
            return "Not Modified";
        case 307:
            return "Temporary Redirect";
        case 308:
            return "Permanent Redirect";
        case 400:
            return "Bad Request";
        case 401:
            return "Unauthorized";
        case 403:
            return "Forbidden";
        case 404:
            return "Not Found";
        case 405:
            return "Method Not Allowed";
        case 406:
            return "Not Acceptable";
        case 407:
            return "Proxy Authentication Required";
        case 408:
            return "Request Timeout";
        case 409:
            return "Conflict";
        case 410:
            return "Gone";
        case 411:
            return "Length Required";
        case 412:
            return "Precondition Failed";
        case 413:
            return "Payload Too Large";
        case 414:
            return "URI Too Long";
        case 415:
            return "Unsupported Media Type";
        case 416:
            return "Requested Range Not Satisfiable";
        case 417:
            return "Expectation Failed";
        case 418:
            return "I'm a teapot";
        case 421:
            return "Misdirected Request";
        case 425:
            return "Too Early";
        case 426:
            return "Upgrade Required";
        case 428:
            return "Precondition Required";
        case 429:
            return "Too Many Requests";
        case 431:
            return "Request Header Fields Too Large";
        case 451:
            return "Unavailable for Legal Reasons";
        case 500:
            return "Internal Server Error";
        case 501:
            return "Not Implemented";
        case 502:
            return "Bad Gateway";
        case 503:
            return "Service Unavailable";
        case 504:
            return "Gateway Timeout";
        case 505:
            return "HTTP Version Not Supported";
        case 506:
            return "Variant Also Negotiates";
        case 507:
            return "Insufficient Storage";
        case 510:
            return "Not Extended";
        case 511:
            return "Network Authentication Required";
        default:
            return "Undefined";
        }
}

/* Obtém a página de erro de acordo com o código de status */
std::string getErrorPage(short statusCode)
{
    std::string statusCodeStr = toString(statusCode);
    std::string statusMessage = getStrSatusCode(statusCode);
    
    return (
        "<html>\r\n"
        "<head><title>" + statusCodeStr + " " + statusMessage + "</title></head>\r\n"
        "<style>\r\n"
        "    body {"
        "        background-color: #f0f0f5;"
        "        font-family: Arial, sans-serif;"
        "        text-align: center;"
        "        color: #333;"
        "    }\r\n"
        "    h1 {"
        "        font-size: 50px;"
        "        margin-top: 20%;"
        "    }\r\n"
        "    p {"
        "        margin-top: 40px;"
        "        font-size: 14px;"
        "        color: #666;"
        "    }\r\n"
        "</style>\r\n"
        "<body>\r\n"
        "<center>\r\n"
        "    <h1>" + statusCodeStr + " " + statusMessage + "</h1>\r\n"
        "</center>\r\n"
        "<p>matcardo e thabeck- Webserv</p>\r\n"
        "</body>\r\n"
        "</html>\r\n"
    );
}

/* Obtém a página de índice automático de um diretório */
int getAutoIndexPage(std::string &dir_name, std::vector<uint8_t> &body, size_t &body_len)
{
    struct dirent   *entityStruct;
    DIR             *directory;
    std::string     dirListPage;
    
    directory = opendir(dir_name.c_str());
    if (directory == NULL)
    {    
        std::cerr << "opendir failed" << std::endl;
        return (1);
    }
    dirListPage.append("<html lang=\"en\">\n");

    dirListPage.append("<head>\n");
    dirListPage.append("<meta charset=\"UTF-8\">\n");
    dirListPage.append("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n");
    dirListPage.append("<style>body,html{height:100%}.banner,body{display:flex}body{margin:0;font-family:Montserrat,sans-serif;font-size:15px;line-height:1.6;align-items:center}.banner{flex-direction:column;justify-content:space-between;align-items:center;width:100%%;height:100vh;background:url(./assets/images/cats_form.png) center/cover no-repeat;background-position-y:0%}.header{display:block;width:100%%;padding-top:10px;position:absolute;top:0;left:0;right:0;z-index:1000;background-color:#1c300DCC}.header_container{display:flex;justify-content:space-between;align-items:center;font-weight:700;font-size:36px;text-transform:uppercase}.nav{font-size:24px}.nav_link{display:inline-block;vertical-align:top;margin:0 24px;position:relative;color:grey;text-decoration:none;transition:color .2s linear}.nav_link:hover{color:#fff}.styled-table{width:600px;border-collapse:collapse;margin:25px 0;font-size:18px;text-align:left;box-shadow:0 0 20px rgba(0,0,0,.15)}.styled-table thead tr{background-color:#1c300d;color:#fff;text-align:left;font-weight:700}.styled-table td,.styled-table th{padding:12px 15px}.styled-table tbody tr{border-bottom:1px solid #ddd}.styled-table tbody tr:nth-of-type(2n){background-color:#f3f3f3}.styled-table tbody tr:last-of-type{border-bottom:2px solid #1c300d}.styled-table tbody tr.active-row{font-weight:700;color:#1c300d}.delete-button{background-color:#991607;color:#fff;border:none;border-radius:5px;padding:8px 12px;cursor:pointer;font-size:14px;transition:background-color .3s,transform .2s}.delete-button:hover{background-color:#c0392b;transform:translateY(-2px)}.delete-button:active{background-color:#a93226;transform:translateY(0)}footer{width:100%;padding-left:20px;color:#000}</style>\n");
    dirListPage.append("<title>Docs - Cats Army</title>\n");
    dirListPage.append("</head>\n");

    dirListPage.append("<body>\n");

    dirListPage.append("<header class=\"header\"><div class =\"header_container\"><a class = \"nav_link\" href=\"index.html\">Cats Army</a><nav class=\"nav\"><a class=\"nav_link\" href=\"about.html\">About US</a><a class=\"nav_link\" href=\"join_us.html\">JOIN US</a><a class=\"nav_link\" href=\"docs.html\">DOCS</a><a class=\"nav_link\" href=\"confidential_no_permission.html\">confidential</a></nav></div></header>\n");
    dirListPage.append("<div class=\"banner\">\n");

    dirListPage.append("<div></div>\n");
    dirListPage.append("<table class=\"styled-table\">\n");
    dirListPage.append("<thead><tr><th>File Name</th><th>Last Modification</th><th>File Size</th><th>Action</th></tr></thead>\n");
    dirListPage.append("<tbody>\n");

    struct stat file_stat;
    std::string file_path;

    while((entityStruct = readdir(directory)) != NULL)
    {
        if(strcmp(entityStruct->d_name, ".") == 0)
            continue;
        file_path = dir_name + entityStruct->d_name;
        stat(file_path.c_str() , &file_stat);
    
        dirListPage.append("<tr>\n");

        // First column
        dirListPage.append("<td>\n");
        dirListPage.append("<a href=\"");
        dirListPage.append(entityStruct->d_name);
        if (S_ISDIR(file_stat.st_mode))
            dirListPage.append("/");
        dirListPage.append("\">");
        dirListPage.append(entityStruct->d_name);
        if (S_ISDIR(file_stat.st_mode))
            dirListPage.append("/");
        dirListPage.append("</a>\n");
        dirListPage.append("</td>\n");

        // Second column
        dirListPage.append("<td>\n");
        dirListPage.append(ctime(&file_stat.st_mtime));
        dirListPage.append("</td>\n");

        // Third column
        dirListPage.append("<td>\n");
        if (!S_ISDIR(file_stat.st_mode))
            dirListPage.append(toString(file_stat.st_size));
        dirListPage.append("</td>\n");
        
        // Fourth column
        // Delete here
        dirListPage.append("<td>\n");
        dirListPage.append("<button class=\"delete-button\">Delete</button>\n");
        dirListPage.append("</td>\n");

        dirListPage.append("</tr>\n");
    }
    dirListPage.append("</tbody>\n");
    dirListPage.append("</table>\n");

    dirListPage.append("<footer><p><span class=\"highlight\">&#169; 2024 by WebServ thabeck- matcardo</p></footer>\n");
    dirListPage.append("</div>\n");

    dirListPage.append("</body>\n");
    dirListPage.append("</html>\n");

    body.insert(body.begin(), dirListPage.begin(), dirListPage.end());
    body_len = body.size();
    return (0);
}

// Checa se a URI passada é válida, ou seja, não possui caminho que vai antes da raiz
bool    checkUriPos(std::string path)
{
    std::string tmp(path);
    char *res = strtok((char*)tmp.c_str(), "/");
    int pos = 0;
    while (res != NULL)
    {
        if (!strcmp(res, ".."))
            pos--;
        else
            pos++;
        if (pos < 0)
            return (1);
        res = strtok(NULL, "/");
    }
    return (0);
}

/**
* Checa se o caractere passado é permitido em um URI de acordo com a RFC 2616
 **/
bool    allowedCharURI(uint8_t ch)
{
    if ((ch >= '#' && ch <= ';') || (ch >= '?' && ch <= '[') || (ch >= 'a' && ch <= 'z') ||
       ch == '!' || ch == '=' || ch == ']' || ch == '_' || ch == '~')
        return (true);
    return (false);
}

/* Remove os espaços em branco à esquerda e à direita de uma string */
void    trimStr(std::string &str)
{
    static const char* spaces = " \t";
    str.erase(0, str.find_first_not_of(spaces));
    str.erase(str.find_last_not_of(spaces) + 1);
}

/* Converte uma string para minúsculas */
void    toLower(std::string &str)
{
    for (size_t i = 0; i < str.length(); ++i)
        str[i] = std::tolower(str[i]);
}

/* Verifica se o arquivo existe */
bool fileExists (const std::string& f)
{
    std::ifstream file(f.c_str());
    return (file.good());
}

/* Verifica se o caminho é um diretório */
bool isDirectory(std::string path)
{
    struct stat file_stat;
    if (stat(path.c_str(), &file_stat) != 0)
        return (false);

    return (S_ISDIR(file_stat.st_mode));
}

/* Combina os caminhos dos arquivos */
std::string combinePaths(std::string p1, std::string p2, std::string p3)
{
    std::string res;
    int         len1;
    int         len2;

    len1 = p1.length();
    len2 = p2.length();
    if (p1[len1 - 1] == '/' && (!p2.empty() && p2[0] == '/') )
        p2.erase(0, 1);
    if (p1[len1 - 1] != '/' && (!p2.empty() && p2[0] != '/'))
        p1.insert(p1.end(), '/');
    if (p2[len2 - 1] == '/' && (!p3.empty() && p3[0] == '/') )
        p3.erase(0, 1);
    if (p2[len2 - 1] != '/' && (!p3.empty() && p3[0] != '/'))
        p2.insert(p1.end(), '/');
    res = p1 + p2 + p3;
    return (res);
}
