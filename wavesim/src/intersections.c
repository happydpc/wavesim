#include "wavesim/intersections.h"
#include "wavesim/face.h"
#include <string.h>
#include <math.h>
#include <float.h>

/* ------------------------------------------------------------------------- */
int
intersect_point_aabb_test(const wsreal_t point[3], const wsreal_t aabb[6])
{
    return (
        point[0] >= aabb[0] && point[0] <= aabb[3] &&
        point[1] >= aabb[1] && point[1] <= aabb[4] &&
        point[2] >= aabb[2] && point[2] <= aabb[5]
    );
}

/* ------------------------------------------------------------------------- */
int
intersect_aabb_aabb_test(const wsreal_t aabb1[6], const wsreal_t aabb2[6])
{
    int i;
    for (i = 0; i != 3; ++i)
        if (aabb1[i+3] < aabb2[i+0] || aabb1[i+0] > aabb2[i+3]) return 0;
    return 1;
}

/* ------------------------------------------------------------------------- */
int
intersect_line_face(intersect_result_t* result,
                    const wsreal_t p1[3], const wsreal_t p2[3],
                    const wsreal_t v1[3], const wsreal_t v2[3], const wsreal_t v3[3])
{
    vec3_t u, v, n, w, dir, w0, intersect;
    wsreal_t determinant, a, b, r, uu, vv, uv, wu, wv, s, t;
    result->count = 0;

    /*
     * Algorithm taken from:
     * http://geomalgorithms.com/a06-_intersect-2.html
     */

    /* Get face edge vectors and plane normal */
    vec3_copy(&u, v2);
    vec3_sub_vec3(u.xyz, v1);
    vec3_copy(&v, v3);
    vec3_sub_vec3(v.xyz, v1);
    n = u;
    vec3_cross(n.xyz, v.xyz);

    /* Not checking for degenerate triangle, but could do so if n == 0 */

    /* Calculate ray direction vector */
    vec3_copy(&dir, p2);
    vec3_copy(&w0, p1);
    vec3_sub_vec3(dir.xyz, p1);
    vec3_sub_vec3(w0.xyz, v1);

    a = -vec3_dot(n.xyz, w0.xyz);
    b = vec3_dot(n.xyz, dir.xyz);
    if (fabs(b) < WS_EPSILON) /* ray is parallel to triangle plane */
        return 0;

    /* Get intersection point of ray with triangle */
    r = a / b;
    if (r < 0.0) /* ray goes away from triangle -> no intersect */
        return 0;
    if (r > 1.0) /* ray is not long enough to reach triangle (comment out for ray-triangle) */
        return 0;

    /* Intersect point of ray with triangle */
    intersect = dir;
    vec3_mul_scalar(intersect.xyz, r);
    vec3_add_vec3(intersect.xyz, p1);

    /* Need to determine if intersect point is inside triangle */

    /* Cache dot products */
    w = intersect;
    vec3_sub_vec3(w.xyz, v1);
    uu = vec3_dot(u.xyz, u.xyz);
    vv = vec3_dot(v.xyz, v.xyz);
    uv = vec3_dot(u.xyz, v.xyz);
    wu = vec3_dot(w.xyz, u.xyz);
    wv = vec3_dot(w.xyz, v.xyz);

    /* Calculate determinant */
    determinant = uv*uv - uu*vv;

    /* Calculate parametric coordinates and test */
    s = (uv*wv - vv*wu) / determinant;
    if (s < 0.0 || s > 1.0) /* intersect point is outside of face */
        return 0;
    t = (uv*wu - uu*wv) / determinant;
    if (t < 0.0 || s+t > 1.0)  /* intersect point is outside of face */
        return 0;

    result->count = 1;
    result->shape[0] = intersect;
    return 1;
}

/* ------------------------------------------------------------------------- */
int
intersect_line_face_test(const wsreal_t p1[3], const wsreal_t p2[3],
                         const wsreal_t v1[3], const wsreal_t v2[3], const wsreal_t v3[3])
{
    intersect_result_t result;
    return intersect_line_face(&result, p1, p2, v1, v2, v3);
}

