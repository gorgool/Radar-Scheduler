#pragma once

#include <QDebug>
#include <QObject>
#include <qqmlcontext.h>
#include <QtQuickControls2/QtQuickControls2>
#include <QJsonDocument>
#include <cassert>
#include <thread>
#include "CommandRecords.h"
#include "QueryRecords.h"
#include "TimelineGraph.h"
#include "../Model_Engine/ModelState.h"
#include "../Model_Engine/ModelEngine.h"
#include "../Model_Engine/Settings.h"

/*
 * Класс для связывания моделей данных и сигналов GUI с системой моделирования
 */

class QMLSchedulerController : public QObject
{
    Q_OBJECT
    // ====== Модель планировщика ======
    ModelEngine model_eng;

    // ====== Модели данных QML =====

    // Активные заявки
    QueryList active_queries;

    // Исполненные заявки
    QueryList processed_queries;

    // Запланированные команды
    CommandList planned_commands;

    // Исполненные команды
    CommandList execed_commands;

    // Флаг валидации
    bool is_valid;

    // Модельное время
    std::uint64_t model_time;

    // Строка ошибок
    QString error_string;

    // ====== Графики КВД ======
    // УЦП до размещения команд
    TimelineGraph dcu_before;
    // УЦП после размещения команд
    TimelineGraph dcu_after;
    // АУ передатчика до размещения команд
    TimelineGraph au_tr_before;
    // АУ передатчика после размещения команд
    TimelineGraph au_tr_after;
    // АУ приемника до размещения команд
    TimelineGraph au_rs_before;
    // АУ приемника после размещения команд
    TimelineGraph au_rs_after;

    // ====== Контекст QML ======
    QQmlContext *ctxt;

public:
    QMLSchedulerController(QQmlContext* context, QObject *parent = 0) : QObject(parent), is_valid(false), model_time(0), ctxt(context)
    {
        assert(ctxt != nullptr);
        ctxt->setContextProperty("SchedulerController", this);
        ctxt->setContextProperty("activeQueryListModel", &active_queries);
        ctxt->setContextProperty("processedQueryListModel", &processed_queries);
        ctxt->setContextProperty("plannedCommandListModel", &planned_commands);
        ctxt->setContextProperty("execedCommandListModel", &execed_commands);
        ctxt->setContextProperty("error_string", QVariant(error_string));
        ctxt->setContextProperty("is_valid", QVariant(is_valid));
        ctxt->setContextProperty("model_time", QVariant("Not initialized"));

        ctxt->setContextProperty("dcu_before_occupation", QVariant(0));
        ctxt->setContextProperty("dcu_after_occupation", QVariant(0));
        ctxt->setContextProperty("au_tr_after_occupation", QVariant(0));
        ctxt->setContextProperty("au_tr_before_occupation", QVariant(0));
        ctxt->setContextProperty("au_rs_before_occupation", QVariant(0));
        ctxt->setContextProperty("au_rs_after_occupation", QVariant(0));


        ctxt->engine()->addImageProvider("dcu_before", &dcu_before);
        ctxt->engine()->addImageProvider("dcu_after", &dcu_after);
        ctxt->engine()->addImageProvider("au_tr_before", &au_tr_before);
        ctxt->engine()->addImageProvider("au_tr_after", &au_tr_after);
        ctxt->engine()->addImageProvider("au_rs_before", &au_rs_before);
        ctxt->engine()->addImageProvider("au_rs_after", &au_rs_after);
    }

    Q_INVOKABLE void run_engine(const quint32 times)
    {
        if (times != 0)
        {
            auto state = model_eng.run(times);
            active_queries.update_data(state.active_queries);
            processed_queries.update_data(state.processed_queries);
            planned_commands.update_data(state.planned_commands);
            execed_commands.update_data(state.execed_commands);

            QStringList commands;
            for (const auto& c : state.command_history)
            {
                commands.append(QString(c.c_str()));
            }
            if (commands.empty() == false)
                emit commandUpdate(commands);

            is_valid = state.valid_state;
            model_time = state.time;
            error_string = state.error_string.c_str();

            if (state.error_string.empty() == false)
            {
                emit disableButtons();
            }

            dcu_before.draw_image(state.dcu_timeline_before);
            dcu_after.draw_image(state.dcu_timeline_after);
            au_tr_before.draw_image(state.au_tr_timeline_before);
            au_tr_after.draw_image(state.au_tr_timeline_after);
            au_rs_before.draw_image(state.au_rs_timeline_before);
            au_rs_after.draw_image(state.au_rs_timeline_after);

            emit imageReady();

            ctxt->setContextProperty("is_valid", QVariant(is_valid));
            ctxt->setContextProperty("model_time", QVariant(QString::number(model_time)));
            ctxt->setContextProperty("error_string", QVariant(error_string));
            ctxt->setContextProperty("dcu_before_occupation", QVariant(state.dcu_timeline_before.occupation()));
            ctxt->setContextProperty("dcu_after_occupation", QVariant(state.dcu_timeline_after.occupation()));
            ctxt->setContextProperty("au_tr_after_occupation", QVariant(state.au_tr_timeline_after.occupation()));
            ctxt->setContextProperty("au_tr_before_occupation", QVariant(state.au_tr_timeline_before.occupation()));
            ctxt->setContextProperty("au_rs_before_occupation", QVariant(state.au_rs_timeline_before.occupation()));
            ctxt->setContextProperty("au_rs_after_occupation", QVariant(state.au_rs_timeline_after.occupation()));
        }
    }

