/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#pragma once

#include <QVBoxLayout>
#include <QPlainTextEdit>
#include "IChatWindow.h"
#include "IrcServerInfoService.h"

class QTextEdit;
class QMutex;
class QTimer;
class QTextTable;
class IrcWindowScrollBar;

//-----------------------------------//

/**
 * IRC window interface. Windows can be divided into different types:
 *		- Status windows
 *		- Channel windows
 *		- Private windows
 * TODO: this needs a redisign. Looks messy to me. -- triton
 */

class IIrcWindow : public IChatWindow
{
	Q_OBJECT
	
protected:
	QVBoxLayout	*		m_pVLayout;
	
	int					m_startOfText;
	QTextEdit *			m_pOutput;
	QTimer *			m_pResizeMarginTimer;
	QPlainTextEdit *	m_pInput;
	
	QExplicitlySharedDataPointer<IrcServerInfoService>
					m_pSharedService;
	
	QTextCodec *		m_pCodec;

	QList< QString >	m_pastCommands;
	
	// custom scroll bar for searching within an IRC window;
	// lines on which items are found will be draw inside
	// the slider area (proportional to the size of the slider area)
	IrcWindowScrollBar *	m_pScrollBar;
	
public:
	IIrcWindow(const QString &title = tr("Untitled"),
			const QSize &size = QSize(500, 300));
	
	// misc functions
	virtual void GiveFocus();
	
	// window type functions
	virtual int GetIrcWindowType() = 0;
	
	// printing functions
	void PrintOutput(const QString &text, const QColor &color = QColor(0, 0, 0));
	void PrintError(const QString &text);
	void PrintDebug(const QString &text);

protected:
	// gets the text from the input control
	QString GetInputText();
	
	// imitates Google Chrome's search, with lines drawn in the scrollbar
	// and keywords highlighted in the document
	void Search(const QString &textToFind);
	
	// changes the codec for the m_pOutput control
	void ChangeCodec(const QString &codecStr);
	
	// handles child widget events
	bool eventFilter(QObject *obj, QEvent *event);
	
	// handles the input for the window
	void HandleInput(const QString &inputText);
	
	virtual void HandleTab() = 0;

public slots:
	// changes the vertical offset that ensures that the
	// text always starts at the bottom of the screen
	// (for the user)
        //void ResizeTopMargin();
};

//-----------------------------------//
