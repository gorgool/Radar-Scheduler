#pragma once

#include <QDebug>
#include <QtGlobal>
#include <QObject>
#include <QAbstractListModel>

#include <vector>
#include "../Model_Engine/ControlCommand.h"

struct CommandRecord
{
    quint32 m_query_id;
    QString m_command_type;
    quint64 m_referance_time;
    quint64 m_execution_time;
    quint64 m_execution_length;

    CommandRecord(const quint32 _query_id, const QString &_command_type, const quint64 _referance_time,
               const quint64 _execution_time, const quint64 _execution_length) :
        m_query_id(_query_id),
        m_command_type(_command_type),
        m_referance_time(_referance_time),
        m_execution_time(_execution_time),
        m_execution_length(_execution_length)
    {}

    CommandRecord() :
        m_query_id(0),
        m_command_type("Not initialized"),
        m_referance_time(0),
        m_execution_time(0),
        m_execution_length(0)
    {}
};

class CommandList : public QAbstractListModel
{
    Q_OBJECT

    QList<CommandRecord> dataList;

public:

    enum CommandRoles
    {
        CommandQueryIDRole = Qt::UserRole + 1,
        CommandTypeRole,
        CommandRefTimeRole,
        CommandExecTimeRole,
        CommandExecLengthRole
    };

    CommandList(QObject* parent = 0) : QAbstractListModel(parent) {}

    int rowCount(const QModelIndex &parent = QModelIndex()) const
    {
        Q_UNUSED(parent)
        return dataList.count();
    }

    QVariant data(const QModelIndex &index, int role) const
    {
        if (index.row() < 0 || index.row() > dataList.count())
            return QVariant();

        const CommandRecord & ct = dataList[index.row()];

        switch (role) {
        case CommandQueryIDRole:
            return ct.m_query_id;
            break;
        case CommandTypeRole:
            return ct.m_command_type;
            break;
        case CommandRefTimeRole:
            return ct.m_referance_time;
            break;
        case CommandExecTimeRole:
            return ct.m_execution_time;
            break;
        case CommandExecLengthRole:
            return ct.m_execution_length;
            break;
        default:
            return QVariant();
            break;
        }
    }

    QHash<int, QByteArray> roleNames() const
    {
        QHash<int, QByteArray> roles;

        roles[CommandQueryIDRole] = "command_query_id";
        roles[CommandTypeRole] = "command_type";
        roles[CommandRefTimeRole] = "referance_time";
        roles[CommandExecTimeRole] = "execution_time";
        roles[CommandExecLengthRole] = "execution_length";

        return roles;
    }

    Q_INVOKABLE QVariantMap get(const int index) const
    {
        if (index < 0 || index > dataList.count())
            return QVariantMap();

        QVariantMap ret;
        const CommandRecord & ct = dataList[index];

        ret["command_query_id"] = ct.m_query_id;
        ret["command_type"] = ct.m_command_type;
        ret["referance_time"] = ct.m_referance_time;
        ret["execution_time"] = ct.m_execution_time;
        ret["execution_length"] = ct.m_execution_length;

        return ret;
    }

    void update_data(const std::vector<ControlCommand>& command_list)
    {
        beginResetModel();

        dataList.clear();
        for(const auto& c : command_list)
        {
            CommandRecord cr;
            cr.m_query_id = c.query_id;
            cr.m_referance_time = c.referance_time;
            cr.m_execution_time = c.execution_time;
            cr.m_execution_length = c.execution_length;

            switch(c.type)
            {
            case CommandType::ct_nop:
                cr.m_command_type = "Не задано";
                break;
            case CommandType::ct_transmit_command:
                cr.m_command_type = "Излучение";
                break;
            case CommandType::ct_receive_command:
                cr.m_command_type = "Прием";
                break;
            case CommandType::ct_tr_rephase_command:
                cr.m_command_type = "Перефазирование АУ (излучение)";
                break;
            case CommandType::ct_rs_rephase_command:
                cr.m_command_type = "Перефазирование АУ (прием)";
                break;
            case CommandType::ct_protect_command:
                cr.m_command_type = "Отключение защиты";
                break;
            case CommandType::ct_tech_command:
                cr.m_command_type = "Запрос состояния";
                break;
            }

            dataList.append(cr);
        }

        endResetModel();
    }
};
