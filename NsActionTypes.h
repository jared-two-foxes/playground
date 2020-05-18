#ifndef ACTIONTYPES_H
#define ACTIONTYPES_H

#include "Common.h"

//@note [jared.watt 01/05/2020]
// I'm implementing a queue like system for actions, the queue
// itself doesnt need to know anything about the message, the 
// context data or what it does but we do so we'll be passing
// these values which are to represent each action.
enum class ActionTypes {
    Unknown = 0, 

    // Actor Actions
    MoveActor
};

//@todo [jared.watt 23/04/2020]
//  Rather than pass pointers for actors and other objects around
//  another identifier is required.  Prefer handles?
struct WorldActor;

struct NsMoveActionContext {
    WorldActor* Actor;
    FbxVector4  Target;
    double      MaxSpeed;
};

#endif // ACTIONTYPES_H