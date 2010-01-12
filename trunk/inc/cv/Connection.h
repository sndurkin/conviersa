/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#pragma once

#include <QObject>
#include <QTimer>

class QTcpSocket;

namespace cv {

// provides connection services to the rest of the irc classes.
// this class deals only with raw bytes and should use the parser
// interface to decode and get any meaningful information about
// the received data.
class Connection : public QObject
{
    Q_OBJECT

    QTcpSocket *    m_pSocket;
    QTimer          m_connectionTimer;

public:
    Connection();
    virtual ~Connection();

    bool connectToHost(const char *pServer, quint16 port);
    bool connectSSL() { return true; }
    void disconnectFromHost();
    bool isConnected() { return m_pSocket != NULL; }

    bool send(const QString &data);

    QString getHost();
    int getPort();

signals:
    // these are emitted from the Connection class
    void connected();
    void disconnected();
    void connectionFailed();
    void dataReceived(const QString &data);

public slots:
    // these are connected to the socket and are called whenever
    // the socket emits the corresponding signal
    void onConnect();
    void onDisconnect();
    void onFailedConnect();
    void onReadyRead();
};

} // end namespace
