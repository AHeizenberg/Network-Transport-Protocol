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
#include <sstream>

using namespace std;
// rates map
unordered_map<string,double> rates;
// print out the rates map
void table(unordered_map<string,double> input){
        cout << "Currency Exchange Table" << endl;
        
        for(unordered_map<string,double>::iterator it = input.begin(); it != input.end(); it++)
            cout << "CAD to" << it->first << " = " << it->second << endl;
        cout << endl; 
        cout << endl;
}

void printError(string message){
    perror(message.c_str());
    exit(0);
}
// print buffer *Code from Prof Sina
void print_buffer(string action, char buffer[], size_t bytes)
{
    printf("%s: %s (%ld bytes)\n", action.c_str(), buffer, bytes);
}

int main(int argc, const char *argv[]){

    char buffer[256];
    // create the table with exchange rate values
    rates.insert(pair<string,double>("USA",0.790000)); // USA
    rates.insert(pair<string,double>("EUR",0.690000)); // Euro
    rates.insert(pair<string,double>("GBP",0.580000)); // Bristish Pound
    rates.insert(pair<string,double>("BTC",0.000016)); // Bitcoin
    rates.insert(pair<string,double>("ETH",0.000211)); // Ethereum (bitcoin)

    string convertFrom;
    string convertTo;
    double value;
    // print out the table to console
    table(rates);

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

    while(1){

        memset(buffer, 0, sizeof(buffer));
        // recv message and store into buffer
        size_t recv_size = recvfrom(server_socket,buffer, sizeof(buffer), 0, server_info->ai_addr, &server_info->ai_addrlen);
        print_buffer("Received",buffer, recv_size);
        string x = (string) buffer;
        // break the input buffer into parts seperated by space and store into input vector
        vector<string> inputs;
        istringstream f(x);
        string s;    
        while (getline(f, s, ' ')) {
            inputs.push_back(s);
        }
        
        stringstream ss;

        // Check if too many arguments are provided
        if(inputs.size() == 3){
            convertFrom = inputs[0];
            convertTo = inputs[1];
            ss << inputs[2];
            ss >> value;

            // Check if first argument is CAD
            if (convertFrom.compare("CAD") == 0){

                unordered_map<string,double>::iterator it = rates.find(convertTo);

                // Check if convert to string is in the table
                if(it != rates.end()){
                    double result = value * it->second; 
                    string output = to_string(result);
                    cout << "Found " << it->first << " Exchange rate " << it->second << endl;
                    sendto(server_socket,output.c_str(),strlen(output.c_str()),0,server_info->ai_addr, server_info->ai_addrlen);
                    print_buffer("Sent",(char *)output.c_str(),output.length());
                    continue;
                } // if not send error to indirection server 
                else {
                string fail = "Convert to value not found in table! Please try again";
                sendto(server_socket,fail.c_str(),strlen(fail.c_str()),0,server_info->ai_addr, server_info->ai_addrlen);
                print_buffer("Sent to indirection server",(char *)fail.c_str(),strlen(fail.c_str()));
                continue;
                }

            // send error if first argument is not CAD
            } else {
                string fail = "First argument must be CAD! Please try again";
                sendto(server_socket,fail.c_str(),strlen(fail.c_str()),0,server_info->ai_addr, server_info->ai_addrlen);
                print_buffer("Sent to indirection server",(char *)fail.c_str(),strlen(fail.c_str()));
                continue;
            }

        // if more than 3 arguments send error to indirection server    
        } else {
            string fail = "Too many arguments. Please enter only 3 valid arguments";
            sendto(server_socket,fail.c_str(),strlen(fail.c_str()),0,server_info->ai_addr, server_info->ai_addrlen);
            print_buffer("Sent to indirection server",(char *)fail.c_str(),strlen(fail.c_str()));
            continue;
        }
    }

    return 0;
}