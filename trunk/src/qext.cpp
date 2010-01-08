/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#include "qext.h"

QString escapeEx(const QString &text)
{
    QString textToReturn = Qt::escape(text);
    textToReturn.prepend("<span style=\"white-space:pre-wrap\">");
    textToReturn.append("</span>");
    return textToReturn;
}
