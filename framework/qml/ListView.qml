/*
 *  Copyright (C) 2016 Michael Bohlender, <michael.bohlender@kdemail.net>
 *  Copyright (C) 2017 Christian Mollekopf, <mollekopf@kolabsystems.com>
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
import QtQuick.Controls 2

ListView {
    id: root
    property var focusProxy: root

    /*
     * The MouseArea + interactive: false + maximumFlickVelocity are required
     * to fix scrolling for desktop systems where we don't want flicking behaviour.
     *
     * See also:
     * ScrollView.qml in qtquickcontrols
     * qquickwheelarea.cpp in qtquickcontrols
     */
    MouseArea {
        anchors.fill: root
        propagateComposedEvents: true

        onWheel: {
            //Some trackpads (mine) emit 0 events in between that we can safely ignore.
            if (wheel.pixelDelta.y) {
                //120 is apparently the factor used in windows(https://chromium.googlesource.com/chromium/src/+/70763eb93a32555910a3b4269aeec51252ab9ec6/ui/events/event.cc)
                listView.flick(0, wheel.pixelDelta.y * 120)
            } else if (wheel.angleDelta.y) {
                //Arbitrary but this seems to work for me...
                listView.flick(0, wheel.angleDelta.y * 10)
            }
        }
    }
    interactive: false
    maximumFlickVelocity: 100000

    boundsBehavior: Flickable.StopAtBounds
}
