#ifndef DISPATCH_QUEUE_H
#define DISPATCH_QUEUE_H

//@note: [jared.watt 23/04/2020]
//  This essentially is building a thread queue.  Investigate 
//  building this such that it internally processes an "action_queue"
//  and then the dispatch queue can be more specific and encapsulate
//  the main app logic.


//@note: [jared.watt 01/05/2020]
//  We need to be able to determine how many actions we have for
//  the processing queue.
int DispatchQueue_GetActionCount();

//@todo: [jared.watt 23/04/2020]
//  Hide some of this implementation detail, #include types then
//  create _AddAction(NsMoveAction& ) etc style functions.  Type
//  safety?
void DispatchQueue_AddAction(int action_id);
void DispatchQueue_AddAction(int action_id, void* action_context, std::size_t context_size);

//@note: [jared.watt 01/05/2020]
//  Action context will be valid until pop action is called.  Returns a pointer to the 
//  internal copy held by the queue.
int  DispatchQueue_GetNextAction(void** action_context, std::size_t* context_size);
void DispatchQueue_PopAction();

#endif // DISPATCH_QUEUE_H