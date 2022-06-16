#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <fstream>
#include <vector>
#include<ctype.h>
#include <stdint.h>
#include <sys/mman.h>
#include <cstdint>
#include <unordered_map>
#include <sstream>
#include <string>


#define BUFSIZE 256
using namespace std;
// create the main menu string
string main_menu(){
    string result;
    result.append("\n");
    result.append("1. Translate");
    result.append("\n");
    result.append("2. Currency Exchange");
    result.append("\n");
    result.append("3. Vote");
    result.append("\n");
    result.append("4. View Voting results");
    result.append("\n");
    result.append("5. View All candidates");
    result.append("\n");
    result.append("0. Close connection");
    result.append("\n");
    result.append("Choice: ");
    return result;
}
// print the buffer to console *Code by prog Sina*
void print_buffer(string action, char buffer[], size_t bytes)
{
    printf("%s: %s (%ld bytes)\n", action.c_str(), buffer, bytes);
}
// Remove the enter keystroke from char array
string remove_enter(char *input){
        string choice;
        char *temp = strtok(input,"\r\n\r\n");
        choice.append(temp);
        return choice;
}

void printError(string message){
    perror(message.c_str());
    exit(0);
}
// Check if run client function returned error meaning UDP server timed out
 string check_timeout(string x){
    if(x.compare("ERROR") == 0){
        x = "Microserver timed out!\n";
        return x;
    }
    else{
        return x;
    }
}

// Takes in IP, port number and string
// Creates a client udp socket and sends the string to the appropriate server
// returns the string recvd from the server
string run_client(const char *IP, string port, string output){
            


    char buffer[256];
    size_t numbytes; 

    addrinfo client_hints, *client_info;
    memset(&client_hints,0,sizeof(client_hints));

    client_hints.ai_family = AF_INET;
    client_hints.ai_socktype = SOCK_DGRAM;

    cout << "Port Number: " << port << endl;
    cout << "IP: " << IP << endl;

    if ((getaddrinfo(IP,port.c_str(),&client_hints,&client_info)) != 0)
    {
        fprintf(stderr, "getaddrinfo() failed.\n");
    }
    else {
        cout << "getaddr successful" << endl;
    }  
    
    int client_socket = socket(client_info->ai_family, client_info->ai_socktype, client_info->ai_protocol);
    if(client_socket < 0){
        printError("Client socket creation error");
    } 
    else {
        cout << "client socket creation successful" << endl;
    }

    if ((numbytes = sendto(client_socket, output.c_str(), strlen(output.c_str()), 0, client_info->ai_addr, client_info->ai_addrlen)) == -1)
    {
        perror("sendto() failed");
        exit(1);
    }
    print_buffer("Sent to microserver",(char *)output.c_str(),strlen(output.c_str()));
    memset(buffer,0,sizeof(buffer));
    // Set up time struct and socket for select
    fd_set set;
    struct timeval timeout;

    FD_ZERO(&set);
    FD_SET(client_socket,&set);
    // 3 second time out
    timeout.tv_sec = 3;
    timeout.tv_usec = 0;
    // store file discriptor into rv 
    int rv = select(client_socket+1,&set,NULL,NULL,&timeout);
    // if no read file discriptors within 3 seconds send time out error
    if(rv == 0){
        perror("UDP recvfrom timeout");
        close(client_socket);
        return "ERROR";
    } 
    else {
    memset(buffer,0,sizeof(buffer));
    size_t recv_size = recvfrom(client_socket,buffer, sizeof(buffer), 0, client_info->ai_addr, &client_info->ai_addrlen);
    print_buffer("Recieved from microserver",buffer,sizeof(buffer));

    output = string (buffer);
    close(client_socket);
    return output;
    }
}

