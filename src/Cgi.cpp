/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: matcardo <matcardo@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/08 18:56:27 by thabeck-          #+#    #+#             */
/*   Updated: 2024/08/11 13:44:40 by matcardo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Cgi.hpp"

Cgi::Cgi() {
	this->_cgi_pid = -1;
	this->_exit_status = 0;
	this->_cgi_path = "";
	this->_ch_env = NULL;
	this->_argv = NULL;
}

Cgi::Cgi(std::string path)
{
	this->_cgi_pid = -1;
	this->_exit_status = 0;
	this->_cgi_path = path;
	this->_ch_env = NULL;
	this->_argv = NULL;
}

Cgi::~Cgi() {

	if (this->_ch_env)
	{
		for (int i = 0; this->_ch_env[i]; i++)
			free(this->_ch_env[i]);
		free(this->_ch_env);
	}
	if (this->_argv)
	{
		for (int i = 0; this->_argv[i]; i++)
			free(_argv[i]);
		free(_argv);
	}
	this->_env.clear();
}

Cgi::Cgi(const Cgi &other)
{
		this->_env = other._env;
		this->_ch_env = other._ch_env;
		this->_argv = other._argv;
		this->_cgi_path = other._cgi_path;
		this->_cgi_pid = other._cgi_pid;
		this->_exit_status = other._exit_status;
}

Cgi &Cgi::operator=(const Cgi &rhs)
{
    if (this != &rhs)
	{
		this->_env = rhs._env;
		this->_ch_env = rhs._ch_env;
		this->_argv = rhs._argv;
		this->_cgi_path = rhs._cgi_path;
		this->_cgi_pid = rhs._cgi_pid;
		this->_exit_status = rhs._exit_status;
	}
	return (*this);
}

void Cgi::setCgiPid(pid_t cgi_pid)
{
    this->_cgi_pid = cgi_pid;
}

void Cgi::setCgiPath(const std::string &cgi_path)
{
	this->_cgi_path = cgi_path;
}

const std::map<std::string, std::string> &Cgi::getEnv() const
{
    return (this->_env);
}

const pid_t &Cgi::getCgiPid() const
{
    return (this->_cgi_pid);
}

const std::string &Cgi::getCgiPath() const
{
    return (this->_cgi_path);
}

// Esta função é chamada para inicializar o ambiente do CGI
// A função recebe um objeto HttpRequest e um iterador para um vetor de Location
// A função verifica se o método da requisição é POST
// Se for POST, a função adiciona o tamanho do corpo da requisição ao ambiente
// A função adiciona as variáveis de ambiente necessárias para o CGI
// A função cria um vetor de strings para armazenar as variáveis de ambiente

void Cgi::initGenericCgiEnv(Request& req, const std::vector<Location>::iterator it_loc)
{
	std::string cgi_exec = ("cgi-bin/" + it_loc->getCgiPath()[0]).c_str();
	char    *cwd = getcwd(NULL, 0);
	if(_cgi_path[0] != '/')
	{
		std::string tmp(cwd);
		tmp.append("/");
		if(_cgi_path.length() > 0)
			_cgi_path.insert(0, tmp);
	}
	if(req.getMethod() == POST)
	{
		std::stringstream out;
		out << req.getBody().length();
		this->_env["CONTENT_LENGTH"] = out.str();
		this->_env["CONTENT_TYPE"] = req.getHeader("content-type");
	}

    this->_env["GATEWAY_INTERFACE"] = std::string("CGI/1.1");
	this->_env["SCRIPT_NAME"] = cgi_exec;//
    this->_env["SCRIPT_FILENAME"] = this->_cgi_path;
    this->_env["PATH_INFO"] = this->_cgi_path;//
    this->_env["PATH_TRANSLATED"] = this->_cgi_path;//
    this->_env["REQUEST_URI"] = this->_cgi_path;//
    this->_env["SERVER_NAME"] = req.getHeader("host");
    this->_env["SERVER_PORT"] ="8002";
    this->_env["REQUEST_METHOD"] = req.getMethodStr();
    this->_env["SERVER_PROTOCOL"] = "HTTP/1.1";
    this->_env["REDIRECT_STATUS"] = "200";
	this->_env["SERVER_SOFTWARE"] = "MathayWebserv";

	std::map<std::string, std::string> request_headers = req.getHeaders();
	for(std::map<std::string, std::string>::iterator it = request_headers.begin();
		it != request_headers.end(); ++it)
	{
		std::string name = it->first;
		std::transform(name.begin(), name.end(), name.begin(), ::toupper);
		std::string key = "HTTP_" + name;
		_env[key] = it->second;
	}
	this->_ch_env = (char **)calloc(sizeof(char *), this->_env.size() + 1);
	std::map<std::string, std::string>::const_iterator it = this->_env.begin();
	for (int i = 0; it != this->_env.end(); it++, i++)
	{
		std::string tmp = it->first + "=" + it->second;
		this->_ch_env[i] = strdup(tmp.c_str());
	}
	this->_argv = (char **)malloc(sizeof(char *) * 3);
	this->_argv[0] = strdup(cgi_exec.c_str());
	this->_argv[1] = strdup(this->_cgi_path.c_str());
	this->_argv[2] = NULL;
}


/* Esta função é chamada para inicializar as variáveis de ambiente necessárias para executar o CGI
* A extensão do arquivo CGI é extraída do caminho do path e é usada para encontrar o caminho do executável
* Setta as variáveis de ambiente presentes na requisição
* Cria um vetor de strings para armazenar as variáveis de ambiente chamado ch_env
* Cria um vetor de strings para armazenar os argumentos do CGI chamado argv
*/
void Cgi::initCgiEnv(Request& req, const std::vector<Location>::iterator it_loc)
{
	int			position;
	std::string extension;
	std::string ext_path;

	extension = this->_cgi_path.substr(this->_cgi_path.find("."));
	std::map<std::string, std::string>::iterator it_path = it_loc->_ext_path.find(extension);
    if (it_path == it_loc->_ext_path.end())
        return ;
    ext_path = it_loc->_ext_path[extension];

	this->_env["AUTH_TYPE"] = "Basic";
	this->_env["CONTENT_LENGTH"] = req.getHeader("content-length");
	this->_env["CONTENT_TYPE"] = req.getHeader("content-type");
    this->_env["GATEWAY_INTERFACE"] = "CGI/1.1";
	position = findStart(this->_cgi_path, "cgi-bin/");
	this->_env["SCRIPT_NAME"] = this->_cgi_path;
    this->_env["SCRIPT_FILENAME"] = ((position < 0 || (size_t)(position + 8) > this->_cgi_path.size()) ? "" : this->_cgi_path.substr(position + 8, this->_cgi_path.size())); // check dif cases after put right parametr from the response
    this->_env["PATH_INFO"] = getPathInfo(req.getPath(), it_loc->getCgiExtension());
    this->_env["PATH_TRANSLATED"] = it_loc->getRootLocation() + (this->_env["PATH_INFO"] == "" ? "/" : this->_env["PATH_INFO"]);
    this->_env["QUERY_STRING"] = extractCgiEnv(req.getQuery());
    this->_env["REMOTE_ADDR"] = req.getHeader("host");
	position = findStart(req.getHeader("host"), ":");
    this->_env["SERVER_NAME"] = (position > 0 ? req.getHeader("host").substr(0, position) : "");
    this->_env["SERVER_PORT"] = (position > 0 ? req.getHeader("host").substr(position + 1, req.getHeader("host").size()) : "");
    this->_env["REQUEST_METHOD"] = req.getMethodStr();
    this->_env["HTTP_COOKIE"] = req.getHeader("cookie");
    this->_env["DOCUMENT_ROOT"] = it_loc->getRootLocation();
	this->_env["REQUEST_URI"] = req.getPath() + req.getQuery();
    this->_env["SERVER_PROTOCOL"] = "HTTP/1.1";
    this->_env["REDIRECT_STATUS"] = "200";
	this->_env["SERVER_SOFTWARE"] = "MathayWebserv";

	this->_ch_env = (char **)calloc(sizeof(char *), this->_env.size() + 1);
	std::map<std::string, std::string>::const_iterator it = this->_env.begin();
	for (int i = 0; it != this->_env.end(); it++, i++)
	{
		std::string tmp = it->first + "=" + it->second;
		this->_ch_env[i] = strdup(tmp.c_str());
	}
	this->_argv = (char **)malloc(sizeof(char *) * 3);
	this->_argv[0] = strdup(ext_path.c_str());
	this->_argv[1] = strdup(this->_cgi_path.c_str());
	this->_argv[2] = NULL;
}

