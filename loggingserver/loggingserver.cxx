// Module:  LOG4CPLUS
// File:    loggingserver.cxx
// Created: 5/2003
// Author:  Tad E. Smith
//
//
// Copyright 2003-2010 Tad E. Smith
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <cstdlib>
#include <iostream>
#include <log4cplus/config.hxx>
#include <log4cplus/configurator.h>
#include <log4cplus/consoleappender.h>
#include <log4cplus/socketappender.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/socket.h>
#include <log4cplus/helpers/threads.h>
#include <log4cplus/spi/loggerimpl.h>
#include <log4cplus/spi/loggingevent.h>


using namespace std;
using namespace log4cplus;
using namespace log4cplus::helpers;
using namespace log4cplus::thread;


namespace loggingserver {
    class ClientThread : public AbstractThread {
    public:
        ClientThread(Socket clientsock_)
        : clientsock(clientsock_) 
        {
            cout << "Received a client connection!!!!" << endl;
        }

        ~ClientThread()
        {
            cout << "Client connection closed." << endl;
        }

        virtual void run();

    private:
        Socket clientsock;
    };

}




int
main(int argc, char** argv)
{
    if(argc < 3) {
        cout << "Usage: port config_file" << endl;
        return 1;
    }
    int port = std::atoi(argv[1]);
    tstring configFile = LOG4CPLUS_C_STR_TO_TSTRING(argv[2]);

    PropertyConfigurator config(configFile);
    config.configure();

    ServerSocket serverSocket(port);
    if (!serverSocket.isOpen()) {
        cout << "Could not open server socket, maybe port "
            << port << " is already in use." << endl;
        return 2;
    }

    while(1) {
        loggingserver::ClientThread *thr = 
            new loggingserver::ClientThread(serverSocket.accept());
        thr->start();
    }

    return 0;
}


////////////////////////////////////////////////////////////////////////////////
// loggingserver::ClientThread implementation
////////////////////////////////////////////////////////////////////////////////


void
loggingserver::ClientThread::run()
{
    while(1) {
        if(!clientsock.isOpen()) {
            return;
        }
        SocketBuffer msgSizeBuffer(sizeof(unsigned int));
        if(!clientsock.read(msgSizeBuffer)) {
            return;
        }

        unsigned int msgSize = msgSizeBuffer.readInt();

        SocketBuffer buffer(msgSize);
        if(!clientsock.read(buffer)) {
            return;
        }
        
        spi::InternalLoggingEvent event = readFromBuffer(buffer);
        Logger logger = Logger::getInstance(event.getLoggerName());
        logger.callAppenders(event);   
    }
}


