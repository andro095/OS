#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
// #include <algorithm>
// #include <netdb.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <map>
// #include <condition_variable>
// #include <time.h>
// #include <chrono>
// #include <netdb.h>
#include "new.pb.h"


using namespace std;
using namespace chat;
using namespace google::protobuf;

#define HOSTNAME "localhost"
#define BUFSIZE 4096    // max number of bytes we can send at once
#define BACKLOG 10          // how many pending connections queue will hold

#define MAXDATASIZE 4096
#define MAX_CLIENTS 15


int sfd, portNumber;
int fd[2];

int cCount = 0;
int idCount = 1;
struct sockaddr_in s_addr;


pthread_t threadPool[MAX_CLIENTS];
pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;
vector<MessageCommunication> publicChat;

void * retvals[MAX_CLIENTS];


struct User{
    string userName;
    string status;
    string ip;
    int socket;
};


map<string, User> users;

void HandleOptions(ClientPetition pet, User *userInfo)
{
    cout << "LA petit op: " << pet.option() << endl;
    if (pet.option() == 4)
    {
        // MessageCommunication* broadMessage(new MessageCommunication);
        // broadMessage->set_message(clip.messagecommunication().sender());

        //Chat publico
        if (pet.messagecommunication().recipient() == "everyone")
        {
            cout << "Chat publico" << endl;
            ServerResponse resp;
            resp.set_option(4);
            resp.set_code(300);

            string serializedString;

            resp.SerializeToString(&serializedString);

            char cstr[serializedString.size() + 1];
            strcpy(cstr, serializedString.c_str());

            send(userInfo->socket, cstr, strlen(cstr), 0);

            // pthread_mutex_lock(&myMutex);
            // publicChat.push_back(clip.messagecommunication());
            // pthread_mutex_unlock(&myMutex);
            // for (MessageCommunication message: publicChat)
            // {
            //     cout << "entro" << endl;
            //     MessageCommunication *msg(new MessageCommunication);
            //     msg->set_sender(message.sender());
            //     msg->set_message(message.message());

            //     ServerResponse response;
            //     response.set_option(4);
            //     response.set_code(200);
            //     response.set_allocated_messagecommunication(msg);

            //     string serString;
            //     response.SerializeToString(&serString);

            //     char cstr[serString.size() + 1];
            //     strcpy(cstr, serString.c_str());
            //     int sent = send(userInfo->socket, cstr, strlen(cstr), 0);
            //     if (sent == 0)
            //     {
            //         fprintf(stderr, "No se envio el mensaje\n");
            //     }
            //     cout << "entro2" << endl;

            // }
            

        }
        //Chat privado
        // else
        // {
        //     if (users.count(clip.messagecommunication().recipient()) > 0)
        //     {
        //         cout << "Ese usuario no esta conectado" << endl;
        //     }
        //     cout << "Chat con" << clip.messagecommunication().recipient() << endl;
        //     broadMessage->set_recipient(clip.messagecommunication().recipient());
                
        //     ServerResponse response;
        //     response.set_option(1);
        //     response.set_allocated_messagecommunication(broadMessage);

        //     string binary;
        //     response.SerializeToString(&binary);

        //     char cstr[binary.size() + 1];
        //     strcpy(cstr, binary.c_str());
        //     int sent = send(userInfo->socket, cstr, strlen(cstr), 0);
        //     if (sent == 0)
        //     {
        //         fprintf(stderr, "No se envio el mensaje\n");
        //     }
        // }
    

    }
}

void * cThreadFunc(void *args) {
    char buffer[MAXDATASIZE];
    User *threadInfo;
    threadInfo = (struct User *) args;


    bool isClosed = false;
    time_t serverTime;
    int count = 0;
    

    while (threadInfo->socket > 0 && !isClosed)
    {
        bzero(buffer, MAXDATASIZE);

        time(&serverTime);

        // int resp = recv(threadInfo->socket, buffer, MAXDATASIZE, 0);

        char buff[MAXDATASIZE];

        int bytesrecieved = recv(threadInfo->socket, buff, MAXDATASIZE, 0);
        buff[bytesrecieved] = '\0';

        string serializedString = buff;

        ClientPetition cpetit;
        cpetit.ParseFromString(serializedString);
        
        cout << "Opción" << cpetit.option() << endl;

        if (bytesrecieved > 0)
        {

            if (*buff == '#')
            {
                cout << "Se perdio la conexion" << endl;
                isClosed = true;
            }
            else
            {                
                HandleOptions(cpetit, threadInfo);
            }
        }
        else
        {
            cout << "Error al recibir" << endl;
        }

        

        count++;

        if (count >= 3)
        {
            break;
        }
        
    }


    close(threadInfo->socket)  ;
    

    cCount--;
    pthread_exit(NULL);
    
}


