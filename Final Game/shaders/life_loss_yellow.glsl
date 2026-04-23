#version 330

uniform sampler2D texture0;

in vec2 fragTexCoord;
in vec2 fragPosition;

out vec4 finalColor;

void main()
{
    vec4 color = texture(texture0, fragTexCoord);
    float intensity = dot(color.rgb, vec3(0.299, 0.587, 0.114));
    finalColor = vec4(intensity, 0.0, 0.0, color.a);
}
