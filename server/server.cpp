#include <string.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include <bits/stdc++.h>
#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <ifaddrs.h>
#include <unistd.h>
#include <linux/if_link.h>
#include <regex.h>
#include <algorithm>

#define MAX_LINE_LENGTH 80
using namespace std;
#define TRUE 1

typedef struct server {
    int     serverID;
    int     neighborID[4];
    int     port;  
    char    ip[20]; 
	int		packets=0;
	int		sockfd=0;
} server;

typedef struct server_array {
    int         count;
    server*    servs[4]; //I set it to 4, we can up the amount if needed
    int        free_slot[4];	
} server_array;

typedef struct costs {
    int cost_array[4][4];
} costs;

typedef struct connection_node {
	char	ip_addr[20];
	int		port;
	int		sock_fd;
} connection_node;

typedef struct connection_array {
	int					count;
	connection_node*	conns[10];
	int					free_conns[10];
} connection_array;

void fillServInfo();
void initServArr(server_array *s);
void addServer(server *s,server_array *a);
server* initServer(string str,server *s);
int initCost(int cost_arr[][4],string line);
void displayCost(int cost[][4]);
void initArr(int cost[][4]);
vector<int> findNeighbors(int cost[][4],int serverID);
void packets (server *s);

void initialize_connection_array(connection_array *ca);
void add_connection_node(connection_node *cn, connection_array *ca);
void remove_connection_node(connection_node *cn, connection_array *ca);
void remove_connection_node_idx(int idx, connection_array *ca);
void close_connection_array(connection_array *ca);
int connector(connection_array *ca, char* ip, int port, char* my_ip, int my_port, fd_set *read, fd_set *send);
void *get_in_addr(struct sockaddr *sa);
connection_node* create_connection_node(int sock_fd);
void myip(char* ip);
void disable(connection_array *ca, int num,server_array servarr, vector<int> nbrID);
void update(int serverA, int serverB, int newCost,int costarray[][4]);

