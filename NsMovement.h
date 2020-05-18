#ifndef MOVEMENT_H
#define MOVEMENT_H

#include "Common.h"

struct WorldActor;

void MovementSystem_Add(WorldActor* Actor, const FbxVector4& Target, double MaxSpeed);
void MovementSystem_Update();

#endif // MOVEMENT_H 