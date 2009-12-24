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
#include "Connection.h"
#include "IChatWindow.h"

//-----------------------------------//

Connection::Connection(IChatWindow *pWindow, QTextCodec *pCodec)
    : m_pWindow(pWindow),
      m_pSocket(NULL),
      m_pCodec(pCodec)
{ }

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

    // connects the signal to the slot so it can start reading from the socket
    QObject::connect(m_pSocket, SIGNAL(connected()), this, SLOT(onConnect()));
    QObject::connect(m_pSocket, SIGNAL(disconnected()), this, SLOT(onDisconnect()));
    QObject::connect(m_pSocket, SIGNAL(readyRead()), this, SLOT(onReceiveData()));

    // QString evtStr = "onReceiveData:";
    // evtStr += m_pWindow->windowTitle();
    // CreateEvent(evtStr);

    // todo: stateChanged(), etc. --> status bar
    m_pSocket->connectToHost(pServer, port);
    if(!m_pSocket->waitForConnected(10000))
    {
        // todo: error messages on output
        QMessageBox errMsg(QMessageBox::NoIcon, "Connection Error", m_pSocket->errorString());
        errMsg.exec();
        delete m_pSocket;
        m_pSocket = NULL;
        return false;
    }

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
    emit connected();

    // todo: use options
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

// is called when there is data to be read from the socket;
// reads the available data and fires the "OnReceiveData" event
void Connection::onReceiveData()
{
    while(true)
    {
        char tempBuf[1024];
        qint64 size = m_pSocket->read(tempBuf, 1023);

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
                QMessageBox msg(QMessageBox::NoIcon, "Error", "There was an error in reading from the socket.");
                msg.exec();
            }
            break;
        }
    }
}

//-----------------------------------//