void conectionInit() {
    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0) {
        cout << "Error al conectarse al socket" << endl;
    }

    bzero((char *) &s_addr, sizeof(s_addr));
    s_addr.sin_family = AF_INET;
    s_addr.sin_addr.s_addr = INADDR_ANY;
    s_addr.sin_port = htons(portNumber);

    int bres = bind(sfd, (struct sockaddr *) &s_addr, sizeof(s_addr));
    if (bres < 0) {
        cout << "Error conexión en el bind" << endl;
        exit(0);
    }
    
    int lres = listen(sfd, MAX_CLIENTS);
    
    if (lres != 0){
        cout << "Error conexión en el listen" << endl;
        exit(0);
    }
    

}

void quitServer()
{
    for (int i=0; i < MAX_CLIENTS; i++)
    {
        if (pthread_join(threadPool[i], &retvals[i]) < 0)
            cout << "Error al cerrar servidor" << endl;
    }
    close(sfd);
}


void listenNewConnections()
{
    while (cCount < MAX_CLIENTS)
    {
        struct sockaddr_in cAddr;
        socklen_t clientLength = sizeof(cAddr);
        
        int nsock = accept(sfd, (struct sockaddr *)&cAddr, &clientLength);
        if (nsock < 0)
        {
            cout << "No se conecto con cliente" << endl;
        }
        else
        {
            cout << "Se conecto el cliente!" << endl;
        }

        char buff[MAXDATASIZE];

        int bytesrecieved = recv(nsock, buff, MAXDATASIZE, 0);
        buff[bytesrecieved] = '\0';

        string serializedString = buff;

        ClientPetition cpetit;
        cpetit.ParseFromString(serializedString);
        
        cout << "Opción" << cpetit.option() << endl;

        if (cpetit.option() == 1) {
            struct User userInfo;
            userInfo.userName = cpetit.registration().username();
            userInfo.status = "ACTIVO";
            userInfo.ip = cpetit.registration().ip();
            userInfo.socket = nsock;

            ServerResponse response;
            response.set_option(4);
            response.set_code(200);

            response.SerializeToString(&serializedString);

            char cstr[serializedString.size() + 1];
            strcpy(cstr, serializedString.c_str());

            send(nsock, cstr, strlen(cstr), 0);

            //users.insert(pair<string,User>(userInfo.username, newuser));

            pthread_create(&threadPool[cCount], NULL, cThreadFunc, (void *)&userInfo);
            
            cCount++;
        } else {
            cout << "Error de petición de registro" << endl;
        }

    }
}


int main(int argc, char *argv[]){
    if (argc != 2)
    {
        cout << "No se ha proveeido de los argumentos exactos" << endl;
        exit(1);
    }

    portNumber = atoi(argv[1]);

    conectionInit();
    listenNewConnections();
    quitServer();

    cout << "Works" << endl;

    
}


// int mainother(int argc, char *argv[])
// {
//     int portno, n;
//     struct sockaddr_in serv_addr;
//     struct hostent *server;
//     bool salir = false;
//     char buffer[BUFSIZE];

//     GOOGLE_PROTOBUF_VERIFY_VERSION;

//     if (argc < 2) {
//        fprintf(stderr,"Not enough arguments given\n", argv[0]);
//        exit(0);
//     }
    
//     cout << "Works" << endl;
    
//     int res = pipe(fd);
//     if (res < 0) {
//         cout << "Error al crear el canal de comunicación" << endl;
//         exit(1);
//     }

//     pthread_t tidod, tidm;

//     portno = atoi(argv[1]);
//     sockfd = socket(AF_INET, SOCK_STREAM, 0);

//     if (sockfd < 0) 
//         err("Socket error");

    
//     cout << "Works2" << endl;
//     server = gethostbyname(argv[2]);

//     cout << "Works3" << endl;
//     if (server == NULL) {
//         fprintf(stderr,"Host name error\n");
//         exit(0);
//     }

//     bzero((char *) &serv_addr, sizeof(serv_addr));
//     serv_addr.sin_family = AF_INET;

//     bcopy((char *)server -> h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
//     serv_addr.sin_port = htons(portno);
//     cout << "Works4" << endl;

