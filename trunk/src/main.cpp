#include <QApplication>
#include "Client.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	
	Client *pClient = new Client("IM Client");
	pClient->show();
      
	return app.exec();
}
