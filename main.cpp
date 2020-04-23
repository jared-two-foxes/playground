
#include "Common.h"
#include "Camera.h"
#include "DrawScene.h"
#include "Fbx.h"
#include "Grid.h"
#include "Intersect.h"
#include "Plane.h"
#include "RenderObjects.h"
#include "RenderPipeline.h"
#include "World.h"

 
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

FbxVector2 sMousePos;

static void
cursor_position_callback(
    GLFWwindow* window, 
    double xpos, 
    double ypos) 
{
    sMousePos[0] = xpos;
    sMousePos[1] = ypos;
}

static void 
mouse_button_callback(
    GLFWwindow* window, 
    int button, 
    int action, 
    int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        // Lets grab the screen ray based on click position!
        const NsPlane kGroundPlane = { {0,1,0}, 0 }; // Lets create a plane for the ground.
        NsRay         lRay         = Camera_ScreenToRay(sMousePos);
        FbxVector4    lHitPoint;
            
        const WorldActor* lSelectedObject = World_RayCast(lRay);
        if (lSelectedObject) {
            printf("Object (%s) was clicked\n", lSelectedObject->Name.c_str());
        }
        else if (Intersect(lRay, kGroundPlane, &lHitPoint)) {
            printf("Ground was hit at [%.2f,%.2f]\n", lHitPoint[0], lHitPoint[2]);
        }
        else {
            printf("Nothing was clicked\n");
        }
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

    // glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

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

    const char* filename = "Male_LOWPOLYGON.fbx";
    FbxScene* lScene = CreateSceneObject("My Scene");
    ImportScene( filename, lScene );

    FbxArray<FbxNode*> lCameraArray;
    FillCameraArray(lScene, lCameraArray);
    FbxAMatrix lDummyGlobalPosition;

    // Okay so now we have pulled teh scene out of the file, now what.
    // Lets assume theres only one node that we are interested in here
    // and we want to extract it so that we can draw it at a later
    // date.  Also going to assume that the only thing I care about is
    // the Mesh & Materials.

    // I think what I want to do is create a structure similar in idea
    // to the FbxScene and then we can "import" an entire Fbx scene
    // into our "world".  
    //  * This would decouple our render from the Fbx api
    //  
    // I also like the way that Fbx associates stuff with the nodes, 
    // each node can have a group of "NodeAttributes" like lights and
    // meshes associated with it keeping these things independant of 
    // the actual nodes themselves.  
    //
    // I think theres probably a way to do this without the actual node
    // graph concept which could be good.  Do I even need anything along
    // those lines?  Can I get by with just a list of Mesh type objects?
    //
    // So I think the default technique will be to just "import" 
    // the nodes of the scene into the "world" but I then need to 
    // figure out how to handle association of these objects to 
    // in app logic representations.

    WorldObject* lFileObject = World_ImportFbxScene( filename, lScene );
    World_AddInstance( FbxVector4(50,0,0), lFileObject, "obj1" );
    World_AddInstance( FbxVector4(-50,0,0), lFileObject, "obj2" );

    //@Note: [Jared.Watt, 21/04/2020]
    //  It would be great to be able to get an AABB from the WorldObject, 
    //  something like AABB aabb = GetAxisAlignedBounds(lFileOBject); 
    //  which would then get passed to the world when we add it if we 
    //  are interested in being able to select that object.
    // 
    //  I'm not sure what file that should belong to.  Will add it to world
    //  for now...
    NsAxisAlignedBox aabb = WorldObject_GetBounds(lFileObject);

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

//
// Setup the ground plane.
//
    Grid lGrid = Grid_Create(50, 10.0f);
    MeshCache lGridCache = InitializeMeshCache(&lGrid);
    //@todo: Add the ground plane to the world [jared.watt]

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
        //@Todo: this function should really exist in the RenderObject
        //   RenderPipeline space rather than in camera.
        Camera_SetViewportAndCamera(lWindowWidth, lWindowHeight);

        // Draw the World
        //InitializeLights( lScene );
        World_Draw();
        DrawMeshCache(&lGridCache, lDummyGlobalPosition); //< Ground plane.

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwDestroyWindow(window);

    glfwTerminate();
    return 1;
}

