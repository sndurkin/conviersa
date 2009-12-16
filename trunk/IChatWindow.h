#ifndef ICHATWINDOW_H
#define ICHATWINDOW_H

#include <QString>
#include <QExplicitlySharedDataPointer>
#include "IWindow.h"
#include "Connection.h"

class IChatWindow : public IWindow
{
protected:
	QExplicitlySharedDataPointer<Connection>	m_pSharedConn;

public:
	IChatWindow(const QString &title = tr("Untitled"),
			const QSize &size = QSize(500, 300));
	
	virtual void HandleData(QString &data) { }
	//virtual void PrintOutput(const QString &text) = 0;
};

#endif
