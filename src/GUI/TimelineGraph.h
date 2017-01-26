#pragma once

#include <QImage>
#include <QDebug>
#include <QObject>
#include <QPainter>
#include <QQuickImageProvider>
#include <vector>
#include "../Model_Engine/Timeline.h"

class TimelineGraph: public QQuickImageProvider
{
public:
    TimelineGraph() : QQuickImageProvider(QQuickImageProvider::Image)
    {
        auto size = image_size / tile_size + 1;
        for (int times = 0; times < size; ++times)
        {
            m_images.append(QImage(tile_size, tile_height, QImage::Format_RGB16));
        }

        color_tbl[TimelineLabel::tll_empty] = Qt::white;
        color_tbl[TimelineLabel::tll_dcu_transmit] = Qt::red;
        color_tbl[TimelineLabel::tll_dcu_receive] = Qt::blue;
        color_tbl[TimelineLabel::tll_dcu_energy_restore] = Qt::yellow;
        color_tbl[TimelineLabel::tll_au_command_delay] = Qt::darkGreen;
        color_tbl[TimelineLabel::tll_au_rephase] = Qt::green;
        color_tbl[TimelineLabel::tll_dcu_receive_prevent] = Qt::cyan;

        draw_image(Timeline());
    }

    // Отобразить текст на общем "виртуальном" изображение по координатам x и y
    void draw_text(std::vector<QPainter>& painters, const int x, const int y, const QString text)
    {
        auto image_idx = x / tile_size;
        painters[image_idx].drawText(x - image_idx * tile_size, y, text);
    }

    // Нарисовать горизонтальную линию на общем "виртуальном" изображение по координатам x и y длиной length
    void draw_line(std::vector<QPainter>& painters, const int x, const int y, const int length)
    {
        auto image_idx_begin = x / tile_size;
        auto image_idx_end = (x + length) / tile_size;

        if (image_idx_begin == image_idx_end)
        {
            painters[image_idx_begin].drawLine(x - image_idx_begin * tile_size, y, x + (length % tile_size) - image_idx_begin * tile_size, y);
            return;
        }

        for (int idx = image_idx_begin; idx <= image_idx_end; ++idx)
        {
            if (idx == image_idx_begin)
                painters[idx].drawLine(x, y, tile_size, y);
            else if (idx == image_idx_end)
                painters[idx].drawLine(0, y, x + (length % tile_size) - idx * tile_size, y);
            else
                painters[idx].drawLine(0, y, tile_size, y);
        }

    }

    // Нарисовать вертикальную зачсечку на общем "виртуальном" изображение по координатам x и y
    void draw_mark(std::vector<QPainter>& painters, const int x, const int y)
    {
        auto image_idx = x / tile_size;
        painters[image_idx].drawLine(x - image_idx * tile_size, y - 3, x - image_idx * tile_size, y + 3);
    }

    // Нарисовать единичный временной дискрет на общем "виртуальном" изображение по координатам x и y и залить цветом color
    void draw_label(std::vector<QPainter>& painters, const int x, const int y, const QColor& color)
    {
        auto image_idx = x / tile_size;
        auto& p = painters[image_idx];
        p.setBrush(QBrush(color));
        p.drawRect(x - image_idx * tile_size, y - 6, 4, 6);

    }

    // Нарисовать изображение КВД по данным tl
    void draw_image(const Timeline& tl)
    {
        auto time = tl.start_time / 1000000;

        const int x_offset = 20;
        const int y_offset = 14;

        std::vector<QPainter> painters(m_images.size());
        for(int idx = 0; idx < m_images.size(); ++idx)
        {
            m_images[idx].fill(Qt::white);
            painters[idx].begin(&(m_images[idx]));
        }

        // ==== Временная шкала ====
        // Горизонтальная ось с отметками;
        //draw_line(painters, 0 + x_offset, 0 + y_offset, 1000 * timeline_depth + 10);
        // Временные метки
        int timestamp_step = 4*100;
        for(int idx = 0; idx < timeline_depth; ++idx)
        {
            draw_mark(painters, x_offset + idx * timestamp_step, y_offset);
            draw_text(painters, x_offset + idx * timestamp_step + 2, y_offset + 14, QString::number(time + idx) + " мкс");
        }
        // ==== Дискреты ====
        int label_step = 4;
        for(int idx = 0; idx < timeline_depth * 100; ++idx)
        {
            draw_label(painters, x_offset + idx * label_step, y_offset, color_tbl[tl.get_value_at(idx)]);
        }

        for(int idx = 0; idx < m_images.size(); ++idx)
        {
            painters[idx].end();
        }
    }

    virtual QImage requestImage(const QString &id, QSize *size, const QSize& requestedSize)
    {
        int width = tile_width;
        int height = tile_height;

        int offset = id.toInt();

        if (size)
        {
            *size = QSize(tile_width, tile_height);
        }

        auto image_idx = offset * width / tile_size;
        return m_images[image_idx].copy(offset * width - image_idx * tile_size, 0, width, height);
    }
private:

    const int timeline_depth = 200;

    const int tile_width = 1000;
    const int tile_height = 30;

    const int image_size = 400000;
    const int tile_size = 20000;

    QVector<QImage> m_images;

    QHash<TimelineLabel, QColor> color_tbl;
};
