#ifndef PARTITION_H
#define PARTITION_H

#include "contour.h"
#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <glm/gtc/epsilon.hpp>
#include <algorithm>
#include <set>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

struct ConvexCell
{
    std::vector<Plane> boundaryPlanes;
    std::vector<Vertex> vertices;
    std::vector<Edge> edges;
};

struct PlaneGroup
{
    std::vector<ContourPlane> planes;
    glm::vec3 normal;
};

class SpacePartitioner
{
public:
    SpacePartitioner(const std::vector<ContourPlane> &contourPlanes);
    std::vector<ConvexCell> computeCells();
    void renderCells();

private:
    std::vector<ContourPlane> originalPlanes;
    std::vector<PlaneGroup> parallelGroups;
    std::vector<ContourPlane> nonParallelPlanes;
    std::vector<ConvexCell> cells;

    void createSlabCell(const Plane &bottom, const Plane &top);
    void findIntersectionVertices(const std::vector<ContourPlane> &planes);
    void createCellFromIntersection(const std::vector<Vertex> &cellVertices, const std::vector<Plane> &boundaryPlanes);
    float computeDistance(const Plane &plane, const glm::vec3 &point);
    void classifyPlanes();
    bool arePlanesParallel(const Plane &p1, const Plane &p2);
    void computeParallelPlaneCells();
    void computeNonParallelPlaneCells();
    glm::vec3 computePlaneIntersection(const Plane &p1, const Plane &p2, const Plane &p3);
};

#endif