#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
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
#include <sstream>
#include <time.h>
#include <algorithm>


using namespace std;
#define hours 3
#define mins 34
// table of voting
unordered_map<int,pair<string,int> >voting_table;
vector<string> voted_IPs;

// Print map into console
void print_table(unordered_map<int,pair<string,int> >input){
        cout << "Candidates Information Table" << endl;
        
        for(unordered_map<int,pair<string,int> >::iterator it = input.begin(); it != input.end(); it++)
            cout << "Name: " << it->second.first << " id: " << it->first << " votes: " << it->second.second << endl;
        cout << endl; 
        cout << endl;
}
// Create a string with candidate names and id's
string create_table(unordered_map<int,pair<string,int> >input) {
    string result;
    result.append("\n");
    for(unordered_map<int,pair<string,int> >::iterator it = input.begin(); it != input.end(); it++){
        result.append("Name: " + it->second.first);
        result.append("  ID: " + to_string(it->first));
        result.append("\n");
    }
    return result;
}
// Create a string with the contents in the voting_table map
string create_report(unordered_map<int,pair<string,int> >input){
    string result;
    result.append("\n");
    for(unordered_map<int,pair<string,int> >::iterator it = input.begin(); it != input.end(); it++)
        result.append("Name: " + it->second.first + " id: " + to_string(it->first) + " votes: " + to_string(it->second.second) + "\n");
    
    return result;
}

void printError(string message){
    perror(message.c_str());
    exit(0);
}
// Prints out the buffer *code by Prof Sina
void print_buffer(string action, char buffer[], size_t bytes)
{
    printf("%s: %s (%ld bytes)\n", action.c_str(), buffer, bytes);
}

