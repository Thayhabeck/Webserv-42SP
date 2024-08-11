/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: matcardo <matcardo@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/07 21:48:04 by thabeck-          #+#    #+#             */
/*   Updated: 2024/08/10 17:56:48 by matcardo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Location.hpp"

Location::Location()
{
	this->_path = "";
	this->_root = "";
	this->_autoindex = false;
	this->_index = "";
	this->_return = "";
	this->_alias = "";
	this->_client_max_body_size = MAX_CONTENT_LENGTH;
	this->_allowed_methods.reserve(5);
	this->_allowed_methods.push_back(1); //GET
	this->_allowed_methods.push_back(0); //POST
	this->_allowed_methods.push_back(0); //DELETE
}

Location::Location(const Location &other)
{
	this->_path = other._path;
	this->_root = other._root;
	this->_autoindex = other._autoindex;
	this->_index = other._index;
	this->_cgi_path = other._cgi_path;
	this->_cgi_ext = other._cgi_ext;
	this->_return = other._return;
	this->_alias = other._alias;
    this->_allowed_methods = other._allowed_methods;
	this->_ext_path = other._ext_path;
	this->_client_max_body_size = other._client_max_body_size;
}

Location &Location::operator=(const Location &rhs)
{
	if (this != &rhs)
	{
		this->_path = rhs._path;
		this->_root = rhs._root;
		this->_autoindex = rhs._autoindex;
		this->_index = rhs._index;
		this->_cgi_path = rhs._cgi_path;
		this->_cgi_ext = rhs._cgi_ext;
		this->_return = rhs._return;
		this->_alias = rhs._alias;
		this->_allowed_methods = rhs._allowed_methods;
		this->_ext_path = rhs._ext_path;
		this->_client_max_body_size = rhs._client_max_body_size;
    }
	return (*this);
}

Location::~Location() { }

/* Setters */
void Location::setPath(std::string param)
{
	this->_path = param;
}

void Location::setRootLocation(std::string param)
{
	if (ConfFile::getPathType(param) != 2) //Root tem que ser um diretório
		throw Server::ServerConfigErrorException("Location root is not a directory");
	this->_root = param;
}

void Location::setMethods(std::vector<std::string> methods)
{
	this->_allowed_methods[0] = 0;
	this->_allowed_methods[1] = 0;

	for (size_t i = 0; i < methods.size(); i++)
	{
		if (methods[i] == "GET")
			this->_allowed_methods[0] = 1;
		else if (methods[i] == "POST")
			this->_allowed_methods[1] = 1;
		else if (methods[i] == "DELETE")
			this->_allowed_methods[2] = 1;
		else
			throw Server::ServerConfigErrorException("Allow method not supported " + methods[i]);
	}
}

void Location::setAutoindex(std::string param)
{
	if (param == "on" || param == "off")
		this->_autoindex = (param == "on");
	else
		throw Server::ServerConfigErrorException("Autoindex value must be on/off");
}

void Location::setIndexLocation(std::string param)
{
	this->_index = param;
}

void Location::setReturn(std::string param)
{
	this->_return = param;
}

void Location::setAlias(std::string param)
{
	this->_alias = param;
}

void Location::setCgiPath(std::vector<std::string> path)
{
	this->_cgi_path = path;
}

void Location::setCgiExtension(std::vector<std::string> extension)
{
	this->_cgi_ext = extension;
}

void Location::setMaxBodySize(std::string param)
{
	unsigned long body_size = 0;

	for (size_t i = 0; i < param.length(); i++)
	{
		if (param[i] < '0' || param[i] > '9')
			throw Server::ServerConfigErrorException("client_max_body_size must be a number");
	}
	if (!ft_strtoi(param))
		throw Server::ServerConfigErrorException("client_max_body_size must be a number");
	body_size = ft_strtoi(param);
	this->_client_max_body_size = body_size;
}

void Location::setMaxBodySize(unsigned long param)
{
	this->_client_max_body_size = param;
}

/* getters */
const std::string &Location::getPath() const
{
	return (this->_path);
}

const std::string &Location::getRootLocation() const
{
	return (this->_root);
}

