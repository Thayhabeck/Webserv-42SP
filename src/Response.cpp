/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thabeck- <thabeck-@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/09 19:09:17 by thabeck-          #+#    #+#             */
/*   Updated: 2024/08/10 01:07:07 by thabeck-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../inc/Response.hpp"

Response::Response()
{
    _target_file = "";
    _body_length = 0;
    _response_content = "";
    _response_body = "";
    _location = "";
    _code = 0;
    _cgi = 0;
    _cgi_response_length = 0;
    _auto_index = 0;
    fillMimeTypes();
}

Response::~Response() {}

Response::Response(Request &req)
{
    request = req;
    _target_file = "";
    _body_length = 0;
    _response_content = "";
    _response_body = "";
    _location = "";
    _code = 0;
    _cgi = 0;
    _cgi_response_length = 0;
    _auto_index = 0;
    fillMimeTypes();
}

/* Monta o header do response */
void    Response::setHeaders()
{
    headerContentType();
    headerContentLength();
    headerConnection();
    headerServer();
    headerLocation();
    headerDate();

    _response_content.append("\r\n");
}

/* Procura no arquivo o tipo de conteúdo com base na extensão do arquivo e seta no header do response */
void   Response::headerContentType()
{
    _response_content.append("Content-Type: ");
    if(_target_file.rfind(".", std::string::npos) != std::string::npos && _code == 200)
        _response_content.append(getMimeType(_target_file.substr(_target_file.rfind(".", std::string::npos))) );
    else
        _response_content.append(getMimeType("default"));
    _response_content.append("\r\n");
}

/* Seta o tamanho do conteúdo no header do response */
void   Response::headerContentLength()
{
    std::stringstream ss;
    ss << _response_body.length();
    _response_content.append("Content-Length: ");
    _response_content.append(ss.str());
    _response_content.append("\r\n");
}

/* Seta o estado da conexão no header do response */
void   Response::headerConnection()
{
    if(request.getHeader("connection") == "keep-alive")
        _response_content.append("Connection: keep-alive\r\n");
}

/* Seta o servidor no header do response */
void   Response::headerServer()
{
    _response_content.append("Server: MathayWebserv\r\n");
}

/* Seta o location no header do response */
void    Response::headerLocation()
{
    if (_location.length())
        _response_content.append("Location: "+ _location +"\r\n");
}

/* Seta a data e hora no header do response */
void    Response::headerDate()
{
    char date[1000];
    time_t now = time(0);
    struct tm tm = *gmtime(&now);
    strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S %Z", &tm);
    _response_content.append("Date: ");
    _response_content.append(date);
    _response_content.append("\r\n");

}

/* Verifica se o pipeline foi bem sucedido, se não, retorna 500
*  Se o pipeline foi bem sucedido, então retorna 0 */
int Response::handleCgiTemp(std::string &location_key)
{
    std::string path;
    path = _target_file;
    _cgi_obj.clearCgiEnv();
    _cgi_obj.setCgiPath(path);
    _cgi = 1;
    if (pipe(_cgi_fd) < 0)
    {
        _code = 500;
        return (1);
    }
    _cgi_obj.initGenericCgiEnv(request, _server.getLocationKey(location_key)); // + URI
    _cgi_obj.cgiExec(this->_code);
    return (0);
}

/* Verifica se o método é permitido */
bool isAllowedMethod(HttpMethod &method, Location &location, short &code)
{
    std::vector<short> methods = location.getMethods();
    if ((method == GET && !methods[0]) || (method == POST && !methods[1]) || (method == DELETE && !methods[2]))
    {
        code = 405;
        return (1);
    }
    return (0);
}

/* Checa se a localização tem um retorno e seta o código e a localização */
bool    checkLocationReturn(Location &loc, short &code, std::string &location)
{
    if (!loc.getReturn().empty())
    {
        code = 301;
        location = loc.getReturn();
        if (location[0] != '/')
            location.insert(location.begin(), '/');
        return (1);
    }
    return (0);
}

/* Substitui o alias do location no caminho do arquivo */
void replaceLocationAlias(Location &location, Request &request, std::string &target_file)
{
    target_file = combinePaths(location.getAlias(), request.getPath().substr(location.getPath().length()), "");
}

/* Adiciona o root ao caminho do arquivo */
void appendRoot(Location &location, Request &request, std::string &target_file)
{
    target_file = combinePaths(location.getRootLocation(), request.getPath(), "");
}