int main(int argc, char* argv[]){


server server1,server2,server3,server4;
server_array servarr;
initServArr(&servarr);
int cost[4][4];
initArr(cost);
int myID;
int routingUpdate;
vector<int> nbrID;
char* top_file = argv[2];
char* routing_interval = argv[4];
int sockfd;
int connectionCount=0;


    string tinput = argv[1];
    string iinput = argv[3];
	
    if(tinput != "-t" ||iinput != "-i"){
        printf("Command line input incorrect. Please try again.\n");
        exit(1);
    }else{
    ifstream myfile(top_file);


        if(myfile.is_open()){

        string line;
        int i=0;
        vector<int> r;
        while(getline(myfile,line)){
            //stringstream ss(line);
            if(i>5){
               myID= initCost(cost,line);
            }else{
            switch (i)
            {
                case 2: addServer(initServer(line,&server1),&servarr);
                        break;
                case 3: addServer(initServer(line,&server2),&servarr);
                        break;
                case 4: addServer(initServer(line,&server3),&servarr);
                        break;
                case 5: addServer(initServer(line,&server4),&servarr);
                        break;
            }
            }i++;
        }
        displayCost(cost);
        myfile.close();

		//need to initialize to 0
		//server1.packets = 0; 
		//server1.packets++; //for testing
		
     }else {
		 cout<<"Unable to open file";
		 exit(1);
	 }
	}

    // cout<<"ServerID:"<<myID<<endl;
    for(int i=0;i<4;i++){
        cout<<servarr.servs[i]->serverID<<" ";
        cout<<servarr.servs[i]->ip<<" ";
        cout<<servarr.servs[i]->port<<endl;
    }
	nbrID=findNeighbors(cost,myID);
	if(!nbrID.empty()){
		printf("neighbors: ");
	for(int i=0;i<nbrID.size();i++){
		if(nbrID.size()< 3){
			cout<<nbrID[i] << " ";
			if(i == 1){
				cout << "inf";
			}
		}
		else{
		cout<<nbrID[i]<<" ";
		}
		}
	}

    connection_array ca;
	initialize_connection_array(&ca);

	// read fd sets
	fd_set master_read;
	fd_set read_fds;

	// send fd sets
	fd_set master_send;
	fd_set send_fds;

	int read_fdmax;
	int send_fdmax;
	int fdmax;

	int listener;
	int newfd;
	struct sockaddr_storage remoteaddr;
	socklen_t addrlen;

	char buf[256];
	int nbytes;

	char remoteIP[INET6_ADDRSTRLEN];

	int yes=1;
	int i, j,rv;

	struct addrinfo hints, *ai, *p;

	FD_ZERO(&master_read);
	FD_ZERO(&read_fds);
	FD_ZERO(&master_send);
	FD_ZERO(&send_fds);

	printf("\nServer started.\n");

	memset(&hints, 0, sizeof hints);
	hints.ai_family		= AF_INET;
	hints.ai_socktype	= SOCK_STREAM;
	hints.ai_flags		= AI_PASSIVE;
    const char *port=to_string(servarr.servs[myID-1]->port).c_str();
	if ((rv = getaddrinfo(NULL, port, &hints, &ai)) != 0) {
		fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
		exit(1);
	}

	for (p=ai; p != NULL; p = p->ai_next) {
		listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (listener < 0) {
			continue;
		}

		setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

		if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
			close(listener);
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "selectserver: failed to bind\n");
		exit(2);
	}

	freeaddrinfo(ai);

	if (listen(listener, 10) == -1) {
		perror("listen");
		exit(3);
	}
	
	FD_SET(0, &master_read);
	FD_SET(listener, &master_read);

	read_fdmax = listener;
	send_fdmax = 0;
	fdmax = listener;
    
  
	if(!nbrID.empty()){
		for(int i=0;i<nbrID.size();i++){
			regex_t ip_regex, port_regex; int reti, their_port;

				char *connection_ip, *port_str;
				char *newline_ip, *newline_port;

				int result=0;
				their_port = servarr.servs[nbrID[i]-1]->port;
				connection_ip=servarr.servs[nbrID[i]-1]->ip;
				char* my_ip;
				my_ip = (char *)malloc(sizeof(char));
				int my_port;
				
				myip(my_ip);
				my_port = servarr.servs[myID-1]->port;
				printf("connecting to %s, on port %d\n",connection_ip,their_port);

				result = connector(&ca, connection_ip, their_port, my_ip, my_port, &master_read, &master_send);
				if(result!=0){
					servarr.servs[nbrID[i]-1]->sockfd=result;
					printf("NeighborID: %d, Sockfd:%d \n", nbrID[i],result);
					//send(result,)
				}
				free(my_ip);
		}
	}

    while (1) {
		read_fds = master_read;
		send_fds = master_send;
		if (select(fdmax+1, &read_fds, &send_fds, NULL, NULL) == -1) {
			perror("Select");
			exit(4);
		}
		
		if (FD_ISSET(0, &read_fds)) { // Handle command input
			// command input
			char *token;
			fgets(buf,256,stdin);
			token=strtok(buf," ");
			char *newline =strchr(token, '\n');
			if(newline)*newline=0;
			if (strcmp(token,"update")==0){
				char*serverA,*serverB,*newCost,*newline_idx;
				int sA,sB,nc;
				serverA=strtok(NULL," ");
				sA=atoi(serverA);
				serverB=strtok(NULL," ");
				sB=atoi(serverB);
				newCost=strtok(NULL," ");
				nc=atoi(newCost);

				newline_idx = strchr(newCost, '\n');
				if (newline_idx) *newline_idx = 0;
				//cost[sA-1][sB-1]=nc;
				//cost[sB-1][sA-1]=nc;
				//displayCost(cost);
				update(sA,sB,nc,cost);
				send(servarr.servs[sB-1]->sockfd,newCost,strlen(newCost),0);
				continue;
			}
			if(strcmp(token,"disable")==0){
				char *id_str, *newline_idx;
				int idx;

				id_str = strtok(NULL, " ");
				if (id_str == NULL) {
					printf("error: USAGE: terminate <connection id>\n");
					continue;
				}
				newline_idx = strchr(id_str, '\n');
				if (newline_idx) *newline_idx = 0;

				idx = atoi(id_str);
				vector<int>::iterator it=find(nbrID.begin(),nbrID.end(),idx);
				if(it!=nbrID.end()){
					disable(&ca, idx,servarr,nbrID);
					FD_CLR(servarr.servs[idx-1]->sockfd,&master_read);
					FD_CLR(servarr.servs[idx-1]->sockfd,&master_send);

					nbrID.erase(it);
				}else{
					printf("serverID %d, is not a neighbor",idx);
				}

			}
			else if(strcmp(token,"crash")==0){
				char message[]="crash";
				int len=strlen(message);
				//string s ="crash";
				//strcpy(message,s.c_str());
				for(int i=0;i<nbrID.size();i++){
							send(servarr.servs[nbrID[i]-1]->sockfd,message,len,0);
						}
						exit(1);
			}

			else if(strcmp(token, "packets") == 0){
				packets(&server1);
			}

			else if(strcmp(token, "step") == 0){
				char message[] = "";
				int len = strlen(message); 
				
				for(int i=0;i<nbrID.size();i++){
					send(servarr.servs[nbrID[i]-1]->sockfd,message,len,0);
				}
			}

			else if(strcmp(token, "display") == 0){
				displayCost(cost); 
			}
			
			else{
				cout << "Incorrect command. Please try again " << endl; 
			}
			
		}

		if (FD_ISSET(listener, &read_fds)) { // Process New Connection
			addrlen = sizeof remoteaddr;
			newfd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen);

			if (newfd == -1) {
				perror("accept");
			} else {
				//needed a way to know which new sockfd is for our neihbors. Could not figure out a way to get neighbors listening port
				for(int i =0; i<nbrID.size();i++){
					if(servarr.servs[nbrID[i]-1]->sockfd==0){
						servarr.servs[nbrID[i]-1]->sockfd=newfd;//assign new connection socket into server struct
						printf("Neighborid:%d , connectsockfd:%d \n ", nbrID[i],servarr.servs[nbrID[i]-1]->sockfd);
						break;
					}
				}
				
				
				connectionCount++;
				printf("Count: %d\n", ca.count);
				if (ca.count >= 4) {
					int bytes_sent;
					char rejection[]  = "Rejected connection: The client you are trying to connect to is full. Try again later.";
					bytes_sent = send(newfd, rejection, strlen(rejection), 0);
        	                        if (bytes_sent < strlen(rejection)) printf("Message not fully sent");
	                                else if (bytes_sent == strlen(rejection)) printf("Rejected socket [%d]\n", newfd);
					close(newfd);
					continue;
				} else {
					FD_SET(newfd, &master_read);
					FD_SET(newfd, &master_send);
					if (newfd > read_fdmax) {
						read_fdmax = newfd;
					}
					if (newfd > send_fdmax) {
						send_fdmax = newfd;
					}
	
					if (read_fdmax > send_fdmax) fdmax = read_fdmax;
					else fdmax = send_fdmax;

					connection_node* cn = create_connection_node(newfd);

					add_connection_node(cn, &ca);
	
					printf(" New connection from %s on port %d\n", inet_ntop(remoteaddr.ss_family, get_in_addr((struct sockaddr *)&remoteaddr), remoteIP, INET6_ADDRSTRLEN), cn->port);
				}
			}
		}

		int k;
		for (k = 0; k < 10; k++) {
			if (ca.conns[k] && FD_ISSET(ca.conns[k]->sock_fd, &read_fds)) {
				if ((nbytes = recv(ca.conns[k]->sock_fd, buf, sizeof buf, 0)) <= 0) { // error or hangup
					if (nbytes == 0) { // hung up
						printf("Connection [%d] hung up, removing from list.\n", nbrID[k]);
						FD_CLR(ca.conns[k]->sock_fd, &master_read);
						FD_CLR(ca.conns[k]->sock_fd, &master_send);
						//remove_connection_node_idx(k+1, &ca);
					}
					else perror("recv");
				} else { // got some data
					printf("Message received from %s\n", ca.conns[k]->ip_addr);
					printf("Sender's port: %d\n", ca.conns[k]->port);
					printf("Message: \"%s\"\n", buf);
					server1.packets++; 

					char msg[]="crash";
					if(strcmp(buf,"crash")==0){
						
						for(int i=0;i<nbrID.size();i++){
							send(servarr.servs[nbrID[i]-1]->sockfd,buf,strlen(buf),0);
						}
						exit(1);
					}
					else {
						int c=atoi(buf);
						update(myID,nbrID[k],c,cost);
					}
				}
			}
		}
	}

	close_connection_array(&ca);

	return 0;
  
}

