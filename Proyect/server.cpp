#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <map>

#include "new.pb.h"


using namespace std;
using namespace chat;

// max number of bytes we can send at once
#define MAXDATASIZE 4096
#define MAX_CLIENTS 15


int sfd, portNumber;

int cCount = 0;
struct sockaddr_in s_addr;


pthread_t threadPool[MAX_CLIENTS];
pthread_mutex_t myMutexPublicChat = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t myMutexPrivateChat = PTHREAD_MUTEX_INITIALIZER;

vector<MessageCommunication> publicChat;

void * retvals[MAX_CLIENTS];



struct User{
    string userName;
    string status;
    string ip;
    int socket;
};

vector<User> users;

void HandleOptions(ClientPetition pet, int sock)
{
    cout << "LA petit op: " << pet.option() << endl;
    
    //Usuarios conectados
    if (pet.option() == 2)
    {
        ConnectedUsersResponse *connectedU(new ConnectedUsersResponse);
        for (struct User& user: users){
            UserInfo* uInfo = connectedU->add_connectedusers();
            uInfo->set_username(user.userName);
            uInfo->set_status(user.status);
            uInfo->set_ip(user.ip);

        }
        
        ServerResponse response;
        response.set_option(2);
        response.set_code(200);
        response.set_allocated_connectedusers(connectedU);
        
        string serString;
        response.SerializeToString(&serString);

        char cstr[serString.size() + 1];
        strcpy(cstr, serString.c_str());
        int sent = send(sock, cstr, strlen(cstr), 0);
        if (sent == 0)
        {
            fprintf(stderr, "No se envio la lista\n");
        }
    }
    // Cambio de estado
    if (pet.option() == 3)
    {
        ChangeStatus *changeS(new ChangeStatus);
        for (User& user: users) {
            if (user.userName.compare(pet.change().username()) ==  0)
            {
                user.status = pet.change().status();
                changeS->set_username(pet.change().username());
                changeS->set_status(pet.change().status());
            }

        }

        ServerResponse response;
        response.set_option(3);
        response.set_code(200);
        response.set_allocated_change(changeS);

        string serString;
        response.SerializeToString(&serString);

        char cstr[serString.size() + 1];
        strcpy(cstr, serString.c_str());
        int sent = send(sock, cstr, strlen(cstr), 0);
        if (sent == 0)
        {
            fprintf(stderr, "No se hizo el cambion");
        }        
    }
    // Chat
    if (pet.option() == 4)
    {
        //Chat publico
        if (pet.messagecommunication().recipient() == "everyone")
        {
            cout << "Chat publico" << endl;

            pthread_mutex_lock(&myMutexPublicChat);
            //publicChat.push_back(pet.messagecommunication());

            MessageCommunication *msg(new MessageCommunication);
            msg->set_sender(pet.messagecommunication().sender());
            msg->set_message(pet.messagecommunication().message());
            msg->set_recipient(pet.messagecommunication().recipient());

            ServerResponse response;
            response.set_option(4);
            response.set_code(200);
            response.set_allocated_messagecommunication(msg);

            string serString;
            response.SerializeToString(&serString);

            char cstr[serString.size() + 1];
            strcpy(cstr, serString.c_str());

            for (User& user: users) {
                int sent = send(user.socket, cstr, strlen(cstr), 0);
                if (sent == -1)
                {
                    fprintf(stderr, "No se envio el mensaje\n");
                    users.erase(std::remove_if(users.begin(),users.end(), [&](User const & user_) {
                    	return user_.userName == user.userName;
                    }), users.end());
                    close(user.socket);
                    cout << "mate usuario " << user.userName << endl;
                }
            }
            
            cout << "Chat enviado" << endl;

            pthread_mutex_unlock(&myMutexPublicChat);

        } else {
            bool founded = false;

            cout << "Chat privado" << endl;
            
            pthread_mutex_lock(&myMutexPrivateChat);

            MessageCommunication *msg(new MessageCommunication);
            msg->set_sender(pet.messagecommunication().sender());
            msg->set_message(pet.messagecommunication().message());
            msg->set_recipient(pet.messagecommunication().recipient());

            ServerResponse response;
            response.set_option(4);
            response.set_code(200);
            response.set_allocated_messagecommunication(msg);

            string serString;
            response.SerializeToString(&serString);
            char cstr[serString.size() + 1];
            strcpy(cstr, serString.c_str());

            for (User& user: users) {
                if (user.userName.compare(pet.messagecommunication().recipient()) ==  0)
                {
                    int sentdest = send(user.socket, cstr, strlen(cstr), 0);
                    if (sentdest == 0)
                    {
                        fprintf(stderr, "No se envio el mensaje\n");
                    }

                    founded = true;                    
                    break;                    
                }             
            }
            
            if (!founded)
            {
                cout << "No se encontró al usuario" << endl;
            }
            

            pthread_mutex_unlock(&myMutexPrivateChat);
        }
    }
    //Informacion de usuario especifico
    if (pet.option() == 5)
    {
        UserInfo *uInfo(new UserInfo);

        for (struct User& user: users){
            if (user.userName.compare(pet.users().user()) ==  0)
            {
                uInfo->set_username(user.userName);
                uInfo->set_status(user.status);
                uInfo->set_ip(user.ip);
                cout << "Socket del encontrado" << user.socket << endl;
            }

        }
        
        ServerResponse response;
        response.set_option(5);
        response.set_code(200);
        response.set_allocated_userinforesponse(uInfo);

        string serString;
        response.SerializeToString(&serString);

        cout << "Socket del que busca" << sock << endl;


        char cstr[serString.size() + 1];
        strcpy(cstr, serString.c_str());
        int sent = send(sock, cstr, strlen(cstr), 0);
        if (sent == 0)
        {
            fprintf(stderr, "No se envio la informacion\n");
        }
    }
}