/* Manipula o arquivo CGI, verifica se a extensão é suportada, se o arquivo existe e é executável, então executa o CGI */
int        Response::handleCgi(std::string &location_key)
{
    std::string path;
    std::string exten;
    size_t      pos;

    path = this->request.getPath();
    if (path[0] && path[0] == '/')
        path.erase(0, 1);
    if (path == "cgi-bin")
        path += "/" + _server.getLocationKey(location_key)->getIndexLocation();
    else if (path == "cgi-bin/")
        path.append(_server.getLocationKey(location_key)->getIndexLocation());

    pos = path.find(".");
    if (pos == std::string::npos)
    {
        _code = 501;
        return (1);
    }
    exten = path.substr(pos);
    if (exten != ".py" && exten != ".sh")
    {
        _code = 501;
        return (1);
    }
    if (ConfFile::getPathType(path) != 1)
    {
        _code = 404;
        return (1);
    }
    if (ConfFile::isFileAccessible(path, 1) == -1 || ConfFile::isFileAccessible(path, 3) == -1)
    {
        _code = 403;
        return (1);
    }
    if (isAllowedMethod(request.getMethod(), *_server.getLocationKey(location_key), _code))
        return (1);
    _cgi_obj.clearCgiEnv();
    _cgi_obj.setCgiPath(path);
    _cgi = 1;
    if (pipe(_cgi_fd) < 0)
    {
        _code = 500;
        return (1);
    }
    _cgi_obj.initCgiEnv(request, _server.getLocationKey(location_key));
    _cgi_obj.cgiExec(this->_code);
    return (0);
}

/* Compara o URI com as localizações do arquivo de configuração e tenta encontrar a melhor correspondência.
 Se a correspondência for encontrada, então location_key é definido para essa localização, 
caso contrário, location_key será uma string vazia.*/
static void    getLocationMatch(std::string &path, std::vector<Location> locations, std::string &location_key)
{
    size_t biggest_match = 0;

    for(std::vector<Location>::const_iterator it = locations.begin(); it != locations.end(); ++it)
    {
        if(path.find(it->getPath()) == 0)
        {
               if( it->getPath() == "/" || path.length() == it->getPath().length() || path[it->getPath().length()] == '/')
               {
                    if(it->getPath().length() > biggest_match)
                    {
                        biggest_match = it->getPath().length();
                        location_key = it->getPath();
                    }
               }
        }
    }
}

/* Manipula o arquivo de destino, verifica se é um diretório, se for um diretório, 
* verifica se tem um arquivo de índice, caso contrário, verifica se a indexação automática está habilitada, caso contrário, retorna 403.*/
int    Response::handleTarget()
{
    std::string location_key;
    getLocationMatch(request.getPath(), _server.getLocations(), location_key);
    if (location_key.length() > 0)
    {
        Location target_location = *_server.getLocationKey(location_key);

        if (isAllowedMethod(request.getMethod(), target_location, _code))
        {
            std::cout << "METHOD NOT ALLOWED \n";
            return (1);
        }
        if (request.getBody().length() > target_location.getMaxBodySize())
        {
            _code = 413;
            return (1);
        }
        if (checkLocationReturn(target_location, _code, _location))
            return (1);

		if (target_location.getPath().find("cgi-bin") != std::string::npos)
		{
            return (handleCgi(location_key));
		}

        if (!target_location.getAlias().empty())
        {
            replaceLocationAlias(target_location, request, _target_file);
        }
        else
            appendRoot(target_location, request, _target_file);

        if (!target_location.getCgiExtension().empty())
        {

            if (_target_file.rfind(target_location.getCgiExtension()[0]) != std::string::npos)
            {
                return (handleCgiTemp(location_key));
            }

        }
        if (isDirectory(_target_file))
        {
            if (_target_file[_target_file.length() - 1] != '/')
            {
                _code = 301;
                _location = request.getPath() + "/";
                return (1);
            }
            if (!target_location.getIndexLocation().empty())
                _target_file += target_location.getIndexLocation();
            else
                _target_file += _server.getIndex();
            if (!fileExists(_target_file))
            {
                if (target_location.getAutoindex())
                {
                    _target_file.erase(_target_file.find_last_of('/') + 1);
                    _auto_index = true;
                    return (0);
                }
                else
                {
                    _code = 403;
                    return (1);
                }
            }
            if (isDirectory(_target_file))
            {
                _code = 301;
                if (!target_location.getIndexLocation().empty())
                    _location = combinePaths(request.getPath(), target_location.getIndexLocation(), "");
                else
                    _location = combinePaths(request.getPath(), _server.getIndex(), "");
                if (_location[_location.length() - 1] != '/')
                    _location.insert(_location.end(), '/');

                return (1);
            }
        }
    }
    else
    {
        _target_file = combinePaths(_server.getRoot(), request.getPath(), "");
        if (isDirectory(_target_file))
        {
            if (_target_file[_target_file.length() - 1] != '/')
            {
                _code = 301;
                _location = request.getPath() + "/";
                return (1);
            }
            _target_file += _server.getIndex();
            if (!fileExists(_target_file))
            {
                _code = 403;
                return (1);
            }
            if (isDirectory(_target_file))
            {
                _code = 301;
                _location = combinePaths(request.getPath(), _server.getIndex(), "");
                if(_location[_location.length() - 1] != '/')
                    _location.insert(_location.end(), '/');
                return (1);
            }
        }
    }
    return (0);
}

