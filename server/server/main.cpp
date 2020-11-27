#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <iostream>
#include <bits/stdc++.h>
#include <pthread.h>

using namespace std;

string parse_request(string request);
vector<string> split(string text, char del);
string handle_get(string file_path);
string handle_post(string request, string file_path);
void* connection_creation(void* params);
vector<string> split_string(string text,string del);

struct parameters{
    int socket;
};

int main(int argc, char const *argv[])
{
    // server_fd:  to hold the server socket descriptor
    // new_socket: to hold the client socket descriptor
    // valread:    to hold the received data from client
    // address:    to hold the server socket's address type and port
    int port = 8081;
    if(argc == 2) port = atoi(argv[1]);
    int server_fd, new_socket; long valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Creating server socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("In socket");
        exit(EXIT_FAILURE);
    }

    // Configure the server socket to work with IP & port=port variable
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( port );

    memset(address.sin_zero, '\0', sizeof address.sin_zero);

    // Bind the server socket to the configure address & port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("In bind");
        exit(EXIT_FAILURE);
    }

    // Listen to the clients requests
    if (listen(server_fd, 10) < 0)
    {
        perror("In listen");
        exit(EXIT_FAILURE);
    }
    while(1)
    {
        printf("\n ---- Waiting for new connection ---- \n\n");
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
        {
            perror("In accept");
            exit(EXIT_FAILURE);
        }

        pthread_t ptid;
        parameters *params = new parameters();
        params->socket = new_socket;
        pthread_create(&ptid, NULL, &connection_creation, (void *) params);
    }
    return 0;
}

void* connection_creation(void* params){
    struct parameters *p = (parameters *) params;
    int new_socket = p->socket;
    while(1){
        // Read request data from client connection and print it
        char buffer[30000] = {0};
        long valread = read(new_socket , buffer, 30000);
        if (valread == 0) break;
        cout << "===== Recieved Request " << valread << "======\n";
        cout << buffer << "\n";
        string response = parse_request(buffer);
        // Return to client the needed response
        int response_size = response.size();
        char *response_ptr = &response[0];
//        while(response_size > 0){
//            cout << "!!=-=-=-=-=!!" <<response_size << "!!=-=-=-=-=-=\n";
            int x = write(new_socket, response_ptr, response_size);
//            cout << "\n=-=-=-=-=-\n" << x << "\n-=-=-=-=\n";
//            response_ptr += x;
//            response_size -= x;
//        }
        cout << "===== Sent Response =====\n" << response << "\n";
        //close(new_socket);
    }
    cout << "\n===========Close Connection===========\n";
    close(new_socket);
}

string parse_request(string request){
    vector<string> lines = split(request, '\n');
    vector<string> request_params = split(lines.front(), ' ');
    string file_path = request_params.at(1);
    if(request_params.at(0) == "GET"){
        return handle_get(file_path);
    } else if(request_params.at(0) == "POST"){
        return handle_post(request, file_path);
    } else {
        return "HTTP/1.1 404 Not Found\r\n";
    }
}

string handle_get(string file_path){
    string response;
    string content_type;
    if (file_path == "/") file_path = "/index.html";
    file_path = "public" + file_path;

    content_type = split(file_path, '.').at(1);
    if (content_type == "txt"){
        content_type = "Content-Type: text/plain\n";
    } else if(content_type == "html"){
        content_type = "Content-Type: text/html\n";
    } else if(content_type == "jpg" || content_type == "jpeg" || content_type == "png" || content_type == "gif"){
        if(content_type == "jpg") content_type = "jpeg";
        content_type = "Content-Type: image/" + content_type + "\n";
    }
    cout << "file_path: " << file_path << "\n";

    ifstream requested_file(file_path);
    if(requested_file.good()){
        string requested_file_text;
        string myText = "";
        while (getline (requested_file, myText)) {
            requested_file_text += myText;
            requested_file_text += "\n";
        }
        response = "HTTP/1.1 200 OK\r\n";
        response += content_type;
        response += "Content-Length: ";
        response += to_string(requested_file_text.size());
        response += "\r\n\r\n";
        response += requested_file_text;
    } else {
        response = "HTTP/1.1 404 Not Found\r\n";
    }
    requested_file.close();
    return response;
}

string handle_post(string request, string file_path){
    if (file_path == "/") file_path = "/default.html";
    ofstream MyFile("post/" + split(file_path, '/').back());
    MyFile << split_string(request, "\r\n\r\n").back();
    MyFile.close();
    cout << "\n=== File Saved in " << file_path << " ===\n";
    string response = "HTTP/1.1 200 OK\r\n";
    return response;
}



vector<string> split(string text, char del){
    vector<string> lines;
    stringstream check1(text);
    string intermediate;

    while(getline(check1, intermediate, del))
    {
        lines.push_back(intermediate);
    }
    return lines;

}


vector<string> split_string(string text,string del){
        vector<string> lines;
        size_t pos = 0;
        string token;
        while ((pos = text.find(del)) != std::string::npos) {

        token = text.substr(0, pos);
        lines.push_back(token);
        text.erase(0, pos + del.length());
        }
        lines.push_back(text) ;
        return lines ;
}

