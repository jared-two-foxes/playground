#ifndef RENDERPIPELINE_H__
#define RENDERPIPELINE_H__

#include "Common.h"

struct LightCache;

namespace RenderPipeline 
{
    GLuint LoadShader(const char* filename, GLenum type);

    GLuint CreateProgram(GLuint vertex_shader, GLuint fragment_shader);
    void UseProgram(GLuint program);
    GLint GetUniformLocation(const char* name);
    GLint GetAttributeLocation(const char* name);

    void BindLight(const LightCache* pLightCache, const FbxAMatrix& GlobalPosition);
}

#endif // RENDERPIPELINE_H__