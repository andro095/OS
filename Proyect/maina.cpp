#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <algorithm>
#include "./new.pb.h"

using namespace std;

int main(int argc, char* argv[]) 
{
    // Variables temporales. En producción se borraran las variables
    string tempbroadchat = "@Lucia: Jajaja ya lograron hacer su proyecto?\n@Ernesto: Pues ya casi, solo me hace falta cosas como el estatus";
    string tempprivchat = "Jajaja ya lograron hacer su proyecto?";
    string tempstatus = "ACTIV";
    string tempconnectedusers[2] = {"Lucia", "Ernesto"};

    // Constantes
    string menu = "Menu de opciones:\n  1. Chat general\n  2. Mensaje privado\n  3. Cambio de status\n  4. Usuarios conectados\n  5. Información de un usuario\n  6. Ayuda\n  7. Salir\n";
    string helpmenu = "Menu de ayuda:\n  1. Enviar el mensaje al chat general\n  2. Enviar un mensaje privado\n  3. Cambio de status\n  4. Usuarios conectados\n  5. Información de usuario\nIngrese la opción a la que requiere ayuda: ";

    // Objeto Client con el cual se enviarán las peticiones
    chat::ClientPetition client_petition;
    

    // Objeto de mensaje para poder chatear
    chat::MessageCommunication* message(new chat::MessageCommunication);

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
            // TO DO: Implementar el jalado de mensajes;
            cout << tempbroadchat << endl;
            

            // Aquí se solicita el mensaje
            cout << "Ingrese su mensaje (Enter si solo desea leer los mensajes):\n> ";
            getline(cin, text_message);

            // Se verifica si el usuario solo desea leer el chat general
            if (text_message.compare("") != 0) {
                // Aqui se prepara el formato para enviarlo
                message->set_message(text_message);
                message->set_recipient("everyone");
                message->set_sender(username);
                client_petition.set_option(4);
                client_petition.set_allocated_messagecommunication(message);
                client_petition.SerializeToString(&serialized_string);
                // TO DO: Enviar el mensaje y volver a implementar el jalado de mensajes para asegurar el correcto envio al servidor.; 
            };                     
        } else if (op == 2)
        {
            cout << "Ingrese el nombre del usuario con quien se desea chatear:\n>";
            getline(cin, recipient);
            // TO DO: Implementar el jalado de mensajes del usuario en específico

            // Aquí se solicita el mensaje
            cout << tempprivchat << endl;
            cout << "Ingrese su mensaje (Enter si solo desea leer los mensajes):\n> ";
            getline(cin, text_message);

            // Se verifica si el usuario solo desea leer el chat general
            if (text_message.compare("") != 0) {
                // Aqui se prepara el formato para enviarlo
                message->set_message(text_message);
                message->set_recipient(recipient);
                message->set_sender(username);
                client_petition.set_option(4);
                client_petition.set_allocated_messagecommunication(message);
                client_petition.SerializeToString(&serialized_string);
                // TO DO: Enviar el mensaje y volver a implementar el jalado de mensajes para asegurar el correcto envio al servidor.;

            }
        } else if (op == 3)
        {
            // Argumento para el cambio de status
            string newStatus;

            // TO DO: Implementar el jalado del estatus actual del usuario
            // Mostramos el estado
            cout << "Tu estatus actual es: " << tempstatus << endl;

            // Preguntamos el estado nuevo
            cout << "Ingresa el estatus nuevo (Enter si solo deseas ver tu estatus):\n>";
            getline(cin, newStatus);

            // Se verifica si el usuario solo desea ver su status
            if (newStatus.compare("") != 0) {
                transform(newStatus.begin(), newStatus.end(), newStatus.begin(), ::toupper);
                if (newStatus.compare(tempstatus) == 0) cout << "Has ingresado el estatus que ya tenias" << endl;
                else {
                    // Se verifica si el status ingresado es el mismo que ya tiene
                    if (newStatus.compare("ACTIVO") == 0 || newStatus.compare("OCUPADO") == 0 || newStatus.compare("INACTIVO") == 0) 
                    {
                        // Objeto del estatus para poder cambiarlo
                        chat::ChangeStatus* status(new chat::ChangeStatus);

                        //Aqui se prepara el formato para enviarlo
                        status->set_username(username);
                        status->set_status(newStatus);
                        client_petition.set_option(3);
                        client_petition.set_allocated_change(status);
                        client_petition.SerializeToString(&serialized_string);
                        // TO DO: Enviar el status y volver a jalar el estatus para confirmar el cambio de estatus
                        
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

            // TO DO: Enviar la petición y esperar la respuesta del servidor y pasarlo a un arreglo

            string connectedUsers[sizeof(tempconnectedusers) / sizeof(tempconnectedusers[0])];
            copy(begin(tempconnectedusers), end(tempconnectedusers), begin(connectedUsers));
            cout << "Lista de usuarios connectados" << endl;
            int counter = 1;
            for(string connectedUser: connectedUsers) cout << "   " << counter++ << ". " << connectedUser << endl;
        } else if (op == 5)
        {
            cout << "Ingrese el nombre del usuario del que desea saber su información (Enter si solo deseas pasar de largo):\n>";
            getline(cin, recipient);

            // Verificamos si el usuario solo quiere continuar
            if (recipient.compare("") != 0) {

                userRequest->set_user(recipient);
                client_petition.set_option(5);
                client_petition.set_allocated_users(userRequest);
                client_petition.SerializeToString(&serialized_string);
                // TO DO: Enviar la petición de información de un usuario y mostrar la información



            }
        } else if (op == 6)
        {
            //Declaramos una variable de opciones para saber que ayuda quiere el usuario
            int op2;

            // Solicitamos la opción al usuario de la cual requiera ayuda
            cout << helpmenu;
            cin >> op2;

            // Evaluamos y mostramos la información necesaria
            switch (op2)
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
                cout << ""

            default:
                break;
            }

        } else if (op == 7)
        {
            cout << "Gracias por destruir mi proyecto, vuelva pronto :v" << endl;
            break;
        } else {
            cout << "Ingresaste una opción no válida" << endl;
        }
        
        
        
        
        break;
    }   
    

    return 0;
}