// Module:  LOG4CPLUS
// File:    loggingserver.cxx
// Created: 5/2003
// Author:  Tad E. Smith
//
//
// Copyright (C) Tad E. Smith  All rights reserved.
//
// This software is published under the terms of the Apache Software
// License version 1.1, a copy of which has been included with this
// distribution in the LICENSE.APL file.
//

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
        ClientThread(Socket clientsock)
        : clientsock(clientsock) 
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
    int port = atoi(argv[1]);
    tstring configFile = LOG4CPLUS_C_STR_TO_TSTRING(argv[2]);

    PropertyConfigurator config(configFile);
    config.configure();

    ServerSocket serverSocket(port);
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