bool Response::requestError()
{
    if(request.getErrorCode())
    {
        _code = request.getErrorCode();
        return (1);
    }
    return (0);
}

/* Retornar a página de erro padrão*/
void Response::buildErrorBody()
{
        if( !_server.getErrorPages().count(_code) || _server.getErrorPages().at(_code).empty() ||
         request.getMethod() == DELETE || request.getMethod() == POST)
        {
            // Insere a página de erro padrão que é uma página HTML com o código de erro e a mensagem de erro
            setServerDefaultErrorPages();
        }
        else
        {
            // Se a página de erro não for padrão, então redireciona para a página de erro
            if(_code >= 400 && _code < 500)
            {
                _location = _server.getErrorPages().at(_code);
                if(_location[0] != '/')
                    _location.insert(_location.begin(), '/');
                _code = 302;
            }

            _target_file = _server.getRoot() +_server.getErrorPages().at(_code);
            short old_code = _code;
            if(readFile())
            {
                _code = old_code;
                _response_body = getErrorPage(_code);
            }
        }

}

void    Response::buildResponse()
{
    if (requestError() || buildResponseBody())
        buildErrorBody();
    if (_cgi)
        return ;
    else if (_auto_index)
    {
        std::cout << CYAN "AUTO index page" RESET << std::endl;
        if (getAutoIndexPage(_target_file, _body, _body_length))
        {
            _code = 500;
            buildErrorBody();
        }
        else
            _code = 200;
        _response_body.insert(_response_body.begin(), _body.begin(), _body.end());
    }
    setStatusLine();
    setHeaders();
    if (request.getMethod() == GET || _code != 200)
        _response_content.append(_response_body);
}

/* Retorna o response inteiro ( Headers + Body ) */
std::string Response::getResponse()
{
    return (_response_content);
}

/* Retorna o tamanho do response inteiro ( Headers + Body ) */
size_t Response::getResponseLen() const
{
	return (_response_content.length());
}

/* Constroi a linha de status com base no código de status. 
* Formato da linha de status: HTTP/1.1 <código de status> <mensagem de status> 
* Exemplo: HTTP/1.1 200 OK */
void        Response::setStatusLine()
{
    _response_content.append("HTTP/1.1 " + toString(_code) + " ");
    _response_content.append(getStrSatusCode(_code));
    _response_content.append("\r\n");
}

void     Response::setServer(Server &server)
{
    _server = server;
}

void    Response::setRequest(Request &req)
{
    request = req;
}

void      Response::setCgiState(int state)
{
    _cgi = state;
}

void Response::setResponseError(short code)
{
    _response_content = "";
    _code = code;
    _response_body = "";
    buildErrorBody();
    setStatusLine();
    setHeaders();
    _response_content.append(_response_body);
}

void Response::setServerDefaultErrorPages()
{
    _response_body = getErrorPage(_code);
}

/* Constroi o corpo da resposta */
int    Response::buildResponseBody()
{
    if (request.getBody().length() > _server.getClientMaxBodySize())
    {
        _code = 413;
        return (1);
    }
    if ( handleTarget() )
        return (1);
    if (_cgi || _auto_index)
        return (0);
    if (_code)
        return (0);
    if (request.getMethod() == GET)
    {
        if (readFile())
            return (1);
    }
     else if (request.getMethod() == POST)
    {
        if (fileExists(_target_file) && request.getMethod() == POST)
        {
            _code = 204;
            return (0);
        }
        std::ofstream file(_target_file.c_str(), std::ios::binary);
        if (file.fail())
        {
            _code = 404;
            return (1);
        }

        if (request.getUploadFlag())
        {
            std::string body = request.getBody();
            body = removeUploadBoundary(body, request.getUploadBoundary());
            file.write(body.c_str(), body.length());
        }
        else
        {
            file.write(request.getBody().c_str(), request.getBody().length());
        }
    }
    else if (request.getMethod() == DELETE)
    {
        if (!fileExists(_target_file))
        {
            _code = 404;
            return (1);
        }
        if (remove( _target_file.c_str() ) != 0 )
        {
            _code = 500;
            return (1);
        }
    }
    _code = 200;
    return (0);
}

