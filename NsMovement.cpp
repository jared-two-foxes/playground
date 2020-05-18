
#include "NsMovement.h"
#include "World.h"

struct ActorInMotion {
    WorldActor* Actor;
    FbxVector4  Target;
    double      MaxSpeed;
};


std::vector<ActorInMotion> ActorsInMotion;

ActorInMotion* MovementSystem_Find(WorldActor* aActor)
{
    for (auto& a : ActorsInMotion) {
        if (a.Actor == aActor) {
            return &a;
        }
    }
    return nullptr;
}
    
void MovementSystem_Add(WorldActor* aActor, const FbxVector4& aTarget, double aMaxSpeed)
{
    ActorInMotion* lThis = MovementSystem_Find(aActor);
    if (!lThis) {
        ActorsInMotion.push_back(ActorInMotion());
        lThis = &ActorsInMotion.back();
        lThis->Actor = aActor;
    }

    lThis->Target   = aTarget;
    lThis->MaxSpeed = aMaxSpeed;
}


void MovementSystem_Update()
{
    //@todo [jared.watt 23/04/2020]
    //  Handle different motion types; ease-in-out?
    //  Rotate object to face heading;  Snap to face, vs turn

    
    for (auto& a : ActorsInMotion) {
        //@Todo: [jared.watt, 23/04/2020]
        //  Handle ease-in & ease-out?
        FbxVector4& rCurrent = a.Actor->WorldPosition.Position;
        
        // Only move if we haven't already arrived.
        if (FbxEqual(rCurrent, a.Target, 2.0)) {
            //@todo: [jared.watt 23/04/2020]
            //  Pop this record from the list?
        }
        else {
            // Simple move to.
            FbxVector4  lDir = a.Target - rCurrent;
            lDir.Normalize();
            rCurrent += lDir * a.MaxSpeed;
            rCurrent[3] = 1.0f;
        }
    }
}