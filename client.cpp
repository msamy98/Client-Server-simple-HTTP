#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <bits/stdc++.h>

using namespace std;

//#define PORT "3490" // the port client will be connecting to 

#define MAXDATASIZE 1024 // max number of bytes we can get at once 

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	//if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	

	//return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
	int sockfd, numbytes, sentBytes=0;  
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	if (argc != 3) {
	    fprintf(stderr,"usage: client hostname or port number\n");
	    exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			perror("client: connect");
			close(sockfd);
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	string stemp = s;
	cout << "client: connecting to " + stemp << endl;

	freeaddrinfo(servinfo); // all done with this structure

	ifstream infile("requests.txt");
	vector<string> lines;
	string line, temp;
	string space = "[\\s*]";
	while (getline(infile,temp))
	{
		
		istringstream iss(line);
		if(!iss){
			break;
		}
		if(!(temp.empty() || all_of(temp.begin(), temp.end(), [](char c){return isspace(c);}))){
			line = line + "\n" + temp;
			continue;
		}
		line.erase(0,1);
		lines.push_back(line);
		line = "";
	}
	
	cout << lines.size() << endl;
	for (auto& line : lines){
		cout << "for" << endl;
		char* request;
		request = &line[0];
		int reqlen = strlen(request);
		if((sentBytes=send(sockfd, request, reqlen-1, 0)) == -1){
			perror("send");
			exit(1);
		}
		cout << sentBytes << endl;
		cout << reqlen << endl;
		cout << request << endl;
		if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
			perror("recv");
			exit(1);
		}
		buf[numbytes] = '\0';
		string buftemp = buf;
		cout << "client: received '" + buftemp + "'" << endl;;
	}
	close(sockfd);

	return 0;
}
