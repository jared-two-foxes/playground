#ifndef COMMON_HPP__
#define COMMON_HPP__

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <fbxsdk.h>
#include "linmath.h"


void ConvertFbxToLinmath( const double* pDouble, GLfloat* pFloat, std::size_t n );

#endif // COMMON_HPP_