#ifndef HELPERS_HPP
#define HELPERS_HPP

#include <GL/freeglut.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// Declare helper functions to be used in BetterBoxer.cpp
void customRotatef(float angle, float axisX, float axisY, float axisZ);
void customTranslatef(float x, float y, float z);

#endif // HELPERS_HPP

