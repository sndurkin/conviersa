/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#include "cv/EventManager.h"

namespace cv {

bool EventManager::createEvent(const QString &name, EventManager *pEvtMgr/* = NULL*/)
{
    if(hash.find(name) == hash.end())
    {
        EventInfo info;
        info.pChainedEvtMgr = pEvtMgr;
        hash.insert(name, info);
        return true;
    }

    return false;
}

bool EventManager::hookEvent(const QString &name, EventCallback callback)
{
    QHash<QString, EventInfo>::iterator iter = hash.find(name);
    if(iter == hash.end())
        return false;

    // new callbacks get prepended, because it acts kinda like a stack;
    // we call the most recently attached callbacks first
    (*iter).callbacks.prepend(callback);
    return true;
}

bool EventManager::unhookEvent(const QString &name, EventCallback callback)
{
    QHash<QString, EventInfo>::iterator iter = hash.find(name);
    if(iter == hash.end())
        return false;

    int callbackIdx = (*iter).callbacks.indexOf(callback);
    if(callbackIdx < 0)
        return false;

    (*iter).callbacks.removeAt(callbackIdx);
    return true;
}

bool EventManager::fireEvent(const QString &name, Event *evt)
{
    CallbackReturnType returnType = EVENT_CONTINUE;
    QHash<QString, EventInfo>::iterator iter = hash.find(name);
    if(iter == hash.end())
        return false;

    // call the chained event manager's pre-hook callbacks, if it exists
    if((*iter).pChainedEvtMgr != NULL)
    {
        returnType = (*iter).pChainedEvtMgr->execPluginCallbacks(evt, PRE_HOOK);
    }

    if(returnType == EVENT_CONTINUE)
        returnType = execPluginCallbacks(evt, PRE_HOOK);
    else
        execPluginCallbacks(evt, PRE_HOOK);

    if(returnType == EVENT_CONTINUE)
    {
        // default functionality callbacks
        QList<EventCallback>::iterator callbackIter = (*iter).callbacks.begin();
        while(callbackIter != (*iter).callbacks.end())
        {
            (*callbackIter)(evt);
            ++callbackIter;
        }
    }

    execPluginCallbacks(evt, POST_HOOK);

    // call the chained event manager's post-hook callbacks, if it exists
    if((*iter).pChainedEvtMgr != NULL)
    {
        (*iter).pChainedEvtMgr->execPluginCallbacks(evt, POST_HOOK);
    }

    return true;
}

CallbackReturnType EventManager::execPluginCallbacks(Event *evt, HookType type)
{
    // TODO: fill in
    return EVENT_CONTINUE;
}

} // end namespace
