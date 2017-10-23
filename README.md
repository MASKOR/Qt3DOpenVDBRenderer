# Qt3DOpenVDBRenderer
Visualize OpenVDB Grids (Level Sets) in Qt3D

Adds Qt3D GeometryRenderer-Components for OpenVDB Grids. Moreover a Qml-OpenVDB vdb-reader exists. The compiled library can be used by a program, that does not need openvdb-headers.

![alt text](https://github.com/MASKOR/Qt3DOpenVDBRenderer/raw/master/screenshot.png "Example")

## Features

* fast, parallelized surface generation
* surface is rendered as dense pointcloud
* three methods for surface generation, varying in speed and quality
* normal generation
* surfel shader

## Qml Types

* QOpenVDBGrid: Wraps openvdb::GridBase
* QOpenVDBGridReader: Reads the first grid of a vdb file.
* QOpenVDBGridGeometry: Can be used as Qt3DRender::QGeometry to render a LevelSet/FloatGrid using Qt3DRender::GeometryRenderer

You can choose a OpenVDBGridPointSurfaceGeometry::generationMethod
* OpenVDBGridPointSurfaceGeometry.MarchingCubesCenter for optimal adaption to the real surface
* OpenVDBGridPointSurfaceGeometry.VoxelCenter for fast surface generation with a voxel-like look
* OpenVDBGridPointSurfaceGeometry.Surrounding for debugging purposes. This needs a bit more work for coloring inside and outside voxels
* future: In addition to OpenVDBGridPointSurfaceGeometry, there might be OpenVDBGridMeshSurfaceGeometry introduced

## Usage

Register Qml-Types:

    qmlRegisterType<QOpenVDBGridReader>("openvdb", 1, 0, "OpenVDBReader");
    qmlRegisterType<QOpenVDBGrid>("openvdb", 1, 0, "OpenVDBGrid");
    qmlRegisterType<QOpenVDBGridPointSurfaceGeometry>("openvdb", 1, 0, "OpenVDBGridPointSurfaceGeometry");

Read OpenVDB Grid in Qml:

    OpenVDBReader {
        id: ovdbReader
        filename: "data/bunny.vdb"
    }

At the moment OpenVDB Grids are rendered as dense pointclouds. This seems to be the fastest/nices way of visualizing OpenVDB Levelset, since this is the method Houdini uses.
Make sure your Entity and Framegraph is configured correctly to render points.

    GeometryRenderer {
        id: bunnyRendererMarchingCubes
        geometry: OpenVDBGridPointSurfaceGeometry {
            grid: ovdbReader.grid
            generationMethod: OpenVDBGridPointSurfaceGeometry.MarchingCubesCenter
        }
        primitiveType: GeometryRenderer.Points
    }

The layer is needed to identify points in the framegraph:

    FrameGraph {
      (...)
        LayerFilter {
            layers: pointLayer
            StateSet {
                renderStates: [
                    PointSize { specification: PointSize.Programmable },
                    DepthTest { func: DepthTest.Less },
                    DepthMask { mask: true }
                ]
            }
        }
      (...)
    }

Instead of PerVertexColorMaterial, a custom shader can be used to enable programmable per vertex point size. Choose a Pointsize near the size of a voxel to simulate a surface.
Example comes with two shaders.

    property Material materialPoint: Material {
        effect: Effect {
            techniques: Technique {
                renderPasses: RenderPass {
                    shaderProgram: ShaderProgram {
                        vertexShaderCode: loadSource("qrc:/shader/pointcloud.vert")
                        fragmentShaderCode: loadSource("qrc:/shader/pointcloud.frag")
                    }
                }
            }
        }
        parameters: Parameter { name: "pointSize"; value: 0.1 }
    }

