#version 410

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexColor;

out vec3 Color;
uniform mat4 MVP;

uniform vec3 ArrayOffset;
uniform int  ArrayCountX;
uniform int  ArrayCountY;
uniform int  ArrayCountZ;

void main () {
    Color = VertexColor;
    
    int x = gl_InstanceID % ArrayCountX;
    int y = (gl_InstanceID / ArrayCountX) % ArrayCountY;
    int z = gl_InstanceID / (ArrayCountX * ArrayCountY);
    
    vec3 arrayPosition = VertexPosition + vec3(
        ArrayOffset.x * float(x),
        ArrayOffset.y * float(y),
        ArrayOffset.z * float(z)
    );
    gl_Position = MVP * vec4(arrayPosition,1.0);
}