int main(int argc, const char *argv[])
{

    string portNum;

    // Set server info
    addrinfo hints, *server_info;
    memset(&hints, 0, sizeof(addrinfo));

    hints.ai_family = AF_INET;
    // TCP
    hints.ai_socktype = SOCK_STREAM;
    // use my IP
    hints.ai_flags = AI_PASSIVE; 
    // use the first input by user as the the port number 
    // NULL because it is the server connecting
    getaddrinfo(NULL, argv[2], &hints, &server_info);

    // create a server socket
    int server_socket = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

    // bind the socket to the proper addrress
    bind(server_socket, server_info->ai_addr, server_info->ai_addrlen);
    // TCP listen
    listen(server_socket, 5);
    // assign client socket to accp_socket
    int accp_socket = accept(server_socket,server_info->ai_addr,&server_info->ai_addrlen);
    cout << "connection accpeted" << endl;

    char buffer[1024];
    // Recv initial message from client; we dont really care what this says
    recv(accp_socket, &buffer, sizeof(buffer), 0);
    print_buffer("Recevied",(char *)remove_enter(buffer).c_str(),strlen(remove_enter(buffer).c_str()));



    while(1){
        // send main menu to client
        send(accp_socket,main_menu().c_str(),strlen(main_menu().c_str()),0);
        //print_buffer("Sent",(char *)main_menu().c_str(),strlen(main_menu().c_str()));

        char buffer[1024];
        // clear the request buffer
        memset(buffer, 0, sizeof(buffer));
        // recv the data from the client and store into request

        recv(accp_socket, &buffer, sizeof(buffer), 0);
        
        // remove enter key stroke from buffer
        string choice = remove_enter(buffer);

        print_buffer("Recevied",(char *)choice.c_str(),strlen(choice.c_str()));

        string output;
        // check if choice is 1; if so ask for word and send it to Translator microserver
        if(choice.compare("1") == 0){
            memset(buffer,0,sizeof(buffer));
            string msg = "Enter an English word: ";
            send(accp_socket,msg.c_str(),strlen(msg.c_str()),0);
            print_buffer("Sent to client",(char *)msg.c_str(),strlen(msg.c_str()));
            recv(accp_socket, &buffer, sizeof(buffer), 0);

            output =remove_enter(buffer);
            print_buffer("Recevied from client",(char *)output.c_str(),strlen(output.c_str()));

            portNum = "1786";
            output = check_timeout(run_client(argv[1],portNum,output));
            if (output.compare("Microserver timed out!\n") == 0) {
                send(accp_socket,output.c_str(),strlen(output.c_str()),0);
                print_buffer("Sent to client",(char *)output.c_str(),strlen(output.c_str()));
                continue;
            } else {
            send(accp_socket,output.c_str(),strlen(output.c_str()),0);
            print_buffer("Sent to client",(char *)output.c_str(),strlen(output.c_str()) );
            continue;
            }
        }
        // Check if choice is 2; ask for input and send it to convertor microserver
        else if(choice.compare("2") == 0){
            memset(buffer,0,sizeof(buffer));
            string msg = "Enter <SRC> <DST> <VALUE>: ";
            send(accp_socket,msg.c_str(),strlen(msg.c_str()),0);
            print_buffer("Sent to client",(char *)msg.c_str(),strlen(msg.c_str()));
            recv(accp_socket, &buffer, sizeof(buffer), 0);
            output = remove_enter(buffer);
            print_buffer("Recevied from client",(char *)output.c_str(),strlen(output.c_str()));
            portNum = "4592";
            output = check_timeout(run_client(argv[1],portNum,output));
            send(accp_socket,output.c_str(),strlen(output.c_str()),0);
            print_buffer("Sent to client",(char *)output.c_str(),strlen(output.c_str()) );
            continue;
        }
        // Vote  
        else if(choice.compare("3") == 0){

            memset(buffer,0,sizeof(buffer));

            // get the ip of client
            sockaddr_in *address = (sockaddr_in *)server_info->ai_addr;
            char ip_str[INET_ADDRSTRLEN];
            inet_ntop(server_info->ai_family, &(address->sin_addr), ip_str, INET_ADDRSTRLEN);
            // Create string for voting microserver
            output = "vote\n";
            output.append(ip_str);
            // Voting microserver port
            portNum = "2968";
            // Run the client socket and check if it timedout and store string result in output
            output = check_timeout(run_client(argv[1],portNum,output));

            string send_msg;
            // Check if an error was sent back and send msg to client
            if(output.compare("NO") == 0){
                send_msg.append("You cannot vote twice!");
                send(accp_socket,send_msg.c_str(),strlen(send_msg.c_str()),0);
                print_buffer("Sent to client",(char *)send_msg.c_str(),strlen(send_msg.c_str()));
                continue;
            }
            // if no microserver timeout error prompt for candidate's id 
            else if (output.compare("Microserver timed out!\n") != 0) {
                send_msg.append(output);
                send_msg.append("Enter candidate's id: ");
                send(accp_socket,send_msg.c_str(),strlen(send_msg.c_str()),0);
                print_buffer("Sent to client",(char *)send_msg.c_str(),strlen(send_msg.c_str()));
                // recv the id from client and send it to microserver
                memset(buffer,0,sizeof(buffer));
                send_msg.clear();
                recv(accp_socket, &buffer, sizeof(buffer), 0);

                output = remove_enter(buffer);
                print_buffer("Recevied from client",(char *)output.c_str(),strlen(output.c_str()));
                output = check_timeout(run_client(argv[1],portNum,output));
                // check if there is a candidate id error; if so send error msg to client
                if(output.compare("NO") == 0){
                    send_msg.append("Candidate id not valid!");
                    send(accp_socket,send_msg.c_str(),strlen(send_msg.c_str()),0);
                    print_buffer("Sent to client",(char *)send_msg.c_str(),strlen(send_msg.c_str()));
                    continue;
                } // Check if udp server timed out and send message to client 
                else {
                    send(accp_socket,output.c_str(),strlen(output.c_str()),0);
                    print_buffer("Sent to client",(char *)output.c_str(),strlen(output.c_str()));
                    continue;
                }
            }
             else {
                send(accp_socket,output.c_str(),strlen(output.c_str()),0);
                print_buffer("Sent to client",(char *)output.c_str(),strlen(output.c_str()));
                continue;
            }
        } // Report
        else if(choice.compare("4") == 0){

            memset(buffer,0,sizeof(buffer));
            // Store client's ip addr into ip_str
            sockaddr_in *address = (sockaddr_in *)server_info->ai_addr;
            char ip_str[INET_ADDRSTRLEN];
            inet_ntop(server_info->ai_family, &(address->sin_addr), ip_str, INET_ADDRSTRLEN);
            // Create str with keyword and ip to send to micro server
            output = "report\n";
            output.append(ip_str);
            // For Voting server port num
            portNum = "2968";
            // run the client and store result into output 
            output = check_timeout(run_client(argv[1],portNum,output));

            string send_msg;
            // Check if microserver returned error
            if(output.compare("NO") == 0){
                send_msg.append("You must vote before view the voting report!");
                send(accp_socket,send_msg.c_str(),strlen(send_msg.c_str()),0);
                print_buffer("Sent to client",(char *)send_msg.c_str(),strlen(send_msg.c_str()));
                continue;
            } // Check if the udp server timedout
            else if (output.compare("Microserver timed out!\n") == 0) {
                send(accp_socket,output.c_str(),strlen(output.c_str()),0);
                print_buffer("Sent to client",(char *)output.c_str(),strlen(output.c_str()));
                continue;
            } // if all clear send the msg from the mc server to client
            else {
                send(accp_socket,output.c_str(),strlen(output.c_str()),0);
                print_buffer("Sent to client",(char *)send_msg.c_str(),strlen(send_msg.c_str()));
                continue;
            }
        } // Show candidate
        else if(choice.compare("5") == 0) {

            memset(buffer,0,sizeof(buffer));
            // get client ip addr and store into ip_str
            sockaddr_in *address = (sockaddr_in *)server_info->ai_addr;
            char ip_str[INET_ADDRSTRLEN];
            inet_ntop(server_info->ai_family, &(address->sin_addr), ip_str, INET_ADDRSTRLEN);
            // create str with keyword and ip to send to mc server
            output = "view\n";
            output.append(ip_str);
            portNum = "2968";
            // run the mc server and send the return msg to client
            output = check_timeout(run_client(argv[1],portNum,output));

            send(accp_socket,output.c_str(),strlen(output.c_str()),0);
            print_buffer("Sent to client",(char *)output.c_str(),strlen(output.c_str()));
            continue;

        } // Close connection; close client server and send msg to client
        else if(choice.compare("0") == 0) {
            string end = "Good bye from server!\n";
            send(accp_socket,end.c_str(),strlen(end.c_str()),0);
            print_buffer("Sent",(char *)end.c_str(),strlen(end.c_str()));
            close(accp_socket);
            break;
        } // Wrong choice; send msg
        else {
            string fail = "Please enter a valid choice! \n";
            send(accp_socket,fail.c_str(),strlen(fail.c_str()),0);
            print_buffer("Sent",(char *)fail.c_str(),strlen(fail.c_str()));
            continue;
        }
    }    
    return 0;
}