    Q_INVOKABLE bool load_queries(const QString filepath)
    {
        // Спарсить файл событий
        QFile event_file(filepath.mid(8));

        if (!event_file.open(QIODevice::ReadOnly))
        {
            error_string = "Error opening events.json file.";
            ctxt->setContextProperty("error_string", QVariant(error_string));
            return false;
        }

        QByteArray event_data = event_file.readAll();
        QJsonDocument event_doc(QJsonDocument::fromJson(event_data));

        if (event_doc.isObject() == false)
        {
            error_string = "Error parsing events.json file.";
            ctxt->setContextProperty("error_string", QVariant(error_string));
            return false;
        }

        if (event_doc.object()["Events"] == QJsonValue::Undefined)
        {
            error_string = "Error parsing events.json file.";
            ctxt->setContextProperty("error_string", QVariant(error_string));
            return false;
        }

        const auto& event_list = event_doc.object()["Events"].toArray();

        if (event_list == QJsonArray())
        {
            error_string = "Error parsing events.json file.";
            ctxt->setContextProperty("error_string", QVariant(error_string));
            return false;
        }

        for (const auto& ev: event_list)
        {
            if (event_doc.object()["trigger_time"] == QJsonValue::Undefined)
            {
                error_string = "Error parsing events.json file.";
                ctxt->setContextProperty("error_string", QVariant(error_string));
                return false;
            }

            if (ev.toObject()["trigger_time"].isString() == false)
            {
                error_string = "Error parsing events.json file.";
                ctxt->setContextProperty("error_string", QVariant(error_string));
                return false;
            }

            if (event_doc.object()["id"] == QJsonValue::Undefined)
            {
                error_string = "Error parsing events.json file.";
                ctxt->setContextProperty("error_string", QVariant(error_string));
                return false;
            }

            auto trigger_time = ev.toObject()["trigger_time"].toString().toULongLong();
            auto id = ev.toObject()["id"].toInt();

            if (ev.toObject()["event_type"].toString() == "search")
            {
                if (event_doc.object()["range"] == QJsonValue::Undefined)
                {
                    error_string = "Error parsing events.json file.";
                    ctxt->setContextProperty("error_string", QVariant(error_string));
                    return false;
                }

                if (event_doc.object()["rcs"] == QJsonValue::Undefined)
                {
                    error_string = "Error parsing events.json file.";
                    ctxt->setContextProperty("error_string", QVariant(error_string));
                    return false;
                }

                double range = ev.toObject()["range"].toDouble();
                double rcs = ev.toObject()["rcs"].toDouble();

                model_eng.add_event(Event(trigger_time, [=](Scheduler& shed)
                {
                  shed.queries.push_back(std::make_shared<Query>(
                                        QueryType::search, id,
                                        settings::search_query_speed,
                                        settings::search_query_threshold,
                                        trigger_time, range, rcs, true));
                  return true;
                }));
            }
            else if (ev.toObject()["event_type"].toString() == "confirm")
            {
                if (event_doc.object()["range"] == QJsonValue::Undefined)
                {
                    error_string = "Error parsing events.json file.";
                    ctxt->setContextProperty("error_string", QVariant(error_string));
                    return false;
                }

                if (event_doc.object()["rcs"] == QJsonValue::Undefined)
                {
                    error_string = "Error parsing events.json file.";
                    ctxt->setContextProperty("error_string", QVariant(error_string));
                    return false;
                }

                double range = ev.toObject()["range"].toDouble();
                double rcs = ev.toObject()["rcs"].toDouble();

                model_eng.add_event(Event(trigger_time, [=](Scheduler& shed)
                {
                  shed.queries.push_back(std::make_shared<Query>(
                                        QueryType::confirm, id,
                                        settings::confirm_query_speed,
                                        settings::confirm_query_threshold,
                                        trigger_time, range, rcs, true));
                  return true;
                }));
            }
            else if (ev.toObject()["event_type"].toString() == "tech")
            {
                model_eng.add_event(Event(trigger_time, [=](Scheduler& shed)
                {
                  shed.queries.push_back(std::make_shared<Query>(
                                        QueryType::tech_control, id,
                                        settings::tech_control_speed,
                                        settings::tech_control_threshold,
                                        trigger_time, 0, 0, true));
                  return true;
                }));
            }
            else
            {
                model_eng.add_event(Event(trigger_time, [=](Scheduler& shed)
                {
                  shed.queries.push_back(std::make_shared<Query>(
                                        QueryType::drop, id,
                                        settings::confirm_query_speed,
                                        settings::confirm_query_threshold,
                                        trigger_time, 0, 0, true));
                  return true;
                }));
            }
        }

        event_file.close();
        return true;
    }

signals:
    Q_SIGNAL void imageReady();
    Q_SIGNAL void commandUpdate(QStringList commands);
    Q_SIGNAL void disableButtons();
};