void * cThreadFunc(void *args) {
    char buffer[MAXDATASIZE];
    struct User *threadInfo;
    threadInfo = (struct User *) args;

    int usoc = threadInfo->socket;
    string uName = threadInfo->userName;

    bool isClosed = false;
    time_t serverTime;    

    while (usoc > 0 && !isClosed)
    {
        //Bezos be like: "I'm not angry, just disappointed"
        bzero(buffer, MAXDATASIZE);

        time(&serverTime);

        char buff[MAXDATASIZE];

        int bytesrecieved = recv(usoc, buff, MAXDATASIZE, 0);
        buff[bytesrecieved] = '\0';

        string serializedString = buff;

        ClientPetition cpetit;
        cpetit.ParseFromString(serializedString);

        if (bytesrecieved > 0)
        {

            if (*buff == '#')
            {
                cout << "Se perdio la conexion" << endl;
                isClosed = true;
            }
            else
            {                
                HandleOptions(cpetit, usoc);
            }
        }
        else
        {
            cout << "Error al recibir" << endl;
            for (User& user: users) {
            if (uName.compare(user.userName) ==  0)
            {
                users.erase(std::remove_if(users.begin(),users.end(), [&](User const & user_) {
                    	return user_.userName == user.userName;
                    }), users.end());
                close(usoc);
                cout << "mate usuario " << uName << endl;
            }
            break;
        }
            
        }

        
    }


    close(usoc)  ;
    

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
            cout << "Conexión abortada" << endl;
        }
        else
        {
            cout << "Conexión salvaje aparece" << endl;
        }

        char buff[MAXDATASIZE];

        int bytesrecieved = recv(nsock, buff, MAXDATASIZE, 0);
        buff[bytesrecieved] = '\0';

        string serializedString = buff;

        ClientPetition cpetit;
        cpetit.ParseFromString(serializedString);
        
        cout << "Opción " << cpetit.option() << endl;
        ServerResponse response;
        response.set_option(1);

        if (cpetit.option() == 1) {
            bool canBeRegistered = true;

            for (auto user = users.begin(); user != users.end(); user++)
            {
                
                if (user->userName == cpetit.registration().username())
                {
                    cout << "Usuario duplicado" << endl;
                    canBeRegistered = false;
                    break;
                }
                
            }

            if (canBeRegistered) {
                struct User user;
                user.userName = cpetit.registration().username();
                user.ip = cpetit.registration().ip();   
                user.status = "ACTIVO";
                user.socket = nsock;

                cout << "Usuario: " << user.userName << " Socket: " << user.socket << endl;

                response.set_code(200);
                response.set_servermessage("Registro existoso");


                users.push_back(user);

                pthread_create(&threadPool[cCount], NULL, cThreadFunc, (void *)&user);
                
                cCount++;

            } else {
                response.set_code(500);
                response.set_servermessage("Registro fallido");
            }

        } 
        
        // else {
        //     cout << "Error de petición de registro" << endl;
        //     response.set_code(500);
        //     response.set_servermessage("Opción invalida");
        // }

        response.SerializeToString(&serializedString);

        char resp[serializedString.size() + 1];
        strcpy(resp, serializedString.c_str());

        send(nsock, resp, strlen(resp), 0);            
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


