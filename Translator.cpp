#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <unistd.h>
#include<ctype.h>
#include <stdint.h>
#include <sys/mman.h>
#include <cstdint>
#include <unordered_map>
#include<unistd.h>

using namespace std;

unsigned int microsecond = 1000000;
// create a map of words and translations
unordered_map<string,string> words;
// print the words map
void table(unordered_map<string,string> input){
        cout << "Table of Words" << endl;
        
        for(unordered_map<string,string>::iterator it = input.begin(); it != input.end(); it++)
            cout << "Eng: " << it->first << " = Fr: " << it->second << endl;
        cout << endl; 
        cout << endl;
}

void printError(string message){
    perror(message.c_str());
    exit(0);
}
// print the buffer *Code from Sina
void print_buffer(string action, char buffer[], size_t bytes)
{
    printf("%s: %s (%ld bytes)\n", action.c_str(), buffer, bytes);
}
// Converts string to lowercase
string lowercase(string ans){
        for (int s = 0; s < ans.length(); s++){
        if(isalpha(ans[s])) {
            ans[s] = tolower(ans[s]);
        }
    }
    return ans;
}


int main(int argc, const char *argv[]){
    // Create the words map
    words.insert(pair<string,string>("hello","Bonjour"));
    words.insert(pair<string,string>("Girl","Fille"));
    words.insert(pair<string,string>("Boy","Garcon"));
    words.insert(pair<string,string>("Dog","Chien"));
    words.insert(pair<string,string>("Cat","Chat"));
    // Print the words table into console
    table(words);

    char buffer[256];

    // Set server info
    addrinfo hints, *server_info;
    memset(&hints, 0, sizeof(addrinfo));

    hints.ai_family = AF_INET;
    // UDP
    hints.ai_socktype = SOCK_DGRAM;
    // use my IP
    hints.ai_flags = AI_PASSIVE; 
    hints.ai_protocol = 0;
    // use the first input by user as the the port number 
    // NULL because it is the server connecting
    getaddrinfo(NULL, argv[1], &hints, &server_info);

    // create a server socket
    int server_socket = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

    // Error handling
    if(server_socket < 0){
        printError("Opening Socket error");
    }


    // bind the socket to the proper addrress
    bind(server_socket, server_info->ai_addr, server_info->ai_addrlen);

    cout << "Waiting for connections....." << endl;

    while(1)
    {
        memset(buffer, 0, sizeof(buffer));

        size_t recv_size = recvfrom(server_socket,buffer, sizeof(buffer), 0, server_info->ai_addr, &server_info->ai_addrlen);
        print_buffer("Received",buffer, recv_size);
        string input = (string) buffer;
        //input = lowercase(input);
        // check if input from indirection server is in the words map
        unordered_map<string,string>::iterator it = words.find(input);
        // if so return the french translation to indirection server
        if(it != words.end()){
            cout << "Found " << it->first << " French " << it->second << endl;
            //usleep(2*microsecond);
            sendto(server_socket,it->second.c_str(),strlen(it->second.c_str()),0,server_info->ai_addr, server_info->ai_addrlen);
            print_buffer("Sent",(char *)it->second.c_str(),it->second.size());
        } // if not send error 
        else {
            string fail = "The given word is not in the list of words!";
            sendto(server_socket,fail.c_str(),strlen(fail.c_str()),0,server_info->ai_addr, server_info->ai_addrlen);
            print_buffer("Sent",(char *)fail.c_str(),strlen(fail.c_str()));
        }
        memset(buffer, 0, sizeof(buffer));

    }

return 0;
}

