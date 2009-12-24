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

class IChatWindow;
class QTcpSocket;
class QTextCodec;

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
    bool isConnected() { return m_pSocket != NULL; }
    bool send(const QString &data);
    void setCodec(QTextCodec *pCodec) { m_pCodec = pCodec; }
    void disconnect();

signals:
    // signal emitted when we are disconnected from
    // the connection to the socket
    void connected();

    // signal emitted when we are disconnected from
    // the connection to the socket
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
