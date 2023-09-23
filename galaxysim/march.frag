#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec2 cloudPositions[500];
uniform vec3 cloudColors[500];
uniform mat4x4 projection;

// Output fragment color
out vec4 finalColor;

void main()
{
    // Texel color fetching from texture sampler
    vec4 texelColor = texture(texture0, fragTexCoord);
    
    float total = 0;
    vec3 color = vec3(0);
    for (int i = 0; i < 500; ++i)
    {
        total += distance(cloudPositions[i], fragTexCoord);
        color += cloudColors[i];
    }
    color /= 500;
    total /= 500;

    vec4 gasColor = vec4(color, total);
    
    finalColor = gasColor;
}

