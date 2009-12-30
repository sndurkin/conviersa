/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#pragma once

#include <QObject>
#include <QMessageBox>
#include <QSharedData>

namespace cv {

class IChatWindow;
class QTcpSocket;
class QTextCodec;

// provides connection services to the rest of the irc classes.
// this class deals only with raw bytes and should use the parser
// interface to decode and get any meaningful information about
// the received data.
class Connection : public QObject, public QSharedData
{
    Q_OBJECT

    IChatWindow *   m_pWindow;
    QTcpSocket *    m_pSocket;
    QTextCodec *    m_pCodec;
    QString         m_prevBuffer;

public:

    Connection(IChatWindow *pWindow, QTextCodec *pCodec);
    virtual ~Connection();

    bool connect(const char *pServer, quint16 port);
    bool connectSSL() { return true; }

    void disconnect();

    bool isConnected() { return m_pSocket != NULL; }

    bool send(const QString &data);
    void setCodec(QTextCodec *pCodec) { m_pCodec = pCodec; }

signals:
    // signal emitted when we get a connection to the server
    void connected();

    // signal emitted when we are disconnected from the server
    void disconnected();

public slots:
    // is called when the socket connects to the server;
    void onConnect();

    // is called when the socket is disconnected
    void onDisconnect();

    // is called when there is data to be read from the socket;
    // reads the available data and fires the "OnReceiveData" event
    void onReceiveData();
};

} // end namespace
