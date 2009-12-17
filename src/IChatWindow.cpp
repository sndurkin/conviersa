#include "IChatWindow.h"

IChatWindow::IChatWindow(const QString &title/* = tr("Untitled")*/,
			const QSize &size/* = QSize(500, 300)*/)
	: IWindow(title, size)
{ }
