/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#include "cv/EventManager.h"

namespace cv {

EventManager::~EventManager()
{
    QHash<QString, QHash<uintptr_t, EventInfo> >::iterator iter = m_eventsHash.begin();
    QString eventStr;
    while(iter != m_eventsHash.end())
    {
        if(!iter.value().isEmpty())
            eventStr += iter.key() + ", ";
        ++iter;
    }

    if(!eventStr.isEmpty())
    {
        eventStr.remove(eventStr.length() - 2, 2);
        qDebug("Events still hooked: %s", eventStr.toLatin1().constData());
    }
}

void EventManager::createEvent(const QString &evtName)
{
    if(m_eventsHash.find(evtName) == m_eventsHash.end())
    {
        QHash<uintptr_t, EventInfo> instancesHash;
        m_eventsHash.insert(evtName, instancesHash);
    }
}

void EventManager::hookEvent(const QString &evtName, void *pEvtInstance, EventCallback callback)
{
    QHash<uintptr_t, EventInfo> *pInstancesHash = getInstancesHash(evtName);
    if(pInstancesHash == NULL)
        return;
    EventInfo *pEvtInfo = getEventInfo(pInstancesHash, pEvtInstance);
    if(pEvtInfo == NULL)
    {
        EventInfo evtInfo;
        evtInfo.firingEvent = false;
        pInstancesHash->insert((uintptr_t) pEvtInstance, evtInfo);
        pEvtInfo = getEventInfo(pInstancesHash, pEvtInstance);
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

void EventManager::unhookEvent(const QString &evtName, void *pEvtInstance, EventCallback callback)
{
    EventInfo *pEvtInfo = getEventInfo(evtName, pEvtInstance);
    if(pEvtInfo == NULL)
        return;

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

        // if it's empty, then we can go ahead and remove the instance's entry
        if(pEvtInfo->callbacks.isEmpty())
        {
            QHash<uintptr_t, EventInfo> *pInstancesHash = getInstancesHash(evtName);
            pInstancesHash->remove((uintptr_t) pEvtInstance);
        }
    }
}

void EventManager::unhookAllEvents(void *pEvtInstance)
{
    QHash<QString, QHash<uintptr_t, EventInfo> >::iterator iter = m_eventsHash.begin();
    while(iter != m_eventsHash.end())
    {
        (*iter).remove((uintptr_t) pEvtInstance);
        ++iter;
    }
}

void EventManager::fireEvent(const QString &evtName, void *pEvtInstance, Event *pEvent)
{
    EventInfo *pEvtInfo = getEventInfo(evtName, pEvtInstance);
    if(pEvtInfo == NULL)
    {
        qDebug("Attempted to fire unrecognized event: %s\n", evtName);
        return;
    }

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

    // if it's empty, then we can go ahead and remove the instance's entry
    if(pEvtInfo->callbacks.isEmpty())
    {
        QHash<uintptr_t, EventInfo> *pInstancesHash = getInstancesHash(evtName);
        pInstancesHash->remove((uintptr_t) pEvtInstance);
    }
}

CallbackReturnType EventManager::execPluginCallbacks(Event *pEvent, HookType type)
{
    // TODO: fill in
    return EVENT_CONTINUE;
}

EventInfo *EventManager::getEventInfo(const QString &evtName, void *pEvtInstance)
{
    QHash<uintptr_t, EventInfo> *pInstancesHash = getInstancesHash(evtName);
    QHash<uintptr_t, EventInfo>::iterator instancesIter = pInstancesHash->find((uintptr_t) pEvtInstance);
    if(instancesIter == pInstancesHash->end())
    {
        qDebug("Instance for event %s not found", evtName);
        return NULL;
    }

    return &(*instancesIter);
}

EventInfo *EventManager::getEventInfo(QHash<uintptr_t, EventInfo> *pInstancesHash, void *pEvtInstance)
{
    QHash<uintptr_t, EventInfo>::iterator instancesIter = pInstancesHash->find((uintptr_t) pEvtInstance);
    if(instancesIter == pInstancesHash->end())
        return NULL;

    return &(*instancesIter);
}

QHash<uintptr_t, EventInfo> *EventManager::getInstancesHash(const QString &evtName)
{
    QHash<QString, QHash<uintptr_t, EventInfo> >::iterator eventsIter = m_eventsHash.find(evtName);
    if(eventsIter == m_eventsHash.end())
    {
        qDebug("Event %s has not been created", evtName);
        return NULL;
    }

    return &(*eventsIter);
}

} // end namespace
