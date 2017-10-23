import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0
import QtQuick.Scene3D 2.0
import Qt3D.Core 2.0 as Q3D
import Qt3D.Render 2.0
import Qt3D.Input 2.0
import Qt3D.Extras 2.9
import QtQuick.Scene2D 2.9

import openvdb 1.0

ApplicationWindow {
    id: window
    title: qsTr("Map Visualization")
    width: 1200
    height: 800
    visible: true

    OpenVDBReader {
        id: ovdbReader
        filename: "data/bunny.vdb"
    }

    GridLayout {
        anchors.fill: parent
        Scene3D {
            id: scene3d
            Layout.minimumWidth: 50
            Layout.fillWidth: true
            Layout.fillHeight: true
            aspects: ["input", "logic"]
            cameraAspectRatioMode: Scene3D.AutomaticAspectRatio
            focus: true
            Q3D.Entity {
                id: sceneRoot

                Camera {
                    id: mainCamera
                    projectionType: CameraLens.PerspectiveProjection
                    fieldOfView: 75
                    aspectRatio: scene3d.width/scene3d.height
                    nearPlane : 0.1
                    farPlane : 1000.0
                    position: Qt.vector3d( 0.0, 0.0, -20.0 )
                    upVector: Qt.vector3d( 0.0, 1.0, 0.0 )
                    viewCenter: Qt.vector3d( 0.0, 0.0, 0.0 )
                }

                FirstPersonCameraController {
                //OrbitCameraController {
                    camera: mainCamera
                }

                components: [
                    RenderSettings {
                        activeFrameGraph: Viewport {
                            id: viewport
                            normalizedRect: Qt.rect(0.0, 0.0, 1.0, 1.0) // From Top Left
                            RenderSurfaceSelector {
                                CameraSelector {
                                    id : cameraSelector
                                    camera: mainCamera
                                    FrustumCulling {
                                        ClearBuffers {
                                            buffers : ClearBuffers.ColorDepthBuffer
                                            clearColor: "white"
                                            NoDraw {}
                                        }
                                        LayerFilter {
                                            layers: solidLayer
                                        }
                                        LayerFilter {
                                            layers: pointLayer
                                            RenderStateSet {
                                                renderStates: [
                                                    // If this is uncommented, following pointsizes are ignored in Qt5.7
                                                    //PointSize { sizeMode: PointSize.Fixed; value: 5.0 }, // exception when closing application in qt 5.7. Moreover PointSize
                                                    PointSize { sizeMode: PointSize.Programmable }, //supported since OpenGL 3.2
                                                    DepthTest { depthFunction: DepthTest.Less }
                                                    //DepthMask { mask: true }
                                                ]
                                            }
                                        }
                                        LayerFilter {
                                            layers: surfelLayer
                                            RenderStateSet {
                                                renderStates: [
                                                    PointSize { sizeMode: PointSize.Programmable }, //supported since OpenGL 3.2
                                                    DepthTest { depthFunction: DepthTest.Less }
                                                    //DepthMask { mask: true }
                                                ]
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    },
                    // Event Source will be set by the Qt3DQuickWindow
                    InputSettings {
                        eventSource: window
                        enabled: true
                    }
                ]
                Layer {
                    id: solidLayer
                }
                Layer {
                    id: pointLayer
                }
                Layer {
                    id: surfelLayer
                }
                Q3D.Transform {
                    id: torusTransform
                    //scale3D: Qt.vector3d(2.5, 2.5, 2.5)
                    translation: Qt.vector3d(0, 0, 10)
                }
                PhongMaterial {
                    id: phongMaterial
                }

                TorusMesh {
                    id: torusMesh
                    radius: 5
                    minorRadius: 1
                    rings: 100
                    slices: 20
                }
                Q3D.Entity {
                    id: torusEntity
                    components: [ solidLayer, torusMesh, phongMaterial, torusTransform ]
                }

                Q3D.Transform {
                    id: floorTransform
                    scale3D: Qt.vector3d(100, 100, 100)
                    translation: Qt.vector3d(0, -5, 0)
                }
                PlaneMesh {
                    id: planeMesh
                }
                Q3D.Entity {
                    id: floorEntity
                    components: [ solidLayer, planeMesh, phongMaterial, floorTransform ]
                }

                Material {
                    id: surfelMaterial
                    effect: Effect {
                        techniques: Technique {
                            renderPasses: RenderPass {
                                shaderProgram: ShaderProgram {
                                    vertexShaderCode: loadSource("qrc:/shader/surfel.vert")
                                    fragmentShaderCode: loadSource("qrc:/shader/surfel.frag")
                                }
                            }
                        }
                    }
                    parameters: [
                        Parameter { name: "pointSize"; value: 0.03 },
                        Parameter { name: "fieldOfView"; value: mainCamera.fieldOfView },
                        Parameter { name: "fieldOfViewVertical"; value: mainCamera.fieldOfView/mainCamera.aspectRatio },
                        Parameter { name: "nearPlane"; value: mainCamera.nearPlane },
                        Parameter { name: "farPlane"; value: mainCamera.farPlane },
                        Parameter { name: "width"; value: scene3d.width },
                        Parameter { name: "height"; value: scene3d.height }
                    ]
                }
                Material {
                    id: pointMaterial
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
                    parameters: [
                        Parameter { name: "pointSize"; value: 0.03 },
                        Parameter { name: "nearPlane"; value: mainCamera.nearPlane }
                    ]
                }

                GeometryRenderer {
                    id: bunnyRendererVoxelCenter
                    geometry: OpenVDBGridPointSurfaceGeometry {
                        grid: ovdbReader.grid
                        generationMethod: OpenVDBGridPointSurfaceGeometry.VoxelCenter
                    }
                    primitiveType: GeometryRenderer.Points
                }
                GeometryRenderer {
                    id: bunnyRendererMarchingCubes
                    geometry: OpenVDBGridPointSurfaceGeometry {
                        grid: ovdbReader.grid
                        generationMethod: OpenVDBGridPointSurfaceGeometry.MarchingCubesCenter
                    }
                    primitiveType: GeometryRenderer.Points
                }
                GeometryRenderer {
                    id: bunnyRendererSurrounding
                    geometry: OpenVDBGridPointSurfaceGeometry {
                        grid: ovdbReader.grid
                        generationMethod: OpenVDBGridPointSurfaceGeometry.Surrounding
                    }
                    primitiveType: GeometryRenderer.Points
                }

                Q3D.Entity {
                    id: bunnyVoxelCenter
                    property var myTransform: Q3D.Transform {
                        property vector3d pos: Qt.vector3d(-20, -4, 0)
                        property real userAngle: rotator.rotationAnimation
                        scale: 0.5
                        translation: pos
                        rotation: fromAxisAndAngle(Qt.vector3d(0, 1, 0), userAngle)
                    }
                    components: [ bunnyRendererVoxelCenter,
                                  pointMaterial,
                                  myTransform,
                                  pointLayer ]
                }
                Q3D.Entity {
                    id: bunnyMarchingCubes
                    property var myTransform: Q3D.Transform {
                        property vector3d pos: Qt.vector3d(0, -4, 0)
                        property real userAngle: rotator.rotationAnimation
                        scale: 0.5
                        translation: pos
                        rotation: fromAxisAndAngle(Qt.vector3d(0, 1, 0), userAngle)
                    }
                    components: [ bunnyRendererMarchingCubes,
                                  surfelMaterial,
                                  myTransform,
                                  surfelLayer ]
                }
                Q3D.Entity {
                    id: bunnySurrounding
                    property var myTransform: Q3D.Transform {
                        property vector3d pos: Qt.vector3d(20, -4, 0)
                        property real userAngle: rotator.rotationAnimation
                        scale: 0.5
                        translation: pos
                        rotation: fromAxisAndAngle(Qt.vector3d(0, 1, 0), userAngle)
                    }
                    components: [ bunnyRendererSurrounding,
                                  pointMaterial,
                                  myTransform,
                                  surfelLayer ]
                }
                Q3D.Entity {
                    TextLabelEntity {
                        text: qsTr("Point Shader; Voxel Center")
                        layer: solidLayer //TO DO: would be awesome if children are rendered, when parent is in "solidLayer" automatically
                    }

                    components: [text1Transform, solidLayer]
                    Q3D.Transform {
                        id: text1Transform
                        translation: bunnyVoxelCenter.myTransform.pos.plus( Qt.vector3d(0, 15, -8) )
                    }
                }
                Q3D.Entity {
                    TextLabelEntity {
                        text: qsTr("Surfel Shader; Marching Cubes Center")
                        layer: solidLayer
                    }
                    components: [text2Transform, solidLayer]
                    Q3D.Transform {
                        id: text2Transform
                        translation: bunnyMarchingCubes.myTransform.pos.plus( Qt.vector3d(0, 15, -8) )
                    }
                }
                Q3D.Entity {
                    TextLabelEntity {
                        text: qsTr("Point Shader; Surface Surrounding Voxels")
                        layer: solidLayer
                    }

                    components: [text3Transform, solidLayer]
                    Q3D.Transform {
                        id: text3Transform
                        translation: bunnySurrounding.myTransform.pos.plus( Qt.vector3d(0, 15, -8) )
                    }
                }
            }
        }
    }
    NumberAnimation {
        id: rotator
        property real rotationAnimation
        target: rotator
        property: "rotationAnimation"
        duration: 60000
        from: -180
        to: 180

        loops: Animation.Infinite
        running: true
    }

    SystemPalette {
        id: palette
    }
}
