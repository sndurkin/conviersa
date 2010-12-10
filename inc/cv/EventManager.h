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
    QList<EventCallback>    callbacks;
    bool                    firingEvent;
    QList<EventCallback>    callbacksToAdd;
    QList<EventCallback>    callbacksToRemove;
};

// TODO: make thread-safe, with maybe some sort of reference counting
class EventManager
{
    QHash<QString, QHash<uintptr_t, EventInfo> > m_eventsHash;

public:
    ~EventManager();

    void createEvent(const QString &evtName);
    void hookEvent(const QString &evtName, void *pEvtInstance, EventCallback callback);
    void unhookEvent(const QString &evtName, void *pEvtInstance, EventCallback callback);
    void unhookAllEvents(void *pEvtInstance);
    void fireEvent(const QString &evtName, void *pEvtInstance, Event *pEvent);

protected:
    CallbackReturnType execPluginCallbacks(Event *pEvent, HookType type);
    EventInfo *getEventInfo(const QString &evtName, void *pEvtInstance);
    EventInfo *getEventInfo(QHash<uintptr_t, EventInfo> *pInstancesHash, void *pEvtInstance);
    QHash<uintptr_t, EventInfo> *getInstancesHash(const QString &evtName);
};

extern EventManager *g_pEvtManager;

} // end namespace
