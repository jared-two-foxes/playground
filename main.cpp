
#include "Common.h"
#include "Camera.h"
#include "DrawScene.h"
#include "Fbx.h"
#include "RenderPipeline.h"

#include <stdlib.h>
#include <stdio.h>


void 
error_callback(
    int error, 
    const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void 
key_callback(
    GLFWwindow* window, 
    int key, 
    int scancode, 
    int action, 
    int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) 
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

void GLAPIENTRY
MessageCallback( GLenum source,
                 GLenum type,
                 GLuint id,
                 GLenum severity,
                 GLsizei length,
                 const GLchar* message,
                 const void* userParam )
{
  fprintf( stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
           ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
            type, severity, message );
}

int
main(
    int argc, 
    char* argv[]) 
{
// 
// Setup window and render context.
//
    GLFWwindow* window;
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
    {
        // Initialization failed
        glfwTerminate();
        return -1;
    }

    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 2 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 0 );

    window = glfwCreateWindow(640, 480, "Simple Example", NULL, NULL);
    if (!window)
    {
        // Window or context creation failed
        glfwTerminate();
        return -1;
    }

    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);
    glEnable( GL_DEBUG_OUTPUT );
    glDebugMessageCallback( MessageCallback, 0 );
    glfwSwapInterval(1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glClearColor(0.f, 0.f, 0.f, 0.f);

//
// Setup Fbx Subsystem.
//
    InitFbx();

    FbxScene* lScene = CreateSceneObject("My Scene");
    ImportScene( "Male_LOWPOLYGON.fbx", lScene );

    FbxArray<FbxNode*> lCameraArray;
    FillCameraArray(lScene, lCameraArray);
    FbxAMatrix lDummyGlobalPosition;

//
// Setup the RenderPipeline Objects.
//
    GLuint vertex_shader = RenderPipeline::LoadShader( 
        "shaders/specular.vert", GL_VERTEX_SHADER );
    GLuint fragment_shader = RenderPipeline::LoadShader( 
        "shaders/specular.frag", GL_FRAGMENT_SHADER );

    GLuint program = RenderPipeline::CreateProgram( 
        vertex_shader, fragment_shader );
    RenderPipeline::UseProgram(program);

    while (!glfwWindowShouldClose(window))
    {   
        float ratio;
        int lWindowWidth, lWindowHeight;
        glfwGetFramebufferSize(window, &lWindowWidth, &lWindowHeight);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);
        // Draw the front face only, except for the texts and lights.
        glEnable(GL_CULL_FACE);

        // Set the view to the current camera settings.
        Camera::SetCamera(lScene, lCameraArray, lWindowWidth, lWindowHeight);

        // Iterate the scene.
        InitializeLights( lScene );
        DrawNodeRecursive( lScene->GetRootNode(), lDummyGlobalPosition);
        //DisplayGrid(lDummyGlobalPosition);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwDestroyWindow(window);

    glfwTerminate();
    return 1;
}
