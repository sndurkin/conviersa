/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#pragma once

#include "irc/Session.h"
#include "cv/Window.h"

using namespace irc;

class QVBoxLayout;
class QTreeView;
class QStandardItemModel;
class QModelIndex;
class QWidget;
class QPushButton;
class QCheckBox;
class QLabel;
class QGroupBox;
class QRadioButton;
class QSpinBox;

namespace cv {

class SearchBar;
class CLineEdit;

namespace irc {

class ChannelTopicDelegate;

/**
 * Document what, why and how.
 */

class ChannelListWindow : public Window
{
    Q_OBJECT

    QExplicitlySharedDataPointer<Session>
                            m_pSharedSession;

    // main layout
    QVBoxLayout *           m_pVLayout;

    QTreeView *             m_pView;
    QStandardItemModel *    m_pModel;
    ChannelTopicDelegate *  m_pDelegate;
    bool                    m_populatingList;

    // upper section that contains the downloading groupbox,
    // filtering groupbox, save list button and channels label
    QWidget *       m_pControlsSection;

    QGroupBox *     m_pDownloadingGroup;
    CLineEdit *     m_pCustomParameters;
    QCheckBox *     m_pTopicDisplay;
    QPushButton *   m_pDownloadListButton;
    QPushButton *   m_pStopDownloadButton;

    QLabel *        m_pChannelsLabel;
    int             m_currChannel;
    int             m_numVisible;

    QPushButton *   m_pSaveButton;

    QGroupBox *     m_pFilteringGroup;
    SearchBar *     m_pSearchBar;
    QTimer *        m_pSearchTimer;
    QString         m_searchStr;
    QRegExp         m_searchRegex;
    QCheckBox *     m_pCheckChanNames;
    QCheckBox *     m_pCheckChanTopics;
    QCheckBox *     m_pUseRegExp;
    QLabel *        m_pMinUsersLabel;
    QSpinBox *      m_pMinUsers;
    QLabel *        m_pMaxUsersLabel;
    QSpinBox *      m_pMaxUsers;
    QPushButton *   m_pApplyFilterButton;
    QPushButton *   m_pStopFilterButton;

    int             m_savedMinUsers;
    int             m_savedMaxUsers;
    bool            m_savedTopicDisplay;

public:
    ChannelListWindow(QExplicitlySharedDataPointer<Session> pSharedSession,
                    const QSize &size = QSize(715, 300));

    void giveFocus();

    // perform anything that needs to be done before
    // channels are added
    void beginPopulatingList();

    // add a channel to the QTreeWidget
    void addChannel(const QString &channel, const QString &numUsers, const QString &topic);

    // perform anything that needs to be done when all
    // channels have been added
    void endPopulatingList();

    // clear the list of all channels
    void clearList();

protected:
    // sets up all the controls that i'll be using to interact
    // with the channel list
    void setupControls();

public slots:
    // handles a connection fired from the Connection object
    void handleConnect();

    // handles a disconnection fired from the Connection object
    void handleDisconnect();

    // requests a new list of channels from the server
    void downloadList();

    // requests to stop the download of the channels from the server
    void stopDownload();

    // joins the channel which is found with the given index
    void joinChannel(const QModelIndex &index);

    // starts the filter
    void startFilter();

    // performs one iteration of a search from the search bar
    void performSearchIteration();

    // stops the filter
    void stopFilter();

    // saves the entire list to a file
    void saveList();
};

} } // end namespaces