/* Esta função é chamada para executar o CGI
* Cria dois pipes para comunicação entre o pai e o filho
* Cria um processo filho para executar o CGI
* Redireciona a entrada e saída padrão do processo filho para os pipes
* Fecha os descritores de arquivo não utilizados
* Executa o CGI
*/
void Cgi::cgiExec(short &error_code)
{
	if (this->_argv[0] == NULL || this->_argv[1] == NULL)
	{
		error_code = 500;
		return ;
	}
	if (pipe(pipe_in) < 0)
	{
		std::cout << RED "CGI pipe() failed" RESET << std::endl;
		error_code = 500;
		return ;
	}
	if (pipe(pipe_out) < 0)
	{
		std::cout << RED "CGI pipe() failed" RESET << std::endl;
		close(pipe_in[0]);
		close(pipe_in[1]);
		error_code = 500;
		return ;
	}
	this->_cgi_pid = fork();
	if (this->_cgi_pid == 0)
	{
		dup2(pipe_in[0], STDIN_FILENO);
		dup2(pipe_out[1], STDOUT_FILENO);
		close(pipe_in[0]);
		close(pipe_in[1]);
		close(pipe_out[0]);
		close(pipe_out[1]);
		this->_exit_status = execve(this->_argv[0], this->_argv, this->_ch_env);
		exit(this->_exit_status);
	}
	else if (this->_cgi_pid > 0){}
	else
	{
        std::cout << RED "CGI Fork failed" RESET << std::endl;
		error_code = 500;
	}
}