/* ------------------------------------------------------------------------- */
int
intersect_face_aabb(intersect_result_t* result,
                    const wsreal_t v1[3], const wsreal_t v2[3], const wsreal_t v3[3],
                    const wsreal_t aabb[6])
{
    intersect_result_t edge;
    result->count = 0;

    /* These are the 8 vertices of the bounding box */
    vec3_t aaa = vec3(aabb[0], aabb[1], aabb[2]);
    vec3_t aab = vec3(aabb[0], aabb[1], aabb[5]);
    vec3_t aba = vec3(aabb[0], aabb[4], aabb[2]);
    vec3_t abb = vec3(aabb[0], aabb[4], aabb[5]);
    vec3_t baa = vec3(aabb[3], aabb[1], aabb[2]);
    vec3_t bab = vec3(aabb[3], aabb[1], aabb[5]);
    vec3_t bba = vec3(aabb[3], aabb[4], aabb[2]);
    vec3_t bbb = vec3(aabb[3], aabb[4], aabb[5]);

    /*
    face_t l1 = face(vertex(aaa, attribute_default()), vertex(aab, attribute_default()), vertex(aba, attribute_default()));
    face_t l2 = face(vertex(aba, attribute_default()), vertex(abb, attribute_default()), vertex(aab, attribute_default()));*/

    /* Test all 12 edges of the bounding box for intersections with the face */
    if (intersect_line_face(&edge, aaa.xyz, aab.xyz, v1, v2, v3))
        result->shape[result->count++] = edge.shape[0];
    if (intersect_line_face(&edge, aaa.xyz, aba.xyz, v1, v2, v3))
        result->shape[result->count++] = edge.shape[0];
    if (intersect_line_face(&edge, aaa.xyz, baa.xyz, v1, v2, v3))
        result->shape[result->count++] = edge.shape[0];
    if (intersect_line_face(&edge, aab.xyz, abb.xyz, v1, v2, v3))
        result->shape[result->count++] = edge.shape[0];
    if (intersect_line_face(&edge, aab.xyz, bab.xyz, v1, v2, v3))
        result->shape[result->count++] = edge.shape[0];
    if (intersect_line_face(&edge, aba.xyz, abb.xyz, v1, v2, v3))
        result->shape[result->count++] = edge.shape[0];
    if (intersect_line_face(&edge, aba.xyz, bba.xyz, v1, v2, v3))
        result->shape[result->count++] = edge.shape[0];
    if (intersect_line_face(&edge, baa.xyz, bab.xyz, v1, v2, v3))
        result->shape[result->count++] = edge.shape[0];
    if (intersect_line_face(&edge, baa.xyz, bba.xyz, v1, v2, v3))
        result->shape[result->count++] = edge.shape[0];
    if (intersect_line_face(&edge, abb.xyz, bbb.xyz, v1, v2, v3))
        result->shape[result->count++] = edge.shape[0];
    if (intersect_line_face(&edge, bab.xyz, bbb.xyz, v1, v2, v3))
        result->shape[result->count++] = edge.shape[0];
    if (intersect_line_face(&edge, bba.xyz, bbb.xyz, v1, v2, v3))
        result->shape[result->count++] = edge.shape[0];

    /* Test the 3 edges of the face for intersections with all 6 faces on the bounding box
    if (intersect_line_face())*/

    return 1;
}

/* ------------------------------------------------------------------------- */
int
intersect_face_aabb_test(const wsreal_t v1[3], const wsreal_t v2[3], const wsreal_t v3[3],
                         const wsreal_t aabb[6])
{
    intersect_result_t edge;

    /*
     * If any of the face vertices are inside the bounding box, then we know
     * the face intersects.
     */
    if (intersect_point_aabb_test(v1, aabb) ||
        intersect_point_aabb_test(v2, aabb) ||
        intersect_point_aabb_test(v3, aabb))
    {
        return 1;
    }

    /* These are the 8 vertices of the bounding box */
    vec3_t aaa = vec3(aabb[0], aabb[1], aabb[2]);
    vec3_t aab = vec3(aabb[0], aabb[1], aabb[5]);
    vec3_t aba = vec3(aabb[0], aabb[4], aabb[2]);
    vec3_t abb = vec3(aabb[0], aabb[4], aabb[5]);
    vec3_t baa = vec3(aabb[3], aabb[1], aabb[2]);
    vec3_t bab = vec3(aabb[3], aabb[1], aabb[5]);
    vec3_t bba = vec3(aabb[3], aabb[4], aabb[2]);
    vec3_t bbb = vec3(aabb[3], aabb[4], aabb[5]);

    /* Test all 12 edges of the bounding box for intersections with the face */
    if (intersect_line_face(&edge, aaa.xyz, aab.xyz, v1, v2, v3)) return 1;
    if (intersect_line_face(&edge, aaa.xyz, aba.xyz, v1, v2, v3)) return 1;
    if (intersect_line_face(&edge, aaa.xyz, baa.xyz, v1, v2, v3)) return 1;
    if (intersect_line_face(&edge, aab.xyz, abb.xyz, v1, v2, v3)) return 1;
    if (intersect_line_face(&edge, aab.xyz, bab.xyz, v1, v2, v3)) return 1;
    if (intersect_line_face(&edge, aba.xyz, abb.xyz, v1, v2, v3)) return 1;
    if (intersect_line_face(&edge, aba.xyz, bba.xyz, v1, v2, v3)) return 1;
    if (intersect_line_face(&edge, baa.xyz, bab.xyz, v1, v2, v3)) return 1;
    if (intersect_line_face(&edge, baa.xyz, bba.xyz, v1, v2, v3)) return 1;
    if (intersect_line_face(&edge, abb.xyz, bbb.xyz, v1, v2, v3)) return 1;
    if (intersect_line_face(&edge, bab.xyz, bbb.xyz, v1, v2, v3)) return 1;
    if (intersect_line_face(&edge, bba.xyz, bbb.xyz, v1, v2, v3)) return 1;

    return 0;
}
