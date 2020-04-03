#ifndef DRAW_SCENE_H__
#define DRAW_SCENE_H__

#include "Common.h"

void InitializeLights(FbxScene* pScene);
void DrawNodeRecursive(FbxNode* pNode, FbxAMatrix& pParentGlobalPosition);

#endif // DRAW_SCENE_H__