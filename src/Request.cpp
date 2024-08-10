/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thabeck- <thabeck-@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/08 21:54:03 by thabeck-          #+#    #+#             */
/*   Updated: 2024/08/10 02:08:06 by thabeck-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Request.hpp"

Request::Request()
{
    _method_str[::GET] = "GET";
    _method_str[::POST] = "POST";
    _method_str[::DELETE] = "DELETE";
    _path = "";
    _query = "";
    _fragment = "";
    _body_str = "";
    _error_code = 0;
    _chunk_length = 0;
    _method = NONE;
    _method_index = 1;
    _state = Request_Line;
    _fields_done_flag = false;
    _body_flag = false;
    _body_done_flag = false;
    _chunked_flag = false;
    _body_length = 0;
    _storage = "";
    _key_storage = "";
    _upload_flag = false;
    _upload_boundary = "";
}

Request::~Request() {}

/*Tabela de tokens permitidos em um nome de campo de acordo com a RFC 2616*/
bool* initTokenTable()
{
    static bool token_table[256] = {false};
    
    for (unsigned char c = '0'; c <= '9'; ++c) token_table[c] = true;
    for (unsigned char c = 'A'; c <= 'Z'; ++c) token_table[c] = true;
    for (unsigned char c = 'a'; c <= 'z'; ++c) token_table[c] = true;
    
    token_table['!'] = true; token_table['#'] = true; token_table['$'] = true;
    token_table['%'] = true; token_table['&'] = true; token_table['\''] = true;
    token_table['*'] = true; token_table['+'] = true; token_table['-'] = true;
    token_table['.'] = true; token_table['^'] = true; token_table['_'] = true;
    token_table['`'] = true; token_table['|'] = true; token_table['~'] = true;
    
    return token_table;
}

static const bool* token_table = initTokenTable();

/* Checa se o caractere passado é permitido em um nome de campo de acordo com a RFC 2616 */
inline bool isToken(uint8_t ch)
{
    return token_table[ch];
}

