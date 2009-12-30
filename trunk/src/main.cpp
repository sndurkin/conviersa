/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#include <QApplication>
#include "Client.h"

using namespace cv;

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	
	Client *pClient = new Client("IM Client");
	pClient->show();
      
	return app.exec();
}