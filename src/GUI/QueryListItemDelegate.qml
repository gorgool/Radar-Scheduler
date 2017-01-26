import QtQuick 2.0

Component
{
    Rectangle
    {
        id: itemBorder
        border.width: 1
        anchors.leftMargin: 2
        anchors.rightMargin: 2
        width: parent.width - 4
        height: idText.contentHeight + 8
        color: (parent.parent.focus === true && parent.parent.currentIndex === index) ? 'lightsteelblue' : 'white';

        MouseArea
        {
            anchors.fill: parent
            onPressed:
            {
                parent.parent.parent.focus = true;
                parent.parent.parent.currentIndex = index;
            }
        }

        Text
        {
            id: idText
            font.family: "Segoi"
            font.pixelSize: 14
            text: "Id : " + query_id
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            anchors.horizontalCenter: parent.horizontalCenter
            height: contentHeight + 4
        }

    }
}
