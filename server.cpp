#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <fstream>
#include <sstream>
#include <bits/stdc++.h>


#define STB_IMAGE_IMPLEMENTATION

using namespace std;
#define PORT "3490"
#define MAXDATASIZE 1024  // the port users will be connecting to

#define BACKLOG 10	 // how many pending connections queue will hold

void sigchld_handler(int s)
{
	(void)s; // quiet unused variable warning

	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}

string response(string request, int new_fd){
	
	string temp, path, filecontent, response, version;
	temp = request.substr(request.find(" ")+2);
	path = temp.substr(0,temp.find(" "));
	temp = request.substr(request.find(path)+path.length()+1);
	version = temp.substr(0, temp.find("\n"));
	temp = request.substr(0,request.find(" "));
	if(temp == "GET"){
		ifstream file(path,ios_base::binary);
		temp = "";
		FILE *fileb;
		char *pathc;
		pathc = &path[0];
		fileb = fopen(pathc,"rb");
			if(fileb){
				
				string filecon;
				char buf [MAXDATASIZE];
				int count = fread(buf, sizeof(char), MAXDATASIZE-1, fileb);
				buf[count] = '\0';
				filecon = buf;
				response = version + " 200 OK\r\n" + filecon;
				
				fclose(fileb);
			}else{
			response = version + " 404 NOT FOUND \r\n";
		}
	}
	else{
		response = version + " 200 OK \r\n";
		int bytes_recieved;
		char *buf;
		if ((bytes_recieved = recv(new_fd,buf,MAXDATASIZE-1,0)) == -1)
					perror("receive");
				buf[bytes_recieved] = '\0';
				string buftemp = buf;
				cout << "server: received: '" + buftemp + "'" <<endl;
	}
	return response;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) 
		return &(((struct sockaddr_in*)sa)->sin_addr);

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	cout << "server: waiting for connections..." << endl;

	while(1) {  // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
			string stemp = s;
		cout << "server: got connection from " + stemp << endl;
		if (!fork()) { // this is the child process
			close(sockfd); // child doesn't need the listener

			int bytes_recieved;
			char buf[MAXDATASIZE];
			if ((bytes_recieved = recv(new_fd,buf,MAXDATASIZE-1,0)) == -1)
				perror("receive");
			buf[bytes_recieved] = '\0';
			string buftemp = buf;
			cout << "server: received: '" + buftemp + "'" <<endl;

			string request = buf;
			string responsemsg = response(request,new_fd);
			char* msg;
			msg = &responsemsg[0];
			int len = strlen(msg);
			if (send(new_fd, msg, len, 0) == -1)
				perror("send");
			close(new_fd);
			exit(0);
		}
		close(new_fd);  // parent doesn't need this
	}
	return 0;
}



