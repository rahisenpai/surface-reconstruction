// projection.h
#ifndef PROJECTION_H
#define PROJECTION_H

#include "contour.h"
#include "partition.h"
#include <unordered_map>
#include <CGAL/Advancing_front_surface_reconstruction.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Triangulation_3.h>
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/Triangulation_vertex_base_3.h>
#include <CGAL/Triangulation_cell_base_3.h>


struct AxisPlanes {
    struct Plane {
        std::vector<Point> corners;  // 4 corners defining the plane
        double position;             // Position along the axis
        char axis;                   // 'x', 'y', or 'z'
    };
    std::vector<Plane> planes;
};

struct ReconstructedMesh {
    CGAL::Surface_mesh<Point> mesh;
    std::vector<Point> vertices;
    std::vector<std::array<size_t, 3>> triangles;
};

struct ProjectedContour {
    const ContourPlane* originalPlane;
    const AxisPlanes::Plane* projectionPlane;
    std::vector<Point> projectedVertices;
    ReconstructedMesh reconstructedSurface;
    bool useExtendedMesh = false;
};
 
struct CellProjections {
    size_t cellIndex;
    std::vector<ProjectedContour> projections;
};

class Projection {
public:
    Projection(const SpacePartitioner& partitioner);
    
    size_t getCellCount() const { return m_cells.size(); }
    const std::vector<SpacePartitioner::ConvexCell>& getCells() const { return m_cells; }
    std::vector<ContourPlane> getPlanesForCell(size_t cellIndex) const;
    void debugPrintCellInfo() const;
    void renderPlanesForAllCells() const;
    const AxisPlanes& getAxisPlanesForCell(size_t cellIndex) const;
    void renderPlanesForCell(const CGAL::Polyhedron_3<ExactKernel>& poly) const;
    void renderAllReconstructions() const;

private:
    std::vector<SpacePartitioner::ConvexCell> m_cells;
    std::vector<ContourPlane> m_contourPlanes;
    std::unordered_map<size_t, AxisPlanes> m_cellPlanes;
    std::vector<CellProjections> m_projectedContours;

    ReconstructedMesh reconstructCellSurface(
    const std::vector<Point>& originalVertices,
    const std::vector<Point>& projectedVertices) const;
    ReconstructedMesh convertExtendedToReconstructedMesh(const ExtendedMesh& extMesh) const;
    ReconstructedMesh triangulateVertices(const std::vector<Point>& vertices) const;
    void reconstructSurface(ProjectedContour& projection);
    void renderReconstructedSurface(const ReconstructedMesh& mesh) const;
    double computePlaneDotProduct(const Plane& contourPlane, 
                                const AxisPlanes::Plane& axisPlane) const;
    const AxisPlanes::Plane* selectProjectionPlane(const ContourPlane& contourPlane,
                                                 const AxisPlanes& axisPlanes) const;
    std::vector<Point> projectVerticesOntoPlane(const std::vector<Point>& vertices,
                                              const AxisPlanes::Plane& plane) const;
    void computeProjections();
    AxisPlanes computeAxisAlignedPlanes(const CGAL::Polyhedron_3<ExactKernel>& poly) const;
    void renderAxisPlanes(const AxisPlanes& planes) const;
};

#endif