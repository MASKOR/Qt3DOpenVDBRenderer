#version 130

in vec3 normal;
in vec3 position;
in vec3 color;
in float radius;

uniform vec3 finalColor;

out vec4 fragColor;

void main()
{
//    float deriv = dFdx(gl_PointCoord.s);
//    float dist = distance( gl_PointCoord.st, vec2(0.5))/deriv;
//    float alpha = 1.0-smoothstep((0.6*radius)-1.0, (0.6*radius), dist);
    fragColor = vec4(color, 1.0);
}
