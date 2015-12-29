#version 410

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexColor;
layout (location = 2) in vec3 InstancePosition;

out vec3 Color;
uniform mat4 ViewProjMatrix;
uniform mat4 RotationMatrix;

void main () {
    Color = VertexColor;
//    gl_Position = ViewProjMatrix * (
//        vec4(InstancePosition, 1.0) +
//        (RotationMatrix * vec4(VertexPosition,1.0)));
    gl_Position = ViewProjMatrix * (vec4(InstancePosition,1.0) + (RotationMatrix * vec4(VertexPosition, 1.0)));
}