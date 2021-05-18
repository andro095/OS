#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include "./new.pb.h"

using namespace std;

int main(int argc, char* argv[]) 
{
    // Variables temporales. En producción se borraran las variables
    string tempbroadchat = "@Lucia: Jajaja ya lograron hacer su proyecto?\n@Ernesto: Pues ya casi, solo me hace falta cosas como el estatus";
    string tempprivchat = "Jajaja ya lograron hacer su proyecto?";
    string tempstatus = "ACTIVO";
    string tempconnectedusers[2] = {"Lucia", "Ernesto"};

    string menu = "Menu de opciones:\n  1. Escribir en el chat general\n  2. Enviar o recibir mensajes privados\n  3. Cambiar mi status\n  4. Listar los usuarios que están conectados\n  5. Desplegar la información de un usuario\n  6. Ayuda\n  7. Salir";
    
    // Objeto Client con el cual se enviarán las peticiones
    chat::ClientPetition client_petition;

    // Objeto de mensaje para poder chatear
    chat::MessageCommunication* message(new chat::MessageCommunication);

    // Objeto del estatus para poder cambiarlo
    chat::ChangeStatus* status(new chat::ChangeStatus);

    // String el cual se enviará entre cliente - servidor
    string serialized_string;
    
    // Nombre de usuario con el que se va a conectar
    string username = argv[1];

    int op;

    // Argumentos para el chateo
    string text_message;
    string recipient;

    // Argumento para el cambio de status
    string newStatus;

    // Argumento para la nueva lista
    string connectedUsers[sizeof(tempconnectedusers)];

    while (true)
    {
        cout << "Ingrese un número como opción: ";
        cin >> op;

        

        cin.ignore();        
        
        switch (op)
        {
        case 1:
            // TO DO: Implementar el jalado de mensajes;
            cout << tempbroadchat << endl;

            // Aquí se solicita el mensaje
            cout << "Ingrese su mensaje (Enter si solo desea leer los mensajes):\n> ";
            getline(cin, text_message);

            // Se verifica si el usuario solo desea leer el chat general
            if (text_message.compare("") == 0) break;
            
            // Aqui se prepara el formato para enviarlo
            message->set_message(text_message);
            message->set_recipient("everyone");
            message->set_sender(username);
            client_petition.set_option(4);
            client_petition.set_allocated_messagecommunication(message);
            client_petition.SerializeToString(&serialized_string);
            // TO DO: Enviar el mensaje y volver a implementar el jalado de mensajes para asegurar el correcto envio al servidor.;

            break;
        case 2:           
            cout << "Ingrese el nombre del usuario con quien se desea chatear:\n>";
            getline(cin, recipient);
            // TO DO: Implementar el jalado de mensajes del usuario en específico

            // Aquí se solicita el mensaje
            cout << tempprivchat << endl;
            cout << "Ingrese su mensaje (Enter si solo desea leer los mensajes):\n> ";
            getline(cin, text_message);

            // Se verifica si el usuario solo desea leer el chat general
            if (text_message.compare("") == 0) break;

            // Aqui se prepara el formato para enviarlo
            message->set_message(text_message);
            message->set_recipient(recipient);
            message->set_sender(username);
            client_petition.set_option(4);
            client_petition.set_allocated_messagecommunication(message);
            client_petition.SerializeToString(&serialized_string);
            // TO DO: Enviar el mensaje y volver a implementar el jalado de mensajes para asegurar el correcto envio al servidor.;

            break;
        
        case 3:
            // TO DO: Implementar el jalado del estatus actual del usuario
            // Mostramos el estado
            cout << "Tu estatus actual es: " << tempstatus << endl;

            // Preguntamos el estado nuevo
            cout << "Ingresa el estatus nuevo (Enter si solo deseas ver tu estatus):\n>";
            getline(cin, newStatus);

            // Se verifica si el usuario solo desea ver su status
            if (newStatus.compare("") == 0) break;

            // Se verifica si el status ingresado es el mismo que ya tiene
            transform(newStatus.begin(), newStatus.end(), newStatus.begin(), ::toupper);
            if (newStatus.compare(tempstatus) == 0) {
                cout << "Has ingresado el estatus que ya tenias" << endl;
                break;
            }
            
            //Aqui se prepara el formato para enviarlo
            status->set_username(username);
            status->set_status(newStatus);
            client_petition.set_option(3);
            client_petition.set_allocated_change(status);
            client_petition.SerializeToString(&serialized_string);
            // TO DO: Enviar el status y volver a jalar el estatus para confirmar el cambio de estatus

            break;

        case 4:
            // TO DO: Jalar la lista de los usuarios conectados

            

            break;

        default:
            break;
        }
        break;
    }   
    


    /*chat::UserInfo user;

    user.set_username("André");
    user.set_status("ACTIVO");
    user.set_ip("192.168.0.1");

    cout << menu.size() << endl;

    cout << "Name:\t" << user.username() << endl;
    cout << "Status:\t" << user.status() << endl;

    cout << "Hello, World!" << endl;*/
    return 0;
}