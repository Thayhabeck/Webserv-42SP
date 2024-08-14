/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: matcardo <matcardo@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/07 22:58:44 by thabeck-          #+#    #+#             */
/*   Updated: 2024/08/13 23:43:05 by matcardo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Server.hpp"

Server::Server()
{
	this->_port = 0;
	this->_host = 0;
	this->_server_name = "";
	this->_root = "";
	this->_client_max_body_size = MAX_CONTENT_LENGTH;
	this->_index = "";
	this->_listen_fd = 0;
	this->_autoindex = false;
	this->initErrorPages();
}

Server::~Server() { }

Server::Server(const Server &other)
{
	if (this != &other)
	{
		this->_server_name = other._server_name;
		this->_root = other._root;
		this->_host = other._host;
		this->_port = other._port;
		this->_client_max_body_size = other._client_max_body_size;
		this->_index = other._index;
		this->_error_pages = other._error_pages;
		this->_locations = other._locations;
		this->_listen_fd = other._listen_fd;
		this->_autoindex = other._autoindex;
		this->_server_address = other._server_address;

	}
	return ;
}

Server &Server::operator=(const Server & rhs)
{
	if (this != &rhs)
	{
		this->_server_name = rhs._server_name;
		this->_root = rhs._root;
		this->_port = rhs._port;
		this->_host = rhs._host;
		this->_client_max_body_size = rhs._client_max_body_size;
		this->_index = rhs._index;
		this->_error_pages = rhs._error_pages;
		this->_locations = rhs._locations;
		this->_listen_fd = rhs._listen_fd;
		this->_autoindex = rhs._autoindex;
		this->_server_address = rhs._server_address;
	}
	return (*this);
}

void Server::initErrorPages(void)
{
	_error_pages[301] = "";
	_error_pages[302] = "";
	_error_pages[400] = "";
	_error_pages[401] = "";
	_error_pages[402] = "";
	_error_pages[403] = "";
	_error_pages[404] = "";
	_error_pages[405] = "";
	_error_pages[413] = "";
	_error_pages[500] = "";
	_error_pages[501] = "";
	_error_pages[502] = "";
	_error_pages[503] = "";
	_error_pages[505] = "";
}

void Server::setServerName(std::string server_name)
{
	checkToken(server_name);
	this->_server_name = server_name;
}

void Server::setHost(std::string param)
{
	checkToken(param);
	if (param == "localhost")
		param = "127.0.0.1";
	if (!checkHost(param))
		throw ServerConfigErrorException("Wrong syntax: host");
	this->_host = inet_addr(param.data());
}

void Server::setRoot(std::string root)
{
	checkToken(root);
	if (ConfFile::getPathType(root) == 2)
	{
		this->_root = root;
		return ;
	}
	char dir[1024];
	getcwd(dir, 1024);
	std::string full_root = dir + root;
	if (ConfFile::getPathType(full_root) != 2)
		throw ServerConfigErrorException("Wrong syntax: root");
	this->_root = full_root;
}

void Server::setPort(std::string param)
{
	unsigned int port;
	
	port = 0;
	checkToken(param);
	for (size_t i = 0; i < param.length(); i++)
	{
		if (!std::isdigit(param[i]))
			throw ServerConfigErrorException("Wrong syntax: port");
	}
	port = ft_strtoi((param));
	if (port < 1 || port > 65636)
		throw ServerConfigErrorException("Wrong syntax: port");
	this->_port = (uint16_t) port;
}

void Server::setClientMaxBodySize(std::string param)
{
	unsigned long body_size;
	
	body_size = 0;
	checkToken(param);
	for (size_t i = 0; i < param.length(); i++)
	{
		if (param[i] < '0' || param[i] > '9')
			throw ServerConfigErrorException("Wrong syntax: client_max_body_size");
	}
	if (!ft_strtoi(param))
		throw ServerConfigErrorException("Wrong syntax: client_max_body_size");
	body_size = ft_strtoi(param);
	this->_client_max_body_size = body_size;
}

void Server::setIndex(std::string index)
{
	checkToken(index);
	this->_index = index;
}

void Server::setAutoindex(std::string autoindex)
{
	checkToken(autoindex);
	if (autoindex != "on" && autoindex != "off")
		throw ServerConfigErrorException("Wrong syntax: autoindex");
	if (autoindex == "on")
		this->_autoindex = true;
}

void Server::setErrorPages(std::vector<std::string> &param)
{
	if (param.empty())
		return;
	if (param.size() % 2 != 0)
		throw ServerConfigErrorException ("someting wrong with error pages pairs");
	for (size_t i = 0; i < param.size() - 1; i++)
	{
		for (size_t j = 0; j < param[i].size(); j++) {
			if (!std::isdigit(param[i][j]))
				throw ServerConfigErrorException("Invalid error code");
		}
		if (param[i].size() != 3)
			throw ServerConfigErrorException("Invalid error code");
		short code_error = ft_strtoi(param[i]);
		if (getStrSatusCode(code_error)  == "Undefined" || code_error < 400)
			throw ServerConfigErrorException ("Incorrect error code: " + param[i]);
		i++;
		std::string path = param[i];
		checkToken(path);
		if (ConfFile::getPathType(path) != 2)
		{
			if (ConfFile::getPathType(this->_root + path) != 1)
				throw ServerConfigErrorException ("Incorrect path for error page file: " + this->_root + path);
			if (ConfFile::isFileAccessible(this->_root + path, 0) == -1 || ConfFile::isFileAccessible(this->_root + path, 4) == -1)
				throw ServerConfigErrorException ("Error page file :" + this->_root + path + " is not accessible");
		}
		std::map<short, std::string>::iterator it = this->_error_pages.find(code_error);
		if (it != _error_pages.end())
			this->_error_pages[code_error] = path;
		else
			this->_error_pages.insert(std::make_pair(code_error, path));
	}
}

void Server::setLocation(std::string path, std::vector<std::string> param)
{
	Location                    new_location;
	std::vector<std::string>    methods;
	bool is_setted_methods = false;
	bool is_setted_autoindex = false;
	bool is_setted_max_size = false;
	int valid;

	new_location.setPath(path);
	for (size_t i = 0; i < param.size(); i++)
	{
		if (param[i] == "root" && (i + 1) < param.size())
		{
			if (!new_location.getRootLocation().empty())
				throw ServerConfigErrorException("Root of location is duplicated");
			checkToken(param[++i]);
			if (ConfFile::getPathType(param[i]) == 2)
				new_location.setRootLocation(param[i]);
			else
				new_location.setRootLocation(this->_root + param[i]);
		}
		else if ((param[i] == "allow_methods" || param[i] == "methods") && (i + 1) < param.size())
		{
			if (is_setted_methods)
				throw ServerConfigErrorException("Allow_methods of location is duplicated");
			std::vector<std::string> methods;
			while (++i < param.size())
			{
				if (param[i].find(";") != std::string::npos)
				{
					checkToken(param[i]);
					methods.push_back(param[i]);
					break ;
				}
				else
				{
					methods.push_back(param[i]);
					if (i + 1 >= param.size())
						throw ServerConfigErrorException("Token is invalid");
				}
			}
			new_location.setMethods(methods);
			is_setted_methods = true;
		}
		else if (param[i] == "autoindex" && (i + 1) < param.size())
		{
			if (path == "/cgi-bin")
				throw ServerConfigErrorException("Param autoindex not allow for CGI");
			if (is_setted_autoindex)
				throw ServerConfigErrorException("Autoindex of location is duplicated");
			checkToken(param[++i]);
			new_location.setAutoindex(param[i]);
			is_setted_autoindex = true;
		}
		else if (param[i] == "index" && (i + 1) < param.size())
		{
			if (!new_location.getIndexLocation().empty())
				throw ServerConfigErrorException("Index of location is duplicated");
			checkToken(param[++i]);
			new_location.setIndexLocation(param[i]);
		}
		else if (param[i] == "return" && (i + 1) < param.size())
		{
			if (path == "/cgi-bin")
				throw ServerConfigErrorException("Param return not allow for CGI");
			if (!new_location.getReturn().empty())
				throw ServerConfigErrorException("Return of location is duplicated");
			checkToken(param[++i]);
			new_location.setReturn(param[i]);
		}
		else if (param[i] == "alias" && (i + 1) < param.size())
		{
			if (path == "/cgi-bin")
				throw ServerConfigErrorException("Param alias not allow for CGI");
			if (!new_location.getAlias().empty())
				throw ServerConfigErrorException("Alias of location is duplicated");
			checkToken(param[++i]);
			new_location.setAlias(param[i]);
		}
		else if (param[i] == "cgi_ext" && (i + 1) < param.size())
		{
			std::vector<std::string> extension;
			while (++i < param.size())
			{
				if (param[i].find(";") != std::string::npos)
				{
					checkToken(param[i]);
					extension.push_back(param[i]);
					break ;
				}
				else
				{
					extension.push_back(param[i]);
					if (i + 1 >= param.size())
						throw ServerConfigErrorException("Token is invalid");
				}
			}
			new_location.setCgiExtension(extension);
		}
		else if (param[i] == "cgi_path" && (i + 1) < param.size())
		{
			std::vector<std::string> path;
			while (++i < param.size())
			{
				if (param[i].find(";") != std::string::npos)
				{
					checkToken(param[i]);
					path.push_back(param[i]);
					break ;
				}
				else
				{
					path.push_back(param[i]);
					if (i + 1 >= param.size())
						throw ServerConfigErrorException("Token is invalid");
				}
				if (param[i].find("/python") == std::string::npos && param[i].find("/bash") == std::string::npos)
					throw ServerConfigErrorException("cgi_path is invalid");
			}
			new_location.setCgiPath(path);
		}
		else if (param[i] == "client_max_body_size" && (i + 1) < param.size())
		{
			if (is_setted_max_size)
				throw ServerConfigErrorException("Maxbody_size of location is duplicated");
			checkToken(param[++i]);
			new_location.setMaxBodySize(param[i]);
			is_setted_max_size = true;
		}
		else if (i < param.size())
			throw ServerConfigErrorException("Param in a location is invalid");
	}
	if (new_location.getPath() != "/cgi-bin" && new_location.getIndexLocation().empty())
		new_location.setIndexLocation(this->_index);
	if (!is_setted_max_size)
		new_location.setMaxBodySize(this->_client_max_body_size);
	valid = new_location.isValidLocation(this->_root);
	if (valid == 1)
		throw ServerConfigErrorException("Failed CGI validation");
	else if (valid == 2)
		throw ServerConfigErrorException("Failed path in locaition validation");
	else if (valid == 3)
		throw ServerConfigErrorException("Failed redirection file in locaition validation");
	else if (valid == 4)
		throw ServerConfigErrorException("Failed alias file in locaition validation");
	this->_locations.push_back(new_location);
}

void	Server::setFd(int fd)
{
	this->_listen_fd = fd;
}

bool Server::checkHost(std::string host) const
{
	struct sockaddr_in sockaddr;
  	return (inet_pton(AF_INET, host.c_str(), &(sockaddr.sin_addr)) ? true : false);
}

bool Server::checkErrorPages()
{
	std::map<short, std::string>::const_iterator it;
	for (it = this->_error_pages.begin(); it != this->_error_pages.end(); it++)
	{
		if (it->first < 100 || it->first > 599)
			return (false);
		if (ConfFile::isFileAccessible(getRoot() + it->second, 0) < 0 || ConfFile::isFileAccessible(getRoot() + it->second, 4) < 0)
			return (false);
	}
	return (true);
}

const std::string &Server::getServerName()
{
	return (this->_server_name);
}

const std::string &Server::getRoot()
{
	return (this->_root);
}

const bool &Server::getAutoindex()
{
	return (this->_autoindex);
}

const in_addr_t &Server::getHost()
{
	return (this->_host);
}

const uint16_t &Server::getPort()
{
	return (this->_port);
}

const size_t &Server::getClientMaxBodySize()
{
	return (this->_client_max_body_size);
}

const std::vector<Location> &Server::getLocations()
{
	return (this->_locations);
}

const std::map<short, std::string> &Server::getErrorPages()
{
	return (this->_error_pages);
}

const std::string &Server::getIndex()
{
	return (this->_index);
}

int   	Server::getServerFd() 
{ 
	return (this->_listen_fd); 
}

const std::string &Server::getPathErrorPage(short key)
{
	std::map<short, std::string>::iterator it = this->_error_pages.find(key);
	if (it == this->_error_pages.end())
		throw ServerConfigErrorException("Error_page not found");
	return (it->second);
}

const std::vector<Location>::iterator Server::getLocationKey(std::string key)
{
	std::vector<Location>::iterator it;
	for (it = this->_locations.begin(); it != this->_locations.end(); it++)
	{
		if (it->getPath() == key)
			return (it);
	}
	throw ServerConfigErrorException("Location path not found");
}

void Server::checkToken(std::string &param)
{
	size_t pos = param.rfind(';');
	if (pos != param.size() - 1)
		throw ServerConfigErrorException("Invalid separation token");
	param.erase(pos);
}

bool Server::checkLocations() const
{
	if (this->_locations.size() < 2)
		return (false);
	std::vector<Location>::const_iterator it1;
	std::vector<Location>::const_iterator it2;
	for (it1 = this->_locations.begin(); it1 != this->_locations.end() - 1; it1++) {
		for (it2 = it1 + 1; it2 != this->_locations.end(); it2++) {
			if (it1->getPath() == it2->getPath())
				return (true);
		}
	}
	return (false);
}

void	Server::setupServer(void)
{
	if ((_listen_fd = socket(AF_INET, SOCK_STREAM, 0) )  == -1 )
    {
        std::cerr << RED "webserv: socket error" RESET << std::endl;
        exit(EXIT_FAILURE);
    }

    int option_value = 1;
    setsockopt(_listen_fd, SOL_SOCKET, SO_REUSEADDR, &option_value, sizeof(int));
    memset(&_server_address, 0, sizeof(_server_address));
    _server_address.sin_family = AF_INET;
    _server_address.sin_addr.s_addr = _host;
    _server_address.sin_port = htons(_port);
    if (bind(_listen_fd, (struct sockaddr *) &_server_address, sizeof(_server_address)) == -1)
    {
        std::cerr << RED "webserv: bind error" RESET << std::endl;
        exit(EXIT_FAILURE);
    }
}
