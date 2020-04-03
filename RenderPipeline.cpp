
#include "RenderPipeline.h"

#include <fstream>
#include <vector>

GLuint sProgram = -1;


GLuint 
RenderPipeline::LoadShader(const char* filename, GLenum type) 
{
    GLint Result = GL_FALSE;
    int InfoLogLength;

    std::ifstream is( filename );
    if (!is) {
        printf("Unable to open shader file, %s", filename );
        return -1;
    }

    // Get length of file
    is.seekg(0, is.end);
    int length = is.tellg();
    is.seekg(0, is.beg);

    // read data as a block
    char* buffer = new char[length+1];
    is.read(buffer,length);
    buffer[length] = '\0';

    GLuint shaderId = glCreateShader(type);
    glShaderSource(shaderId, 1, &buffer, NULL);
    glCompileShader(shaderId);

    // Check shader status
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> ShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(shaderId, InfoLogLength, NULL, &ShaderErrorMessage[0]);
    fprintf(stdout, "%s\n", &ShaderErrorMessage[0]);

    delete[] buffer;
    return shaderId;
}

GLuint 
RenderPipeline::CreateProgram(GLuint vertex_shader, GLuint fragment_shader) 
{
    GLint Result = GL_FALSE;
    int InfoLogLength;

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    // Check the program
    glGetProgramiv(program, GL_LINK_STATUS, &Result);
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> ProgramErrorMessage( InfoLogLength );
    glGetProgramInfoLog(program, InfoLogLength, NULL, &ProgramErrorMessage[0]);
    fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

    return program;
}

void 
RenderPipeline::UseProgram(GLuint program) 
{
    if (sProgram != program) 
    {
        glUseProgram(program);
        sProgram = program;
    }
}

GLint 
RenderPipeline::GetUniformLocation(const char* name) 
{
    return glGetUniformLocation(sProgram, name);
}

GLint 
RenderPipeline::GetAttributeLocation(const char* name)
{
    return glGetAttribLocation(sProgram, name);
}

void 
RenderPipeline::BindLight(const LightCache* pLightCache, const FbxAMatrix& GlobalPosition)
{
    GLuint light_position_l = GetUniformLocation("light.position");

    const FbxVector4 position = GlobalPosition.GetT();
    GLfloat c[4];
    for( int i = 0; i<4; ++i ) 
    {
        c[i] = (GLfloat)position[i];
    }
    glUniform4fv(light_position_l, 1, c);

    // const GLfloat lLightColor[4] = {pLightCache->mColorRed, pLightCache->mColorGreen, pLightCache->mColorBlue, 1.0f};
    // const GLfloat lConeAngle = pLightCache->mConeAngle;

    // glColor3fv(lLightColor);

    // // The transform have been set, so set in local coordinate.
    // if (pLightCache->mType == FbxLight::eDirectional)
    // {
    //     glLightfv(pLightCache->mLightIndex, GL_POSITION, DEFAULT_DIRECTION_LIGHT_POSITION);
    // }
    // else
    // {
    //     glLightfv(pLightCache->mLightIndex, GL_POSITION, DEFAULT_LIGHT_POSITION);
    // }

    // glLightfv(pLightCache->mLightIndex, GL_DIFFUSE, lLightColor);
    // glLightfv(pLightCache->mLightIndex, GL_SPECULAR, lLightColor);
    
    // if (pLightCache->Type == FbxLight::eSpot && lConeAngle != 0.0)
    // {
    //     glLightfv(mLightIndex, GL_SPOT_DIRECTION, DEFAULT_SPOT_LIGHT_DIRECTION);

    //     // If the cone angle is 0, equal to a point light.
    //     if (lConeAngle != 0.0f)
    //     {
    //         // OpenGL use cut off angle, which is half of the cone angle.
    //         glLightf(mLightIndex, GL_SPOT_CUTOFF, lConeAngle/2);
    //     }
    // }
    // glEnable(mLightIndex);
}