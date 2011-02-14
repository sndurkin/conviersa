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

// if [var] is NULL, then it prints a message using qDebug() and
// returns from the current scope
#define EM_DEBUG_CHECK(var, func, evtName, evtType, reason) \
    if(var == NULL) \
    { \
        qDebug("[EM::%sEvent] Attempted to %s %s event \"%s\" which %s", func, func, evtType, evtName, reason); \
        return; \
    }

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

enum EventType
{
    INSTANCE_EVENT,
    STRING_EVENT
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

    EventInfo()
      : firingEvent(false)
    { }
};

//-----------------------------------//

// This class is used to achieve a system for one class
// to notify unrelated classes of events and provide information
// about them; it helps minimize decoupling across the project.
//
// The implementation is fairly straightforward:
//   EventManager
//    |
//    --> events (uniquely identified by name)
//         |
//         --> pointers to instances
//         |    |
//         |    --> callbacks
//         |
//         --> strings
//              |
//              --> callbacks
//
// A simpler implementation would just involve events (identified by name)
// and their callbacks. There is a second level (event types) for more functionality:
//  1) pointers to instances are used because some events only make sense in
//     the context of an instance; for example, a Session instance can fire
//     a "connected" event, which indicates the client is connected to a specific
//     server
//  2) strings are currently only used for the "configChanged" event, which is fired
//     whenever any config option is changed; the string holds the config option
//     that was modified
//
// More information:
// http://code.google.com/p/conviersa/wiki/Event_System
//
//
// TODO
//  - make thread-safe, with maybe some sort of reference counting
class EventManager
{
    // separate hashes are used for each event type; global events
    // will be
    QHash<QString, QHash<uintptr_t, EventInfo> >    m_instanceEvents;
    QHash<QString, QHash<QString, EventInfo> >      m_stringEvents;

public:
    ~EventManager();

    void createEvent(const QString &evtName, EventType type = INSTANCE_EVENT);

    void hookGlobalEvent(const QString &evtName, EventType type, EventCallback callback);
    void hookEvent(const QString &evtName, void *pEvtInstance, EventCallback callback);
    void hookEvent(const QString &evtName, const QString &evtString, EventCallback callback);

    void fireEvent(const QString &evtName, void *pEvtInstance, Event *pEvent);
    void fireEvent(const QString &evtName, const QString &evtString, Event *pEvent);

    void unhookGlobalEvent(const QString &evtName, EventType type, EventCallback callback);
    void unhookEvent(const QString &evtName, void *pEvtInstance, EventCallback callback);
    void unhookEvent(const QString &evtName, const QString &evtString, EventCallback callback);

    void unhookAllEvents(void *pEvtInstance);
    void unhookAllEvents(const QString &evtString);

protected:
    template<class T>
    void hookEvent(QHash<T, EventInfo> *pEventsHash, const T &evtTypeId, EventCallback callback)
    {
        EventInfo *pEvtInfo = getEventInfo(pEventsHash, evtTypeId);
        if(pEvtInfo == NULL)
        {
            EventInfo evtInfo;
            pEventsHash->insert(evtTypeId, evtInfo);
            pEvtInfo = getEventInfo(pEventsHash, evtTypeId);
        }

        // if we're currently firing the event, then add the callback
        // to a temp list which will get added after the entire list
        // of callbacks has been iterated
        if(pEvtInfo->firingEvent)
            pEvtInfo->callbacksToAdd.append(callback);
        else
            // new callbacks get prepended, because it acts kinda like a stack;
            // we call the most recently attached callbacks first
            pEvtInfo->callbacks.prepend(callback);
    }

    template<class T>
    void fireEvent(QHash<T, EventInfo> *pEventsHash, const T &evtTypeId, Event *pEvent)
    {
        EventInfo *pEvtInfo = getEventInfo(pEventsHash, evtTypeId);
        if(pEvtInfo == NULL)
            // there are currently no callbacks hooked into this event, so exit normally
            return;

        // actually fire the event
        pEvtInfo->firingEvent = true;

        CallbackReturnType returnType = execPluginCallbacks(pEvent, PRE_HOOK);

        if(returnType == EVENT_CONTINUE)
        {
            // default functionality callbacks
            QList<EventCallback>::iterator callbackIter = pEvtInfo->callbacks.begin();
            while(callbackIter != pEvtInfo->callbacks.end())
            {
                (*callbackIter)(pEvent);
                ++callbackIter;
            }
        }

        execPluginCallbacks(pEvent, POST_HOOK);

        // prepend all the callbacks that were added with hookEvent() calls
        // while it was firing
        pEvtInfo->firingEvent = false;
        while(!pEvtInfo->callbacksToAdd.isEmpty())
        {
            pEvtInfo->callbacks.prepend(pEvtInfo->callbacksToAdd.takeFirst());
        }

        // remove all the callbacks that were added with unhookEvent() calls
        // while it was firing
        while(!pEvtInfo->callbacksToRemove.isEmpty())
        {
            EventCallback callback = pEvtInfo->callbacksToRemove.takeFirst();
            int callbackIdx = pEvtInfo->callbacks.indexOf(callback);
            if(callbackIdx >= 0)
                pEvtInfo->callbacks.removeAt(callbackIdx);
        }

        // if it's empty, then we can go ahead and remove the type id's entry
        if(pEvtInfo->callbacks.isEmpty())
            pEventsHash->remove(evtTypeId);
    }

    template<class T>
    void unhookEvent(const QString &evtName, QHash<T, EventInfo> *pEventsHash, const T &evtTypeId, EventCallback callback)
    {
        EventInfo *pEvtInfo = getEventInfo(pEventsHash, evtTypeId);
        EM_DEBUG_CHECK(pEvtInfo, "unhook", evtName.toLatin1().constData(), "an", "wasn't being hooked")

        // if we're currently firing the event, then add the callback
        // to a temp list which will get removed after the event is
        // done being fired
        if(pEvtInfo->firingEvent)
        {
            pEvtInfo->callbacksToRemove.append(callback);
        }
        else
        {
            int callbackIdx = pEvtInfo->callbacks.indexOf(callback);
            if(callbackIdx >= 0)
                pEvtInfo->callbacks.removeAt(callbackIdx);
        }

        // if it's empty, then we can go ahead and remove the instance's entry
        if(pEvtInfo->callbacks.isEmpty())
            pEventsHash->remove(evtTypeId);
    }

    template<class T>
    EventInfo *getEventInfo(QHash<T, EventInfo> *pEventsHash, const T &evtTypeId)
    {
        QHash<T, EventInfo>::iterator eventsIter = pEventsHash->find(evtTypeId);
        if(eventsIter == pEventsHash->end())
            return NULL;

        return &(*eventsIter);
    }

    QHash<uintptr_t, EventInfo> *getInstancesHash(const QString &evtName);
    QHash<QString, EventInfo> *getStringsHash(const QString &evtName);

    // TODO: implement with plugin API
    CallbackReturnType execPluginCallbacks(Event *pEvent, HookType type);
};

extern EventManager *g_pEvtManager;

} // end namespace
