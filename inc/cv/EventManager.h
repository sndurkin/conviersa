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

namespace cv {

// Event interface
class Event
{
public:
    Event() { }
    virtual ~Event() { }
};

enum CallbackReturnType
{
    EVENT_CONTINUE,
    EVENT_HANDLED
};

enum HookType
{
    PRE_HOOK,
    POST_HOOK
};

typedef FastDelegate1<Event *> EventCallback;
class EventManager;

struct EventInfo
{
    EventManager *       pChainedEvtMgr;
    QList<EventCallback> callbacks;
};

// TODO: make thread-safe, with maybe some sort of reference counting
class EventManager
{
private:
    QHash<QString, EventInfo> hash;

public:
    bool createEvent(const QString &name, EventManager *pEvtMgr = NULL);
    bool hookEvent(const QString &name, EventCallback callback);
    bool unhookEvent(const QString &name, EventCallback callback);
    bool fireEvent(const QString &name, Event *evt);

protected:
    CallbackReturnType execPluginCallbacks(Event *evt, HookType type);
};

extern EventManager *g_pEvtManager;

} // end namespace
