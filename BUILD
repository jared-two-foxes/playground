load("@rules_cc//cc:defs.bzl", "cc_import", "cc_library", "cc_binary")

cc_import(
    name = "xml2",
    static_library = select({
        "@bazel_tools//src/conditions:windows": "fbx_sdk/lib/vs2017/x64/debug/libxml2-md.lib",
    })
)

cc_import(
    name = "zlib",
    static_library = select({
        "@bazel_tools//src/conditions:windows": "fbx_sdk/lib/vs2017/x64/debug/zlib-md.lib",
    })
)

cc_import(
    name = 'fbxsdk_import',
    hdrs = glob([
        'fbx_sdk/include/**/*.hpp',
        'fbx_sdk/include/**/*.h'    
    ]),
    interface_library = select({
        "@bazel_tools//src/conditions:windows": 'fbx_sdk/lib/vs2017/x64/debug/libfbxsdk.lib',
    }),
    shared_library = select({
        "@bazel_tools//src/conditions:windows": 'fbx_sdk/lib/vs2017/x64/debug/libfbxsdk.dll',
    }),
    # static_library = select({
    #     "@bazel_tools//src/conditions:darwin": 'fbx_sdk/lib/gcc/x64/debug/libfbxsdk.a'
    # }),
    visibility = ['//visibility:public']
)

cc_library(
    name = 'fbxsdk',
    hdrs = glob([
        'fbx_sdk/include/**/*.hpp',
        'fbx_sdk/include/**/*.h'    
    ]),
    includes = ['fbx_sdk/include'],
    defines = select({
        "@bazel_tools//src/conditions:windows": ["FBXSDK_SHARED"]
    }),
    deps = [':fbxsdk_import',':zlib',':xml2'],
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
        "@bazel_tools//src/conditions:darwin": "glfw/lib/libglfw3.a" 
    })
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
        "@bazel_tools//src/conditions:darwin": [
            '-ldl','-lpthread','-lz']
    }),
    visibility = ["//visibility:public"]
)

cc_library(
    name = 'glad',
    srcs = glob(['glad/*.c']),
    hdrs = glob(['glad/*.h']),
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