int main(int argc, const char *argv[]){
    // Initialize the table with values 
    voting_table.insert(pair<int,pair<string,int> >(1111,pair<string,int>("Ahad Hamirani",22)) );
    voting_table.insert(pair<int,pair<string,int> >(2222,pair<string,int>("Carey Williamson",33)) );
    voting_table.insert(pair<int,pair<string,int> >(3333,pair<string,int>("Sina Keshvadi",44)) );
    voting_table.insert(pair<int,pair<string,int> >(4444,pair<string,int>("Barry Allen",55)) );
    voting_table.insert(pair<int,pair<string,int> >(5555,pair<string,int>("Bruce Wayne",66)) );

    char buffer[256];

    int voter_id;

    int key = 5;
    // Print the table to console
    print_table(voting_table);

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

    //voted_IPs.push_back("127.0.0.1");

    cout << "Waiting for connections....." << endl;

    while(1){
        memset(buffer, 0, sizeof(buffer));
        // Set up time struct for select function
        time_t current_time;
        struct tm* ptime;
        time(&current_time);
        // Set up sockets for select function
        fd_set read_sock;

        FD_ZERO(&read_sock);
        FD_SET(server_socket,&read_sock);
        // store the number of file discriptors in int report
        int report = select(server_socket+1,&read_sock,NULL,NULL,NULL);
        // if there is a file ready to read from aka ready for recvfrom
        // Set ptime to the current universal time
        if(report > 0){
            ptime = gmtime(&current_time);
        }
        // recv info from indirection server 
        size_t recv_size = recvfrom(server_socket,buffer, sizeof(buffer), 0, server_info->ai_addr, &server_info->ai_addrlen);
        print_buffer("Received",buffer, recv_size);
        // break the buffer into the keyword and ip and store them
        char *temp = strtok(buffer,"\n");
        
        string input = (string) temp;
        cout << "input: " << input << endl;

        char *x = strtok(NULL,"\n");
        string ip_str;
        ip_str.push_back(*x);
        cout << "ip_str: " << ip_str << endl;
        // if keyword is view return list of candidates and their ids.
        if(input.compare("view") == 0){
            sendto(server_socket,create_table(voting_table).c_str(),strlen(create_table(voting_table).c_str()),0,server_info->ai_addr, server_info->ai_addrlen);
            print_buffer("Sent",(char *)create_table(voting_table).c_str(),create_table(voting_table).length());
        }
        // if keyword is report check for conditions 
        else if(input.compare("report") == 0){
            // Check if the curr time is above deadline time
            if(ptime->tm_hour <= hours){
                if(ptime->tm_min <= mins){
                    cout << "Hours: " << ptime->tm_hour << " Mins: " << ptime->tm_min << endl; 
                    string fail = "Votings in progress!\nYou can only view the results after voting deadline";
                    sendto(server_socket,fail.c_str(),strlen(fail.c_str()),0,server_info->ai_addr, server_info->ai_addrlen);
                    print_buffer("Sent",(char *)fail.c_str(),strlen(fail.c_str()));
                    continue;
                }
            }   // Check if client has already voted
                if ( std::find(voted_IPs.begin(), voted_IPs.end(), ip_str) != voted_IPs.end() ){
                    sendto(server_socket,create_report(voting_table).c_str(),strlen(create_report(voting_table).c_str()),0,server_info->ai_addr, server_info->ai_addrlen);
                    print_buffer("Sent",(char *)create_report(voting_table).c_str(),create_report(voting_table).length());
                }
                // if client has already voted return error 
                else {
                    string fail = "NO"; //You must vote before view the voting report!
                    sendto(server_socket,fail.c_str(),strlen(fail.c_str()),0,server_info->ai_addr, server_info->ai_addrlen);
                    print_buffer("Sent",(char *)fail.c_str(),strlen(fail.c_str()));
                }
        }
        // For vote
        else {
            // Check if client has already voted with this ip
            if ( std::find(voted_IPs.begin(), voted_IPs.end(), ip_str) != voted_IPs.end() ){
                string fail = "NO";
                sendto(server_socket,fail.c_str(),strlen(fail.c_str()),0,server_info->ai_addr, server_info->ai_addrlen);
                print_buffer("Sent",(char *)fail.c_str(),strlen(fail.c_str()));


            }
            else if (input.compare("vote") == 0){
                // set up and send encryption key to indirection server
                string encrypt_msg = "Encryption key: ";
                encrypt_msg.append(to_string(key));
                encrypt_msg.append("\n");

                sendto(server_socket,encrypt_msg.c_str(),strlen(encrypt_msg.c_str()),0,server_info->ai_addr, server_info->ai_addrlen);
                print_buffer("Sent",(char *)encrypt_msg.c_str(),strlen(encrypt_msg.c_str()) );

                memset(buffer,0,sizeof(buffer));
                // Wait until read socket is avalible; if so get the time
                int vote = select(server_socket+1,&read_sock,NULL,NULL,NULL);
                if(vote > 0){
                    ptime = gmtime(&current_time);
                    // Check if curr time is under or equal to voting deadline time
                    if(ptime->tm_hour >= hours){
                        if(ptime->tm_min >= mins){
                            cout << "Hours: " << ptime->tm_hour << " Mins: " << ptime->tm_min << endl; 
                            string fail = "Time to vote has passed! Unable to vote! \n";
                            recv_size = recvfrom(server_socket,buffer, sizeof(buffer), 0, server_info->ai_addr, &server_info->ai_addrlen);
                            print_buffer("Received",buffer, recv_size);
                            memset(buffer,0,sizeof(buffer));
                            sendto(server_socket,fail.c_str(),strlen(fail.c_str()),0,server_info->ai_addr, server_info->ai_addrlen);
                            print_buffer("Sent",(char *)fail.c_str(),strlen(fail.c_str()));
                            continue;
                        }
                    } // if curr time is under deadline time run normal voting
                        recv_size = recvfrom(server_socket,buffer, sizeof(buffer), 0, server_info->ai_addr, &server_info->ai_addrlen);
                        print_buffer("Received",buffer, recv_size);
                        input = (string) buffer;
                        // Change the buffer into int
                        string x = (string)input;
                        stringstream ss;
                        int encrypted_id;
                        ss << x;
                        ss >> encrypted_id;
                        // Decode message
                        voter_id = encrypted_id / key;
                        unordered_map<int,pair<string,int> >::iterator it = voting_table.find(voter_id);
                        // Check if id is in the voting table
                        // If so add the vote to the appropirate client
                        if(it != voting_table.end()){
                            it->second.second += 1;
                            string msg = "Successfully voted for " + it->second.first;
                            voted_IPs.push_back(ip_str);
                            sendto(server_socket,msg.c_str(),strlen(msg.c_str()),0,server_info->ai_addr, server_info->ai_addrlen);
                            print_buffer("Sent",(char *)msg.c_str(),msg.length());
                        }
                        // if not send invalid id message 
                        else {
                            string fail = "NO";//Invalid voter id!
                            sendto(server_socket,fail.c_str(),strlen(fail.c_str()),0,server_info->ai_addr, server_info->ai_addrlen);
                            print_buffer("Sent",(char *)fail.c_str(),strlen(fail.c_str()));
                        }
                    
                } 
            }
        }

    }

    return 0;
}