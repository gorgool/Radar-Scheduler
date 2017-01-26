#pragma once

#include <QtGlobal>
#include <QObject>
#include <QAbstractListModel>

#include <vector>

#include "../Model_Engine/Query.h"

/*
 * Модель данных одной заявки на осблуживание
 */
struct QueryRecord
{
    // Идентификатор заявки
    quint32 m_query_id;
    // Тип заявки
    QString m_query_type;
    // Коэффициент роста приоритета заявки
    double m_speed_coef;
    // Порог исполнения заявки
    double m_threshold;
    // Время предыдущего исполнения заявки, нс
    quint64 m_prev_time;
    // Значение динамического приоритета
    double m_priority;
    // Прогнозируемая дальность, м
    double m_range;
    // Прогнозируемая ЭПР, кв.м
    double m_rcs;

    QueryRecord(const quint32 _query_id, const QString &_query_type, const double _speed_coef,
               const double _threshold, const quint64 _prev_time, const double _prioirity,
               const double _range, const double _rcs) :
        m_query_id(_query_id),
        m_query_type(_query_type),
        m_speed_coef(_speed_coef),
        m_threshold(_threshold),
        m_prev_time(_prev_time),
        m_priority(_prioirity),
        m_range(_range),
        m_rcs(_rcs)
    {}

    QueryRecord() :
        m_query_id(0),
        m_query_type("Not initialized"),
        m_speed_coef(0),
        m_threshold(0),
        m_prev_time(0),
        m_priority(0),
        m_range(0),
        m_rcs(0)
    {}
};

/*
 * Модель данных списка заявок на обслуживание.
 */

class QueryList : public QAbstractListModel
{
    Q_OBJECT

    // Список заявок
    QList<QueryRecord> dataList;

public:

    enum QueryRoles
    {
        QueryIDRole = Qt::UserRole + 1,
        QueryTypeRole,
        QuerySpeedRole,
        QueryThresholdRole,
        QueryPTimeRole,
        QueryPriorityRole,
        QueryRangeRole,
        QueryRCSRole
    };

    QueryList(QObject* parent = 0) : QAbstractListModel(parent) {}

    int rowCount(const QModelIndex &parent = QModelIndex()) const
    {
        Q_UNUSED(parent)
        return dataList.count();
    }

    QVariant data(const QModelIndex &index, int role) const
    {
        if (index.row() < 0 || index.row() > dataList.count())
            return QVariant();

        const QueryRecord & ct = dataList[index.row()];

        switch (role) {
        case QueryIDRole:
            return ct.m_query_id;
            break;
        case QueryTypeRole:
            return ct.m_query_type;
            break;
        case QuerySpeedRole:
            return ct.m_speed_coef;
            break;
        case QueryThresholdRole:
            return ct.m_threshold;
            break;
        case QueryPTimeRole:
            return ct.m_prev_time;
            break;
        case QueryPriorityRole:
            return ct.m_priority;
            break;
        case QueryRangeRole:
            return ct.m_range;
            break;
        case QueryRCSRole:
            return ct.m_rcs;
            break;
        default:
            return QVariant();
            break;
        }
    }

    QHash<int, QByteArray> roleNames() const
    {
        QHash<int, QByteArray> roles;

        roles[QueryIDRole] = "query_id";
        roles[QueryTypeRole] = "query_type";
        roles[QuerySpeedRole] = "speed_coef";
        roles[QueryThresholdRole] = "threshold";
        roles[QueryPTimeRole] = "prev_time";
        roles[QueryPriorityRole] = "priority";
        roles[QueryRangeRole] = "range";
        roles[QueryRCSRole] = "rcs";

        return roles;
    }

    Q_INVOKABLE QVariantMap get(const int index) const
    {
        if (index < 0 || index > dataList.count())
            return QVariantMap();

        QVariantMap ret;
        const QueryRecord & ct = dataList[index];

        ret["query_id"] = ct.m_query_id;
        ret["query_type"] = ct.m_query_type;
        ret["speed_coef"] = ct.m_speed_coef;
        ret["threshold"] = ct.m_threshold;
        ret["prev_time"] = ct.m_prev_time;
        ret["priority"] = ct.m_priority;
        ret["range"] = ct.m_range;
        ret["rcs"] = ct.m_rcs;

        return ret;
    }

    void update_data(const std::vector<Query>& query_list)
    {
        beginResetModel();

        dataList.clear();
        for(const auto& q : query_list)
        {
            QueryRecord qr;
            qr.m_query_id = q.id;
            qr.m_speed_coef = q.k;
            qr.m_threshold = q.p_threshold;
            qr.m_prev_time = q.t_prev;
            qr.m_priority = q.p_value;
            qr.m_range = q.range;
            qr.m_rcs = q.rcs;

            switch(q.type)
            {
            case QueryType::search:
                qr.m_query_type = "Поиск";
                break;
            case QueryType::confirm:
                qr.m_query_type = "Дообнаружение";
                break;
            case QueryType::capture:
                qr.m_query_type = "Захват";
                break;
            case QueryType::tracking:
                qr.m_query_type = "Сопровождение";
                break;
            case QueryType::signal_change:
                qr.m_query_type = "Спецобслуживание";
                break;
            case QueryType::tech_control:
                qr.m_query_type = "Запрос технического состояния";
                break;
            case QueryType::drop:
                qr.m_query_type = "Сброс заявки";
                break;
            }

            dataList.append(qr);
        }

        endResetModel();
    }
};
