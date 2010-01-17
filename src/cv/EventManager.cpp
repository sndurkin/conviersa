/************************************************************************
*
* The MIT License
*
* Copyright (c) 2007-2009 Conviersa Project
*
************************************************************************/

#include "cv/EventManager.h"

bool EventManager::CreateEvent(const QString &name)
{
    if(hash.find(name) == hash.end())
    {
        hash.insert(name, QList<EventCallback>());
        return true;
    }

    return false;
}

bool EventManager::CreateEvent(const QString &name, EventCallback defaultCallback)
{
    if(hash.find(name) == hash.end())
    {
        QList<EventCallback> list;
        list.append(defaultCallback);
        hash.insert(name, list);
        return true;
    }

    return false;
}

bool EventManager::HookEvent(const QString &name, EventCallback callback)
{
    QHash< QString, QList<EventCallback> >::iterator iter = hash.find(name);
    if(iter == hash.end())
        return false;

    // new callbacks get prepended, because it acts kinda like a stack;
    // we call the callbacks from the first to the last in the QList
    (*iter).prepend(callback);
    return true;
}

bool EventManager::UnhookEvent(const QString &name, EventCallback callback)
{
    QHash< QString, QList<EventCallback> >::iterator iter = hash.find(name);
    if(iter == hash.end())
        return false;

    int callbackIdx = (*iter).indexOf(callback);
    if(callbackIdx < 0)
        return false;

    (*iter).removeAt(callbackIdx);
    return true;
}

bool EventManager::FireEvent(const QString &name, Event *evt)
{
    QHash< QString, QList<EventCallback> >::iterator iter = hash.find(name);
    if(iter == hash.end())
        return false;

    QList<EventCallback>::iterator callbackIter = (*iter).begin();
    while(callbackIter != (*iter).end())
    {
        (*callbackIter)(evt);
        ++callbackIter;
    }

    return true;
}
