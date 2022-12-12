#version 410 core

out vec4 FragColor;          // fragment output color

uniform sampler2D SourceTexture;          // texture to blur
uniform vec2 InverseScreenSize;           // texture resolution

uniform float radius;                // blur radius
uniform int axis;

void main()
{
    float x, y, rr = radius * radius, d, w, w0;
    
    vec2 p = gl_FragCoord.xy * InverseScreenSize;

    vec4 pixel = texture(SourceTexture, p);
    if (pixel.a == 0.0) discard;

    vec4 col=vec4(0.0,0.0,0.0,0.0);

    vec4 temp;

    w0 = 0.5135 / pow(radius, 0.96);

    if (axis == 0) 
    {
        for (d = InverseScreenSize.x, x = -radius, p.x += x * d; x <= radius; x++, p.x += d)
        { 
            w = w0 * exp((-x * x) / (2.0 * rr));

            temp = texture(SourceTexture, p);
            if (temp.a == 0.0) temp = pixel;

            col += temp * w; 
        }
    }

    if (axis == 1) 
    {
        for (d = InverseScreenSize.y, y = -radius, p.y += y * d; y <= radius; y++, p.y += d)
        { 
            w = w0 * exp((-y * y) / (2.0 * rr)); 
            temp = texture(SourceTexture, p);
            if (temp.a == 0.0) temp = pixel;

            col += temp * w; 
        }
    }

    FragColor = col;
}