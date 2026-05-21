import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Popup {
    id: emojiPicker

    signal emojiPicked(string emoji)

    width: 340
    height: 280
    modal: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    // Emoji categories
    property var categories: [
        {
            name: "Smileys",
            emojis: ["😀","😃","😄","😁","😆","😅","🤣","😂","🙂","😊","😇","🥰","😍","🤩","😘","😗","😚","😋","😛","😜","🤪","😝","🤑","🤗","🤭","🤫","🤔","🤐","🤨","😐","😑","😶","😏","😒","🙄","😬","🤥","😌","😔","😪","🤤","😴","😷","🤒","🤕","🤢","🤮","🥴","😵","🤯","🥳","🥺","😢","😭","😤","😠","😡","🤬","💀","☠️"]
        },
        {
            name: "Gestures",
            emojis: ["👋","🤚","🖐","✋","🖖","👌","🤌","🤏","✌️","🤞","🤟","🤘","🤙","👈","👉","👆","🖕","👇","☝️","👍","👎","✊","👊","🤛","🤜","👏","🙌","👐","🤲","🤝","🙏","✍️","💅","🤳","💪","🦵","🦶","👂","🦻","👃","🧠","🫀","🫁","🦷","🦴","👀","👁","👅","👄"]
        },
        {
            name: "Objects",
            emojis: ["❤️","🧡","💛","💚","💙","💜","🖤","🤍","🤎","💔","❣️","💕","💞","💓","💗","💖","💘","💝","🌟","✨","⭐️","🔥","💥","💫","💦","💨","🕳️","💣","💬","👁️‍🗨️","🗨️","🗯️","💭","💤","🎵","🎶","🎶","🏆","🥇","🥈","🥉","🏅","🎖️","🎗️","🎪","🎭","🎨","🎬","🎤","🎧","🎼","🎹","🥁","🎷","🎺","🪇","🎸","🪕","🎻","🎲","♟️","🎯","🎳","🎮","🕹️"]
        }
    ]

    property int currentCategory: 0

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8

        // Category tabs
        RowLayout {
            Layout.fillWidth: true
            spacing: 4

            Repeater {
                model: categories.length

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    radius: 4
                    color: currentCategory === index ? appCore.themeManager.primary : appCore.themeManager.surfaceAlt

                    Text {
                        anchors.centerIn: parent
                        text: categories[index].emojis[0]
                        font.pixelSize: 16
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: currentCategory = index
                    }
                }
            }
        }

        // Emoji grid
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            GridView {
                id: emojiGrid
                model: categories[currentCategory].emojis
                cellWidth: 40
                cellHeight: 40
                width: parent.width

                delegate: Item {
                    width: emojiGrid.cellWidth
                    height: emojiGrid.cellHeight

                    Text {
                        anchors.centerIn: parent
                        text: modelData
                        font.pixelSize: 22
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            emojiPicker.emojiPicked(modelData)
                            emojiPicker.close()
                        }
                    }
                }
            }
        }
    }
}
