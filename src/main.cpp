/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#include <QApplication>
#include "cv/gui/Client.h"

using namespace cv;
using namespace gui;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Client *pClient = new Client("Conviersa");
    pClient->show();

    return app.exec();
}