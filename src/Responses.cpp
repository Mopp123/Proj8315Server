
#include "Responses.h"


/*


"HTTP/1.1 200 OK\n"
"Content-Type: application/octet-stream\n"
"Content-Length: 2\n"
"Access-Control-Allow-Origin: *\n"
"\n";

 * */

Response::Response(char* data, size_t size) : 
	_contentSize(size)
{

	std::string header = createHeader(_contentSize);
	const size_t headerSize = header.size();
	_totalSize = headerSize + _contentSize;

	_pData = new char[_totalSize];
	
	memcpy(_pData, header.data(), headerSize);
	
	if(data && _contentSize > 0)
		memcpy(_pData + headerSize, data, _contentSize);
}

Response::~Response()
{
	delete[] _pData;
}

std::string Response::createHeader(size_t contentLength) const
{

	std::string header = _defaultHeader;
	header += "Content-Length: " + std::to_string(contentLength) + "\n\n";
	return header;
}
