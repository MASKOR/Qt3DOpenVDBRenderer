#version 130

in vec3 vertexPosition;
in vec3 vertexNormal;
in vec3 vertexColor;

out vec3 position;
out vec3 normal;
out vec3 color;
out float radius;

uniform mat4 modelView;
uniform mat3 modelViewNormal;
uniform mat4 mvp;
uniform mat4 projectionMatrix;
uniform mat4 viewportMatrix;
uniform float nearPlane;

uniform float pointSize;

float random(int seed) {
        seed = (seed * 1103515245 + 12345);
        return float(seed) / 4294967296.0;
}

void main()
{
    float lit = (1.0 + dot(normalize(vertexNormal), normalize(vec3(0.5,2.0,1.0))))*0.5;
    normal = normalize(modelViewNormal * vertexNormal);
    position = vec3(modelView * vec4(vertexPosition, 1.0));
    color = vec3(lit);//vertexPosition * 0.1;//vertexColor;
    gl_Position = mvp * vec4(vertexPosition, 1.0);

    float rand = random(gl_VertexID)*2.0-1.0;
    float dist = max(1.0, gl_Position.w + rand);
    dist = mix(gl_Position.w, dist, clamp(gl_Position.w-nearPlane, 0.0,1.0));
    radius = max(0.5, viewportMatrix[1][1] * projectionMatrix[1][1] * pointSize / dist);
    gl_PointSize = radius;
}
