/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfFile.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thabeck- <thabeck-@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/07 18:33:05 by thabeck-          #+#    #+#             */
/*   Updated: 2024/08/07 20:30:42 by thabeck-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/ConfFile.hpp"

ConfFile::ConfFile() : _size(0) { }

ConfFile::ConfFile(std::string const path) : _path(path), _size(0) { }

ConfFile::~ConfFile() { }


/* Define se o caminho é um arquivo (1), diretório (2) ou outra coisa (3) */
int ConfFile::getPathType(std::string const path)
{
    // struct stat é uma estrutura que contém informações sobre um arquivo.
	struct stat	fileInfo;
	
	// A função stat() é usada para obter informações sobre o arquivo apontado por path
	// e preencher uma estrutura com essas informações.
	// Se o arquivo não existir, a função falhará e retornará -1.
	// Se a função for bem-sucedida, ela retornará 0.
	if (stat(path.c_str(), &fileInfo) == 0)
	{
		// verifica se o arquivo é um arquivo regular
		if (fileInfo.st_mode & S_IFREG)
			return (1);
		// verifica se o arquivo é um diretório
		else if (fileInfo.st_mode & S_IFDIR)
			return (2);
		else
			return (3);
	}
	else
		return (-1);
}

/* Lê o arquivo */
std::string	ConfFile::readFile(std::string path)
{
	if (path.empty() || path.length() == 0)
		return (NULL);
	std::ifstream conf_file(path.c_str());
	if (!conf_file || !conf_file.is_open())
		return (NULL);

	std::stringstream stream;
	stream << conf_file.rdbuf();
	// stream.str() retorna o conteúdo do fluxo de saída como uma string.
	return (stream.str());
}

/* Checa se o arquivo existe e é acessível isFileExistAndReadable*/
int ConfFile::checkFile(std::string const path, std::string const index)
{
    // Verifica se o caminho é um arquivo e se é acessível.
	if (getPathType(index) == 1 && isFileAccessible(index, 4) == 0)
		return (0);
    // Verifica se o caminho é um diretório e se é acessível.
	if (getPathType(path + index) == 1 && isFileAccessible(path + index, 4) == 0)
		return (0);
	return (-1);
}

/* Checa se o arquivo é acessível checkFile*/
int	ConfFile::isFileAccessible(std::string const path, int mode)
{
	// A função access() verifica se o processo tem permissão de acesso ao arquivo especificado por path.
	// mode 4 = R_OK, 2 = W_OK, 1 = X_OK. R_OK verifica se o arquivo pode ser lido.
	return (access(path.c_str(), mode));
}

/* Acessores */
std::string ConfFile::getPath()
{
	return (this->_path);
}

int ConfFile::getSize()
{
	return (this->_size);
}