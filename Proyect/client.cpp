#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <strings.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <algorithm>
#include <netdb.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <condition_variable>
#include <map>
#include <time.h>
#include <chrono>
#include <netdb.h>
#include "new.pb.h"


using namespace std;

#define BUFSIZE 4096    // max number of bytes we can get at once

int sfd;

// Variables controladoras
vector<chat::MessageCommunication> publicChat;
map<string, vector<chat::MessageCommunication>> privChat;
string status = "ACTIVO";

pthread_mutex_t locki = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;


void imprimirChatGeneral() {
    for (chat::MessageCommunication& message : publicChat)
    {
        cout << "@" << message.sender() << ": " << message.message() << endl;
    }
    
}

void imprimirChatPrivado(string userName) {
    if (privChat.find(userName) != privChat.end())
    {
        vector<chat::MessageCommunication> messages =  privChat[userName];
        for(chat::MessageCommunication& msg: messages){
            cout << "@" << msg.sender() << ": " << msg.message() << endl;
        }
    }
    
}

void mensajeGeneral(chat::MessageCommunication messageComunication) {
    publicChat.push_back(messageComunication);
}

void mensajePrivadoFrom(chat::MessageCommunication messageComunication) {
    if(privChat.find(messageComunication.recipient()) == privChat.end()) {
        vector<chat::MessageCommunication> newMessage;
        newMessage.push_back(messageComunication);
        privChat[messageComunication.recipient()] = newMessage;
    } else {
        vector<chat::MessageCommunication> messages = privChat[messageComunication.recipient()];
        messages.push_back(messageComunication);
        privChat[messageComunication.recipient()] = messages;
    }
}

void mensajePrivado(chat::MessageCommunication messageComunication) {
    if(privChat.find(messageComunication.sender()) == privChat.end()) {
        vector<chat::MessageCommunication> newMessage;
        newMessage.push_back(messageComunication);
        privChat[messageComunication.sender()] = newMessage;
    } else {
        vector<chat::MessageCommunication> messages = privChat[messageComunication.sender()];
        messages.push_back(messageComunication);
        privChat[messageComunication.sender()] = messages;
    }
}

void *readPet( void *arg) {
    //Espera recibir señales del servidor
    while (true)
    {
        char buff[BUFSIZE];

        bzero(buff, BUFSIZE);
        int brev = recv(sfd, buff, BUFSIZE, 0);
        buff[brev] = '\0';

        string resp = buff;
        chat::ServerResponse response;
        response.ParseFromString(resp);

        int op = response.option();

        if (op == 2)
        {
            for(const chat::UserInfo& user: response.connectedusers().connectedusers()) {
                cout << "Usuario: " << user.username() << endl;
                cout << "Ip: " << user.ip() << endl;
                cout << "Status: " << user.status()<< "\n" << endl;
            }
            
            pthread_cond_signal(&cond1);
        }        
        else if (op == 3) {
            status = response.change().status();

            cout << "Estatus nuevo: " << status << endl;

            pthread_cond_signal(&cond1);
            
        }
        else if (op == 4)
        {
            if (response.messagecommunication().recipient().compare("everyone") == 0)
            {
                mensajeGeneral(response.messagecommunication());
                
            } else {
                mensajePrivado(response.messagecommunication());
            }
        }
        else if (op == 5)
        {
            cout << "Usuario: " << response.userinforesponse().username() << endl;
            cout << "Status: " << response.userinforesponse().status() << endl;
            cout << "Ip: " << response.userinforesponse().ip() << endl;

            pthread_cond_signal(&cond1);

        }
        
        
    }
    
    
}

// Función para mostrar el menu de ayuda
void helpmen(int op){
    switch (op)
            {
            case 1:
                cout << "Para poder escribir en el chat general solo tiene que ingresar el mensaje que desee enviar y presionar enter.\n";
                cout << "En caso de no querer enviar un mensaje y solo leer los mensajes recientes solo necesitas presionar Enter" << endl;
                break;
            
            case 2:
                cout << "Para poder escribir un mensaje privado primero necesitas escribir el nombre del destinatario.\n";
                cout << "Si el usuario está conectado al servidor se te dejará escribir el mensaje que quieras. Caso contrario te avisa que el usuario no está disponible.\n";
                cout << "Luego de enviar el mensaje si el usuario sigue conectado entonces se le colocará el mensaje. Caso contrario te avisa no se pudo" << endl;
                break;

            case 3:
                cout << "Para cambio de estatus solo tienes que escribir el nuevo status que deseas tener.\n";
                cout << "Si este no es el mismo que el actual entonces se procederá a cambiarlo";
                break;
            
            case 4:
                cout << "Para listar los usuarios no tienes que hacer nada solo leer la lista :)" << endl;

            case 5:
                cout << "Para desplegar la información de un usuario, se ingresa el nombre de usuario\n";
                cout << "Si este está disponible entonces se retorna la información del usuario" << endl;

            default:
                cout << "Has ingresado una opción no válida" << endl;
                break;
            }

}


