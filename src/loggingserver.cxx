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
// $Log: not supported by cvs2svn $
// Revision 1.4  2003/06/23 20:56:43  tcsmith
// Modified to support the changes in the spi::InternalLoggingEvent class.
//
// Revision 1.3  2003/05/04 08:43:11  tcsmith
// Now takes a port and a config file name as parameters on startup.
//
// Revision 1.2  2003/05/04 07:39:48  tcsmith
// Removed a debug message.
//
// Revision 1.1  2003/05/04 07:25:16  tcsmith
// Initial version.
//

#include <log4cplus/config.h>
#include <log4cplus/configurator.h>
#include <log4cplus/consoleappender.h>
#include <log4cplus/socketappender.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/socket.h>
#include <log4cplus/helpers/threads.h>
#include <log4cplus/spi/loggerimpl.h>
#include <log4cplus/spi/loggingevent.h>

#include <iostream>

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


