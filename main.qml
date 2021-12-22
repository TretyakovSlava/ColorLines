import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.12
import ItemModel 1.0

ApplicationWindow {
    id:window
    width: 800
    height: 700
    visible: true
    title: qsTr("ColorLines")
    property  bool started: false
    header:Pane {
        id:headerRect
        height: window.height/2
        Material.elevation: 2
        Label {
            id:label
            anchors.centerIn: parent
            text: "Color Lines"
            font.pixelSize: 24
            color: "#E91E63"
            font.family: perfogramaFont.name
        }
    }

    Item {
        id:states
        state: itemModel.state
        states: [
            State {
                name: "start"
                AnchorChanges { target: newGameBtn; anchors.horizontalCenter: parent.horizontalCenter; anchors.verticalCenter: parent.verticalCenter }
                PropertyChanges { target: footerRect; height: window.height / 2 }
                PropertyChanges { target: headerRect; height: window.height / 2 }
                PropertyChanges { target: scoreText; visible: false}
                PropertyChanges { target: scoreValue; visible: false}
                PropertyChanges { target: label; text: "Color Lines"}
            },
            State {
                name: "game"
                AnchorChanges { target: newGameBtn; anchors.left: footerRect.left; anchors.top:parent.top; anchors.verticalCenter: undefined; anchors.horizontalCenter: undefined}
                AnchorChanges { target: scoreText;  anchors.right: scoreValue.left; anchors.verticalCenter: footerRect.verticalCenter}
                AnchorChanges { target: scoreValue; anchors.right: footerRect.right; anchors.verticalCenter: footerRect.verticalCenter}
                PropertyChanges { target: footerRect; height: 50 }
                PropertyChanges { target: headerRect; height: 50 }
                PropertyChanges { target: scoreText; visible: true }
                PropertyChanges { target: scoreValue; visible: true}
            },
            State {
                name: "finish"
                when: itemModel.finished
                AnchorChanges { target: newGameBtn; anchors.horizontalCenter: footer.horizontalCenter }
                AnchorChanges { target: scoreText;  anchors.top: footerRect.top; anchors.horizontalCenter: footerRect.horizontalCenter}
                AnchorChanges { target: scoreValue;  anchors.top: footerRect.top; anchors.left:scoreText.right}
                PropertyChanges { target: footerRect; height: window.height / 2 }
                PropertyChanges { target: headerRect; height: window.height / 2 }
                PropertyChanges { target: scoreText; visible: true}
                PropertyChanges { target: scoreValue; visible: true}
                PropertyChanges { target: label; text: "Конец"}
            }
        ]

        transitions: Transition {
            AnchorAnimation { duration: 500 }
            PropertyAnimation{properties: "height"; duration: 500 }
        }
    }

    FontLoader {
        id: perfogramaFont
        source: "assets/fonts/Perfograma.otf"
    }

    ItemModel{
        id:itemModel
    }

    Pane{
        width: 600;
        height: 600;
        Material.elevation: 4
        anchors.centerIn: parent
        GridView {
            id:view
            width: 576;
            height: 576
            anchors.centerIn: parent
            cellWidth: 64;
            cellHeight: 64
            model: itemModel

            property var startItem : null;
            property var targetItem : null;
            property int targetIndex;

            delegate: ItemDelegate {
                id:item
                width: 64
                height: 64
                property string src : getSrcImageByColor(color)
                property alias pic : pic
                property int pos : index
                property bool startDeleteAnim : itemModel.toDelete.indexOf(index)!==-1;
                onSrcChanged:if(index != view.targetIndex)
                                 createAnim.running = true;
                Image {
                    width:60
                    height: 60
                    anchors.centerIn: parent
                    source:"assets/images/circles/empty.svg"
                }
                Rectangle {
                    width: 60
                    height: 60
                    radius: 30
                    color: "#E91E63"
                    visible: view.startItem == item && !moveAnim.running
                    anchors.centerIn: parent
                }
                Image {
                    id:pic
                    width:54
                    height: 54
                    anchors.centerIn: parent
                    source: src
                }
                onClicked: {
                    if(moveAnim.running)
                        return;

                    if(color != 0){
                        view.startItem = item;
                    } else if(view.startItem){
                        view.targetItem = item;
                        view.targetIndex = index;
                    }
                }

                NumberAnimation {id:createAnim; target: pic; property: "opacity"; from: 0; to: 1; duration: 400; easing.type: Easing.Linear }
                NumberAnimation {id:removeAnim; target: pic; property: "scale"; from: 1; to: 0; duration: 400; easing.type: Easing.InOutQuad;running: item.startDeleteAnim;
                    onFinished: {itemModel.clearColor(index); pic.scale = 1}}
            }

            Image {
                id:movePic
                width:54
                height: 54
                source: view.startItem ? view.startItem.src : ""
                visible: false
            }
            ParallelAnimation {
                id:moveAnim
                NumberAnimation { target: movePic; property: "x"; from: view.startItem ? view.startItem.x + view.startItem.pic.x : 0; to: view.targetItem ? view.targetItem.x + view.targetItem.pic.x : 0; duration: 500;easing.type: Easing.InOutQuad}
                NumberAnimation { target: movePic; property: "y"; from: view.startItem ? view.startItem.y + view.startItem.pic.y : 0; to: view.targetItem ? view.targetItem.y + view.targetItem.pic.y : 0; duration: 500;easing.type: Easing.InOutQuad }
                onStarted: {
                    movePic.visible = true;
                    view.startItem.pic.visible = false;
                }
                onFinished: {
                    itemModel.swap(view.startItem.pos, view.targetItem.pos);
                    view.startItem.pic.visible = true;
                    movePic.visible = false
                    view.targetItem = null;
                    view.startItem = null;
                    if(!itemModel.findLines(view.targetIndex))
                        itemModel.setupColors();
                }
                running: view.targetItem
            }
        }
    }

    function getSrcImageByColor(color){
        switch (color) {
        case 0:
            return "";
        case 1:
            return "assets/images/circles/green.svg";
        case 2:
            return "assets/images/circles/blue.svg";
        case 3:
            return "assets/images/circles/yellow.svg";
        case 4:
            return "assets/images/circles/brown.svg";
        }
    }


    footer:Rectangle{
        id:footerRect
        color: "#333"
        height: window.height/2
        ItemDelegate{
            id:newGameBtn
            width: 205
            text: "НОВАЯ ИГРА"
            font.family: perfogramaFont.name
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            font.pixelSize: 24
            Material.foreground: "#E91E63"
            onClicked: {itemModel.state = "game"; itemModel.restart()}
        }

        Text {
            id:scoreText
            text: "СЧЁТ"
            horizontalAlignment: Text.AlignRight
            font.pixelSize: 24
            color: "#E91E63"
            font.family: perfogramaFont.name
        }

        Text {
            id:scoreValue
            property int value: itemModel.score
            width:50
            text: value
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: 24
            color: "#E91E63"
            font.family: perfogramaFont.name
            visible:started
            Behavior on value { NumberAnimation { duration: 1000; easing.type: Easing.InOutQuad }}
        }
    }
}
