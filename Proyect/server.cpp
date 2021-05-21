#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <map>
#include <list>
#include <iterator>

#include "./new.pb.h"

using namespace std;
using namespace chat;
using namespace google::protobuf;

#define HOSTNAME "localhost"
#define MAXDATASIZE 4096    // max number of bytes we can send at once
#define BACKLOG 10          // how many pending connections queue will hold

// Quitar por si no es necesario
#define MAX_CLIENTS 15

int sockfd, portno;
int clientCount = 0;
int idCount = 1;
struct sockaddr_in serv_addr;
pthread_t threadPool[MAX_CLIENTS];
void * retvals[MAX_CLIENTS];

struct User{
    string username;
    string status;
    //int socket;
    char ip;
}

map<string, User> users;

int main(int argc, char *argv[])
{

    int portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    bool salir = false;
    char buffer[BUFSIZE];

    GOOGLE_PROTOBUF_VERIFY_VERSION;

    if (argc < 2) {
       fprintf(stderr,"Not enough arguments given\n", argv[0]);
       exit(0);
    }

    int res = pipe(fd);
    if (res < 0) {
        cout << "Error al crear el canal de comunicación" << endl;
        exit(1);
    }

    pthread_t tidod, tidm;

    exit(0);

    portno = atoi(argv[3]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) 
        err("Socket error");

    
    server = gethostbyname(argv[2]);

    if (server == NULL) {
        fprintf(stderr,"Host name error\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;

    bcopy((char *)server -> h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        err("Connection error");
        
    
    User currentClient;
    

    ClientPetition clip;
    clip.ParseFromString(buffer);
    int option = clip.option();
    
    //Registrar Cliente
    if (option == 1)
    {
        if (users.count(clip.UserRegistration().username()) > 0)
        {
            cout << "Ese usuario ya esta registrado" << endl;
            break;
        }
        
        //Guardar
        currentClient.username = clip.UserRegistration().username();
        currentClient.ip = clip.UserRegistration().ip();
        currentClient.status = "Activo";

        struct User newuser;
        newuser.username = currentClient.username;
        newuser.ip = currentClient.ip;

        mymap.insert (pair<string,User>(currentClient.username, newuser));

        //Enviar mensaje de confirmacion
        ServerResponse response;
        response.set_option(1);
        response.set_code(200);
        response.set_servermessage("Registrado con éxito");

        string binary;
        response.SerializeToString(&binary);

        char cstr[binary.size() + 1];
        strcpy(cstr, binary.c_str());
        int sent = send(sockfd, cstr, strlen(cstr), 0);
        if (sent == 0)
        {
            fprintf(stderr, "No se pudo notificar \n");
        }
    }


    //Lista de clientes conectados

    if (option == 2){
        string connectedUsers = "";
        cout << "Lista de usuarios conectados: \n" << endl;
        for (auto i = users.begin(); i != users.end(); ++i)
        {
            //connectedUsers += advance(firstUser, i);
            string itrUsername = i->first;
	    	// Obtener el usuario del diccionario
			User u = i->second;

			cout << "\tUSER: " << itrUsername << "\tSTATUS: " << u.status << "\tIP: " << u.ip << endl;
        }

        ConnectedUsersResponse * userList(new ConnectedUsersResponse);

        userList->set_connectedUsers(connectedUsers);

        ServerResponse sResponse;
        sResponse.set_option(3);
        sResponse.set_allocated_message(userList);

        
    }
    

    //Cambio de estado
    if (option == 3){


    }

    //Mensajes
    if (option == 4){
        MessageCommunication broadMessage;
        broadMessage->set_message(clip.MessageCommunication().message())
        broadMessage->set_sender(clip.MessageCommunication().sender()) 

        //Chat publico
        if (clip.MessageCommunication().recipient() == "everyone")
        {
            cout << "Chat publico" << endl;
            for(auto i = users.begin(); i != users.end(); ++i)
            {
                if (i->first != currentClient.username)
                {
                    broadMessage->set_recipient(clip.MessageCommunication().recipient())  
                    
                    ServerMessage response;
                    response.set_option(1);
                    response.set_allocated_messagecommunication(broadMessage);

                    string binary;
                    response.SerializeToString(&binary);

                    char cstr[binary.size() + 1];
                    strcpy(cstr, binary.c_str());
                    int sent = send(sockfd, cstr, strlen(cstr), 0);
                    if (sent == 0)
                    {
                        fprintf(stderr, "No se envio el mensaje\n");
                    }

                }
            }

        }
        //Chat privado
        else
        {
            if (users.count(clip.MessageCommunication().recipient()) > 0)
            {
                cout << "Ese usuario no esta conectado" << endl;
                break;
            }
            cout << "Chat con" << clip.MessageCommunication().recipient() << endl;
            broadMessage->set_recipient(clip.MessageCommunication().recipient())  
                    
            ServerMessage response;
            response.set_option(1);
            response.set_allocated_messagecommunication(broadMessage);

            string binary;
            response.SerializeToString(&binary);

            char cstr[binary.size() + 1];
            strcpy(cstr, binary.c_str());
            int sent = send(sockfd, cstr, strlen(cstr), 0);
            if (sent == 0)
            {
                fprintf(stderr, "No se envio el mensaje\n");
            }
        }
        

    }

    //Informacion de usuario especifico
    if (option == 5){
        if (users.count(clip.UserRequest().user()) == 0)
        {
            cout << "Ese usuario no esta conecctad" << endl;
            break;
        }
        else
        {
            auto u = users.find(clip.UserRequest().user());
            cout << "User: " << u->first << u->second << endl;
        }

    }
    
}
