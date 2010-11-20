/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#pragma once

#include "cv/gui/OverlayPanel.h"

class QLabel;
class QLineEdit;

namespace cv { namespace gui {

class ServerConnectionPanel : public OverlayPanel, public QSharedData
{
    Q_OBJECT

    // all the form widgets used to connect
    // to the server
    QPushButton *       m_pConnectButton;
    QPushButton *       m_pCloseButton;

    QLabel *            m_pServerLbl;
    QLineEdit *         m_pServerInput;
    QLabel *            m_pPortLbl;
    QLineEdit *         m_pPortInput;

    QLabel *            m_pNameLbl;
    QLineEdit *         m_pNameInput;

    QLabel *            m_pNickLbl;
    QLineEdit *         m_pNickInput;
    QLabel *            m_pAltNickLbl;
    QLineEdit *         m_pAltNickInput;

public:
    ServerConnectionPanel(QWidget *parent);

    // if all fields are valid:
    //   connect to the server
    // otherwise:
    //   set the focus to the first invalid field
    void validateAndConnect();

signals:
    void connect(QString server, int port, QString name, QString nick, QString altNick);

public slots:
    void onCloseClicked();
    void onEnter();

protected:
    void createForm();
};

} } // end namespaces
