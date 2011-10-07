// Copyright (c) 2011 Conviersa Project. Use of this source code
// is governed by the MIT License.

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
