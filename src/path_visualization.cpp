#include "path_visualization.h"

#include <iostream>

void gl::PathVisualization::init(const Sample &sample)
{
}

void gl::PathVisualization::draw(
    GLfloat (&model)[4][4],
    GLfloat (&view)[4][4],
    GLfloat (&projection)[4][4]
) {
    std::cout << "path visualization draw!\n" << std::endl;
}
