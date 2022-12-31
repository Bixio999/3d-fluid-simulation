/*
    OpenGL 4.1 Core - Dynamic Obstacle Velocity Voxelization - Geometry Shader

    This shader is part of the program that computes and stores the velocity of
    the dynamic obstacles in the voxel grid. 

    The Velocity Voxelization algorithm calculates the velocity of the dynamic
    obstacles in the voxel grid. This is done by rendering the dynamic obstacle
    mesh and calculating the velocity of each vertex by comparing the current
    position of the vertex with the position of the vertex in the previous frame,
    and dividing the difference by the time elapsed between the two frames. The
    previous frame position is obtained by transforming the vertex position by
    the previous model matrix. From the per-vertex velocity, the velocity of the
    voxel is calculated by interpolation between the velocities of the vertices
    that compose the triangle that the voxel is inside. The velocity of the voxel
    for each slice of the grid is calculated in the geometry stage by the 
    intersection between each triangle of the mesh with the plane that represents 
    the current slice of the grid: if the entire triangle lays in the plane, it is
    entirely outputted, otherwise if the intersection is a segment, it is outputted
    as a quad obtained by extruding the segment along the normal projection of the
    triangle normal on the plane normal, and with an offset equal to the diagonal
    of the voxel.

    THis geometry shader is used to identify those triangles that intersect the
    current slice of the grid, and to output the triangles that are entirely in
    the plane, or the segments that are intersected by the plane as quads calculated 
    as described above. Also, the layered rendering is enabled by setting the
    gl_Layer variable to the current slice of the grid. The intersection between
    the triangle and the plane is calculated by the SegmentPlaneIntersection
    function applied to each edge of the triangle. If the intersection is a segment,
    the segment is outputted as a quad, otherwise if the intersection is a triangle,
    the triangle is outputted as a triangle.

    The Velocity Voxelization program is composed by the following shaders:
    - Vertex Shader: Computes the velocity of the vertices of the mesh, and the 
        plane that represents the current slice of the grid.
    - Geometry Shader: this shader
    - Fragment Shader: obstacle_velocity.frag - Adds in the obstacle velocity 
        texture the velocity of the voxel.
*/

#version 410 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 6) out; // 2 triangles per quad or a single triangle

in int vInstance[3]; // instance id

// plane
in vec3 planePoint[3];
in vec3 planeNormal[3];

// Vertex data
in vec3 vertVelocity[3];
in vec4 mPos[3];

uniform float grid_depth; // Number of slices of the grid

uniform float texelDiagonal; // Diagonal of a voxel

uniform mat4 projection; // Projection matrix
uniform mat4 view; // View matrix

out vec3 velocity; // Velocity of the vertex
out float layer; // Current slice of the grid

// Auxiliary structure to store the data of a vertex
struct Point {
    vec3 position;
    vec3 velocity;
};

// Convert world coordinates to clip coordinates
vec4 World2ClipCoords(vec3 pos)
{
    vec4 temp = projection * view * vec4(pos, 1.0);
    return temp / temp.w;
}

// Calculate the intersection between a segment and a plane and return an
// integer that indicates the type of intersection:
// 0 - no intersection
// 1 - segment intersects the plane
// 2 - segment lies in the plane
// The parameter t is the distance from the segment point to the intersection
// point along the segment direction
int SegmentPlaneIntersection(vec3 segmentPoint, vec3 segmentDir, vec3 planePoint, vec3 planeNormal, out float t)
{
    float denom = dot(segmentDir, planeNormal);

    vec3 v = planePoint - segmentPoint;
    float num = dot(v, planeNormal);

    if (abs(denom) < 0.0001)
    {
        t = 0.0;
        if (abs(num) < 0.0001)
            return 2; // segment lies in plane
        return 0; // segment is parallel to plane
    }
    
    t = num / denom;
    if (t < 0 || t > 1)
        return 0; // no intersection

    return 1;
}

// Emit a triangle by outputting the vertices
void EmitTriangle(Point a, Point b, Point c)
{
    gl_Position = World2ClipCoords(a.position);
    velocity = a.velocity;
    EmitVertex();

    gl_Position = World2ClipCoords(b.position);
    velocity = b.velocity;
    EmitVertex();

    gl_Position = World2ClipCoords(c.position);
    velocity = c.velocity;
    EmitVertex();

    EndPrimitive();
}

