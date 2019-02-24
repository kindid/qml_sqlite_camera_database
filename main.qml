import QtQuick 2.12
import QtQuick.Controls 2.4
import QtQuick.Window 2.12
import QtQuick.LocalStorage 2.12
import QtMultimedia 5.8

Window {
    visible: true
    width: 800
    height: 600
    title: qsTr("qml_sqlite_camera_database")
    TextInput {
        x: 0
        y: 0
        width: parent.width
        height: parent.height
        text: "hi there"
        onTextChanged: {
            // when this changes i want to get the latest version of this image - this is HARD
            // because you don't know the cache key - answer, look it up.
            print("text = ", text);
            print("url  = ", user_images.getUrl(text))
            image_thing.source = user_images.getUrl(text)
        }
    }

    Item {
        x: 0
        y: 20
        width: 400
        height: 300
        Camera {
            id: camera
            captureMode: Camera.CaptureStillImage
            imageCapture {
                onImageCaptured: {
                    // update preview
                    photoPreview.source = preview
                }
            }
        }
        VideoOutput {
            source: camera
            anchors.fill: parent
            //sourceRect: Qt.rect(camera.)
            focus : visible // to receive focus and capture key events when visible
        }
        MouseArea {
            anchors.fill: parent
            // when clicked perform a capture
            onClicked: camera.imageCapture.capture()
        }
    }
    // add a text edit here used to get/set as appropriate
    Image {
        x:400
        y:20
        width: 400
        height: 300
        id: photoPreview
        onSourceChanged: {
            grabToImage(function(result) {
                    // this uses our database object to update our database from a captured
                    // image of this "Image" object. It will return a new URL
                    // the "cache key" part differs although it not actually used in the
                    // lookup of the stored image - it is just there so that the URLs
                    // do not match as the underlying image system caches textures based
                    // on URL
                    var url = user_images.updateImage("first_key", result.image)
                    // point image to new image url
                    image_thing.source = url
                },
                Qt.size(width, height))
        }
    }
    Image {
        id:image_thing
        x : 200
        y : 300
        width: 400
        height: 300
        // this will return you a default image
        source: "image://user_images/"
    }
}