void initServArr(server_array *a){
    a->count=0;
    int i;
    for(i=0;i<4;i++){
        a->servs[i]=NULL;
        a->free_slot[i]=1;
    }
}
void addServer(server* s,server_array *a){
    int i;
    for(i=0;i<4;i++){
        if(a->free_slot[i]==1){
            a->servs[i]=s;
            a->free_slot[i]=0;
            break;
        }
    }
}
void startUp(string str){
    
}
server* initServer(string str, server *s){
    string token;
    int j=0;
    stringstream ss(str);
    while(getline(ss,token,' ')){
        switch(j)
        {
            case 0: s->serverID=stoi(token);
                    break;
            case 1: strcpy(s->ip,token.c_str());
                    break;
            case 2: s->port=stoi(token);
                    break;
        }
        j++;
    }
    //cout<<"ID: "<<s->serverID<<" ";
    //cout<<"IP: "<<s->ip<<" ";
    //cout<<"Port: "<<s->port<<endl;
    return s;
}
int initCost(int cost_arr[][4],string line){
    int host, neighbor,cost;
    string token;
    int j=0;
    stringstream ss(line);
    while(getline(ss,token,' ')){
        switch (j)
        {
            case 0: host=stoi(token);
                    break;
            case 1: neighbor=stoi(token);
                    break;
            case 2: cost=stoi(token);
                    break;
        }j++;
    }
    cost_arr[host-1][neighbor-1] = cost;
    return host;
}
void displayCost(int cost[][4]){
    for(int i =0;i<4;i++){
        for(int j=0;j<4;j++){
            cout<<cost[i][j]<<" ";
        } cout<<endl;
    }
}
void initArr(int cost[][4]){
    for(int i =0;i<4;i++){
        for(int j=0;j<4;j++){
            cost[i][j]=0;
        }
    }
}

