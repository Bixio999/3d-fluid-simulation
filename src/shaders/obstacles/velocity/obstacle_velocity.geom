#version 410 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 6) out;

in int vInstance[3];

in vec3 planePoint[3];
in vec3 planeNormal[3];
in vec3 vertVelocity[3];
in vec4 mPos[3];

uniform float grid_depth;
uniform float texelDiagonal;
uniform mat4 projection;
uniform mat4 view;

out vec3 velocity;
out float layer;

// debugging
out int numIntersections;
out float t;
out int result;
out vec3 worldPos;
////////////

struct Point {
    vec3 position;
    vec3 velocity;
    int intersectionResult;
    float t;
};

vec4 World2ClipCoords(vec3 pos)
{
    vec4 temp = projection * view * vec4(pos, 1.0);
    return temp / temp.w;
}

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

void EmitTriangle(Point a, Point b, Point c)
{
    gl_Position = World2ClipCoords(a.position);
    velocity = a.velocity;
    t = a.t;
    result = a.intersectionResult;
    worldPos = a.position;
    EmitVertex();

    gl_Position = World2ClipCoords(b.position);
    velocity = b.velocity;
    t = b.t;
    result = b.intersectionResult;
    worldPos = b.position;
    EmitVertex();

    gl_Position = World2ClipCoords(c.position);
    velocity = c.velocity;
    t = c.t;
    result = c.intersectionResult;
    worldPos = c.position;
    EmitVertex();

    EndPrimitive();
}

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

void main()
{
    gl_Layer = vInstance[0];
    layer = vInstance[0] + 0.5;

    vec3 p0 = planePoint[0];
    vec3 n = planeNormal[0];

    Point intersections[3];
    numIntersections = 0;

    for (int i = 0; i < 3 && numIntersections < 3; i++)
    {
        vec3 a = mPos[i].xyz;
        vec3 b = mPos[(i + 1) % 3].xyz;

        vec3 vel_a = vertVelocity[i];
        vec3 vel_b = vertVelocity[(i + 1) % 3];

        vec3 ab = b - a;

        // float t;

        result = SegmentPlaneIntersection(a, ab, p0, n, t);

        switch(result)
        {
            case 0:
                break;
            case 1:
                intersections[numIntersections].position = a + ab * t;
                intersections[numIntersections].intersectionResult = result;
                intersections[numIntersections].t = t;
                intersections[numIntersections++].velocity = mix(vel_a, vel_b, t);
                break;
            case 2:
                if (numIntersections < 2)
                {
                    intersections[numIntersections].position = a;
                    intersections[numIntersections].intersectionResult = result;
                    intersections[numIntersections].t = t;
                    intersections[numIntersections++].velocity = vel_a;

                    intersections[numIntersections].position = b;
                    intersections[numIntersections].intersectionResult = result;
                    intersections[numIntersections].t = t;
                    intersections[numIntersections++].velocity = vel_b;
                }
                else
                {
                    intersections[numIntersections].position = b;
                    intersections[numIntersections].intersectionResult = result;
                    intersections[numIntersections].t = t;
                    intersections[numIntersections++].velocity = vel_b;
                }
                break;
        }
    }

    // check for duplicates
    switch(numIntersections)
    {
        case 2:
            if (intersections[0].position == intersections[1].position)
                numIntersections = 1;
            break;
        case 3:
            if (intersections[0].position == intersections[1].position)
            {
                intersections[0] = intersections[2];
                numIntersections = 2;
            }
            else if (intersections[0].position == intersections[2].position || intersections[1].position == intersections[2].position)
                numIntersections = 2;
            break;
        default:
            break;
    }

    // emit the triangles
    switch(numIntersections)
    {
        case 0:
            break;
        case 1:
            gl_Position = World2ClipCoords(intersections[0].position);
            velocity = intersections[0].velocity;
            t = intersections[0].t;
            worldPos = intersections[0].position;
            result = intersections[0].intersectionResult;
            EmitVertex();
            // EmitVertex();
            // EmitVertex();
            EndPrimitive();
            break;
        case 2:
            EmitSegment(intersections[0], intersections[1]);
            break;
        case 3:
            EmitTriangle(intersections[0], intersections[1], intersections[2]);
            break;
    }
}
