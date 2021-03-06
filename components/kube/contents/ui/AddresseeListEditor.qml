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
import QtQuick.Layouts 1.1

import org.kube.framework 1.0 as Kube

FocusScope {
    id: root
    property variant completer
    property alias model: listView.model

    signal added(string text)
    signal removed(string text)

    implicitHeight: listView.height + lineEdit.height
    height: implicitHeight

    Column {
        anchors.fill: parent

        spacing: Kube.Units.smallSpacing

        ListView {
            id: listView
            anchors {
                left: parent.left
                right: parent.right
            }
            height: contentHeight
            spacing: Kube.Units.smallSpacing
            delegate: Rectangle {
                height: Kube.Units.gridUnit + Kube.Units.smallSpacing * 2 //smallSpacing for padding
                width: parent.width
                color: Kube.Colors.buttonColor
                Kube.Label {
                    anchors {
                        top: parent.top
                        left: parent.left
                        right: button.left
                        margins: Kube.Units.smallSpacing
                    }
                    text: display
                    elide: Text.ElideRight
                }
                Kube.IconButton {
                    id: button
                    anchors {
                        verticalCenter: parent.verticalCenter
                        right: parent.right
                        margins: Kube.Units.smallSpacing
                    }
                    height: Kube.Units.gridUnit
                    width: height
                    onClicked: root.removed(display);
                    padding: 0
                    iconName: Kube.Icons.remove
                }
            }
        }

        FocusScope {
            height: Kube.Units.gridUnit * Kube.Units.smallSpacing * 2
            width: parent.width
            focus: true

            Kube.TextButton {
                id: button
                text: "+ " + qsTr("Add recipient")
                textColor: Kube.Colors.highlightColor
                focus: true
                onClicked: {
                    lineEdit.visible = true
                    lineEdit.forceActiveFocus()
                }
            }

            Kube.AutocompleteLineEdit {
                id: lineEdit
                anchors {
                    left: parent.left
                    right: parent.right
                }
                visible: false

                placeholderText: "+ " + qsTr("Add recipient")
                model: root.completer.model
                onSearchTermChanged: root.completer.searchString = searchTerm
                onAccepted: {
                    root.added(text);
                    clear()
                    visible = false
                    button.forceActiveFocus()
                }
                onAborted: {
                    clear()
                    visible = false
                    button.forceActiveFocus()
                }
            }
        }
    }
}
