

cxx_library(
    name = 'nativeLibs',
    srcs = [ 'native.cpp' ],
    exported_linker_flags=[
        '-ldl',
        '-lpthread',
        '-lz',
        '-lxml2'
    ],
)

prebuilt_cxx_library(
    name = 'glfw',
    static_lib = 'glfw/lib/libglfw3.a',
    exported_headers = subdir_glob([
        ('glfw/include', '**/*.hpp'),
        ('glfw/include', '**/*.h')
    ]),
    exported_deps=[
        ':nativeLibs'
    ]
)

prebuilt_cxx_library(
    name = 'fbx_sdk',
    static_lib = 'fbx_sdk/lib/gcc/x64/debug/libfbxsdk.a',
    exported_headers = subdir_glob([
        ('fbx_sdk/include', '**/*.hpp'),
        ('fbx_sdk/include', '**/*.h')
    ]),
    exported_deps=[
        ':nativeLibs'
    ]
)

cxx_library(
    name = 'glad',
    srcs = [ 'glad/glad_gl.c', 'glad/glad_vulkan.c' ],
    exported_headers = subdir_glob([
        ('glad', '**/*.h')
    ]),
    exported_deps=[
        ':glfw'
    ],
    header_namespace = 'glad'
)

cxx_binary(
    name = 'demo',
    srcs = [
        'main.cpp',
        'Camera.cpp',
        'DrawScene.cpp',
        'Fbx.cpp',
        'RenderPipeline.cpp',
        'targa.cpp',
    ],
    deps = [
        ':glad',
        ':fbx_sdk'
    ]
)