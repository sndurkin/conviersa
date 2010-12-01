/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#pragma once

#include <QThread>
#include <QMutex>
#include <QAbstractSocket>

class QTcpSocket;
class QTimer;

namespace cv {

class Connection : public QObject
{
    Q_OBJECT

    QTcpSocket *m_pSocket;
    QTimer *    m_pConnectionTimer;

public:
    Connection();
    ~Connection();

    bool isConnected();
    QAbstractSocket::SocketError error();

signals:
    void connecting();
    void connected();
    void disconnected();
    void connectionFailed();
    void dataReceived(const QString &data);

public slots:
    void connectToHost(const QString &host, quint16 port);
    void disconnectFromHost();
    void send(const QString &data);

    // these are connected to the socket and are called whenever
    // the socket emits the corresponding signal
    void onConnect();
    void onConnectionTimeout();
    void onReadyRead();
};


class ThreadedConnection : public QThread
{
    Q_OBJECT

    Connection *    m_pConnection;
    QMutex          m_mutex;

public:
    ThreadedConnection(QObject *pParent = NULL);
    ~ThreadedConnection();

    void connectToHost(const QString &host, quint16 port);
    bool connectSSL() { return true; }
    void disconnectFromHost();
    bool isConnected();

    void send(const QString &data);

    QAbstractSocket::SocketError error();

protected:
    void run();

signals:
    // these are emitted from the ThreadedConnection class
    void connecting();
    void connected();
    void disconnected();
    void connectionFailed();
    void dataReceived(const QString &data);

    // signals to call into the alternate thread
    void connectToHostSignal(const QString &host, quint16 port);
    void disconnectFromHostSignal();
    void sendSignal(const QString &data);
};

} // end namespace
