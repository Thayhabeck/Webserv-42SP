/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfFile.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: matcardo <matcardo@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/07 18:33:05 by thabeck-          #+#    #+#             */
/*   Updated: 2024/08/13 22:53:14 by matcardo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/ConfFile.hpp"

ConfFile::ConfFile() : _size(0) { }

ConfFile::ConfFile(std::string const path) : _path(path), _size(0) { }

ConfFile::~ConfFile() { }

int ConfFile::getPathType(std::string const path)
{
	struct stat	fileInfo;
	
	if (stat(path.c_str(), &fileInfo) == 0)
	{
		if (fileInfo.st_mode & S_IFREG)
			return (1);
		else if (fileInfo.st_mode & S_IFDIR)
			return (2);
		else
			return (3);
	}
	else
		return (-1);
}

std::string	ConfFile::readFile(std::string path)
{
	if (path.empty() || path.length() == 0)
		return (NULL);
	std::ifstream conf_file(path.c_str());
	if (!conf_file || !conf_file.is_open())
		return (NULL);

	std::stringstream stream;
	stream << conf_file.rdbuf();
	return (stream.str());
}

int ConfFile::checkFile(std::string const path, std::string const index)
{
	if (getPathType(index) == 1 && isFileAccessible(index, 4) == 0)
		return (0);
	if (getPathType(path + index) == 1 && isFileAccessible(path + index, 4) == 0)
		return (0);
	return (-1);
}

int	ConfFile::isFileAccessible(std::string const path, int mode)
{
	return (access(path.c_str(), mode));
}

std::string ConfFile::getPath()
{
	return (this->_path);
}

int ConfFile::getSize()
{
	return (this->_size);
}
