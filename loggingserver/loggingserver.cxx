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
#include <log4cplus/configurator.h>
#include <log4cplus/socketappender.h>
#include <log4cplus/helpers/socket.h>
#include <log4cplus/helpers/threads.h>
#include <log4cplus/spi/loggingevent.h>


namespace loggingserver
{

class ClientThread : public log4cplus::thread::AbstractThread
{
public:
    ClientThread(log4cplus::helpers::Socket clientsock)
    : clientsock(clientsock) 
    {
        std::cout << "Received a client connection!!!!" << std::endl;
    }

    ~ClientThread()
    {
        std::cout << "Client connection closed." << std::endl;
    }

    virtual void run();

private:
    log4cplus::helpers::Socket clientsock;
};

}




int
main(int argc, char** argv)
{
    if(argc < 3) {
        std::cout << "Usage: port config_file" << std::endl;
        return 1;
    }
    int port = std::atoi(argv[1]);
    const log4cplus::tstring configFile = LOG4CPLUS_C_STR_TO_TSTRING(argv[2]);

    log4cplus::PropertyConfigurator config(configFile);
    config.configure();

    log4cplus::helpers::ServerSocket serverSocket(port);
    if (!serverSocket.isOpen()) {
        std::cout << "Could not open server socket, maybe port "
            << port << " is already in use." << std::endl;
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
        log4cplus::helpers::SocketBuffer msgSizeBuffer(sizeof(unsigned int));
        if(!clientsock.read(msgSizeBuffer)) {
            return;
        }

        unsigned int msgSize = msgSizeBuffer.readInt();

        log4cplus::helpers::SocketBuffer buffer(msgSize);
        if(!clientsock.read(buffer)) {
            return;
        }
        
        log4cplus::spi::InternalLoggingEvent event
            = log4cplus::helpers::readFromBuffer(buffer);
        log4cplus::Logger logger
            = log4cplus::Logger::getInstance(event.getLoggerName());
        logger.callAppenders(event);   
    }
}
