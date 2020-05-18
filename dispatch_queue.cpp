#include "Common.h"
#include "dispatch_queue.h"

#include <queue>

//@todo: [jared.watt 01/05/2020]
//  Should I be using the heap for the Context's?  Purhaps I should
//  allocate a chunk of memory and handle the allocation of this directly?

struct Action 
{
    int ActionIdentifier;
    void* Context;
    std::size_t ContextSize;
};

std::queue<Action > sActionList;

int 
DispatchQueue_GetActionCount() 
{
    return sActionList.size();
}

void 
DispatchQueue_AddAction(int pActionId) 
{
    DispatchQueue_AddAction(pActionId, nullptr, 0);
}

void 
DispatchQueue_AddAction(int pActionId, void* pActionContext, std::size_t pContextSize) 
{
    sActionList.push(Action());
    Action& action = sActionList.back();
    action.ActionIdentifier = pActionId;
    void* buffer = nullptr;
    if (pContextSize > 0)  {
        buffer = malloc(pContextSize);
        memcpy(buffer, pActionContext, pContextSize);
    }
    action.Context = buffer;
    action.ContextSize = pContextSize;
}

int 
DispatchQueue_GetNextAction(void** action_context, std::size_t* context_size)
{
    Action& action = sActionList.front();
    if (action_context != nullptr) {
        *action_context = action.Context;
    }
    if (context_size != nullptr ) {
        *context_size = action.ContextSize;
    }
    return action.ActionIdentifier;
}

void 
DispatchQueue_PopAction() 
{
    Action& action = sActionList.front();
    if (action.Context) {
        free(action.Context);
        action.Context = nullptr;
    }
    sActionList.pop();
}

