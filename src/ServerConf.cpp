/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConf.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: matcardo <matcardo@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/07 20:54:50 by thabeck-          #+#    #+#             */
/*   Updated: 2024/08/13 23:01:35 by matcardo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/ServerConf.hpp"

ServerConf::ServerConf()
{
	this->_nb_server = 0;
}

ServerConf::~ServerConf() { }

int ServerConf::createServerPool(const std::string &config_file)
{
	std::string		content;
	ConfFile		file(config_file);

	if (file.getPathType(file.getPath()) != 1)
		throw ServerConfigErrorException("Configuration File is invalid");
	if (file.isFileAccessible(file.getPath(), 4) == -1)
		throw ServerConfigErrorException("Configuration File is not accessible");
	content = file.readFile(config_file);
	if (content.empty())
		throw ServerConfigErrorException("Configuration File is empty");
	rmComments(content);
	rmSpaces(content);
	splitServerConfStr(content);
	if (this->_server_config.size() != this->_nb_server)
		throw ServerConfigErrorException("Failed server config validation");
	for (size_t i = 0; i < this->_nb_server; i++)
	{
		Server server;
		createServer(this->_server_config[i], server);
		this->_servers.push_back(server);
	}
	if (this->_nb_server > 1)
		checkCompareServersParams();
	return (0);
}

void ServerConf::rmComments(std::string &content)
{
	size_t pos;

	pos = content.find('#');
	while (pos != std::string::npos)
	{
		size_t pos_end;
		pos_end = content.find('\n', pos);
		content.erase(pos, pos_end - pos);
		pos = content.find('#');
	}
}

void ServerConf::rmSpaces(std::string &content)
{
	size_t	i = 0;

	while (content[i] && isspace(content[i]))
		i++;
	content = content.substr(i);
	i = content.length() - 1;
	while (i > 0 && isspace(content[i]))
		i--;
	content = content.substr(0, i + 1);
}

void ServerConf::splitServerConfStr(std::string &content)
{
	size_t start = 0;
	size_t end = 1;

	if (content.find("server", 0) == std::string::npos)
		throw ServerConfigErrorException("Server not found");
	while (start != end && start < content.length())
	{
		start = getServerStartPosition(start, content);
		end = getServerEndPosition(start, content);
		if (start == end)
			throw ServerConfigErrorException("Failed server config validation");
		this->_server_config.push_back(content.substr(start, end - start + 1));
		this->_nb_server++;
		start = end + 1;
	}
}

size_t ServerConf::getServerStartPosition (size_t start, std::string &content)
{
	size_t i;

	for (i = start; content[i]; i++)
	{
		if (content[i] == 's')
			break ;
		if (!isspace(content[i]))
			throw  ServerConfigErrorException("Server scope contains a character not allowed");
	}
	if (!content[i])
		return (start);
	if (content.compare(i, 6, "server") != 0)
		throw ServerConfigErrorException("Server scope contains a character not allowed");
	i += 6;
	while (content[i] && isspace(content[i]))
		i++;
	if (content[i] == '{')
		return (i);
	else
		throw  ServerConfigErrorException("Server scope contains a character not allowed");

}

size_t ServerConf::getServerEndPosition (size_t start, std::string &content)
{
	size_t	i;
	size_t	scope;
	
	scope = 0;
	for (i = start + 1; content[i]; i++)
	{
		if (content[i] == '{')
			scope++;
		if (content[i] == '}')
		{
			if (!scope)
				return (i);
			scope--;
		}
	}
	return (start);
}

std::vector<std::string> splitStrParams(std::string line, std::string sep)
{
	std::vector<std::string>	str;
	std::string::size_type		start, end;

	start = end = 0;
	while (1)
	{
		end = line.find_first_of(sep, start);
		if (end == std::string::npos)
			break;
		std::string tmp = line.substr(start, end - start);
		str.push_back(tmp);
		start = line.find_first_not_of(sep, end);
		if (start == std::string::npos)
			break;
	}
	return (str);
}

void ServerConf::createServer(std::string &config, Server &server)
{
	std::vector<std::string>	params;
	std::vector<std::string>	error_codes;
	int		is_setted_location = 1;
	bool	is_setted_autoindex = false;
	bool	is_setted_max_size = false;

	params = splitStrParams(config += ' ', std::string(" \n\t"));
	if (params.size() < 3)
		throw  ServerConfigErrorException("Failed server config validation");
	for (size_t i = 0; i < params.size(); i++)
	{
		if (params[i] == "listen" && (i + 1) < params.size() && is_setted_location)
		{
			if (server.getPort())
				throw  ServerConfigErrorException("Duplicated parameter 'Port'");
			server.setPort(params[++i]);
		}
		else if (params[i] == "host" && (i + 1) < params.size() && is_setted_location)
		{
			if (server.getHost())
				throw  ServerConfigErrorException("Duplicate parameter 'Host'");
			server.setHost(params[++i]);
		}
		else if (params[i] == "root" && (i + 1) < params.size() && is_setted_location)
		{
			if (!server.getRoot().empty())
				throw  ServerConfigErrorException("Duplicate parameter 'Root'");
			server.setRoot(params[++i]);
		}
		else if (params[i] == "error_page" && (i + 1) < params.size() && is_setted_location)
		{
			while (++i < params.size())
			{
				error_codes.push_back(params[i]);
				if (params[i].find(';') != std::string::npos)
					break ;
				if (i + 1 >= params.size())
					throw ServerConfigErrorException("Error Page scope contains a character not allowed");
			}
			server.setErrorPages(error_codes);
		}
		else if (params[i] == "client_max_body_size" && (i + 1) < params.size() && is_setted_location)
		{
			if (is_setted_max_size)
				throw  ServerConfigErrorException("Duplicated parameter 'Max Body Size'");
			server.setClientMaxBodySize(params[++i]);
			is_setted_max_size = true;
		}
		else if (params[i] == "server_name" && (i + 1) < params.size() && is_setted_location)
		{
			if (!server.getServerName().empty())
				throw  ServerConfigErrorException("Duplicated parameter 'Server Name'");
			server.setServerName(params[++i]);
		}
		else if (params[i] == "index" && (i + 1) < params.size() && is_setted_location)
		{
			if (!server.getIndex().empty())
				throw  ServerConfigErrorException("Duplicated parameter 'Index'");
			server.setIndex(params[++i]);
		}
		else if (params[i] == "autoindex" && (i + 1) < params.size() && is_setted_location)
		{
			if (is_setted_autoindex)
				throw ServerConfigErrorException("Duplicated parameter 'Autoindex'");
			server.setAutoindex(params[++i]);
			is_setted_autoindex = true;
		}
		else if (params[i] == "location" && (i + 1) < params.size())
		{
			std::string	path;
			i++;
			if (params[i] == "{" || params[i] == "}")
				throw  ServerConfigErrorException("location scope contains a character not allowed");
			path = params[i];
			std::vector<std::string> location_params;
			if (params[++i] != "{")
				throw  ServerConfigErrorException("location scope contains a character not allowed");
			i++;
			while (i < params.size() && params[i] != "}")
				location_params.push_back(params[i++]);
			server.setLocation(path, location_params);
			if (i < params.size() && params[i] != "}")
				throw  ServerConfigErrorException("location scope contains a character not allowed");
			is_setted_location = 0;
		}
		else if (params[i] != "}" && params[i] != "{")
		{
			if (!is_setted_location)
				throw  ServerConfigErrorException("Has a parameter after location");
			else
				throw  ServerConfigErrorException("Unsupported directive");
		}
	}
	if (server.getRoot().empty())
		server.setRoot("/;");
	if (server.getHost() == 0)
		server.setHost("localhost;");
	if (server.getIndex().empty())
		server.setIndex("index.html;");
	if (ConfFile::checkFile(server.getRoot(), server.getIndex()))
		throw ServerConfigErrorException("Config File Index inacessible");
	if (server.checkLocations())
		throw ServerConfigErrorException("Duplicated location");
	if (!server.getPort())
		throw ServerConfigErrorException("Port not found");
	if (!server.checkErrorPages())
		throw ServerConfigErrorException("Error Pages not found or invalid");
}

int	ServerConf::strCompare(std::string str1, std::string str2, size_t pos)
{
	size_t	i;

	i = 0;
	while (pos < str1.length() && i < str2.length() && str1[pos] == str2[i])
	{
		pos++;
		i++;
	}
	if (i == str2.length() && pos <= str1.length() && (str1.length() == pos || isspace(str1[pos])))
		return (0);
	return (1);
}

void ServerConf::checkCompareServersParams()
{
	std::vector<Server>::iterator it1;
	std::vector<Server>::iterator it2;

	for (it1 = this->_servers.begin(); it1 != this->_servers.end() - 1; it1++)
	{
		for (it2 = it1 + 1; it2 != this->_servers.end(); it2++)
		{
			if (it1->getPort() == it2->getPort() && it1->getHost() == it2->getHost() && it1->getServerName() == it2->getServerName())
				throw ServerConfigErrorException("Servers with same port, host or server name");
		}
	}
}

std::vector<Server>	ServerConf::getServerPool()
{
	return (this->_servers);
}
