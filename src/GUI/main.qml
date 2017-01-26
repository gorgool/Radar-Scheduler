import QtQuick 2.7
import QtQml 2.2
import QtQuick.Controls 2.0
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.0
import QtQuick.Controls 1.4 as C14
import QtQuick.Dialogs 1.2


C14.ApplicationWindow
{
    id: mainWindow
    visible: true
    width: 1600
    height: 800

    title: qsTr("Планировщик")

    QtObject
    {
        id: custom_font
        property font main_text: Qt.font({
                                             family: "Segoi",
                                             pixelSize: 14
                                         })
    }

    statusBar: C14.StatusBar
    {
        RowLayout
        {
            anchors.fill: parent
            Label { id: error_text; text: error_string; font.bold: true; color: 'red'; }
        }

        implicitWidth: error_text.contentWidth
        implicitHeight: error_text.contentHeight + 6
    }

    FileDialog
    {
        id: fileDialog
        title: "Выберите файл сценария"

        nameFilters: [ "Json files (*.json)", "All files (*)" ]

        onAccepted:
        {
            if (SchedulerController.load_queries(fileUrl) === true)
            {
                nextBotton.enabled = true;
                nextManyBotton.enabled = true;
            }
        }
    }

    Column
    {
        id: listsColumn
        height: parent.height
        padding: 4
        topPadding: 0
        spacing: 2
        // Панель "Заявки"
        Rectangle
        {
            id : queryPanel
            border.width: 2
            radius: 6
            height: (parent.height - controlPanel.height - 8 - parent.spacing) * 0.5
            width: activeQueryBorder.width + execQueryBorder.width + showParamsBorder.width + 12

            Column
            {
                id: queryColumn
                anchors.fill: parent

                Text
                {
                    anchors.topMargin: 4
                    anchors.horizontalCenter: queryColumn.horizontalCenter
                    id: queryPanelTitle
                    font.family: "Segoi"
                    font.pixelSize: 16
                    text: "Завяки на обслуживание"
                }

                Row
                {
                    padding: 4
                    spacing: 2
                    // Все активные заявки
                    Rectangle
                    {
                        id : activeQueryBorder
                        height: queryColumn.height - queryPanelTitle.contentHeight - parent.padding - 4
                        border.width: 1
                        radius: 5
                        width: 240

                        Column
                        {
                            id: activeQueryColumn
                            padding: 4

                            Text
                            {
                                id: activeQueryTitle
                                font.family: "Segoi"
                                font.pixelSize: 12
                                text: "Очередь активных заявок"
                                anchors.horizontalCenter: parent.horizontalCenter
                            }

                            ListView
                            {
                                topMargin: 4
                                height: activeQueryBorder.height - activeQueryTitle.contentHeight - 8
                                width: activeQueryBorder.width - 4
                                id: queryList
                                spacing: 2
                                model: activeQueryListModel
                                delegate: Component
                                {
                                    Rectangle
                                    {
                                        id: itemBorder
                                        border.width: 1
                                        anchors.leftMargin: 2
                                        anchors.rightMargin: 2
                                        width: parent.width - 4
                                        height: idText.contentHeight + 8
                                        color: (queryList.focus === true && queryList.currentIndex === index) ? 'lightsteelblue' : 'white';

                                        MouseArea
                                        {
                                            anchors.fill: parent
                                            onPressed:
                                            {
                                                var currentSelectedItem;
                                                queryList.focus = true;
                                                queryList.currentIndex = index;
                                                currentSelectedItem = queryList.model.get(index);

                                                typeText.text = "Тип: " + currentSelectedItem.query_type;
                                                speedCoefText.text = "Ск. роста приоритета: " + currentSelectedItem.speed_coef;
                                                thresholdText.text = "Порог активации: " + currentSelectedItem.threshold;
                                                prevTimeText.text = "Предыдущее время: " + currentSelectedItem.prev_time;
                                                priorityText.text = "Значение приоритета: " + currentSelectedItem.priority.toFixed(4);
                                                rangeText.text = "Дальность: " + currentSelectedItem.range;
                                                rcsText.text = "ЭПР: " + currentSelectedItem.rcs;
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
                                clip: true
                                focus: true
                            }

                        }
                    }

                    // Исполненные на такте заявки
                    Rectangle
                    {
                        id : execQueryBorder
                        height: queryColumn.height - queryPanelTitle.contentHeight - parent.padding - 4
                        border.width: 1
                        radius: 5
                        width: 240

                        Column
                        {
                            id: execQueryColumn
                            padding: 4

                            Text
                            {
                                id: execQueryTitle
                                font.family: "Segoi"
                                font.pixelSize: 12
                                text: "Очередь исполненных заявок"
                                anchors.horizontalCenter: parent.horizontalCenter
                            }

                            ListView
                            {
                                topMargin: 4
                                height: execQueryBorder.height - execQueryTitle.contentHeight - 8
                                width: execQueryBorder.width - 4

                                id: execQueryList
                                spacing: 2
                                model: processedQueryListModel
                                delegate: Component
                                {
                                    Rectangle
                                    {
                                        id: itemBorder
                                        border.width: 1
                                        anchors.leftMargin: 2
                                        anchors.rightMargin: 2
                                        width: parent.width - 4
                                        height: idText.contentHeight + 8
                                        color: (execQueryList.focus === true && execQueryList.currentIndex === index) ? 'lightsteelblue' : 'white';

                                        MouseArea
                                        {
                                            anchors.fill: parent
                                            onPressed:
                                            {
                                                var currentSelectedItem;
                                                execQueryList.focus = true;
                                                execQueryList.currentIndex = index;
                                                currentSelectedItem = execQueryList.model.get(index);

                                                typeText.text = "Тип заявки: " + currentSelectedItem.query_type;
                                                speedCoefText.text = "Ск. роста приоритета: " + currentSelectedItem.speed_coef;
                                                thresholdText.text = "Порог активации: " + currentSelectedItem.threshold;
                                                prevTimeText.text = "Предыдущее время: " + currentSelectedItem.prev_time;
                                                priorityText.text = "Значение приоритета: " + currentSelectedItem.priority.toFixed(4);
                                                rangeText.text = "Дальность: " + currentSelectedItem.range;
                                                rcsText.text = "ЭПР: " + currentSelectedItem.rcs;
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
                                clip: true
                            }
                        }
                    }

                    // Параметры выбранной заявки
                    Rectangle
                    {
                        id : showParamsBorder
                        height: queryColumn.height - queryPanelTitle.contentHeight - parent.padding - 4
                        border.width: 1
                        radius: 5
                        width: 320

                        Text
                        {
                            id: showParamsTitle
                            font.family: "Segoi"
                            font.pixelSize: 12
                            text: "Параметры выбранной заявки"
                            anchors.horizontalCenter: parent.horizontalCenter
                        }


                        Column
                        {

                            topPadding: showParamsTitle.contentHeight + 10
                            spacing: 4
                            leftPadding: 8
                            width: parent.width
                            id: showParamsColumn

                            Text
                            {
                                id: typeText
                                font: custom_font.main_text
                            }
                            Text
                            {
                                id: speedCoefText
                                font: custom_font.main_text
                            }
                            Text
                            {
                                id: thresholdText
                                font: custom_font.main_text
                            }
                            Text
                            {
                                id: prevTimeText
                                font: custom_font.main_text
                            }
                            Text
                            {
                                id: priorityText
                                font: custom_font.main_text
                            }
                            Text
                            {
                                id: rangeText
                                font: custom_font.main_text
                            }
                            Text
                            {
                                id: rcsText
                                font: custom_font.main_text
                            }
                        }
                    }
                }
            }
        } // Окончание панель "Заявки"


        // Панель "Команды"
        Rectangle
        {
            id : commandPanel
            border.width: 2
            radius: 6
            anchors.topMargin: 4
            height: (parent.height - controlPanel.height - 8 - parent.spacing) * 0.5
            width: plannedCommandBorder.width + execCommandBorder.width + showCommandBorder.width + 12

            Column
            {
                id: commandColumn
                anchors.fill: parent

                Text
                {
                    anchors.topMargin: 4
                    anchors.horizontalCenter: commandColumn.horizontalCenter
                    id: commandPanelTitle
                    font.family: "Segoi"
                    font.pixelSize: 16
                    text: "Команды управления"
                }

                Row
                {
                    spacing: 2
                    padding: 4

                    // Все запланированные команды
                    Rectangle
                    {
                        id : plannedCommandBorder
                        height: commandColumn.height - commandPanelTitle.contentHeight - parent.padding - 4
                        border.width: 1
                        radius: 5
                        width: 240

                        Column
                        {
                            id: plannedCommandColumn
                            padding: 4

                            Text
                            {
                                id: plannedCommandTitle
                                font.family: "Segoi"
                                font.pixelSize: 12
                                text: "Очередь запланированных команд"
                                anchors.horizontalCenter: parent.horizontalCenter
                            }

                            ListView
                            {
                                id: plannedCommandList
                                topMargin: 4
                                height: plannedCommandBorder.height - plannedCommandTitle.contentHeight - 8
                                width: plannedCommandBorder.width - 4
                                spacing: 2
                                model: plannedCommandListModel
                                delegate: Component
                                {
                                    Rectangle
                                    {
                                        id: itemBorder
                                        border.width: 1
                                        anchors.leftMargin: 2
                                        anchors.rightMargin: 2
                                        width: parent.width - 4
                                        height: idText.contentHeight + 8
                                        color: (plannedCommandList.focus === true && plannedCommandList.currentIndex === index) ? 'lightsteelblue' : 'white';

                                        MouseArea
                                        {
                                            anchors.fill: parent
                                            onPressed:
                                            {
                                                plannedCommandList.focus = true;
                                                plannedCommandList.currentIndex = index;
                                                var currentSelectedItem = plannedCommandList.model.get(index);

                                                commandTypeText.text = "Тип: " + currentSelectedItem.command_type;
                                                commandReferanceTimeText.text = "Время пересылки: " + currentSelectedItem.referance_time;
                                                commandExecutionTimeText.text = "Время исполнения: " + currentSelectedItem.execution_time;
                                                commandExecutionLengthText.text = "Длительность исполнения: " + currentSelectedItem.execution_length;
                                            }
                                        }

                                        Text
                                        {
                                            id: idText
                                            font.family: "Segoi"
                                            font.pixelSize: 14
                                            text: "Id : " + command_query_id
                                            horizontalAlignment: Text.AlignHCenter
                                            verticalAlignment: Text.AlignVCenter
                                            anchors.horizontalCenter: parent.horizontalCenter
                                            height: contentHeight + 4
                                        }

                                    }
                                }
                                clip: true
                            }
                        }
                    }

                    // Исполненные на такте команды
                    Rectangle
                    {
                        id : execCommandBorder
                        height: commandColumn.height - commandPanelTitle.contentHeight - parent.padding - 4
                        border.width: 1
                        radius: 5

                        width: 240

                        Column
                        {
                            anchors.fill: parent
                            id: execCommandColumn
                            padding: 4

                            Text
                            {
                                id: execCommandTitle
                                font.family: "Segoi"
                                font.pixelSize: 12
                                text: "Очередь исполненных команд"
                                anchors.horizontalCenter: parent.horizontalCenter
                            }

                            ListView
                            {
                                id: execCommandList
                                topMargin: 4
                                height: plannedCommandBorder.height - plannedCommandTitle.contentHeight - 8
                                width: plannedCommandBorder.width - 4
                                spacing: 2
                                model: execedCommandListModel
                                delegate: Component
                                {
                                    Rectangle
                                    {
                                        id: itemBorder
                                        border.width: 1
                                        anchors.leftMargin: 2
                                        anchors.rightMargin: 2
                                        width: parent.width - 4
                                        height: idText.contentHeight + 8
                                        color: (execCommandList.focus === true && execCommandList.currentIndex === index) ? 'lightsteelblue' : 'white';

                                        MouseArea
                                        {
                                            anchors.fill: parent
                                            onPressed:
                                            {
                                                execCommandList.focus = true;
                                                execCommandList.currentIndex = index;
                                                var currentSelectedItem = execCommandList.model.get(index);

                                                commandTypeText.text = "Тип: " + currentSelectedItem.command_type;
                                                commandReferanceTimeText.text = "Время пересылки: " + currentSelectedItem.referance_time;
                                                commandExecutionTimeText.text = "Время исполнения: " + currentSelectedItem.execution_time;
                                                commandExecutionLengthText.text = "Длительность исполнения: " + currentSelectedItem.execution_length;
                                            }
                                        }

                                        Text
                                        {
                                            id: idText
                                            font.family: "Segoi"
                                            font.pixelSize: 14
                                            text: "Id : " + command_query_id
                                            horizontalAlignment: Text.AlignHCenter
                                            verticalAlignment: Text.AlignVCenter
                                            anchors.horizontalCenter: parent.horizontalCenter
                                            height: contentHeight + 4
                                        }

                                    }
                                }
                                clip: true
                            }
                        }
                    }

                    // Параметры выбранной команды
                    Rectangle
                    {
                        id : showCommandBorder
                        height: commandColumn.height - commandPanelTitle.contentHeight - parent.padding - 4
                        border.width: 1
                        radius: 5

                        width: 320

                        Text
                        {
                            id: showCommandTitle
                            font.family: "Segoi"
                            font.pixelSize: 12
                            text: "Параметры выбранной команды"
                            anchors.horizontalCenter: parent.horizontalCenter
                        }

                        Column
                        {
                            topPadding: showCommandTitle.contentHeight + 10
                            spacing: 4
                            leftPadding: 8
                            id: showCommandColumn

                            Text
                            {
                                id: commandTypeText
                                font: custom_font.main_text
                            }
                            Text
                            {
                                id: commandReferanceTimeText
                                font: custom_font.main_text
                            }
                            Text
                            {
                                id: commandExecutionTimeText
                                font: custom_font.main_text
                            }
                            Text
                            {
                                id: commandExecutionLengthText
                                font: custom_font.main_text
                            }
                        }
                    }
                }
            }
        } // Окончание панель "Команды"

        // Панель кнопок
        Rectangle
        {
            id : controlPanel
            border.width: 2
            radius: 6
            height: 70
            width: plannedCommandBorder.width + execCommandBorder.width + showCommandBorder.width + 12

            Text
            {
                anchors.topMargin: 4
                anchors.horizontalCenter: controlPanel.horizontalCenter
                id: controlPanelTitle
                font.family: "Segoi"
                font.pixelSize: 16
                text: "Управление исполнением"
            }

            Button
            {
                id: openBotton
                anchors.top: controlPanelTitle.bottom
                anchors.left: controlPanel.left
                anchors.leftMargin: 16
                anchors.topMargin: 6

                background: Rectangle
                {
                    Image
                    {
                        anchors.centerIn: parent
                        width: 24
                        height: 24
                        source: "open_file.png"
                    }
                    width: 32
                    height: 32
                    border.width: 2
                }

                onClicked:
                {
                    fileDialog.visible = true;
                }
            }



            // Вперед на 1 шаг
            Button
            {
                enabled: false
                id: nextBotton
                anchors.top: controlPanelTitle.bottom
                anchors.left: controlPanel.left
                anchors.leftMargin: 60
                anchors.topMargin: 8
                font.family: "Segoi"
                font.pixelSize: 14
                text: "Вперед"

                background: Rectangle
                {
                    implicitWidth: 100
                    implicitHeight: 25
                    border.width: 2
                }

                onClicked:
                {
                    SchedulerController.run_engine(1);
                }
            }

            // Вперед на N шагов
            Button
            {
                enabled: false
                id: nextManyBotton
                anchors.top: controlPanelTitle.bottom
                anchors.topMargin: 8
                anchors.left: nextBotton.right
                anchors.leftMargin: 32
                font.family: "Segoi"
                font.pixelSize: 14
                text: "Вперед на ..."

                background: Rectangle
                {
                    implicitWidth: 100
                    implicitHeight: 25
                    border.width: 2
                }

                onClicked:
                {
                    SchedulerController.run_engine(parseInt(stepCount.text));
                }
            }

            Connections
            {
                target: SchedulerController
                onDisableButtons:
                {
                    nextBotton.enabled = false;
                    nextManyBotton.enabled = false;
                }
            }

            // Поле ввода количества шагов
            Rectangle
            {
                id: stepCountInput
                anchors.top: controlPanelTitle.bottom
                anchors.topMargin: 8
                anchors.left: nextManyBotton.right
                anchors.leftMargin: 8
                border.width: 1

                width: 80
                height: 25 + 4

                TextInput
                {
                    id: stepCount
                    anchors.fill: parent
                    font.family: "Segoi"
                    font.pixelSize: 14
                    text: "100"
                    leftPadding: 12
                    padding: 6
                }
            }

            Text
            {
                id: modelTimeText
                text: "Модельное время: " + model_time
                anchors.top: controlPanelTitle.bottom
                anchors.topMargin: 8 + 4
                anchors.left: stepCountInput.right
                anchors.leftMargin: 32
                font.family: "Segoi"
                font.pixelSize: 14
            }

            Text
            {
                id: validFlagText
                text: "Признак валидации :"
                anchors.top: controlPanelTitle.bottom
                anchors.topMargin: 8 + 4
                anchors.left: stepCountInput.right
                anchors.leftMargin: 270
                font.family: "Segoi"
                font.pixelSize: 14
            }

            // Валидация
            Rectangle
            {
                id: validIcon
                anchors.top: controlPanelTitle.bottom
                anchors.topMargin: 8 + 6
                anchors.left: validFlagText.right
                anchors.leftMargin: 2
                width: 14
                height: width
                color: is_valid ? 'green' : 'red'
                radius: width * 0.5
            }

        }
    }

    Rectangle
    {
        id: timelinePanel
        height: listsColumn.height - 6
        width: parent.width - listsColumn.width - 4
        anchors.left: listsColumn.right
        anchors.top: mainWindow.top
        border.width: 2
        radius: 6

        Rectangle
        {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.leftMargin: 8
            anchors.topMargin: 8
            border.width: 2
            width: parent.width - 2 * anchors.leftMargin
            height: parent.height - 2 * anchors.topMargin

            // Легенда
            Rectangle
            {
                id: legend_box
                anchors.top : parent.top
                anchors.left: parent.left
                anchors.leftMargin: 20
                anchors.topMargin: 8
                width: parent.width - 2 * anchors.leftMargin
                height: legend_row.height + legend_title.contentHeight
                border.width: 2
                clip: true

                Text
                {
                    id: legend_title
                    font: custom_font.main_text
                    text: "Легенда"
                    anchors.top : parent.top
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Row
                {
                    id: legend_row
                    anchors.top: legend_title.bottom
                    padding: 8
                    spacing: 20

                    Rectangle
                    {
                        width: white_rect.width + white_text.contentWidth
                        height: white_rect.height + white_text.contentHeight
                        Rectangle
                        {
                            id: white_rect
                            y: 4
                            border.width: 1
                            width: 10
                            height: 10
                            color: 'white'
                        }
                        Text
                        {
                            id: white_text
                            anchors.left: white_rect.right
                            anchors.leftMargin: 4
                            font: custom_font.main_text
                            text: "Свободно"
                        }
                    }

                    Rectangle
                    {
                        width: red_rect.width + red_text.contentWidth
                        height: red_rect.height + red_text.contentHeight
                        Rectangle
                        {
                            id: red_rect
                            y: 4
                            border.width: 1
                            width: 10
                            height: 10
                            color: 'red'
                        }
                        Text
                        {
                            id: red_text
                            anchors.left: red_rect.right
                            anchors.leftMargin: 4
                            font: custom_font.main_text
                            text: "Излучение"
                        }
                    }
                    Rectangle
                    {
                        width: blue_rect.width + blue_text.contentWidth
                        height: blue_rect.height + blue_text.contentHeight
                        Rectangle
                        {
                            id: blue_rect
                            y: 4
                            border.width: 1
                            width: 10
                            height: 10
                            color: 'blue'
                        }
                        Text
                        {
                            anchors.left: blue_rect.right
                            anchors.leftMargin: 4
                            id: blue_text
                            font: custom_font.main_text
                            text: "Прием"
                        }
                    }
                    Rectangle
                    {
                        width: yellow_rect.width + yellow_text.contentWidth
                        height: yellow_rect.height + yellow_text.contentHeight
                        Rectangle
                        {
                            id: yellow_rect
                            y: 4
                            border.width: 1
                            width: 10
                            height: 10
                            color: 'yellow'
                        }
                        Text
                        {
                            anchors.left: yellow_rect.right
                            anchors.leftMargin: 4
                            id: yellow_text
                            font: custom_font.main_text
                            text: "Скважность"
                        }
                    }
                    Rectangle
                    {
                        width: dgreen_rect.width + dgreen_text.contentWidth
                        height: dgreen_rect.height + dgreen_text.contentHeight
                        Rectangle
                        {
                            id: dgreen_rect
                            y: 4
                            border.width: 1
                            width: 10
                            height: 10
                            color: 'darkGreen'
                        }
                        Text
                        {
                            anchors.left: dgreen_rect.right
                            anchors.leftMargin: 4
                            id: dgreen_text
                            font: custom_font.main_text
                            text: "Запрет управления АУ"
                        }
                    }
                    Rectangle
                    {
                        width: green_rect.width + green_text.contentWidth
                        height: green_rect.height + green_text.contentHeight
                        Rectangle
                        {
                            id: green_rect
                            y: 4
                            border.width: 1
                            width: 10
                            height: 10
                            color: 'green'
                        }
                        Text
                        {
                            anchors.left: green_rect.right
                            anchors.leftMargin: 4
                            id: green_text
                            font: custom_font.main_text
                            text: "Перефазировка АУ"
                        }
                    }
                    Rectangle
                    {
                        width: cyan_rect.width + cyan_text.contentWidth
                        height: cyan_rect.height + cyan_text.contentHeight
                        Rectangle
                        {
                            id: cyan_rect
                            y: 4
                            border.width: 1
                            width: 10
                            height: 10
                            color: 'cyan'
                        }
                        Text
                        {
                            anchors.left: cyan_rect.right
                            anchors.leftMargin: 4
                            id: cyan_text
                            font: custom_font.main_text
                            text: "Когерентный прием"
                        }
                    }
                }
            }

            Flickable
            {
                id: timeline_box
                property real timeline_width: 80050.0
                property real timeline_height: 30.0
                flickableDirection: Flickable.HorizontalFlick
                x: parent.border.width
                y: parent.border.width
                clip: true
                interactive: true
                boundsBehavior: Flickable.StopAtBounds
                contentHeight: timeline_height
                contentWidth: timeline_width
                anchors.top: legend_box.bottom
                anchors.topMargin: 8
                height: images_panel.height
                width: parent.width - 2*parent.border.width
                contentX: -2

                Column
                {
                    id: images_panel
                    padding: 4
                    spacing: 8

                    Text
                    {
                        font: custom_font.main_text
                        text: "КВД УЦП до момента размещения команд\t\t\t\t\t[" + dcu_before_occupation.toFixed(3) + "]"
                        x: timeline_box.contentX + 40
                    }
                    Row
                    {
                        Repeater
                        {
                            model: 81
                            delegate: Image
                            {
                                property string file_name: 'image://dcu_before/' + index
                                id: dcu_before_tile
                                width: 1000
                                height: 30
                                cache: false
                                source: 'image://dcu_before/' + index

                                Connections
                                {
                                    target: SchedulerController
                                    onImageReady:
                                    {
                                        dcu_before_tile.source = '';
                                        dcu_before_tile.source = dcu_before_tile.file_name;
                                    }
                                }
                            }
                        }
                    }

                    Text
                    {
                        font: custom_font.main_text
                        text: "КВД УЦП после момента размещения команд\t\t\t\t\t[" + dcu_after_occupation.toFixed(3) + "]"
                        x: timeline_box.contentX + 40
                    }
                    Row
                    {
                        Repeater
                        {
                            model: 81
                            delegate: Image
                            {
                                property string file_name: 'image://dcu_after/' + index
                                id: dcu_after_tile
                                width: 1000
                                height: 30
                                cache: false
                                source: 'image://dcu_after/' + index

                                Connections
                                {
                                    target: SchedulerController
                                    onImageReady:
                                    {
                                        dcu_after_tile.source = '';
                                        dcu_after_tile.source = dcu_after_tile.file_name;
                                    }
                                }
                            }
                        }
                    }

                    Text
                    {
                        font: custom_font.main_text
                        text: "КВД АУ передатчика до момента размещения команд\t\t\t\t[" + au_tr_before_occupation.toFixed(3) + "]"
                        x: timeline_box.contentX + 40
                    }
                    Row
                    {
                        Repeater
                        {
                            model: 81
                            delegate: Image
                            {
                                property string file_name: 'image://au_tr_before/' + index
                                id: au_tr_before_tile
                                width: 1000
                                height: 30
                                cache: false
                                source: 'image://au_tr_before/' + index

                                Connections
                                {
                                    target: SchedulerController
                                    onImageReady:
                                    {
                                        au_tr_before_tile.source = '';
                                        au_tr_before_tile.source = au_tr_before_tile.file_name;
                                    }
                                }
                            }
                        }
                    }

                    Text
                    {
                        font: custom_font.main_text
                        text: "КВД АУ передатчика после момента размещения команд\t\t\t\t[" + au_tr_after_occupation.toFixed(3) + "]"
                        x: timeline_box.contentX + 40
                    }
                    Row
                    {
                        Repeater
                        {
                            model: 81
                            delegate: Image
                            {
                                property string file_name: 'image://au_tr_after/' + index
                                id: au_tr_after_tile
                                width: 1000
                                height: 30
                                cache: false
                                source: 'image://au_tr_after/' + index

                                Connections
                                {
                                    target: SchedulerController
                                    onImageReady:
                                    {
                                        au_tr_after_tile.source = '';
                                        au_tr_after_tile.source = au_tr_after_tile.file_name;
                                    }
                                }
                            }
                        }
                    }

                    Text
                    {
                        font: custom_font.main_text
                        text: "КВД АУ приемника до момента размещения команд\t\t\t\t[" + au_rs_before_occupation.toFixed(3) + "]"
                        x: timeline_box.contentX + 40
                    }
                    Row
                    {
                        Repeater
                        {
                            model: 81
                            delegate: Image
                            {
                                property string file_name: 'image://au_rs_before/' + index
                                id: au_rs_before_tile
                                width: 1000
                                height: 30
                                cache: false
                                source: 'image://au_rs_before/' + index

                                Connections
                                {
                                    target: SchedulerController
                                    onImageReady:
                                    {
                                        au_rs_before_tile.source = '';
                                        au_rs_before_tile.source = au_rs_before_tile.file_name;
                                    }
                                }
                            }
                        }
                    }

                    Text
                    {
                        font: custom_font.main_text
                        text: "КВД АУ приемника после момента размещения команд\t\t\t\t[" + au_rs_after_occupation.toFixed(3) + "]"
                        x: timeline_box.contentX + 40
                    }
                    Row
                    {
                        Repeater
                        {
                            model: 81
                            delegate: Image
                            {
                                property string file_name: 'image://au_rs_after/' + index
                                id: au_rs_after_tile
                                width: 1000
                                height: 30
                                cache: false
                                source: 'image://au_rs_after/' + index

                                Connections
                                {
                                    target: SchedulerController
                                    onImageReady:
                                    {
                                        au_rs_after_tile.source = '';
                                        au_rs_after_tile.source = au_rs_after_tile.file_name;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            Rectangle
            {
                id: command_history
                anchors.top : timeline_box.bottom
                anchors.bottom : parent.bottom
                anchors.left: parent.left
                anchors.leftMargin: 20
                anchors.topMargin: 8
                anchors.bottomMargin: 8
                width: parent.width - 2 * anchors.leftMargin
                border.width: 2

                Text
                {
                    id: command_title
                    font: custom_font.main_text
                    text: "История команд"
                    anchors.top : parent.top
                    anchors.topMargin: 2
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Rectangle
                {
                    anchors.top: command_title.bottom
                    anchors.left: command_history.left
                    anchors.topMargin: 8
                    anchors.leftMargin: 8
                    border.width: 1
                    width: parent.width - 16
                    height: parent.height - command_title.contentHeight - 18

                    Flickable
                    {
                        id: command_list_area
                        flickableDirection: Flickable.VerticalFlick
                        x: parent.border.width
                        y: parent.border.width
                        clip: true
                        interactive: true
                        boundsBehavior: Flickable.StopAtBounds
                        contentHeight: command_list.contentHeight + 16
                        contentWidth: parent.width
                        anchors.fill: parent

                        TextArea
                        {
                            id: command_list
                            anchors.fill: parent
                            font: custom_font.main_text
                            selectByMouse : true
                            wrapMode: TextEdit.WordWrap
                            onTextChanged:
                            {
                                command_list_area.contentY = Math.max(command_list.contentHeight + 16 - command_list_area.height, 0);
                            }
                        }
                    }
                    Connections
                    {
                        target: SchedulerController
                        onCommandUpdate:
                        {
                            var max_length = 10000;

                            if (commands.size > max_length)
                            {
                                command_list.append(commands.mid(commands.size() - max_length).join("\n"));
                            }
                            else
                            {
                                if (command_list.lineCount > max_length)
                                {
                                    command_list.text = "";
                                }
                                command_list.append(commands.join("\n"));
                            }
                        }
                    }
                }

            }
        }
    }

}
