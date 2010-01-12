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

#include "cv/Connection.h"

const int CONFIG_CONNECTION_TIMEOUT_MSEC = 10000;

namespace cv {

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

bool Connection::connectToHost(const char *pServer, quint16 port)
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
    QObject::connect(m_pSocket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    QObject::connect(&m_connectionTimer, SIGNAL(timeout()), this, SLOT(onFailedConnect()));

    m_pSocket->connectToHost(pServer, port);
    m_connectionTimer.start(CONFIG_CONNECTION_TIMEOUT_MSEC);

    // TODO: log this

    return true;
}

//-----------------------------------//

void Connection::disconnectFromHost()
{
    if(isConnected())
        m_pSocket->close();
}

//-----------------------------------//

bool Connection::send(const QString &data)
{
    if(!isConnected())
        return false;

    m_pSocket->write(data.toAscii());
    return true;
}

//-----------------------------------//

QString Connection::getHost()
{
    if(isConnected())
        return m_pSocket->peerName();

    return "";
}

//-----------------------------------//

int Connection::getPort()
{
    if(isConnected())
        return m_pSocket->peerPort();

    return -1;
}

//-----------------------------------//

void Connection::onConnect()
{
    m_connectionTimer.stop();
    emit connected();
}

//-----------------------------------//

void Connection::onDisconnect()
{
    // todo: delete later?
    //delete m_pSocket;
    m_pSocket = NULL;
    emit disconnected();
}

//-----------------------------------//

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

void Connection::onReadyRead()
{
    while(true)
    {
        char buffer[SOCKET_BUFFER_SIZE];
        qint64 size = m_pSocket->read(buffer, SOCKET_BUFFER_SIZE-1);

        if(size > 0)
        {
            buffer[size] = '\0';
            QString data(buffer);
            emit dataReceived(data);
        }
        else
        {
            if(size < 0)
            {
                // TODO: log this
            }
            break;
        }
    }
}

//-----------------------------------//

} // end namespace
