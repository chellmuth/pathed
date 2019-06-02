#include "bvh.h"

#include "aabb.h"
#include "color.h"
#include "lambertian.h"
#include "material.h"

#include <algorithm>
#include <assert.h>

static Material *fakeMaterial = new Lambertian(
    Color(0.f, 0.f, 0.f),
    Color(0.f, 0.f, 0.f)
);

BVH::BVH()
    : Primitive()
{}

static const float RayEta = 1e-4;

static Intersection IntersectionUnion(Intersection i1, Intersection i2)
{
    Intersection result = IntersectionHelper::miss;

    if (i1.hit && i1.t > RayEta && i1.t < result.t) {
        result = i1;
    }

    if (i2.hit && i2.t > RayEta && i2.t < result.t) {
        result = i2;
    }

    return result;
}

static bool compareNodeAlongX(IndexedCentroid ic1, IndexedCentroid ic2)
{
    return ic1.centroid.x() < ic2.centroid.x();
}

static bool compareNodeAlongY(IndexedCentroid ic1, IndexedCentroid ic2)
{
    return ic1.centroid.y() < ic2.centroid.y();
}

static bool compareNodeAlongZ(IndexedCentroid ic1, IndexedCentroid ic2)
{
    return ic1.centroid.z() < ic2.centroid.z();
}

void BVH::bake(const std::vector<std::shared_ptr<Primitive>> &primitives)
{
    m_root = new BVHNode();

    bakeHelper(primitives, m_root, 0);
}

void BVH::bakeHelper(
    const std::vector<std::shared_ptr<Primitive>> &primitives,
    BVHNode *node,
    int depth
) {
    node->aabb = new AABB();

    for (auto primitive : primitives) {
        primitive->updateAABB(node->aabb);
    }

    node->aabb->bake();

    if (primitives.size() <= BVHNode_MaxPrimitives) {
        node->primitiveCount = primitives.size();
        for (int i = 0; i < primitives.size(); i++) {
            node->primitives[i] = primitives[i];
        }
        return;
    } else {
        node->primitiveCount = 0;
    }

    std::vector<IndexedCentroid> centroids;

    for (int i = 0; i < primitives.size(); i++) {
        std::shared_ptr<Primitive> primitive = primitives[i];

        IndexedCentroid centroid = {
            primitive->centroid(),
            i
        };

        centroids.push_back(centroid);
    }

    int lastIndex = centroids.size() - 1;

    std::sort(centroids.begin(), centroids.end(), compareNodeAlongX);
    float rangeX = centroids[lastIndex].centroid.x() - centroids[0].centroid.x();

    std::sort(centroids.begin(), centroids.end(), compareNodeAlongY);
    float rangeY = centroids[lastIndex].centroid.y() - centroids[0].centroid.y();

    std::sort(centroids.begin(), centroids.end(), compareNodeAlongZ);
    float rangeZ = centroids[lastIndex].centroid.z() - centroids[0].centroid.z();

    if (rangeX >= rangeY && rangeX >= rangeZ) {
        std::sort(centroids.begin(), centroids.end(), compareNodeAlongX);
    } else if (rangeY >= rangeX && rangeY >= rangeZ) {
        std::sort(centroids.begin(), centroids.end(), compareNodeAlongY);
    } else if (rangeZ >= rangeX && rangeZ >= rangeY) {
        std::sort(centroids.begin(), centroids.end(), compareNodeAlongZ);
    }

    std::vector<std::shared_ptr<Primitive>> leftPrimitives;
    std::vector<std::shared_ptr<Primitive>> rightPrimitives;

    for (int i = 0; i < centroids.size(); i++) {
        std::shared_ptr<Primitive> primitive = primitives[centroids[i].primitiveIndex];

        if (i < centroids.size()/2) {
            leftPrimitives.push_back(primitive);
        } else {
            rightPrimitives.push_back(primitive);
        }
    }

    node->left = new BVHNode();
    node->right = new BVHNode();

    bakeHelper(leftPrimitives, node->left, depth + 1);
    bakeHelper(rightPrimitives, node->right, depth + 1);
}

Intersection BVH::castRay(const Ray &ray) const
{
    return castRay(ray, m_root);
}

Intersection BVH::castRay(const Ray &ray, BVHNode *node) const
{
    Intersection result = IntersectionHelper::miss;

    return castRay(ray, node, result);
}

Intersection BVH::castRay(const Ray &ray, BVHNode *node, Intersection result) const
{
    if (!node->aabb->testHit(ray, result.t)) {
        return result;
    }

    if (node->primitiveCount > 0) {
        for (int i = 0; i < node->primitiveCount; i++) {
            std::shared_ptr<Primitive> primitive = node->primitives[i];
            Intersection intersection = primitive->testIntersect(ray);

            if (!intersection.hit) { continue; }
            if (intersection.t < RayEta) { continue; }
            if (intersection.t > result.t) { continue; }

            result = intersection;
        }

        return result;
    }

    Intersection leftResult = castRay(ray, node->left, result);
    Intersection rightResult = castRay(ray, node->right, leftResult);

    return IntersectionUnion(leftResult, rightResult);
}

Intersection BVH::testIntersect(const Ray &ray) const
{
    return castRay(ray);
}

Point3 BVH::centroid() const
{
    return m_root->aabb->getCentroid();
}

void BVH::updateAABB(AABB *aabb)
{
    AABB *localAABB = m_root->aabb;
    aabb->update(localAABB->bottomLeftFront());
    aabb->update(localAABB->topRightBack());
}

static void debugNode(BVHNode *node)
{
    printf("  <Node>\n");
    node->aabb->debug();
    if (node->primitiveCount > 0) {
        printf("    <Primitives count=%i>\n", node->primitiveCount);
        for (int i = 0; i < node->primitiveCount; i++) {
            node->primitives[i]->debug();
        }
        printf("    </Primitives>\n");
    } else {
        printf("    <Left>\n");
        debugNode(node->left);
        printf("    </Left>\n");
        printf("    <Right>\n");
        debugNode(node->right);
        printf("    </Right>\n");
    }
    printf("  </Node>\n");
}

void BVH::debug() const
{
    printf("<BVH>\n");
    debugNode(m_root);
    printf("</BVH>\n");
}
