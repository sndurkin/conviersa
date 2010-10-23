/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#include <QTime>
#include "cv/EventManager.h"

namespace cv {

bool EventManager::createEvent(const QString &name, EventManager *pEvtMgr/* = NULL*/)
{
    if(m_hash.find(name) == m_hash.end())
    {
        EventInfo evtInfo;
        evtInfo.pChainedEvtMgr = pEvtMgr;
        evtInfo.firingEvent = false;
        m_hash.insert(name, evtInfo);
        return true;
    }

    return false;
}

bool EventManager::hookEvent(const QString &name, EventCallback callback)
{
    QHash<QString, EventInfo>::iterator iter = m_hash.find(name);
    if(iter == m_hash.end())
        return false;
    EventInfo &evtInfo = *iter;

    // if we're currently firing the event, then add the callback
    // to a temp list which will get prepended after the entire list
    // of callbacks has been iterated
    if(evtInfo.firingEvent)
        evtInfo.callbacksToAdd.append(callback);
    else
        // new callbacks get prepended, because it acts kinda like a stack;
        // we call the most recently attached callbacks first
        evtInfo.callbacks.prepend(callback);
    return true;
}

bool EventManager::unhookEvent(const QString &name, EventCallback callback)
{
    QHash<QString, EventInfo>::iterator iter = m_hash.find(name);
    if(iter == m_hash.end())
        return false;

    EventInfo &evtInfo = *iter;

    // if we're currently firing the event, then add the callback
    // to a temp list which will get removed after the event is
    // done being fired
    if(evtInfo.firingEvent)
    {
        evtInfo.callbacksToRemove.append(callback);
    }
    else
    {
        int callbackIdx = evtInfo.callbacks.indexOf(callback);
        if(callbackIdx < 0)
            return false;
        evtInfo.callbacks.removeAt(callbackIdx);
    }

    return true;
}

void EventManager::fireEvent(const QString &name, Event *evt)
{
    CallbackReturnType returnType = EVENT_CONTINUE;
    QHash<QString, EventInfo>::iterator iter = m_hash.find(name);
    if(iter == m_hash.end())
        return;
    EventInfo &evtInfo = *iter;
    evtInfo.firingEvent = true;

    // call the chained event manager's pre-hook callbacks, if it exists
    if(evtInfo.pChainedEvtMgr != NULL)
    {
        returnType = evtInfo.pChainedEvtMgr->execPluginCallbacks(evt, PRE_HOOK);
    }

    if(returnType == EVENT_CONTINUE)
        returnType = execPluginCallbacks(evt, PRE_HOOK);
    else
        execPluginCallbacks(evt, PRE_HOOK);

    if(returnType == EVENT_CONTINUE)
    {
        // default functionality callbacks
        QList<EventCallback>::iterator callbackIter = evtInfo.callbacks.begin();
        while(callbackIter != evtInfo.callbacks.end())
        {
            (*callbackIter)(evt);
            ++callbackIter;
        }
    }

    execPluginCallbacks(evt, POST_HOOK);

    // call the chained event manager's post-hook callbacks, if it exists
    if(evtInfo.pChainedEvtMgr != NULL)
    {
        evtInfo.pChainedEvtMgr->execPluginCallbacks(evt, POST_HOOK);
    }

    // prepend all the callbacks that were added with hookEvent() calls
    // while it was firing
    evtInfo.firingEvent = false;
    while(!evtInfo.callbacksToAdd.isEmpty())
    {
        evtInfo.callbacks.prepend(evtInfo.callbacksToAdd.takeFirst());
    }

    // remove all the callbacks that were added with unhookEvent() calls
    // while it was firing
    while(!evtInfo.callbacksToRemove.isEmpty())
    {
        EventCallback callback = evtInfo.callbacksToRemove.takeFirst();
        int callbackIdx = evtInfo.callbacks.indexOf(callback);
        if(callbackIdx < 0)
            continue;
        evtInfo.callbacks.removeAt(callbackIdx);
    }

    return;
}

CallbackReturnType EventManager::execPluginCallbacks(Event *evt, HookType type)
{
    // TODO: fill in
    return EVENT_CONTINUE;
}

} // end namespace
