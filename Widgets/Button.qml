/*
 * Copyright 2012 Canonical Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 1.1
import TelephonyApp 0.1

/*!
    \qmlclass Button
    \inqmlmodule UbuntuUIToolkit
    \brief The Button class is DOCME

    \b{This component is under heavy development.}

    Examples:
    \qml
        Button {
            text: "Send"
            onClicked: print("clicked text-only Button")
        }

        Button {
            iconSource: "call_icon.png"
            color: "green"
            onClicked: print("clicked icon-only Button")
        }

        Button {
            iconSource: "call_icon.png"
            text: "Icon on left"
            iconPosition: "left"
            onClicked: print("clicked text and icon Button")
        }
    \endqml
*/
ButtonWithForeground {
    id: button

    width: 87
    height: 39

    /*!
       \preliminary
       DOCME
    */
    property color color: "#e3e5e8"

    /*!
       \preliminary
       DOCME
    */
    property color pressedColor: color

    /*!
       \preliminary
       DOCME
    */
    property bool darkBorder: false

    // pick either a clear or dark text color depending on the luminance of the
    // background color to maintain good contrast (works in most cases)
    textColor: __luminance(base.color) <= 0.72 ? "white" : "#757373"

    function __luminance(hexcolor){
        hexcolor = String(hexcolor)
        var r = parseInt(hexcolor.substr(1,2),16);
        var g = parseInt(hexcolor.substr(3,2),16);
        var b = parseInt(hexcolor.substr(5,2),16);
        return ((r*212)+(g*715)+(b*73))/1000/255;
    }

    /* If the height of the button is smaller than the height of the shape,
       scale the shape down by streching it entirely instead of conserving its
       borders.
    */
    property bool __stretching: height < shape.sourceSize.height

    Item {
        z: -1
        anchors.fill: parent

        // FIXME: think of using distance fields
        BorderImage {
            id: shape

            anchors.fill: parent
            visible: false

            horizontalTileMode: BorderImage.Stretch
            verticalTileMode: BorderImage.Stretch
            source: button.darkBorder ? "artwork/ButtonShapeDark.png" : "artwork/ButtonShape.png"
            border {
                left: button.__stretching ? 0 : 18
                right: button.__stretching ? 0 : 18
                top: button.__stretching ? 0 : 15
                bottom: button.__stretching ? 0 : 15
            }
            smooth: true
        }

        // FIXME: might become a paper texture
        Rectangle {
            id: base

            anchors.fill: shape
            color: button.state != "pressed" ? button.color : button.pressedColor

            effect: ButtonMaskEffect {
                gradientStrength: button.state != "pressed" ? (button.darkBorder ? 0.5 : 0.36) : 0.0
                dark: button.darkBorder
                Behavior on gradientStrength {NumberAnimation {duration: 100; easing.type: Easing.OutQuad}}

                mask: shape
            }
        }

        // FIXME: could be generated from the shape (shadow parameters specified in guidelines)
        BorderImage {
            id: border

            anchors.fill: parent
            horizontalTileMode: BorderImage.Stretch
            verticalTileMode: BorderImage.Stretch
            source: if (button.darkBorder) return button.state == "pressed" ? "artwork/ButtonBorderDarkPressed.png" : "artwork/ButtonBorderDarkIdle.png"
                    else return button.state == "pressed" ? "artwork/ButtonBorderPressed.png" : "artwork/ButtonBorderIdle.png"
            border {
                left: button.__stretching ? 0 : 14
                right: button.__stretching ? 0 : 15
                top: button.__stretching ? 0 : 17
                bottom: button.__stretching ? 0 : 18
            }
            smooth: true
        }
    }
}