/* Recebe os dados do cliente e os armazena em um buffer, que é processado pelo parser
* para extrair as informações da requisição passo a passo */
void Request::feedRequest(char *data, size_t size)
{
    u_int8_t character;
    static std::stringstream s;

    for (size_t i = 0; i < size; ++i)
    {
        character = data[i];
        switch (_state)
        {
            //Este case trata a primeira linha da requisição, que contém o método, a URI e a versão do HTTP
            case Request_Line:
            {
                if (character == 'G')
                    _method = GET;
                else if (character == 'P')
                {
                    _method = POST;
                }
                else if (character == 'D')
                    _method = DELETE;
                else
                {
                    _error_code = 501;
                    std::cout << RED "Request Error: Method not implemented" RESET << std::endl;
                    return ;
                }
                _state = Request_Line_Method;
                break ;
            }
            //Compara o caracter atual com o caracter da string do método na posição _method_index
            case Request_Line_Method:
            {
                // Compara se o caracter atual é igual ao caracter da string do método na posição _method_index
                if (character == _method_str[_method][_method_index])
                    _method_index++;
                else
                {
                    _error_code = 501;
                    std::cout << RED "Request Error: Method not implemented" RESET << std::endl;
                    return ;
                }

                if ((size_t) _method_index == _method_str[_method].length())
                    _state = Request_Line_First_Space;
                break ;
            }
            //Verifica se o caracter atual é um espaço
            case Request_Line_First_Space:
            {
                if (character != ' ')
                {
                    _error_code = 400;
                    std::cout << RED "Request Error: Bad Character (Request_Line_First_Space)" RESET << std::endl;
                    return ;
                }
                _state = Request_Line_URI_Path_Slash;
                continue ;
            }
            //Verifica se o caracter atual é uma barra
            case Request_Line_URI_Path_Slash:
            {
                if (character == '/')
                {
                    _state = Request_Line_URI_Path;
                    _storage.clear();
                }
                else
                {
                    _error_code = 400;
                    std::cout << RED "Request Error: Bad Character (Request_Line_URI_Path_Slash)" RESET << std::endl;
                    return ;
                }
                break ;
            }
            //Verifica se o caracter atual é um espaço, interrogação ou cerquilha
            case Request_Line_URI_Path:
            {
                if (character == ' ')
                {
                    _state = Request_Line_Ver;
                    _path.append(_storage);
                    _storage.clear();
                    continue ;
                }
                else if (character == '?')
                {
                    _state = Request_Line_URI_Query;
                    _path.append(_storage);
                    _storage.clear();
                    continue ;
                }
                else if (character == '#')
                {
                    _state = Request_Line_URI_Fragment;
                    _path.append(_storage);
                    _storage.clear();
                    continue ;
                }
                else if (!allowedCharURI(character))
                {
                    _error_code = 400;
                    std::cout << RED "Request Error: Bad Character (Request_Line_URI_Path)" RESET << std::endl;
                    return ;
                }
                else if ( i > MAX_URI_LENGTH)
                {
                    _error_code = 414;
                    std::cout << RED "Request Error: URI Too Long (Request_Line_URI_Path)" RESET << std::endl;
                    return ;
                }
                break ;
            }
            //Verifica se o caracter atual é um espaço ou cerquilha
            case Request_Line_URI_Query:
            {
                if (character == ' ')
                {
                    _state = Request_Line_Ver;
                    _query.append(_storage);
                    _storage.clear();
                    continue ;
                }
                else if (character == '#')
                {
                    _state = Request_Line_URI_Fragment;
                    _query.append(_storage);
                    _storage.clear();
                    continue ;
                }
                else if (!allowedCharURI(character))
                {
                    _error_code = 400;
                    std::cout << RED "Request Error: Bad Character (Request_Line_URI_Query)" RESET << std::endl;
                    return ;
                }
                else if ( i > MAX_URI_LENGTH)
                {
                    _error_code = 414;
                    std::cout << RED "Request Error: URI Too Long (Request_Line_URI_Query)" RESET << std::endl;
                    return ;
                }
                break ;
            }
            //Verifica se o caracter atual é um espaço
            case Request_Line_URI_Fragment:
            {
                if (character == ' ')
                {
                    _state = Request_Line_Ver;
                    _fragment.append(_storage);
                    _storage.clear();
                    continue ;
                }
                else if (!allowedCharURI(character))
                {
                    _error_code = 400;
                    std::cout << RED "Request Error: Bad Character (Request_Line_URI_Fragment)" RESET << std::endl;
                    return ;
                }
                else if ( i > MAX_URI_LENGTH)
                {
                    _error_code = 414;
                    std::cout << RED "Request Error: URI Too Long (Request_Line_URI_Fragment)" RESET << std::endl;
                    return ;
                }
                break ;
            }
            //Verifica se o caracter atual remete a uma pasta que vai antes da raiz
            case Request_Line_Ver:
            {
                if (checkUriPos(_path))
                {
                    _error_code = 400;
                    std::cout << RED "Request Error: Bad URI (Uri before root)" RESET << std::endl;
                    return ;
                }
                if (character != 'H')
                {
                    _error_code = 400;
                    std::cout << RED "Request Error: Bad Character (Request_Line_Ver)" RESET << std::endl;
                    return ;
                }
                _state = Request_Line_HTTP_1;
                break ;
            }
            //Verifica se o caracter atual é um T
            case Request_Line_HTTP_1:
            {
                if (character != 'T')
                {
                    _error_code = 400;
                    std::cout << RED "Request Error: Bad Character (Request_Line_HTTP_1)" RESET << std::endl;
                    return ;
                }
                _state = Request_Line_HTTP_2;
                break ;
            }
            //Verifica se o caracter atual é um T
            case Request_Line_HTTP_2:
            {
                if (character != 'T')
                {
                    _error_code = 400;
                    std::cout << RED "Request Error: Bad Character (Request_Line_HTT)" RESET << std::endl;
                    return ;
                }
                _state = Request_Line_HTTP_3;
                break ;
            }
            //Verifica se o caracter atual é um P
            case Request_Line_HTTP_3:
            {
                if (character != 'P')
                {
                    _error_code = 400;
                    std::cout << RED "Request Error: Bad Character (Request_Line_HTTP_3)" RESET << std::endl;
                    return ;
                }
                _state = Request_Line_HTTP_Slash;
                break ;
            }
            //Verifica se o caracter atual é uma barra
            case Request_Line_HTTP_Slash:
            {
                if (character != '/')
                {
                    _error_code = 400;
                    std::cout << RED "Request Error: Bad Character (Request_Line_HTTP_Slash)" RESET << std::endl;
                    return ;
                }
                _state = Request_Line_Major;
                break ;
            }
            //Verifica se o caracter atual é um número
            case Request_Line_Major:
            {
                if (!isdigit(character))
                {
                    _error_code = 400;
                    std::cout << RED "Request Error: Bad Character (Request_Line_Major)" RESET << std::endl;
                    return ;
                }
                _ver_major = character;

                _state = Request_Line_Dot;
                break ;
            }
            //Verifica se o caracter atual é um ponto
            case Request_Line_Dot:
            {
                if (character != '.')
                {
                    _error_code = 400;
                    std::cout << RED "Request Error: Bad Character (Request_Line_Dot)" RESET << std::endl;
                    return ;
                }
                _state = Request_Line_Minor;
                break ;
            }
            //Verifica se o caracter atual é um número
            case Request_Line_Minor:
            {
                if (!isdigit(character))
                {
                    _error_code = 400;
                    std::cout << RED "Request Error: Bad Character (Request_Line_Minor)" RESET << std::endl;
                    return ;
                }
                _ver_minor = character;
                _state = Request_Line_CR;
                break ;
            }
            //Verifica se o caracter atual é um tipo de quebra de linha (CR)
            case Request_Line_CR:
            {
                if (character != '\r')
                {
                    _error_code = 400;
                    std::cout << RED "Request Error: Bad Character (Request_Line_CR)" RESET << std::endl;
                    return ;
                }
                _state = Request_Line_LF;
                break ;
            }
            //Verifica se o caracter atual é um tipo de quebra de linha (LF)
            case Request_Line_LF:
            {
                if (character != '\n')
                {
                    _error_code = 400;
                    std::cout << RED "Request Error: Bad Character (Request_Line_LF)" RESET << std::endl;
                    return ;
                }
                _state = Field_Name_Start;
                _storage.clear();
                continue ;
            }
            //Verifica se o caracter atual é permitido em um nome de campo
            case Field_Name_Start:
            {
                if (character == '\r')
                    _state = Fields_End;
                else if (isToken(character))
                    _state = Field_Name;
                else
                {
                    _error_code = 400;
                    std::cout << RED "Request Error: Bad Character (Field_Name_Start)" RESET << std::endl;
                    return ;
                }
                break ;
            }
            //Verifica se o caracter atual é um dois pontos
            case Fields_End:
            {
                if (character == '\n')
                {
                    _storage.clear();
                    _fields_done_flag = true;
                    _handle_header_fields();
                    // Se o corpo da mensagem estiver presente, o estado é alterado para Message_Body
                    if (_body_flag == 1)
                    {
                        if (_chunked_flag == true)
                            _state = Chunked_Length_Begin;
                        else
                        {
                            _state = Message_Body;
                        }
                    }
                    else
                    {
                        _state = Parsing_Done;
                    }
                    continue ;
                }
                else
                {
                    _error_code = 400;
                    std::cout << RED "Request Error: Bad Character (Fields_End)" RESET << std::endl;
                    return ;
                }
                break ;
            }
            //Verifica se o caracter atual é um dois pontos
            case Field_Name:
            {
                if (character == ':')
                {
                    _key_storage = _storage;
                    _storage.clear();
                    _state = Field_Value;
                    continue ;
                }
                else if (!isToken(character))
                {
                    _error_code = 400;
                    std::cout << RED "Request Error: Bad Character (Field_Name)" RESET << std::endl;
                    return ;
                }
                break ;
            }
            //Verifica se o caracter atual é um CR
            case Field_Value:
            {
                if ( character == '\r' )
                {
                    setHeader(_key_storage, _storage);
                    _key_storage.clear();
                    _storage.clear();
                    _state = Field_Value_End;
                    continue ;
                }
                break ;
            }
            //Verifica se o caracter atual é um LF
            case Field_Value_End:
            {
                if ( character == '\n' )
                {
                    _state = Field_Name_Start;
                    continue ;
                }
                else
                {
                    _error_code = 400;
                    std::cout << RED "Request Error: Bad Character (Field_Value_End)" RESET << std::endl;
                    return ;
                }
                break ;
            }
            //Verifica se o caracter atual é um número hexadecimal
            case Chunked_Length_Begin:
            {
                if (isxdigit(character) == 0)
                {
                    _error_code = 400;
                    std::cout << RED "Request Error: Bad Character (Chunked_Length_Begin)" RESET << std::endl;
                    return ;
                }
                s.str("");
                s.clear();
                s << character;
                s >> std::hex >> _chunk_length;
                if (_chunk_length == 0)
                    _state = Chunked_Length_CR;
                else
                    _state = Chunked_Length;
                continue ;
            }
            //Verifica se o caracter atual é um número hexadecimal
            case Chunked_Length:
            {
                if (isxdigit(character) != 0)
                {
                    int temp_len = 0;
                    s.str("");
                    s.clear();
                    s << character;
                    s >> std::hex >> temp_len;
                    _chunk_length *= 16;
                    _chunk_length += temp_len;
                }
                else if (character == '\r')
                    _state = Chunked_Length_LF;
                else
                    _state = Chunked_Ignore;
                continue ;
            }
            //Verifica se o caracter atual é um CR
            case Chunked_Length_CR:
            {
                if ( character == '\r')
                    _state = Chunked_Length_LF;
                else
                {
                    _error_code = 400;
                    std::cout << RED "Request Error: Bad Character (Chunked_Length_CR)" RESET << std::endl;
                    return ;
                }
                continue ;
            }
            //Verifica se o caracter atual é um LF
            case Chunked_Length_LF:
            {
                if ( character == '\n')
                {
                    if (_chunk_length == 0)
                        _state = Chunked_End_CR;
                    else
                        _state = Chunked_Data;
                }
                else
                {
                    _error_code = 400;
                    std::cout << RED "Request Error: Bad Character (Chunked_Length_LF)" RESET << std::endl;
                    return ;
                }
                continue ;
            }
            //Ignora o caracter atual
            case Chunked_Ignore:
            {
                if (character == '\r')
                    _state = Chunked_Length_LF;
                continue ;
            }
            //Verifica se o caracter atual é um número hexadecimal
            case Chunked_Data:
            {
				_body.push_back(character);
				--_chunk_length;
                if (_chunk_length == 0)
                    _state = Chunked_Data_CR;
				continue ;
            }
            //Verifica se o caracter atual é um CR
            case Chunked_Data_CR:
            {
                if ( character == '\r')
                    _state = Chunked_Data_LF;
                else
                {
                    _error_code = 400;
                    std::cout << RED "Request Error: Bad Character (Chunked_Data_CR)" RESET << std::endl;
                    return ;
                }
                continue ;
            }
            //Verifica se o caracter atual é um LF
            case Chunked_Data_LF:
            {
                if ( character == '\n')
                    _state = Chunked_Length_Begin;
                else
                {
                    _error_code = 400;
                    std::cout << RED "Request Error: Bad Character (Chunked_Data_LF)" RESET << std::endl;
                    return ;
                }
                continue ;
            }
            //Verifica se o caracter atual é um CR
            case Chunked_End_CR:
            {
                if (character != '\r')
                {
                    _error_code = 400;
                    std::cout << RED "Request Error: Bad Character (Chunked_End_CR)" RESET << std::endl;
                    return ;
                }
                _state = Chunked_End_LF;
                continue ;

            }
            //Verifica se o caracter atual é um LF
            case Chunked_End_LF:
            {
                if (character != '\n')
                {
                    _error_code = 400;
                    std::cout << RED "Request Error: Bad Character (Chunked_End_LF)" RESET << std::endl;
                    return ;
                }
                _body_done_flag = true;
                _state = Parsing_Done;
                continue ;
            }
            //Verifica se o caracter atual é um caractere permitido em um corpo de mensagem
            case Message_Body:
            {
                if (_body.size() < _body_length )
                    _body.push_back(character);
                if (_body.size() == _body_length )
                {
                    _body_done_flag = true;
                    _state = Parsing_Done;
                }
                break ;
            }
            case Parsing_Done:
            {
                return ;
            }
        }//end of swtich
        _storage += character;
    }
    if( _state == Parsing_Done)
    {
        _body_str.append((char*)_body.data(), _body.size());
    }
}

