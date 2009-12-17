#include "qext.h"

QString escapeEx(const QString &text)
{
	QString textToReturn = Qt::escape(text);
	textToReturn.prepend("<span style=\"white-space:pre-wrap\">");
	textToReturn.append("</span>");
	return textToReturn;
}
