load("@rules_cc//cc:defs.bzl", "cc_import", "cc_library", "cc_binary")


cc_import(
    name = "xml2",
    static_library = "fbx_sdk/lib/vs2017/x64/debug/libxml2-md.lib"
)

cc_import(
    name = "zlib",
    static_library = "fbx_sdk/lib/vs2017/x64/debug/zlib-md.lib"
)

cc_import(
    name = 'fbxsdk_import',
    hdrs = glob([
        'fbx_sdk/include/**/*.hpp',
        'fbx_sdk/include/**/*.h'    
    ]),
    interface_library = 'fbx_sdk/lib/vs2017/x64/debug/libfbxsdk.lib',
    shared_library = 'fbx_sdk/lib/vs2017/x64/debug/libfbxsdk.dll',
    visibility = ['//visibility:public'],
)

cc_import(
    name = 'fbxsdk_linux_import',
    hdrs = glob([
        'fbx_sdk/include/**/*.hpp',
        'fbx_sdk/include/**/*.h'    
    ]),
    static_library = 'fbx_sdk/lib/gcc/x64/debug/libfbxsdk.a',
    visibility = ['//visibility:public'],
)
    

cc_library(
    name = 'fbxsdk',
    hdrs = glob([
        'fbx_sdk/include/**/*.hpp',
        'fbx_sdk/include/**/*.h'    
    ]),
    includes = ['fbx_sdk/include'],
    defines = select({
        "@bazel_tools//src/conditions:windows": ["FBXSDK_SHARED"],
        "//conditions:default": ["EMPTY_DEFINE"],
    }),
    linkopts = select({
        "@bazel_tools//src/conditions:windows": [
            "-DEFAULTLIB:kernel32.lib", 
            "-DEFAULTLIB:User32.lib",
            "-DEFAULTLIB:Gdi32.lib",
            "-DEFAULTLIB:shell32.lib",
            "-DEFAULTLIB:winspool.lib",
            "-DEFAULTLIB:ole32.lib",
            "-DEFAULTLIB:uuid.lib",
            "-DEFAULTLIB:comdlg32.lib",
            "-DEFAULTLIB:advapi32.lib"],
        "@bazel_tools//src/conditions:linux_x86_64": [
            "-lxml2","-lz" ]
    }),
    deps = select({
        "@bazel_tools//src/conditions:windows": [':fbxsdk_import',':zlib',':xml2'],
        "@bazel_tools//src/conditions:linux_x86_64": [':fbxsdk_linux_import'],
    }),
    visibility = ['//visibility:public']
)


cc_import(
    name = 'glfw_import',
    hdrs = glob([
        'glfw/include/**/*.hpp',
        'glfw/include/**/*.h'    
    ]),
    static_library = select({
        "@bazel_tools//src/conditions:windows": "glfw/lib/glfw3.lib",
        "@bazel_tools//src/conditions:linux_x86_64": "glfw/lib/libglfw3.a" 
    }),
    visibility = ["//visibility:public"]
)

cc_library(
    name = 'glfw',
    hdrs = glob([
        'glfw/include/**/*.hpp',
        'glfw/include/**/*.h'    
    ]),
    includes = ['glfw/include'],
    deps = ['glfw_import'],
    linkopts = select({
        "@bazel_tools//src/conditions:windows": [
            "-DEFAULTLIB:kernel32.lib", 
            "-DEFAULTLIB:User32.lib",
            "-DEFAULTLIB:Gdi32.lib",
            "-DEFAULTLIB:shell32.lib",
            "-DEFAULTLIB:winspool.lib",
            "-DEFAULTLIB:ole32.lib",
            "-DEFAULTLIB:uuid.lib",
            "-DEFAULTLIB:comdlg32.lib",
            "-DEFAULTLIB:advapi32.lib"],
        "@bazel_tools//src/conditions:linux_x86_64": [
            '-ldl','-lpthread']
    }),
    visibility = ["//visibility:public"]
)


cc_library(
    name = 'glad',
    srcs = glob(['glad/src/**/*.c']),
    hdrs = glob(['glad/include/**/*.h']),
    includes = ['glad/include'],
    deps = [':glfw'],
    visibility = ["//visibility:public"]
)



cc_binary(
    name = 'demo',
    srcs = glob([
        '*.h',
        '*.cpp'
    ]),
    deps = [
        ':glad', ':fbxsdk'
    ]
)