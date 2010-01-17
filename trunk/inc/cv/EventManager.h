/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#pragma once

#include <QHash>
#include <QList>
#include "cv/FastDelegate.h"

using namespace fastdelegate;

// just so it can be inherited
class Event
{
public:
    Event() { }
    virtual ~Event() { }
};

enum CallbackReturnType
{
    CONTINUE,
    HANDLED
};

typedef FastDelegate1<Event *> EventCallback;

class EventManager
{
private:
    QHash< QString, QList<EventCallback> > hash;

public:
    bool CreateEvent(const QString &name);
    bool CreateEvent(const QString &name, EventCallback defaultCallback);
    bool HookEvent(const QString &name, EventCallback callback);
    bool UnhookEvent(const QString &name, EventCallback callback);
    bool FireEvent(const QString &name, Event *evt);
};