vector<int> findNeighbors(int cost[][4], int serverID){
	vector<int> temp;
	for(int i=0;i<4;i++){
		if(cost[serverID-1][i]!=0){
			temp.push_back(i+1);
		}
	}
	return temp;
}
connection_node* create_connection_node(int sock_fd) {

	connection_node *cn;
    cn = (connection_node*)malloc(sizeof(connection_node));
	int port;
	struct sockaddr_in addr;
	socklen_t addr_size = sizeof(struct sockaddr_in);
	int res = getpeername(sock_fd, (struct sockaddr *)&addr, &addr_size);

	strcpy(cn->ip_addr, inet_ntoa(addr.sin_addr));
	//printf("IP Address: %s\n", cn->ip_addr);
	cn->port = ntohs(addr.sin_port);
	cn->sock_fd = sock_fd;

	return cn;
}

void initialize_connection_array(connection_array *ca) {
	ca->count = 1;

	int i;
	for (i = 0; i < 10; i++) {
		ca->conns[i] = NULL;
		ca->free_conns[i] = 1;
	}

	return;
}

void add_connection_node(connection_node *cn, connection_array *ca) {
	int i;
	for (i = 0; i < 10; i++) {
		if (ca->free_conns[i] == 1) {
			ca->conns[i] = cn;
			ca->count++;
			ca->free_conns[i] = 0;
			return;
		}
	}

	perror("Connection Array full please remove a connection to add this one.");
	return;
}

void remove_connection_node(connection_node *cn, connection_array *ca) {
	int i;
	for (i = 0; i < 10; i++) {
		if (ca->conns[i] == cn) {
			free(ca->conns[i]);

			ca->conns[i] = NULL;
			ca->free_conns[i] = 1;
			ca->count--;
			return;
		}
	}

	perror("Could not find connection in the connection array.");
}

