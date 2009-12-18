/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#pragma once

#include "IChatWindow.h"

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
class IrcChanTopicDelegate;
class SearchBar;
class CLineEdit;

/**
 * Document what, why and how.
 */

class IrcChanListWindow : public IChatWindow
{
	Q_OBJECT
	
	// main layout
	QVBoxLayout *		m_pVLayout;
	
	QTreeView *			m_pView;
	QStandardItemModel *	m_pModel;
	IrcChanTopicDelegate *	m_pDelegate;
	bool				m_populatingList;
	
	// upper section that contains the downloading groupbox,
	// filtering groupbox, save list button and channels label
	QWidget *			m_pControlsSection;
	
	QGroupBox *			m_pDownloadingGroup;
	CLineEdit *			m_pCustomParameters;
	QCheckBox *			m_pTopicDisplay;
	QPushButton *		m_pDownloadListButton;
	QPushButton *		m_pStopDownloadButton;
	
	QLabel *			m_pChannelsLabel;
	int					m_currChannel;
	int					m_numVisible;
	
	QPushButton *		m_pSaveButton;
	
	QGroupBox *			m_pFilteringGroup;
	SearchBar *			m_pSearchBar;
	QTimer *			m_pSearchTimer;
	QString				m_searchStr;
	QRegExp				m_searchRegex;
	QCheckBox *			m_pCheckChanNames;
	QCheckBox *			m_pCheckChanTopics;
	QCheckBox *			m_pUseRegExp;
	QLabel *			m_pMinUsersLabel;
	QSpinBox *			m_pMinUsers;
	QLabel *			m_pMaxUsersLabel;
	QSpinBox *			m_pMaxUsers;
	QPushButton *		m_pApplyFilterButton;
	QPushButton *		m_pStopFilterButton;
	
	int					m_savedMinUsers;
	int					m_savedMaxUsers;
	bool				m_savedTopicDisplay;

public:
	IrcChanListWindow(QExplicitlySharedDataPointer<Connection> pSharedConn,
					const QSize &size = QSize(715, 300));
	
	void GiveFocus();
	
	// perform anything that needs to be done before
	// channels are added
	void BeginPopulatingList();
	
	// add a channel to the QTreeWidget
	void AddChannel(const QString &channel, const QString &numUsers, const QString &topic);
	
	// perform anything that needs to be done when all
	// channels have been added
	void EndPopulatingList();
	
	// clear the list of all channels
	void ClearList();
	
protected:
	// sets up all the controls that i'll be using to interact
	// with the channel list
	void SetupControls();

public slots:
	// handles a connection fired from the Connection object
	void HandleConnect();
	
	// handles a disconnection fired from the Connection object
	void HandleDisconnect();
	
	// requests a new list of channels from the server
	void DownloadList();
	
	// requests to stop the download of the channels from the server
	void StopDownload();
	
	// joins the channel which is found with the given index
	void JoinChannel(const QModelIndex &index);
	
	// starts the filter
	void StartFilter();
	
	// performs one iteration of a search from the search bar
	void PerformSearchIteration();
	
	// stops the filter
	void StopFilter();
	
	// saves the entire list to a file
	void SaveList();
};