int main(int argc, char* argv[]) 
{
    // Variables temporales. En producción se borraran las variables
    string tempconnectedusers[2] = {"Lucia", "Ernesto"};

    // Constantes
    string menu = "Menu de opciones:\n  1. Chat general\n  2. Mensaje privado\n  3. Cambio de status\n  4. Usuarios conectados\n  5. Información de un usuario\n  6. Ayuda\n  7. Salir\n";
    string helpmenu = "Menu de ayuda:\n  1. Enviar el mensaje al chat general\n  2. Enviar un mensaje privado\n  3. Cambio de status\n  4. Usuarios conectados\n  5. Información de usuario\nIngrese la opción a la que requiere ayuda: ";

    int portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    GOOGLE_PROTOBUF_VERIFY_VERSION;

    if (argc < 3) {
       fprintf(stderr,"No se dieron todos los argumentos \n");
       exit(0);
    }

    server = gethostbyname(argv[2]);

    if (server == NULL) {
        fprintf(stderr,"Host name error\n");
        exit(0);
    }

    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0) cout << "Socket error" << endl;

    bzero(&serv_addr, sizeof(serv_addr));
    portno = atoi(argv[3]);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    serv_addr.sin_addr = *((struct in_addr *)server->h_addr);

/* condition */
    if (connect(sfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        cout << "Conection error" << endl;

    cout << "Usuario Conectado" << endl;
    

    chat::UserRegistration * userinfo(new chat::UserRegistration);
    userinfo->set_username(argv[1]);
    userinfo->set_ip(inet_ntoa(serv_addr.sin_addr));

    // Se crea instancia de Mensaje, se setea los valores deseados
    chat::ClientPetition clip;
    clip.set_option(1);
    clip.set_allocated_registration(userinfo);

    // Se serializa el message a string
    string binstr;
    clip.SerializeToString(&binstr);

    // Se transfiere el string a un arreglo de char y se copia la petición
    char cstr[binstr.size() + 1];
    strcpy(cstr, binstr.c_str());

    // Se envia la petición de registro al server
    send(sfd, cstr, strlen(cstr), 0);

    // Se prepara la recepción de la respuesta del servidor
    char bff[BUFSIZE];

    // Se recive la respuestá
    int bytesreceived = recv(sfd, bff, BUFSIZE, 0);
    
    // Se añade un EOF
    bff[bytesreceived] = '\0';

    // Se traspasa a un string para parseo
    string resp = bff;

    // Se trata de leer la petición del servidor
    chat::ServerResponse response;
    response.ParseFromString(resp);

    if (response.code() == 200 && response.option() == 1)
    {
        cout << "Estado de registro: Fino Señores" << endl;
    } else {
        cout << response.servermessage() << endl;
        cout << "No se pudo registrar/* condition */F en el chat"<< endl;
        exit(0);
    }

    pthread_t thear;
    pthread_create(&thear, NULL, readPet, NULL);

    // Objeto Client con el cual se enviarán las peticiones
    chat::ClientPetition client_petition;

    // Objeto de userRequest para solicitar 
    chat::UserRequest* userRequest(new chat::UserRequest);


    // String el cual se enviará entre cliente - servidor
    string serialized_string;
    
    // Nombre de usuario con el que se va a conectar
    string username = argv[1];

    int op;

    // Argumentos para el chateo
    string text_message;
    string recipient;

    while (true)
    {
        cout << menu << "Ingrese un número como opción: ";
        cin >> op;        

        cin.ignore();        

        if (op == 1) {
            // Imprimimos el chat general
            imprimirChatGeneral();            

            // Aquí se solicita el mensaje
            cout << "Ingrese su mensaje (Enter si solo desea leer los mensajes):\n> ";
            text_message = "";
            getline(cin, text_message);
            // Se verifica si el usuario solo desea leer el chat general
            if (text_message.compare("") != 0) {
                
                //INSTANCIAR MENSAJE OTRA VEZ!!!!!!!!!!!
                chat::MessageCommunication* message(new chat::MessageCommunication);

                // Aqui se prepara el formato para enviarlo
                message->set_message(text_message);
                message->set_recipient("everyone");
                message->set_sender(username);
                client_petition.set_option(4);
                client_petition.set_allocated_messagecommunication(message);
                client_petition.SerializeToString(&serialized_string);
  
                char cstr[serialized_string.size() + 1];
                strcpy(cstr, serialized_string.c_str());

                send(sfd, cstr, strlen(cstr), 0);
                
            };                     
        } else if (op == 2)
        {
            cout << "Ingrese el nombre del usuario con quien se desea chatear:\n>";
            recipient = "";
            getline(cin, recipient);
            
            // Implementar el jalado de mensajes del usuario en específico
            imprimirChatPrivado(recipient);
            
            // Aquí se solicita el mensaje
            cout << "Ingrese su mensaje (Enter si solo desea leer los mensajes):\n> ";
            text_message = "";
            getline(cin, text_message);

            // Se verifica si el usuario solo desea leer el chat general
            if (text_message.compare("") != 0) {
                //INSTANCIAR MENSAJE OTRA VEZ!!!!!!!!!!!
                chat::MessageCommunication* message(new chat::MessageCommunication);

                // Aqui se prepara el formato para enviarlo
                message->set_message(text_message);
                message->set_recipient(recipient);
                message->set_sender(username);
                client_petition.set_option(4);
                client_petition.set_allocated_messagecommunication(message);
                client_petition.SerializeToString(&serialized_string);

                chat::MessageCommunication sendermsg;
                sendermsg.set_message(text_message);
                sendermsg.set_recipient(recipient);
                sendermsg.set_sender(username);

                mensajePrivadoFrom(sendermsg);

                char cstr[serialized_string.size() + 1];
                strcpy(cstr, serialized_string.c_str());

                send(sfd, cstr, strlen(cstr), 0);

            }
        } else if (op == 3)
        {
            // Argumento para el cambio de status
            string newStatus;
            
            // Mostramos el estado
            cout << "Tu estatus actual es: " << status << endl;

            // Preguntamos el estado nuevo
            cout << "Ingresa el estatus nuevo (Enter si solo deseas ver tu estatus):\n>";
            getline(cin, newStatus);

            // Se verifica si el usuario solo desea ver su status
            if (newStatus.compare("") != 0) {
                transform(newStatus.begin(), newStatus.end(), newStatus.begin(), ::toupper);
                if (newStatus.compare(status) == 0) cout << "Has ingresado el estatus que ya tenias" << endl;
                else {
                    // Se verifica si el status ingresado es el mismo que ya tiene
                    if (newStatus.compare("ACTIVO") == 0 || newStatus.compare("OCUPADO") == 0 || newStatus.compare("INACTIVO") == 0) 
                    {
                        status = newStatus;

                        // Objeto del estatus para poder cambiarlo
                        chat::ChangeStatus* status(new chat::ChangeStatus);

                        //Aqui se prepara el formato para enviarlo
                        status->set_username(username);
                        status->set_status(newStatus);
                        client_petition.set_option(3);
                        client_petition.set_allocated_change(status);
                        client_petition.SerializeToString(&serialized_string);

                        char cstr[serialized_string.size() + 1];
                        strcpy(cstr, serialized_string.c_str());

                        send(sfd, cstr, strlen(cstr), 0);

                        pthread_cond_wait(&cond1, &locki);
                        
                    } else {
                        cout << "Estatus no Hallado" << endl;
                    }
                }
            };

        } else if (op == 4)
        {
            // Realizamos el pedido de la información de 
            userRequest->set_user("everyone");
            client_petition.set_option(2);
            client_petition.set_allocated_users(userRequest);
            client_petition.SerializeToString(&serialized_string);

            char cstr[serialized_string.size() + 1];
            strcpy(cstr, serialized_string.c_str());

            send(sfd, cstr, strlen(cstr), 0);

            // TO DO: Enviar la petición y esperar la respuesta del servidor y pasarlo a un arreglo
            pthread_cond_wait(&cond1, &locki);

            string connectedUsers[sizeof(tempconnectedusers) / sizeof(tempconnectedusers[0])];
            copy(begin(tempconnectedusers), end(tempconnectedusers), begin(connectedUsers));
            cout << "Lista de usuarios connectados" << endl;
            int counter = 1;
            for(string connectedUser: connectedUsers) cout << "   " << counter++ << ". " << connectedUser << endl;
        } else if (op == 5)
        {
            cout << "Ingrese el nombre del usuario del que desea saber su información (Enter si solo deseas pasar de largo):\n>";
            recipient = "";
            getline(cin, recipient);

            // Verificamos si el usuario solo quiere continuar
            if (recipient.compare("") != 0) {

                chat::UserRequest* userRequest(new chat::UserRequest);

                userRequest->set_user(recipient);
                client_petition.set_option(5);
                client_petition.set_allocated_users(userRequest);
                client_petition.SerializeToString(&serialized_string);
                
                char cstr[serialized_string.size() + 1];
                strcpy(cstr, serialized_string.c_str());

                send(sfd, cstr, strlen(cstr), 0);

                pthread_cond_wait(&cond1, &locki);

            }
        } else if (op == 6)
        {
            //Declaramos una variable de opciones para saber que ayuda quiere el usuario
            int op2;

            // Solicitamos la opción al usuario de la cual requiera ayuda
            cout << helpmenu;
            cin >> op2;

            // Evaluamos y mostramos la información necesaria
            helpmen(op2);
            
        } else if (op == 7)
        {
            cout << "Gracias por destruir mi proyecto, vuelva pronto :v" << endl;
            break;
        } else {
            cout << "Ingresaste una opción no válida" << endl;
        }
        
        
        
    }   
    

    return 0;
}