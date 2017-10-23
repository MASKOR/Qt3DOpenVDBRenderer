#version 150

in vec3 vertexPosition;
in vec3 vertexNormal;
//in vec3 vertexColor;

uniform mat4 modelView;
uniform mat3 modelViewNormal;
uniform mat4 mvp;
uniform mat4 projectionMatrix;
uniform mat4 viewportMatrix;
uniform float fieldOfView;
uniform float fieldOfViewVertical;
uniform float nearPlane;

out vec3 viewspacePosition;
out vec3 viewspaceNormal;
out vec3 viewspaceTang;
out vec3 viewspaceBitang;
out vec3 color;
out float radius;

uniform float pointSize;

float random(int seed) {
        seed = (seed * 1103515245 + 12345);
        return float(seed) / 4294967296.0;
}

void main(void)
{
    vec4 viewSpacePos = modelView * vec4(vertexPosition, 1.0);
    viewspacePosition = viewSpacePos.xyz;
    viewspaceNormal = modelViewNormal * vertexNormal;
    viewspaceTang = normalize(cross(viewspaceNormal, vec3(0.0,0.0,1.0))); //longest radius
    viewspaceBitang = normalize(cross(viewspaceTang, viewspaceNormal)); //smallest radius
    gl_Position = projectionMatrix * viewSpacePos;

    // VERY SLOW
    //vec2 csize = max(max(max(max(max(abs(Ap.xy-Bp.xy), abs(Ap.xy-Cp.xy)), abs(Ap.xy-Dp.xy)), abs(Bp.xy-Cp.xy)), abs(Bp.xy-Dp.xy)), abs(Cp.xy-Dp.xy));

    float rand = random(gl_VertexID)*2.0-1.0;
    float dist = max(1.0, gl_Position.w + rand);
    dist = mix(gl_Position.w, dist, clamp(gl_Position.w-nearPlane, 0.0,1.0));

    // x2 would be perfect. But the smaller the points are, the better performance is.
    radius = max(0.5, 1.9 * viewportMatrix[1][1] * projectionMatrix[1][1] * pointSize / dist);
    gl_PointSize = radius;
    float lit = (1.0 + dot(normalize(vertexNormal), normalize(vec3(0.5,2.0,1.0))))*0.5;
    color = vec3(lit);
}