// Emit a quad calculated by extruding a segment along the normal projection of
// the triangle normal on the plane normal, and with an offset equal to the diagonal
// of the voxel
void EmitSegment(Point a1, Point b1)
{
    // find the triangle face normal
    vec3 v1 = normalize(mPos[1].xyz - mPos[0].xyz);
    vec3 v2 = normalize(mPos[2].xyz - mPos[0].xyz);

    vec3 faceNormal = normalize(cross(v1, v2));

    // find the projection of the face normal onto the plane
    vec3 projFaceNormal = faceNormal - dot(faceNormal, planeNormal[0]) * planeNormal[0];

    // extrude the segment along the projection of the face normal
    vec3 extrude = - normalize(projFaceNormal) * texelDiagonal;

    Point a2 = Point(a1.position + extrude, a1.velocity, a1.intersectionResult, a1.t);
    Point b2 = Point(b1.position + extrude, b1.velocity, b1.intersectionResult, b1.t);

    // emit the two triangles
    EmitTriangle(a1, b1, a2);
    EmitTriangle(b1, b2, a2);
}

// Main function
void main()
{
    // Calculate the z coordinate for the current slice of the grid
    // that will be used to access the obstacle velocity texture
    gl_Layer = vInstance[0]; 
    layer = vInstance[0] + 0.5;

    // Get the plane data
    vec3 p0 = planePoint[0];
    vec3 n = planeNormal[0];

    // Calculate the intersection between the triangle and the plane
    Point intersections[3];
    numIntersections = 0;

    // For each edge of the triangle calculate the intersection with the plane
    // until 3 intersections are found or all the edges have been checked
    for (int i = 0; i < 3 && numIntersections < 3; i++)
    {
        // Get the edge data
        vec3 a = mPos[i].xyz;
        vec3 b = mPos[(i + 1) % 3].xyz;

        // Get the velocity data
        vec3 vel_a = vertVelocity[i];
        vec3 vel_b = vertVelocity[(i + 1) % 3];

        // Calculate the edge direction
        vec3 ab = b - a;

        // Calculate the intersection
        result = SegmentPlaneIntersection(a, ab, p0, n, t);

        // Check for the intersection type
        switch(result)
        {
            case 0: // no intersection
                break;
            case 1: // segment intersects the plane, store the intersection
                intersections[numIntersections].position = a + ab * t;
                intersections[numIntersections++].velocity = mix(vel_a, vel_b, t);
                break;
            case 2: // segment lies in the plane, store both vertices
                if (numIntersections < 2) // check if there is space for both vertices
                {
                    intersections[numIntersections].position = a;
                    intersections[numIntersections++].velocity = vel_a;

                    intersections[numIntersections].position = b;
                    intersections[numIntersections++].velocity = vel_b;
                }
                else // store only the second vertex, which is the last one of the triangle
                {
                    intersections[numIntersections].position = b;
                    intersections[numIntersections++].velocity = vel_b;
                }
                break;
        }
    }

    // check for duplicates
    switch(numIntersections)
    {
        case 2: // if the two intersections are the same, remove the second one
                // by setting the number of intersections to 1
            if (intersections[0].position == intersections[1].position)
                numIntersections = 1;
            break;
        case 3: // if two intersections are the same, remove the third one

            // if the first two intersections are the same, remove the first one
            if (intersections[0].position == intersections[1].position)  
            {
                intersections[0] = intersections[2];
                numIntersections = 2;
            }
            // if the first (or second) and third intersections are the same, remove the third one
            else if (intersections[0].position == intersections[2].position || intersections[1].position == intersections[2].position)
                numIntersections = 2;
            break;
        default:
            break;
    }

    // emit the triangles
    switch(numIntersections)
    {
        case 0: // no intersection
            break;
        case 1: // one intersection, emit a point
            gl_Position = World2ClipCoords(intersections[0].position);
            velocity = intersections[0].velocity;
            EmitVertex();
            EndPrimitive();
            break;
        case 2: // two intersections, emit the segment as quad
            EmitSegment(intersections[0], intersections[1]);
            break;
        case 3: // three intersections, emit the triangle
            EmitTriangle(intersections[0], intersections[1], intersections[2]);
            break;
    }
}
