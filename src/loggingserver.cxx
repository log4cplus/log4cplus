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
// Revision 1.1  2003/05/04 07:25:16  tcsmith
// Initial version.
//

#include <log4cplus/config.h>
#include <log4cplus/consoleappender.h>
#include <log4cplus/socketappender.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/socket.h>
#include <log4cplus/helpers/threads.h>
#include <log4cplus/spi/loggingevent.h>

#include <iostream>

using namespace std;
using namespace log4cplus;
using namespace log4cplus::helpers;
using namespace log4cplus::thread;


namespace loggingserver {
    class ClientThread : public AbstractThread {
    public:
        ClientThread(SharedAppenderPtr appender, Socket clientsock)
        : appender(appender), clientsock(clientsock) 
        {
            cout << "Received a client connection!!!!" << endl;
        }

        ~ClientThread()
        {
            cout << "Client connection closed." << endl;
        }

        virtual void run();
    private:
        SharedAppenderPtr appender;
        Socket clientsock;
    };

}




int
main()
{
    SharedAppenderPtr append_1(new ConsoleAppender());
    append_1->setName("First");
    append_1->setLayout( std::auto_ptr<Layout>(new log4cplus::TTCCLayout()) );

    ServerSocket serverSocket(9998);
    while(1) {
        loggingserver::ClientThread *thr = 
            new loggingserver::ClientThread(append_1, serverSocket.accept());
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
        appender->doAppend( readFromBuffer(buffer) );   
    }
}


