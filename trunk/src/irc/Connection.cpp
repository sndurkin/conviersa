/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#include <QTextCodec>
#include <QTcpSocket>
#include <QSslSocket>

#include "irc/Connection.h"

const int CONFIG_CONNECTION_TIMEOUT_MSEC = 10000;

namespace irc {

//-----------------------------------//

Connection::Connection()
    : m_pSocket(NULL)
{
    m_connectionTimer.setSingleShot(true);
}

//-----------------------------------//

Connection::~Connection()
{
    if(isConnected())
        m_pSocket->close();
}

//-----------------------------------//

bool Connection::connect(const char *pServer, quint16 port)
{
    if(isConnected())
    {
        // todo: change this horrible error response
        return false;
    }

    m_pSocket = new (std::nothrow) QTcpSocket;
    if(!m_pSocket)
        return false;

    // connects the necessary signals to each corresponding slot
    QObject::connect(m_pSocket, SIGNAL(connected()), this, SLOT(onConnect()));
    QObject::connect(m_pSocket, SIGNAL(disconnected()), this, SLOT(onDisconnect()));
    QObject::connect(m_pSocket, SIGNAL(readyRead()), this, SLOT(onReceiveData()));
    QObject::connect(&m_connectionTimer, SIGNAL(timeout()), this, SLOT(onFailedConnect()));

    m_pSocket->connectToHost(pServer, port);
    m_connectionTimer.start(CONFIG_CONNECTION_TIMEOUT_MSEC);

    // TODO: log this

    return true;
}

//-----------------------------------//

bool Connection::send(const QString &data)
{
    if(!m_pSocket)
        return false;

    QString temp(m_pCodec->toUnicode(data.toAscii()) + "\r\n");
    m_pSocket->write(temp.toAscii());
    return true;
}

//-----------------------------------//

void Connection::disconnect()
{
    if(isConnected())
    {
        m_pSocket->close();
    }
}

//-----------------------------------//

// is called when the socket connects to the server;
void Connection::onConnect()
{
    m_connectionTimer.stop();
    emit connected();

    // todo: use options
    // todo: move to irc::Session
    char *buf = "PASS hello\nNICK seand`\nUSER guest tolmoon tolsun :Ronnie Reagan\n";
    m_pSocket->write(buf, strlen(buf));
}

//-----------------------------------//

// is called when the socket is disconnected
void Connection::onDisconnect()
{
    m_pSocket = NULL;
    emit disconnected();
}

//-----------------------------------//

// is called when the timer times out before the socket can connect
void Connection::onFailedConnect()
{
    // checks to see if it has emitted the connected() signal, to prevent
    // a possible race condition
    if(m_pSocket->state() == QAbstractSocket::ConnectedState)
        return;

    delete m_pSocket;
    m_pSocket = NULL;
    emit connectionFailed();
}

//-----------------------------------//

const int SOCKET_BUFFER_SIZE = 1024;

// is called when there is data to be read from the socket
void Connection::onReceiveData()
{
    while(true)
    {
        char tempBuf[SOCKET_BUFFER_SIZE];
        qint64 size = m_pSocket->read(tempBuf, SOCKET_BUFFER_SIZE-1);

        if(size > 0)
        {
            tempBuf[size] = '\0';
            m_prevBuffer += tempBuf;

            // check for a message within the buffer, to ensure only
            // whole messages are handled
            int numChars;
            while((numChars = m_prevBuffer.indexOf('\n') + 1) > 0)
            {
                // retrieves the entire message up to and including the terminating '\n' character
                QString msg = m_prevBuffer.left(numChars);
                m_pWindow->handleData(msg);
                m_prevBuffer.remove(0, numChars);
            }
        }
        else
        {
            if(size < 0)
            {
                // TODO: log this

                //QMessageBox msg(QMessageBox::NoIcon, "Error", "There was an error in reading from the socket.");
                //msg.exec();
            }
            break;
        }
    }
}

//-----------------------------------//

} // end namespace
