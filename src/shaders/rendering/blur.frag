/*
    OpenGL 4.1 Core - Blur filter - Fragment Shader

    This shader is part of the program that apply a gaussian blur filter
    to the given texture in input. 

    The algorithm is based on the one shared in the following discussion:
    https://stackoverflow.com/questions/64837705/opengl-blurring

    The gaussian blur is applied in two passes, one for the horizontal
    direction and one for the vertical direction. The weights are computed
    with a gaussian function and the number of samples is computed based
    on the blur radius. The parameters of the gaussian weights are hard-coded
    and defined by the original author of the algorithm; they are chosen 
    to give a good result for integer values of the blur radius, because
    the sum of the weights is always near 1.0. To avoid artifacts due to
    oscillations of the weighting function, the final color is adjusted
    by subtracting a percentage equal to the excess of the sum of the weights
    over 1.0. This shader is used to apply a blur filter to the liquid 
    rendering, to smooth the surface that is affected by a dithering effect
    caused by the jittering of the ray during the raymarching step introduced
    to reduce the banding artifacts.

    This shaders is composed by the following shaders:
    - Vertex Shader: load_vertices.vert - Load the vertices of the quad
    - Fragment Shader: this shader
*/

#version 410 core

out vec4 FragColor; // fragment output color

uniform sampler2D SourceTexture; // texture to blur
uniform vec2 InverseScreenSize; // inverse texture resolution

uniform float radius; // blur radius
uniform int axis; // 0 for horizontal blur, 1 for vertical blur

// Main function
void main()
{
    // Define the local variables
    float x, y, d, w, w0, tot = 0;

    // Compute the squared radius
    float rr = radius * radius;
    
    // Compute the pixel position in the texture
    vec2 p = gl_FragCoord.xy * InverseScreenSize;

    // Read the color of the pixel in the texture
    // and check if is actually a fluid pixel
    vec4 pixel = texture(SourceTexture, p);
    if (length(pixel.xyz) == 0.0 || pixel.a == 0.0) discard;

    vec4 col = vec4(0.0);
    vec4 temp;

    // Compute the weight for the gaussian function
    w0 = 0.5135 / pow(radius, 0.96);

    // Horizontal blur
    if (axis == 0) 
    {
        // Compute the number of samples on the kernel
        for (d = InverseScreenSize.x, x = -radius, p.x += x * d; x <= radius; x++, p.x += d)
        { 
            // Compute the weight for the current sample
            w = w0 * exp((-x * x) / (2.0 * rr));

            // Read the color of the current sample
            temp = texture(SourceTexture, p);

            // Check if the current sample is a fluid pixel
            if (length(temp.xyz) == 0.0 || temp.a == 0.0) 
                temp = pixel; // if not, use the color of the pixel to blur

            // Accumulate the weighted color
            col += temp * w; 
            tot += w;
        }
    }

    // Vertical blur
    if (axis == 1) 
    {
        // Compute the number of samples on the kernel
        for (d = InverseScreenSize.y, y = -radius, p.y += y * d; y <= radius; y++, p.y += d)
        { 
            // Compute the weight for the current sample
            w = w0 * exp((-y * y) / (2.0 * rr)); 

            // Read the color of the current sample
            temp = texture(SourceTexture, p);

            // Check if the current sample is a fluid pixel
            if (length(temp.xyz) == 0.0 || temp.a == 0.0) 
                temp = pixel; // if not, use the color of the pixel to blur

            // Accumulate the weighted color
            col += temp * w; 
            tot += w;
        }
    }

    // Compute the excess of the sum of the weights over 1.0
    float fraction = fract(max(1.0, tot));

    // Adjust the final color
    col *= 1 - fraction/tot;

    // Write the final color
    FragColor = col;
}