const std::string &Location::getIndexLocation() const
{
	return (this->_index);
}

const std::vector<short> &Location::getMethods() const
{
	return (this->_allowed_methods);
}

const std::vector<std::string> &Location::getCgiPath() const
{
	return (this->_cgi_path);
}

const std::vector<std::string> &Location::getCgiExtension() const
{
	return (this->_cgi_ext);
}

const bool &Location::getAutoindex() const
{
	return (this->_autoindex);
}

const std::string &Location::getReturn() const
{
	return (this->_return);
}

const std::string &Location::getAlias() const
{
	return (this->_alias);
}

const std::map<std::string, std::string> &Location::getExtensionPath() const
{
	return (this->_ext_path);
}

const unsigned long &Location::getMaxBodySize() const
{
	return (this->_client_max_body_size);
}

// Valida o location
int Location::isValidLocation(std::string &serverRoot)
{
	Location& location = *this; // O objeto location será o próprio objeto que chamou a função
	if (location.getPath() == "/cgi-bin")
	{
		if (location.getCgiPath().empty() || location.getCgiExtension().empty() || location.getIndexLocation().empty())
			return (1);
		if (ConfFile::isFileAccessible(location.getIndexLocation(), 4) < 0)
		{
			std::string path = location.getRootLocation() + location.getPath() + "/" + location.getIndexLocation();
			if (ConfFile::getPathType(path) != 1)
			{				
				std::string root = getcwd(NULL, 0);
				location.setRootLocation(root);
				path = root + location.getPath() + "/" + location.getIndexLocation();
			}
			if (path.empty() || ConfFile::getPathType(path) != 1 || ConfFile::isFileAccessible(path, 4) < 0)
				return (1);
		}
		if (location.getCgiPath().size() != location.getCgiExtension().size())
			return (1);
		std::vector<std::string>::const_iterator it;
		for (it = location.getCgiPath().begin(); it != location.getCgiPath().end(); ++it)
		{
			if (ConfFile::getPathType(*it) < 0)
				return (1);
		}
		std::vector<std::string>::const_iterator it_path;
		for (it = location.getCgiExtension().begin(); it != location.getCgiExtension().end(); ++it)
		{
			std::string tmp = *it;
			if (tmp != ".py" && tmp != ".sh" && tmp != "*.py" && tmp != "*.sh")
				return (1);
			for (it_path = location.getCgiPath().begin(); it_path != location.getCgiPath().end(); ++it_path)
			{
				std::string tmp_path = *it_path;
				if (tmp == ".py" || tmp == "*.py")
				{
					if (tmp_path.find("python") != std::string::npos)
						location._ext_path.insert(std::make_pair(".py", tmp_path));
				}
				else if (tmp == ".sh" || tmp == "*.sh")
				{
					if (tmp_path.find("bash") != std::string::npos)
						location._ext_path[".sh"] = tmp_path;
				}
			}
		}
		if (location.getCgiPath().size() != location.getExtensionPath().size())
			return (1);
	}
	else
	{
		if (location.getPath()[0] != '/')
			return (2);
		if (location.getRootLocation().empty()) {
			location.setRootLocation(serverRoot);
		}
		if (ConfFile::checkFile(location.getRootLocation() + location.getPath() + "/", location.getIndexLocation()))
			return (5);
		if (!location.getReturn().empty())
		{
			if (ConfFile::checkFile(location.getRootLocation(), location.getReturn()))
				return (3);
		}
		if (!location.getAlias().empty())
		{
			if (ConfFile::checkFile(location.getRootLocation(), location.getAlias()))
			 	return (4);
		}
	}
	return (0);
}

/* Função para debbugar */
std::string Location::printMethods() const
{
	std::string res;
	if (_allowed_methods[2])
	{
		if (!res.empty())
			res.insert(0, ", ");
		res.insert(0, "DELETE");
	}
	if (_allowed_methods[1])
	{
		if (!res.empty())
			res.insert(0, ", ");
		res.insert(0, "POST");
	}
	if (_allowed_methods[0])
	{
		if (!res.empty())
			res.insert(0, ", ");
		res.insert(0, "GET");
	}
	return (res);
}
