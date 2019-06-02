#pragma once

#include "aabb.h"
#include "intersection.h"
#include "point.h"
#include "primitive.h"
#include "ray.h"
#include "primitive.h"

#include <vector>

const int BVHNode_MaxPrimitives = 2;

struct BVHNode {
    BVHNode *left;
    BVHNode *right;

    AABB *aabb;

    int primitiveCount;
    Primitive *primitives[BVHNode_MaxPrimitives];
};

struct IndexedCentroid {
    Point3 centroid;
    int primitiveIndex;
};

class BVH : public Primitive {
public:
    BVH();

    void bake(const std::vector<Primitive *> &primitives);

    Intersection castRay(const Ray &ray) const;
    Intersection testIntersect(const Ray &ray) const override;

    Point3 centroid() const override;
    void updateAABB(AABB *aabb) override;

    void debug() const override;

private:
    void bakeHelper(
        const std::vector<Primitive *> &primitives,
        BVHNode *root,
        int index
    );

    Intersection castRay(const Ray &ray, BVHNode *node) const;
    Intersection castRay(const Ray &ray, BVHNode *node, Intersection result) const;

    BVHNode *m_root;
};
