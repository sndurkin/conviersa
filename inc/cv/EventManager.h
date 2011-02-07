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

#define DCAST(eventType, var) dynamic_cast<eventType *>(var)

using namespace fastdelegate;

namespace cv {

// Event interface
class Event
{
public:
    Event() { }
    virtual ~Event() { }
};

//-----------------------------------//

enum CallbackReturnType
{
    EVENT_CONTINUE,
    EVENT_HANDLED
};

//-----------------------------------//

enum HookType
{
    PRE_HOOK,
    POST_HOOK
};

//-----------------------------------//

typedef FastDelegate1<Event *> EventCallback;
class EventManager;

struct EventInfo
{
    QList<EventCallback>    callbacks;
    bool                    firingEvent;
    QList<EventCallback>    callbacksToAdd;
    QList<EventCallback>    callbacksToRemove;
};

//-----------------------------------//

// This class is used to achieve a system for one class
// to notify unrelated classes of events and provide information
// about them; it helps minimize decoupling across the project.
//
// The implementation is fairly straightforward:
//  - there is one globally shared EventManager object
//  - each event is uniquely identified by name
//  - each event has an arbitrary number of instances (of any type)
//  - each instance has an arbitrary number of callbacks (functions
//    that get called when the event is fired)
//
// More information:
// http://code.google.com/p/conviersa/wiki/Event_System
//
// TODO
//  - make thread-safe, with maybe some sort of reference counting
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
