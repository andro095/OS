#include <iostream>
#include <string>
#include <time.h>
#include "user.h"

using namespace std;


//Define user class

User::User(string inUser, string inIP, int inSocket)
{
	userName = inUser;
	ip = inIP;
	usedsocket = inSocket;
	userStatus = "Online";
	isInactive = false;
}


User::User()
{
	userName = "Diego";
	ip = "localhost";
	usedsocket = 8080;
	userStatus = "Online";
	isInactive = false;
}


void User::setLastTimeMessage(time_t new_timestamp)
{
	msg_timestamp=new_timestamp;

}

int User::setStatus(int state)
{
	if (state > 0 &&  state <= 3)
	{
		status = state;
		return 1;
	}
	return 0;	
}


time_t User::getLastTimeMessage()
{
	return msg_timestamp;
}

string User::getStatus()
{
    return userStatus;
}

string User::Info()
{
	string info;
	info = "User: \t" + userName + " \nIp: \t\t" + ip + "\nEstado: \t" + User::getStatus() + "\n";
	return info;

}