//     if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
//         err("Connection error");
        
    
//     struct User currentClient;
//     cout << "Works5" << endl;
    
    
//     while (true)
//     {
//         /* code */
//         ClientPetition clip;
//         recv(sockfd, buffer, BUFSIZE, 0);
//         clip.ParseFromString(buffer);
//         int option = clip.option();
        
//         Registrar Cliente
//         if (option == 1)
//         {
//             if (users.count(clip.registration().username()) > 0)
//             {
//                 cout << "Ese usuario ya esta registrado" << endl;
//             }
            
//             Guardar
//             currentClient.username = clip.registration().username();
//             currentClient.ip = clip.registration().ip();
//             currentClient.status = "Activo";

//             struct User newuser;
//             newuser.username = currentClient.username;
//             newuser.ip = currentClient.ip;

//             users.insert(pair<string,User>(currentClient.username, newuser));

//             Enviar mensaje de confirmacion
//             ServerResponse response;
//             response.set_option(1);
//             response.set_code(200);
//             response.set_servermessage("Registrado con éxito");

//             string binary;
//             response.SerializeToString(&binary);

//             char cstr[binary.size() + 1];
//             strcpy(cstr, binary.c_str());
//             int sent = send(sockfd, cstr, strlen(cstr), 0);
//             if (sent == 0)
//             {
//                 fprintf(stderr, "No se pudo notificar \n");
//             }
//         }


//         Lista de clientes conectados

//         else if (option == 2){
//             string connectedUsers = "";
//             cout << "Lista de usuarios conectados: \n" << endl;
//             for (auto i = users.begin(); i != users.end(); ++i)
//             {
//                 connectedUsers += advance(firstUser, i);
//                 string itrUsername = i->first;
//                 Obtener el usuario del diccionario
//                 struct User u = i->second;

//                 cout << "\tUSER: " << itrUsername << "\tSTATUS: " << u.status << "\tIP: " << u.ip << endl;
//             }

//             ConnectedUsersResponse * userList(new ConnectedUsersResponse);

//             userList->set_allocated_connectedusers(connectedUsers);

//             ServerResponse sResponse;
//             sResponse.set_option(3);
//             sResponse.set_allocated_connectedusers(userList);

            
//         }
        

//         Cambio de estado
//         else if (option == 3){


//         }

//         Mensajes
//         else if (option == 4){
//             MessageCommunication* broadMessage(new MessageCommunication);
//             broadMessage->set_message(clip.messagecommunication().sender());

//             Chat publico
//             if (clip.messagecommunication().recipient() == "everyone")
//             {
//                 cout << "Chat publico" << endl;
//                 for(auto i = users.begin(); i != users.end(); ++i)
//                 {
//                     if (i->first != currentClient.username)
//                     {
//                         broadMessage->set_recipient(clip.messagecommunication().recipient());
                        
//                         ServerResponse response;
//                         response.set_option(1);
//                         response.set_allocated_messagecommunication(broadMessage);

//                         string binary;
//                         response.SerializeToString(&binary);

//                         char cstr[binary.size() + 1];
//                         strcpy(cstr, binary.c_str());
//                         int sent = send(sockfd, cstr, strlen(cstr), 0);
//                         if (sent == 0)
//                         {
//                             fprintf(stderr, "No se envio el mensaje\n");
//                         }

//                     }
//                 }

//             }
//             Chat privado
//             else
//             {
//                 if (users.count(clip.messagecommunication().recipient()) > 0)
//                 {
//                     cout << "Ese usuario no esta conectado" << endl;
//                 }
//                 cout << "Chat con" << clip.messagecommunication().recipient() << endl;
//                 broadMessage->set_recipient(clip.messagecommunication().recipient());
                        
//                 ServerResponse response;
//                 response.set_option(1);
//                 response.set_allocated_messagecommunication(broadMessage);

//                 string binary;
//                 response.SerializeToString(&binary);

//                 char cstr[binary.size() + 1];
//                 strcpy(cstr, binary.c_str());
//                 int sent = send(sockfd, cstr, strlen(cstr), 0);
//                 if (sent == 0)
//                 {
//                     fprintf(stderr, "No se envio el mensaje\n");
//                 }
//             }
            

//         }

//         Informacion de usuario especifico
//         else if (option == 5){
        
//             if (users.count(clip.users().user()) == 0)
//             {
//                 cout << "Ese usuario no esta conecctado" << endl;
//             }
//             else
//             {
//                 auto u = users.find(clip.users().user());
//                 cout << "User: " << u->first << "IP: " << u->second.ip << endl;
//             }

//         }   
//     }

// }