/* Lê o arquivo e armazena o conteúdo no corpo da resposta */
int Response::readFile()
{
    std::ifstream file(_target_file.c_str());

    if (file.fail())
    {
        _code = 404;
        return (1);
    }
    std::ostringstream ss;
	ss << file.rdbuf();
    _response_body = ss.str();
    return (0);
}

void        Response::breakResponse(size_t i)
{
    _response_content = _response_content.substr(i);
}

void   Response::clearResponse()
{
    _target_file.clear();
    _body.clear();
    _body_length = 0;
    _response_content.clear();
    _response_body.clear();
    _location.clear();
    _code = 0;
    _cgi = 0;
    _cgi_response_length = 0;
    _auto_index = 0;
}

int      Response::getCode() const
{
    return (_code);
}

int    Response::getCgiState()
{
    return (_cgi);
}

/* Remove o boundary do upload do corpo da requisição */
std::string Response::removeUploadBoundary(std::string &body, std::string &boundary)
{
    std::string buffer;
    std::string new_body;
    std::string filename;
    bool is_boundary = false;
    bool is_content = false;

    if (body.find("--" + boundary) != std::string::npos && body.find("--" + boundary + "--") != std::string::npos)
    {
        for (size_t i = 0; i < body.size(); i++)
        {
            buffer.clear();
            while(body[i] != '\n')
            {
                buffer += body[i];
                i++;
            }
            if (!buffer.compare(("--" + boundary + "--\r")))
            {
                is_content = true;
                is_boundary = false;
            }
            if (!buffer.compare(("--" + boundary + "\r")))
            {
                is_boundary = true;
            }
            if (is_boundary)
            {
                if (!buffer.compare(0, 31, "Content-Disposition: form-data;"))
                {
                    size_t start = buffer.find("filename=\"");
                    if (start != std::string::npos)
                    {
                        size_t end = buffer.find("\"", start + 10);
                        if (end != std::string::npos)
                            filename = buffer.substr(start + 10, end);
                    }
                }
                else if (!buffer.compare(0, 1, "\r") && !filename.empty())
                {
                    is_boundary = false;
                    is_content = true;
                }

            }
            else if (is_content)
            {
                if (!buffer.compare(("--" + boundary + "\r")))
                {
                    is_boundary = true;
                }
                else if (!buffer.compare(("--" + boundary + "--\r")))
                {
                    new_body.erase(new_body.end() - 1);
                    break ;
                }
                else
                    new_body += (buffer + "\n");
            }

        }
    }

    body.clear();
    return (new_body);
}

/* Preenche o map com os tipos MIME */
void Response::fillMimeTypes()
{
    _mime[".html"] = "text/html";
    _mime[".css"] = "text/css";
    _mime[".js"] = "text/javascript";
    _mime[".jpg"] = "image/jpeg";
    _mime[".jpeg"] = "image/jpeg";
    _mime[".png"] = "image/png";
    _mime[".gif"] = "image/gif";
    _mime[".bmp"] = "image/bmp";
    _mime[".ico"] = "image/x-icon";
    _mime[".svg"] = "image/svg+xml";
    _mime[".mp3"] = "audio/mpeg";
    _mime[".mp4"] = "video/mp4";
    _mime[".webm"] = "video/webm";
    _mime[".ogg"] = "audio/ogg";
    _mime[".wav"] = "audio/wav";
    _mime[".avi"] = "video/x-msvideo";
    _mime[".mpeg"] = "video/mpeg";
    _mime[".txt"] = "text/plain";
    _mime[".pdf"] = "application/pdf";
    _mime[".zip"] = "application/zip";
    _mime[".tar"] = "application/x-tar";
    _mime[".rar"] = "application/x-rar-compressed";
    _mime[".tar.gz"] = "application/gzip";
    _mime[".7z"] = "application/x-7z-compressed";
    _mime[".xml"] = "application/xml";
    _mime[".json"] = "application/json";
    _mime[".woff"] = "application/font-woff";
    _mime[".csv"] = "text/csv";
    _mime[".xls"] = "application/vnd.ms-excel";
    _mime[".xlsx"] = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
    _mime["default"] = "text/html";
}

std::string Response::getMimeType(std::string extension)
{
    if (_mime.count(extension))
        return (_mime[extension]);
    return (_mime["default"]);
}