/* Esta função é chamada para encontrar o início da string */
int Cgi::findStart(const std::string path, const std::string delim)
{
	if (path.empty())
		return (-1);
	size_t posit = path.find(delim);
	if (posit != std::string::npos)
		return (posit);
	else
		return (-1);
}

/* Esta função é chamada para extrair as variáveis de ambiente do caminho do path */
std::string Cgi::extractCgiEnv(std::string &path)
{
	size_t token = path.find("%");
	while (token != std::string::npos)
	{
		if (path.length() < token + 2)
			break ;
		char decimal = ft_hexToDec(path.substr(token + 1, 2));
		path.replace(token, 3, toString(decimal));
		token = path.find("%");
	}
	return (path);
}

/* Obtém a variável de ambiente PATH_INFO */
std::string Cgi::getPathInfo(std::string& path, std::vector<std::string> extensions)
{
	std::string tmp;
	size_t start, end;

	for (std::vector<std::string>::iterator it_ext = extensions.begin(); it_ext != extensions.end(); it_ext++)
	{
		start = path.find(*it_ext);
		if (start != std::string::npos)
			break ;
	}
	if (start == std::string::npos)
		return "";
	if (start + 3 >= path.size())
		return "";
	tmp = path.substr(start + 3, path.size());
	if (!tmp[0] || tmp[0] != '/')
		return "";
	end = tmp.find("?");
	return (end == std::string::npos ? tmp : tmp.substr(0, end));
}

/* Limpa o ambiente do CGI */
void		Cgi::clearCgiEnv()
{
	this->_cgi_pid = -1;
	this->_exit_status = 0;
	this->_cgi_path = "";
	this->_ch_env = NULL;
	this->_argv = NULL;
	this->_env.clear();
}
