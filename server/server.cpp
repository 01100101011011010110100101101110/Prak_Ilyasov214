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
#include <sys/wait.h>

using namespace std;

class Server {
private:
    int serv_sd;
    char request[1024];
    struct sockaddr_in saddr;
public:
    static short port;
    Server(short port); //first init and just listen
    void Run();
	//short get_port() const { return port; }
    ~Server() {
        close(serv_sd);
    }
};

short Server::port = 5000;

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

int char_search(char * str, char c) {
		int j = 0;
		while ((str[j] != '\0') && (str[j] != c)) {
			j++;
		}
		if (str[j]=='0') {
			return 0;
		}
		return j;
}

const char *get_prog_name(char *str) {
	int j = 8;	//skip cgi-bin
	string ans;
	while ((str[j] != '\0') && (str[j] != '?')) {
		ans += str[j];
		j++;
	}

	return ans.c_str();

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
		//printf("%s", client_request);	//otladka
		cout << client_request << endl;
       	send(client_sd, client_request, len, 0);
		shutdown(client_sd, 1);
		close(client_sd);
		return;
    }
	int i = 5;
	int fd;
	while(request[i] && (request[i++] != ' ')) {}
	request[i-1] = 0;
	i = 5;
	while(request[i] && (request[i++] != '.')) {}
	cont_type = 2;
	if (request[i]) {
		if(!strncmp(request + i, "html", 4)) {
			cont_type = 0;
		} else if (!strncmp(request + i, "jpeg", 4)) {
			cont_type = 1;
        }
    } else if (!strncmp(request, "GET /cgi-bin/", 13)) {
		cont_type = 0;
		int pid, status = 0;
		if ((pid = fork()) < 0) {
            cerr << "Fork error" << endl;
			exit(1);
		}
		if (pid == 0) {		//need to autoparse
			chdir("./cgi-bin");
			//filename
			char *argv[2];
			int offset1 = char_search(request + 5, '?');
			argv[0] = new char[offset1 + 1];
			strcpy(argv[0], get_prog_name(request + 5));
			argv[1] = NULL;
			//log file
			string tmp_file = "pid" + to_string(getpid()) + ".txt";
			int tmp = open(tmp_file.c_str(), O_TRUNC | O_CREAT | O_WRONLY, 0644);
			dup2(tmp, 1);
			close(tmp);
			int offset = char_search(request + 5, '?');
			(request + 5)[offset] = '\0';
			//env var
			char **envp = new char *[7];
			envp[0] = new char[(int) strlen("SERVER_ADDR=127.0.0.1") + 1];
			strcpy(envp[0], "SERVER_ADDR=127.0.0.1");
			envp[1] = new char[(int) strlen("CONTENT_TYPE=text/plain") + 1];
			strcpy(envp[1], "CONTENT_TYPE=text/plain");
			envp[2] = new char[(int) strlen("SERVER_PROTOCOL=HTTP/1.1") + 1];
			strcpy(envp[2],"SERVER_PROTOCOL=HTTP/1.1");
			envp[3] = new char[(int) strlen("SCRIPT_NAME=cgi-bin/cgi") + 1];
			strcpy(envp[3],"SCRIPT_NAME=cgi-bin/cgi");
			envp[4]=new char[(int) strlen("SERVER_PORT=") + 5];
			strcpy(envp[4], "SERVER_PORT=");
			strcat(envp[4], to_string(Server::port).c_str());
			envp[5] = new char[(int) strlen("QUERY_STRING=") + 1 + (int)strlen(request + 17)];
			strcpy(envp[5], "QUERY_STRING=");
			strcat(envp[5], request + 17);
			envp[6] = NULL;

			for (int i = 0; i <= 6; i++) {
				cerr << envp[i] << endl;
			}

			execve(argv[0], argv, envp);
			cerr << "cgi error\n";
			delete []envp;
			exit(1);
		}
		wait(&status);
		if (WIFEXITED(status)) {
			if (WEXITSTATUS(status) == 0) {
				string tmp_file = "cgi-bin/pid" + to_string(pid) + ".txt";
				int fd = open(tmp_file.c_str(), O_RDONLY);
				strcpy(client_request, "HTTP/1.0 200 OK modelserver\nServer: Model HTTP Server/0.1\nAllow: GET\nConnection: keep-alive\nDate: ");
				time_t tm = time(NULL);
				strcat(client_request, asctime(localtime(&tm)));
				strcat(client_request, "Content-length: ");
				char *cl = itoa(fcount(fd));	//fd empty, need cgi res
				strcat(client_request, cl);
				delete []cl;
				strcat(client_request, "\nContent-type: text/plain\n\n");
				len = strlen(client_request);
				send(client_sd, &client_request, len, 0);
				while((len = read(fd, client_request, 1024)) > 0) {
					send(client_sd, &client_request, len, 0);
				}
				close(fd);
				shutdown(client_sd, SHUT_RDWR);
				close(client_sd);
				
				return;
            } else {
				cerr << "CGI has finihed with status " << WEXITSTATUS(status) << endl;
				int fd = open("cgi.html", O_RDONLY);
				strcpy(client_request, "HTTP/1.0 500 Internal Server Error modelserver\nServer: Model HTTP Server/0.1\nAllow: GET\nConnection: keep-alive\nDate: ");
				time_t tm = time(NULL);
				strcat(client_request, asctime(localtime(&tm)));
				strcat(client_request, "Content-length: ");
				char *cl = itoa(fcount(fd));
				strcat(client_request, cl);
				delete []cl;
				strcat(client_request, "\nContent-type: text/html\n\n");
				len = strlen(client_request);
				send(client_sd, &client_request, len, 0);
				while((len = read(fd, client_request, 1024)) > 0) {
					send(client_sd, &client_request, len, 0);
				}
				close(fd);
				shutdown(client_sd, SHUT_RDWR);
				close(client_sd);
				
				return;
			}
        } else if (WIFSIGNALED(status)) {
			cerr << "CGI has finished with signal " << WIFSIGNALED(status) << endl;
			int fd = open("cgi.html", O_RDONLY);
			strcpy(client_request, "HTTP/1.0 500 Internal Server Error modelserver\nServer: Model HTTP Server/0.1\nAllow: GET\nConnection: keep-alive\nDate: ");
			time_t tm = time(NULL);
			strcat(client_request, asctime(localtime(&tm)));
			strcat(client_request, "Content-length: ");
			char *cl = itoa(fcount(fd));
			strcat(client_request, cl);
			delete []cl;
			strcat(client_request, "\nContent-type: text/html\n\n");
			len = strlen(client_request);
			send(client_sd, &client_request, len, 0);
			while((len = read(fd, client_request, 1024)) > 0) {
				send(client_sd, &client_request, len, 0);
			}
			close(fd);
			shutdown(client_sd, SHUT_RDWR);
			close(client_sd);
		
			return;
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
        	//printf("%s", request);
        	//printf("%s", client_request);   //otladka
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
    //printf("%s", client_request);       //otladka
	send(client_sd, &client_request, len, 0);
	while((len = read(fd, client_request, 1024))>0) {
		send(client_sd, &client_request, len, 0);
	}
	close(fd);
	shutdown(client_sd, SHUT_RDWR);
	close(client_sd);
}

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
    Server::port = atoi(argv[1]);
    Server server(atoi(argv[1]));
    server.Run();
    return 0;
}