void remove_connection_node_idx(int idx, connection_array *ca) {

	if (ca->free_conns[idx] == 0) {
		free(ca->conns[idx]);

		ca->conns[idx] = NULL;
		ca->count--;
		ca->free_conns[idx] = 1;
		return;
	} else {
		perror("No active connection at that index, slot already empty");
		return;
	}
}

void close_connection_array(connection_array *ca) {
	int i;
	for (i = 0; i < 10; i++) {
		if (ca->free_conns[i] == 0) { // in use
			free(ca->conns[i]);
			ca->conns[i] = NULL;
			ca->free_conns[i] = 1;
			ca->count--;
		}
	}
	return;
}


int connector(connection_array *ca, char* ip, int port, char* my_ip, int my_port, fd_set *read, fd_set *send) {

    int sockfd;
    int connected;

    for (int i = 0; i < 10; i++) {
        // Error checking for existing connection, self connection and space

        if (ca->count == 0) {
            // Setting up the socket for connection
            if (!strcmp(my_ip, ip) && (my_port == port)) {
                printf("You cannot connect to yourself, please try again.\n");
                return 0;
            }
            sockfd = socket(AF_INET, SOCK_STREAM, 0);
            struct sockaddr_in server_add;
            server_add.sin_family = AF_INET;
            server_add.sin_port = htons(port);
            inet_aton(ip, &server_add.sin_addr);

            // Checking if the connection was successful
            if((connected = connect(sockfd, (struct sockaddr *)&server_add, sizeof(server_add))) < 0) {
                printf("Connection was unsuccessful. Please try again.\n");
				return 0;
            }
            else {
                connection_node *cn = create_connection_node(sockfd);
                add_connection_node(cn, ca);
                printf("Connection to %s, on port %d!\n", ip, port);
                return sockfd;
            }
        }
       if (ca->count == 4) {
            printf("You're at the maxmium number of connections [10]. Please terminate a connection to add another.\n");
            return 0;
        }

        // Setting up the socket for connection
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in server_add;
        server_add.sin_family = AF_INET;
        server_add.sin_port = htons(port);
        inet_aton(ip, &server_add.sin_addr);

        // Checking if the connection was successful
        if((connected = connect(sockfd, (struct sockaddr *)&server_add, sizeof(server_add))) < 0) {
            printf("Connection was unsuccessful. Please try again.\n");
			return 0;
        }
        else {
            connection_node *cn = create_connection_node(sockfd);
            add_connection_node(cn, ca);
	        FD_SET(sockfd, read);
	        FD_SET(sockfd, send);
            printf("Connection established with %s, on port %d!\n", ip, port);
            return sockfd;
        }
    }
	return 0;
}
void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
void myip(char* ip) {
	// struct ifaddrs *ifaddr, *ifa;
	// int family, s;
	// char host[NI_MAXHOST];

	
    struct ifaddrs *ifap, *ifa;
    struct sockaddr_in *sa;
    char *addr;
    getifaddrs(&ifap);
    for(ifa=ifap ; ifa; ifa = ifa->ifa_next){
        if(ifa->ifa_addr && ifa->ifa_addr->sa_family==AF_INET){
            sa=(struct sockaddr_in*)ifa->ifa_addr;
            addr=inet_ntoa(sa->sin_addr);
            //printf("%s\n",addr);
            break;
        }
	}
}
void packets(server *s) {
    int packets = s->packets;
    cout << "Number of packets: " << packets << endl;
    s->packets = 0; //sets packets to 0 after command has been called
}

void disable(connection_array *ca, int nbr, server_array servarr, vector<int> nbrID) {

	close(servarr.servs[nbr-1]->sockfd);
	   for(int i=0;i<nbrID.size();i++){
		   if(nbrID[i]==nbr){
			   remove_connection_node_idx(i, ca);
		   }
	   }
	        printf("Connection [%d] was terminated.\n", nbr);
	    
	}
	
	void update(int serverA, int serverB, int newCost, int costarray[][4]){
		costarray[serverA-1][serverB-1]=newCost;
		costarray[serverB-1][serverA-1]=newCost;
		displayCost(costarray);
	}