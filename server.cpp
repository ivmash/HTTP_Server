#include <iostream>
#include <cstring>
#include <string>
#include <thread>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

using namespace std;

void client(int acc);
bool find_file(char *msg);
int set_nonblock(int fd);

int main(int argc, char **argv)
{
	getopt (argc, argv, "hpd:");
	if (fork()) return 0; // deamonizing
	chdir(argv[6]);
	int MasterSocket = socket(AF_INET, SOCK_STREAM, 0);
	
	struct sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(atoi(argv[4]));
	sa.sin_addr.s_addr = inet_addr(argv[2]);
	
	if (bind(MasterSocket, (struct sockaddr*)(&sa), sizeof(sa)) == -1)
		return 0;
	listen(MasterSocket, 128);

	while (1)
	{
		socklen_t sa_len = sizeof(sa);
		int acc = accept(MasterSocket, (struct sockaddr*)(&sa), &sa_len);
		std::thread(client, acc).detach();
	}
	return 0;
}

void client(int acc)
{
	int i = 0;
	std::string response;
	char* msg = (char*)calloc(1024*sizeof(char), sizeof(char));

	// get request
	int recv_size = recv(acc, msg, 2048, 0);
	
	// response only for GET
	if (!strstr(msg, "GET")) return;
	
	// file path processing
	char* file_path = (char*)malloc(128*sizeof(char));

	for (i = 0; (msg[4+i] != ' ') && (msg[4+i] != EOF) && (msg[4+i] != '\n') && (msg[4+i] != '?'); i++)
	{
		file_path[i] = msg[4+i];
	}
	file_path[i] = '\0';

	char offset = 0;
	if (file_path[0] == '/'){
		file_path++;
		offset = 1;
	}

	// open file
	int fd = open(file_path, O_RDONLY | 0666);
	
	// if file not found - 404 response	
	if (fd == -1)
	{
		send(acc,"HTTP/1.0 404 NOT FOUND\r\nContent-Length: 0\r\n", 43, 0);
		shutdown(acc, SHUT_RDWR);
		close(fd);
		free(msg);
		free(file_path-offset);
		close(acc);
		return;
	}
	
	// int and string file size
	struct stat st;
	stat(file_path, &st);
	int file_size = st.st_size;
	std::string str_file_size(std::to_string(file_size));
	
	// send headers
	for(int i = 0; i < 17; i++) response.push_back("HTTP/1.0 200 OK\r\n"[i]);
	for(int i = 0; i < 16; i++) response.push_back("Content-Length: "[i]);
	for(int i = 0; i < str_file_size.size(); i++) response.push_back(str_file_size.c_str()[i]);
	if (strstr(file_path, ".html"))
	{
		for(int i = 0; i < 25; i++) response.push_back("\r\nContent-Type: text/html"[i]);
	}
	for(int i = 0; i < 4; i++) response.push_back("\r\n\r\n"[i]);

	// send file
	free(msg);
	msg = (char*)malloc(file_size*sizeof(char));

	read(fd, msg, file_size);
	for (int i = 0; i < file_size; i++) response.push_back(msg[i]);
	
	response.size();
	send(acc, response.c_str(), response.size(), 0);

	// close connection, file and free memory
	shutdown(acc, SHUT_RDWR);
	close(fd);
	free(msg);
	free(file_path-offset);
	close(acc);
}

int set_nonblock(int fd)
{
	int flags;
#if defined(O_NONBLOCK)
	if (-1 == (flags = fcntl(fd,F_GETFL,0)))
		flags = 0;
	return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else
	flags = 1;
	return ioctl(fd, FIOBIO, &flags);
#endif
}
