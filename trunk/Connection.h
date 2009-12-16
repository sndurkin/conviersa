#ifndef CONNECTION_H
#define CONNECTION_H

#include <QObject>
#include <QMessageBox>
#include <QSharedData>

class IChatWindow;
class QTcpSocket;
class QTextCodec;

class Connection : public QObject, public QSharedData
{
	Q_OBJECT
	
	IChatWindow *	m_pWindow;
	QTcpSocket *	m_pSocket;
	QTextCodec *	m_pCodec;
	QString 		m_prevBuffer;

public:
	Connection(IChatWindow *pWindow, QTextCodec *pCodec);
	virtual ~Connection();
	bool Connect(const char *pServer, quint16 port);
	bool Connect_SSL() { return true; }
	bool IsConnected() { return m_pSocket != NULL; }
	bool Send(const QString &data);
	void SetCodec(QTextCodec *pCodec) { m_pCodec = pCodec; }
	void Disconnect();

signals:
	// signal emitted when we are disconnected from
	// the connection to the socket
	void Connected();
	
	// signal emitted when we are disconnected from
	// the connection to the socket
	void Disconnected();
	
public slots:
	// is called when the socket connects to the server;
	void OnConnect();
	
	// is called when the socket is disconnected
	void OnDisconnect();
	
	// is called when there is data to be read from the socket;
	// reads the available data and fires the "OnReceiveData" event
	void OnReceiveData();
};

#endif
