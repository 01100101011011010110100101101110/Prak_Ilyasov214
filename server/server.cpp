#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <ctime>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

using namespace std;

char *itoa(int i) {
	char *str = new char[11];
	sprintf(str, "%d", i);
	return str;
}

int fcount(int fd) {
	int size = 0,len;
	char c[1024];
	while ((len = read(fd, &c, 1024)) > 0) {
        size += len;
    }
	lseek(fd, 0, 0);
	return size;
}

void Process_request(char *request, int serv_sd, int client_sd) {
    int cont_type;
    int len;
    char client_request[4096];
    if (strncmp(request, "GET /", 5)) {
		strcpy(client_request, "HTTP/1.0 501 NotImplemented modelserver\nServer: Model HTTP Server/0.1\nConnection: keep-alive\nAllow: GET\nDate: ");
		time_t tm = time(NULL);
		strcat(client_request, asctime(localtime(&tm)));
		strcat(client_request, "\n\n");
		len = strlen(client_request);
		//printf("%s", client_request);	//OTLADKA!!!!!!!!!!!!!!!!!!!!!!!!
		cout << client_request << endl;
        send(client_sd, client_request, len, 0);
		shutdown(client_sd, 1);
		close(client_sd);
        return;
    }
    int i = 5;
    int fd;
	while(request[i] && (request[i++] != ' ')) {
        ;
    }
	request[i-1] = 0;
	i = 5;
	while(request[i] && (request[i++] != '.')){
        ;
    }
	cont_type = 2;
	if (request[i]) {
	    if(!strncmp(request + i, "html", 4)) {
            cont_type = 0;
        } else if (!strncmp(request + i, "jpeg", 4)) {
            cont_type = 1;
        }
    }
    if(strlen(request) == 5) {
		cont_type = 0;
		fd = open("index.html", O_RDONLY);
	} else if ((fd = open(request + 5, O_RDONLY)) < 0) {
		fd = open("err404.html", O_RDONLY);
		strcpy(client_request, "HTTP/1.0 404 PageNotFound modelserver\nServer: Model HTTP Server/0.1\nAllow: GET\nConnection: keep-alive\nContent-type: text/html\nContent-length: 377\nDate: ");
		time_t tm = time(NULL);
		strcat(client_request, asctime(localtime(&tm)));
		strcat(client_request, "\n\n");
		len = strlen(client_request);
        printf("%s", request);
        printf("%s", client_request);   //!!!!!!!!!!!!!!!!
		send(client_sd, &client_request, len, 0);
		while((len = read(fd, client_request, 1024)) > 0) {
			send(client_sd, &client_request, len, 0);
        }
		close(fd);
		shutdown(client_sd, 1);
		close(client_sd);
		return;
	}
	strcpy(client_request, "HTTP/1.0 200 OK modelserver\nServer: Model HTTP Server/0.1\nAllow: GET\nConnection: keep-alive\nDate: ");
	time_t tm = time(NULL);
	strcat(client_request, asctime(localtime(&tm)));
	strcat(client_request, "Content-length: ");
	char * cl = itoa(fcount(fd));
	strcat(client_request, cl);
	delete [] cl;
	switch (cont_type) {
		case 0:
            strcat(client_request, "\nContent-type: text/html\n\n");
            break;
		case 1:
            strcat(client_request,"\nContent-type: image/jpeg\n\n");
            break;
		case 2:
            strcat(client_request,"\nContent-type: text/plain\n\n");
            break;
	}
	len = strlen(client_request);
    printf("%s", client_request);       //!!!!!!!!!!!!!!!!!
	send(client_sd, &client_request, len, 0);
	while((len = read(fd, client_request, 1024))>0) {
		send(client_sd, &client_request, len, 0);
    }

	close(fd);
	shutdown(client_sd, SHUT_RDWR);
	close(client_sd);
}

class Server {      //NOTE!! CLOSE SERV_FD B4 EXIT()
private:
    int serv_sd;
    short port;
    char request[1024];
    struct sockaddr_in saddr;
public:
    Server(short port); //first init and just listen
    void Run();
    ~Server() {
        close(serv_sd);
    }
};

Server::Server(short port) {
    if ((serv_sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        //raise error
        exit(1);
    }
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family= AF_INET;
    saddr.sin_port = htons(port);
    saddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (bind(serv_sd, (struct sockaddr *) &saddr, sizeof(saddr)) < 0) {
        //raise error
        exit(1);
    }
    if (listen(serv_sd, 5) != 0) {
        //raise error
        exit(1);
    }
}

void Server::Run() {
    for(;;) {
        struct sockaddr_in caddr;
        int caddrlen = sizeof(caddr);
        int client_sd = accept(serv_sd, (struct sockaddr*) &caddr, (socklen_t*) &caddrlen);
        if (client_sd < 0) {
            //raise error
            exit(1);
        }
        int get_request = recv(client_sd, request, 4096, 0);
        if (get_request < 0) {
            //raise error
            exit(1);
        }
        Process_request(request, serv_sd, client_sd);
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        cout << "Usage: " << argv[0] << " <port number>" << endl;
        return 0;
    } else if (atoi(argv[1]) < 1024 || atoi(argv[1]) > 49151) {
        cout << "Please provide user ports in range 1024-49151" << endl;
        return 0;
    }
    Server server(atoi(argv[1]));
    server.Run();
    return 0;
}
