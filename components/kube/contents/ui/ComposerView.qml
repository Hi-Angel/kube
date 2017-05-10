/*
 *  Copyright (C) 2017 Michael Bohlender, <michael.bohlender@kdemail.net>
 *  Copyright (C) 2017 Christian Mollekopf, <mollekopf@kolabsys.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


import QtQuick 2.7
import QtQuick.Controls 1.3
import QtQuick.Controls 2.0 as Controls2
import QtQuick.Layouts 1.1

import org.kube.framework 1.0 as Kube

Kube.View {
    id: root

    property bool loadAsDraft: false
    property variant message: {}

    //FIXME mean hack to unfuck hiding
    property variant _composerController: Kube.ComposerController {
        id: composerController
        onDone: Kube.Fabric.postMessage(Kube.Messages.componentDone, {})
    }

    //actions
    property variant sendAction: composerController.sendAction
    property variant saveAsDraftAction: composerController.saveAsDraftAction

    Component.onCompleted: loadMessage(root.message, root.loadAsDraft)

    function loadMessage(message, loadAsDraft) {
        if (message) {
            composerController.loadMessage(message, loadAsDraft)
        }
    }

    //Drafts
    Rectangle {
        width: Kube.Units.gridUnit * 20
        Layout.minimumWidth: Kube.Units.gridUnit * 5
        anchors {
            top: parent.top
            bottom: parent.bottom
        }

        color: Kube.Colors.textColor
        focus: true

        ColumnLayout {
            anchors {
                fill: parent
                margins: Kube.Units.largeSpacing
            }
            spacing: Kube.Units.smallSpacing

            Kube.PositiveButton {
                anchors {
                    left: parent.left
                    right: parent.right
                    margins: Kube.Units.largeSpacing
                }
                text: qsTr("New Mail")
                onClicked: root.incrementCurrentIndex()
            }
            Kube.Label{
                text: qsTr("Drafts")
                color: Kube.Colors.highlightedTextColor
            }
            ListView {
                id: listView
                Layout.fillHeight: true
                anchors {
                    left: parent.left
                    right: parent.right
                }

                clip: true
                focus: true

                // Controls2.ScrollBar.vertical: Controls2.ScrollBar {
                //     id: scrollbar
                // }

                //BEGIN keyboard nav
                onActiveFocusChanged: {
                    if (activeFocus && currentIndex < 0) {
                        currentIndex = 0
                    }
                }

                Keys.onDownPressed: {
                    listView.incrementCurrentIndex()
                }
                Keys.onUpPressed: {
                    listView.decrementCurrentIndex()
                }
                //END keyboard nav

                onCurrentItemChanged: {
                    root.loadMessage(currentItem.currentData.domainObject, true)
                }

                model: Kube.MailListModel {
                    id: mailListModel
                    showDrafts: true
                }

                delegate: Item {
                    property variant currentData: model

                    width: delegateRoot.width
                    height: delegateRoot.height

                    Item {
                        id: delegateRoot

                        property variant mail : model.domainObject

                        // width: scrollbar.visible ? listView.width - scrollbar.width : listView.width
                        width: listView.width
                        height: Kube.Units.gridUnit * 3

                        states: [
                        State {
                            name: "selected"
                            when: listView.currentIndex == index

                            PropertyChanges {target: background; color: Kube.Colors.highlightColor}
                            PropertyChanges {target: subject; color: Kube.Colors.highlightedTextColor}
                        },
                        State {
                            name: "hovered"
                            when: ( mouseArea.containsMouse || buttons.containsMouse )

                            PropertyChanges {target: background; color: Kube.Colors.highlightColor; opacity: 0.6}
                            PropertyChanges {target: subject; color: Kube.Colors.highlightedTextColor}
                        }
                        ]

                        MouseArea {
                            id: mouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: listView.currentIndex = index
                        }

                        Rectangle {
                            id: background
                            anchors.fill: parent
                            color: Kube.Colors.viewBackgroundColor
                            border.color: Kube.Colors.backgroundColor
                            border.width: 1
                        }

                        Item {
                            id: content

                            anchors {
                                top: parent.top
                                bottom: parent.bottom
                                left: parent.left
                                right: parent.right
                                margins: Kube.Units.smallSpacing
                            }

                            Column {
                                anchors {
                                    verticalCenter: parent.verticalCenter
                                    left: parent.left
                                    leftMargin: Kube.Units.largeSpacing
                                }

                                Kube.Label{
                                    id: subject
                                    width: content.width - Kube.Units.gridUnit * 3
                                    text: model.subject
                                    color: model.unread ? Kube.Colors.highlightColor : Kube.Colors.textColor
                                    maximumLineCount: 2
                                    wrapMode: Text.WrapAnywhere
                                    elide: Text.ElideRight
                                }
                            }

                            Kube.Label {
                                id: date

                                anchors {
                                    right: parent.right
                                    bottom: parent.bottom
                                }
                                text: Qt.formatDateTime(model.date, "dd MMM yyyy")
                                font.italic: true
                                color: Kube.Colors.disabledTextColor
                                font.pointSize: 9
                            }
                        }
                    }
                }
            }
        }
    }

    //Content
    Rectangle {
        Layout.fillWidth: true
        Layout.minimumWidth: Kube.Units.gridUnit * 5
        anchors {
            top: parent.top
            bottom: parent.bottom
        }

        ColumnLayout {
            anchors {
                fill: parent
                margins: Kube.Units.largeSpacing
                leftMargin: Kube.Units.largeSpacing + Kube.Units.gridUnit * 2
                rightMargin: Kube.Units.largeSpacing + Kube.Units.gridUnit * 2
            }
            Kube.TextField {
                id: subject
                Layout.fillWidth: true

                placeholderText: "Enter Subject..."
                text: composerController.subject
                onTextChanged: composerController.subject = text;
            }

            Controls2.TextArea {
                id: content
                Layout.fillWidth: true
                Layout.fillHeight: true

                text: composerController.body
                onTextChanged: composerController.body = text;
            }
        }
    }

    //Recepients
    Rectangle {
        width: Kube.Units.gridUnit * 20
        Layout.minimumWidth: Kube.Units.gridUnit * 5
        anchors {
            top: parent.top
            bottom: parent.bottom
        }
        ColumnLayout {
            anchors {
                fill: parent
                margins: Kube.Units.largeSpacing
            }
            width: parent.width

            Kube.Label {
                text: "Sending Email to"
            }
            Kube.AutocompleteLineEdit {
                id: to
                Layout.fillWidth: true
                text: composerController.to
                onTextChanged: composerController.to = text
                model: composerController.recipientCompleter.model
                onSearchTermChanged: composerController.recipientCompleter.searchString = searchTerm
            }

            Kube.Label {
                text: "Sending Copy to (CC)"
            }
            Kube.AutocompleteLineEdit {
                id: cc
                Layout.fillWidth: true
                text: composerController.cc
                onTextChanged: composerController.cc = text
                model: composerController.recipientCompleter.model
                onSearchTermChanged: composerController.recipientCompleter.searchString = searchTerm
            }

            Kube.Label {
                text: "Sending Secret Copy to (Bcc)"
            }
            Kube.AutocompleteLineEdit {
                id: bcc
                Layout.fillWidth: true
                text: composerController.bcc
                onTextChanged: composerController.bcc = text;
                model: composerController.recipientCompleter.model
                onSearchTermChanged: composerController.recipientCompleter.searchString = searchTerm
            }

            Item {
                Layout.fillHeight: true
            }


            Item {
                Layout.fillHeight: true
            }


            Kube.Button {
                id: saveDraftButton

                text: "Save as Draft"
                //TODO enabled: saveAsDraftAction.enabled
                onClicked: {
                    saveAsDraftAction.execute()
                }
            }
            Kube.Button {
                text: "Discard"
                onClicked: Kube.Fabric.postMessage(Kube.Messages.componentDone, {})
            }

            Kube.Label {
                text: "You are sending this from:"
            }
            Kube.ComboBox {
                id: identityCombo
                model: composerController.identitySelector.model
                textRole: "displayName"
                Layout.fillWidth: true
                onCurrentIndexChanged: {
                    composerController.identitySelector.currentIndex = currentIndex
                }
            }

            Kube.PositiveButton {
                width: saveDraftButton.width

                text: "Send"
                enabled: sendAction.enabled
                onClicked: {
                    sendAction.execute()
                }
            }
        }
    }
}