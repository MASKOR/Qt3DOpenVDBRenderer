import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.1
import Qt3D.Extras 2.9
import QtQuick.Scene2D 2.9
import Qt3D.Core 2.0
import Qt3D.Render 2.0

Entity {
    id: textLabelEntity
    property alias textureWidth: offscreenTextureText.width
    property alias textureHeight: offscreenTextureText.height
//    readonly property alias texture: offscreenTextureText
//    readonly property alias textureMaterial: textMaterial
    property alias text: innerTextLabel.text
    property Layer layer

    components: [textTransform, textMaterial, planeMesh, layer]

    PlaneMesh {
        id: planeMesh
        mirrored: true
    }
    Transform {
        function multiplyQuaternion(q1, q2) {
            return Qt.quaternion(q1.scalar * q2.scalar - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z,
                                 q1.scalar * q2.x + q1.x * q2.scalar + q1.y * q2.z - q1.z * q2.y,
                                 q1.scalar * q2.y + q1.y * q2.scalar + q1.z * q2.x - q1.x * q2.z,
                                 q1.scalar * q2.z + q1.z * q2.scalar + q1.x * q2.y - q1.y * q2.x);
        }
        property quaternion textLabelRotation: multiplyQuaternion(fromAxisAndAngle(Qt.vector3d(0, 1, 0), 180), fromAxisAndAngle(Qt.vector3d(1, 0, 0), 90))
        id: textTransform
        rotation: textLabelRotation
        scale3D: Qt.vector3d(10, 1,1)//10*(textLabelEntity.height/textLabelEntity.width), 1)
    }

    TextureMaterial {
        id: textMaterial
        texture: offscreenTextureText
    }
    Scene2D {
        id: qmlTexture
        output: RenderTargetOutput {
            attachmentPoint: RenderTargetOutput.Color0
            texture: Texture2D {
                id: offscreenTextureText
                width: 640
                height: 64
                format: Texture.RGBA8_UNorm
                generateMipMaps: true
                maximumAnisotropy: 16
                magnificationFilter: Texture.Linear
                minificationFilter: Texture.LinearMipMapLinear
                wrapMode {
                    x: WrapMode.ClampToEdge
                    y: WrapMode.ClampToEdge
                }
            }
        }
        mouseEnabled: false

        Rectangle {
            id: root
            color: "transparent"
            focus: true
            enabled: false
            width: offscreenTextureText.width
            height: offscreenTextureText.height
            RowLayout {
                anchors.fill: parent
                Text {
                    id: innerTextLabel
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    renderType: Text.NativeRendering // avoid subpixel rendering
                    font.pixelSize: offscreenTextureText.height*0.5
                    text: "not set"
                }
            }
        }
    }

}

