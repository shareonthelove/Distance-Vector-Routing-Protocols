#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <vector>
#include <bits/stdc++.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#define MAX_LINE_LENGTH 80
using namespace std;

typedef struct server {
    int     serverID;
    int     neighbors[4];
    int     port;
    char    ip[20];
    int     packets;
} server;

typedef struct server_array {
    int         count;
    server* servs[4]; //I set it to 4, we can up the amount if needed
    int        free_slot[4];
} server_array;

typedef struct costs {
    int     cost_array[4][4];
} costs;

void fillServInfo();
void initServArr(server_array* s);
void addServer(server* s, server_array* a);
void initServer(string str, server* s);
void initCost(int cost_arr[][4], string line);
void displayCost(int cost[][4]);
void initArr(int cost[][4]);

void packets(server s);

int main(int argc, char* argv[]) {

    server server1, server2, server3, server4;
    server_array servarr;
    initServArr(&servarr);
    int cost[4][4];
    initArr(cost);
    char* top_file = argv[2];
    char* routing_interval = argv[4];

    ifstream myfile(top_file);

    if (myfile.is_open())
    {
        string line;
        int i = 0;
        vector<int> r;
        while (getline(myfile, line)) {
            //stringstream ss(line);
            if (i > 5) {
                initCost(cost, line);
            }
            else {
                switch (i)
                {
                case 2: initServer(line, &server1);
                    break;
                case 3: initServer(line, &server2);
                    break;
                case 4:initServer(line, &server3);
                    break;
                case 5: initServer(line, &server4);
                    break;
                }

            }i++;
        }
        displayCost(cost);
        myfile.close();
        //for testing, would be incremented when server gets a vector
        server1.packets++; 
        packets(server1); 
    }
    else cout << "Unable to open file" << endl;
}
void fillServInfo() {

}
void initServArr(server_array* a) {
    a->count = 0;
    int i;
    for (i = 0; i < 4; i++) {
        a->servs[i] = NULL;
        a->free_slot[i] = 1;
    }
}
void addServer(server* s, server_array* a) {
    int i;
    for (i = 0; i < 4; i++) {
        if (a->free_slot[i] == 1) {
            a->servs[i] = s;
            a->free_slot[i] = 0;
        }
    }
}
void initServer(string str, server* s) {
    string token;
    int j = 0;
    stringstream ss(str);
    while (getline(ss, token, ' ')) {
        switch (j)
        {
        case 0: s->serverID = stoi(token);
            break;
        case 1: strcpy(s->ip, token.c_str());
            break;
        case 2: s->port = stoi(token);
            break;
        }
        j++;
    }
    cout << "ID: " << s->serverID << " ";
    cout << "IP: " << s->ip << " ";
    cout << "Port: " << s->port << endl;
}
void initCost(int cost_arr[][4], string line) {
    int host, neighbor, cost;
    string token;
    int j = 0;
    stringstream ss(line);
    while (getline(ss, token, ' ')) {
        switch (j)
        {
        case 0: host = stoi(token) - 1;
            break;
        case 1: neighbor = stoi(token) - 1;
            break;
        case 2: cost = stoi(token);
            break;
        }j++;
    }
    cost_arr[host][neighbor] = cost;
}
void displayCost(int cost[][4]) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            cout << cost[i][j] << " ";
        } cout << endl;
    }
}
void initArr(int cost[][4]) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            cost[i][j] = 0;
        }
    }
}
//displays how many packets/distance vectors the server has received for host server
void packets(server s) {
    int packets = s.packets;
    cout << "Number of packets: " << packets << endl;
    s.packets = 0; //sets packets to 0 after command has been called
    cout << s.packets << endl; 
}