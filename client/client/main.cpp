#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <iostream>
#include <arpa/inet.h>
#include <bits/stdc++.h>
using namespace std;

vector<string> split(string text, char del);
void send_post(int sock, string line);
void send_get(int sock, string line);
void parse_requests(int sock);
vector<string> split_string(string text,string del);


int main(int argc, char const *argv[])
{
    int sock = 0;
    char *server_ip = "127.0.0.1";
    int server_port = 8081;
    if (argc == 3){
        strcpy(server_ip, argv[1]);
        server_port = atoi(argv[2]);
    }
    struct sockaddr_in serv_addr;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);

    if(inet_pton(AF_INET, server_ip, &serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }
    parse_requests(sock);

    return 0;
}


void parse_requests(int sock){
    ifstream requsts_list_file("requests/requests_list.txt");
    string line = "";
    while (getline (requsts_list_file, line)) {
        if(split(line, ' ').at(0) == "client_get"){
            send_get(sock, line);
        } else {
            send_post(sock, line);
        }
    }
    requsts_list_file.close();
    close(sock);
}

void send_get(int sock, string line){
    vector<string> request_components = split(line, ' ');
    string file_path = request_components.at(1);
    string request = "GET "+ file_path + " HTTP/1.1\r\n\r\n";
    send(sock ,&request[0] ,request.size() ,0 );
    cout << "=========== Request Sent ===========\n" << request << "\n";
    char buffer[30000] = {0};
    long valread = read(sock, buffer, 30000);
    cout << "=========== Response Received ===========\n" << buffer << "\n";
    if(string(buffer).substr(9,3) == "200"){
        if (file_path == "/") file_path = "/default.html";
        ofstream MyFile("responses/" + split(file_path, '/').back());
        MyFile << split_string(string(buffer), "\r\n\r\n").back();
        MyFile.close();
        cout << "\n=== File Saved in " << file_path << " ===\n";
    }
}

void send_post(int sock, string line){
    vector<string> request_components = split(line, ' ');
    string file_path = request_components.at(1);
    string request = "POST "+ file_path + " HTTP/1.1\r\n";

    file_path = "public" + file_path;

    string content_type = split(file_path, '.').at(1);
    if (content_type == "txt"){
        content_type = "Content-Type: text/plain\n";
    } else if(content_type == "html"){
        content_type = "Content-Type: text/html\n";
    } else if(content_type == "jpg" || content_type == "jpeg" || content_type == "png" || content_type == "gif"){
        if(content_type == "jpg") content_type = "jpeg";
        content_type = "Content-Type: image/" + content_type + "\n";
    }
    cout << "file_path: " << file_path << "\n";

    ifstream posted_file(file_path);
    string posted_file_text;
    string myText = "";
    while (getline (posted_file, myText)) {
        posted_file_text += myText;
        posted_file_text += "\n";
    }
    request += content_type;
    request += "Content-Length: ";
    request += to_string(posted_file_text.size());
    request += "\r\n\r\n";
    request += posted_file_text;
    posted_file.close();

    send(sock ,&request[0] ,request.size() ,0 );
    cout << "=========== Request Sent ===========\n" << request << "\n";
    char buffer[30000] = {0};
    long valread = read(sock, buffer, 30000);
    cout << "=========== Response Received ===========\n" << buffer << "\n";
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