bool    Request::isParsed()
{
    return (_state == Parsing_Done);
}

HttpMethod  &Request::getMethod()
{
    return (_method);
}

std::string &Request::getPath()
{
    return (_path);
}

std::string &Request::getQuery()
{
    return (_query);
}

std::string &Request::getFragment()
{
    return (_fragment);
}

std::string Request::getHeader(std::string const &name)
{
    return (_request_headers[name]);
}

const std::map<std::string, std::string> &Request::getHeaders() const
{
	return (this->_request_headers);
}

std::string Request::getMethodStr()
{
	return (_method_str[_method]);
}

std::string &Request::getBody()
{
    return (_body_str);
}

std::string     Request::getServerName()
{
    return (this->_server_name);
}

std::string     &Request::getUploadBoundary()
{
    return (this->_upload_boundary);
}

short     Request::getErrorCode()
{
    return (this->_error_code);
}

bool    Request::getUploadFlag()
{
    return (this->_upload_flag);
}

void    Request::setBody(std::string body)
{
    _body.clear();
    _body.insert(_body.begin(), body.begin(), body.end());
    _body_str = body;
}

void    Request::setMethod(HttpMethod & method)
{
    _method = method;
}

void    Request::setHeader(std::string &name, std::string &value)
{
    trimStr(value);
    toLower(name);
    _request_headers[name] = value;
}

