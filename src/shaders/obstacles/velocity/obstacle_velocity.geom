#version 410 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 6) out;

in int vInstance[3];

in vec3 planePoint[3];
in vec3 planeNormal[3];
in vec3 vertVelocity[3];

out vec3 velocity;
out float layer;

struct Point {
    vec3 position;
    vec3 velocity;
};

int SegmentPlaneIntersection(vec3 segmentPoint, vec3 segmentDir, vec3 planePoint, vec3 planeNormal, out float t)
{
    float denom = dot(planeNormal, segmentDir);

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
    gl_Position = vec4(a.position, 1);
    velocity = a.velocity;
    EmitVertex();

    gl_Position = vec4(b.position, 1);
    velocity = b.velocity;
    EmitVertex();

    gl_Position = vec4(c.position, 1);
    velocity = c.velocity;
    EmitVertex();

    EndPrimitive();
}

void EmitSegment(Point a1, Point b1)
{
    // find the triangle face normal
    vec3 v1 = gl_in[0].gl_Position.xyz - gl_in[1].gl_Position.xyz;
    vec3 v2 = gl_in[0].gl_Position.xyz - gl_in[2].gl_Position.xyz;

    vec3 faceNormal = normalize(cross(v1, v2));

    // find the projection of the face normal onto the plane
    vec3 projFaceNormal = faceNormal - dot(faceNormal, planeNormal[0]) * planeNormal[0];

    // extrude the segment along the projection of the face normal
    vec3 extrude = normalize(projFaceNormal) * 1.41421;

    Point a2 = Point(a1.position + extrude, a1.velocity);
    Point b2 = Point(b1.position + extrude, b1.velocity);

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
    int numIntersections = 0;

    for (int i = 0; i < 3 && numIntersections < 3; i++)
    {
        vec3 a = gl_in[i].gl_Position.xyz;
        vec3 b = gl_in[(i + 1) % 3].gl_Position.xyz;

        vec3 vel_a = vertVelocity[i];
        vec3 vel_b = vertVelocity[(i + 1) % 3];

        vec3 ab = normalize(b - a);

        float t;

        int result = SegmentPlaneIntersection(a, ab, p0, n, t);

        switch(result)
        {
            case 0:
                break;
            case 1:
                intersections[numIntersections++].position = a + ab * t;
                intersections[numIntersections++].velocity = mix(vel_a, vel_b, t);
                break;
            case 2:
                if (numIntersections < 2)
                {
                    intersections[numIntersections++].position = a;
                    intersections[numIntersections++].velocity = vel_a;

                    intersections[numIntersections++].position = b;
                    intersections[numIntersections++].velocity = vel_b;
                }
                else
                {
                    intersections[numIntersections++].position = b;
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
            gl_Position = vec4(intersections[0].position, 1);
            velocity = intersections[0].velocity;
            EmitVertex();
            EmitVertex();
            EmitVertex();
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