void    Request::setMaxBodySize(size_t size)
{
    _max_body_size = size;
}


void        Request::printRequest()
{
    std::cout << _method_str[_method] + " " + _path + "?" + _query + "#" + _fragment
              + " " + "HTTP/" << _ver_major  << "." << _ver_minor << std::endl;

    for (std::map<std::string, std::string>::iterator it = _request_headers.begin();
    it != _request_headers.end(); ++it)
    {
        std::cout << it->first + ":" + it->second << std::endl;
    }
    for (std::vector<u_int8_t>::iterator it = _body.begin(); it != _body.end(); ++it)
    {
        std::cout << *it;
    }
    std::cout << std::endl << "END OF BODY" RESET << std::endl;

    std::cout << RED "BODY FLAG =" << _body_flag << "  _BODY_done_flag= " 
    << _body_done_flag << "FEIDLS FLAG = " << _fields_done_flag << std::endl;
}

/* Trata os campos do cabeçalho da requisição */
void        Request::_handle_header_fields()
{
    std::stringstream ss;

    if (_request_headers.count("content-length"))
    {
        _body_flag = true;
        ss << _request_headers["content-length"];
        ss >> _body_length;
    }
    if ( _request_headers.count("transfer-encoding"))
    {
        if (_request_headers["transfer-encoding"].find_first_of("chunked") != std::string::npos)
            _chunked_flag = true;
        _body_flag = true;
    }
    if (_request_headers.count("host"))
    {
        size_t pos = _request_headers["host"].find_first_of(':');
        _server_name = _request_headers["host"].substr(0, pos);
    }
    if (_request_headers.count("content-type") && _request_headers["content-type"].find("multipart/form-data") != std::string::npos)
    {
        size_t pos = _request_headers["content-type"].find("boundary=", 0);
        if (pos != std::string::npos)
            this->_upload_boundary = _request_headers["content-type"].substr(pos + 9, _request_headers["content-type"].size());
        this->_upload_flag = true;
    }
}

/* Limpa a requisição */
void    Request::clearRequest()
{
    _path.clear();
    _error_code = 0;
    _query.clear();
    _fragment.clear();
    _method = NONE;
    _method_index = 1;
    _state = Request_Line;
    _body_length = 0;
    _chunk_length = 0x0;
    _storage.clear();
    _body_str = "";
    _key_storage.clear();
    _request_headers.clear();
    _server_name.clear();
    _body.clear();
    _upload_boundary.clear();
    _fields_done_flag = false;
    _body_flag = false;
    _body_done_flag = false;
    _complete_flag = false;
    _chunked_flag = false;
    _upload_flag = false;
}

/* Checa o valor do cabeçalho "Connection". Se for keep-alive, não fecha a conexão. */
bool        Request::keepConnected()
{
    if (_request_headers.count("connection"))
    {
        if (_request_headers["connection"].find("close", 0) != std::string::npos)
            return (false);
    }
    return (true);
}

/* Quebra o corpo da requisição */
void            Request::breakRequestBody(int bytes)
{
    _body_str = _body_str.substr(bytes);
}
