#ifndef JSONDATAADAPTER_H
#define JSONDATAADAPTER_H

#include "interfaces/idata.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QDebug>

class JSONDataAdapter : public IData
{
private:
    QVector<QPointF> m_points;
    QString m_type;

public:
    JSONDataAdapter() : m_type("json") {}

    QString getType() const override {
        return m_type;
    }

    QVector<QPointF> getPoints() const override {
        return m_points;
    }

    bool load(const QString& source) override {
        m_points.clear();
        QFile file(source);
        if (!file.open(QIODevice::ReadOnly)) {
            qDebug() << "Failed to open JSON file:" << source;
            return false;
        }
        QByteArray data = file.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isArray()) {
            QJsonArray array = doc.array();
            for (int i = 0; i < array.size(); ++i) {
                QJsonObject obj = array[i].toObject();
                if (obj.contains("x") && obj.contains("y")) {
                    double x = obj["x"].toDouble();
                    double y = obj["y"].toDouble();
                    m_points.append(QPointF(x, y));
                } else if (obj.contains("value")) {
                    double y = obj["value"].toDouble();
                    m_points.append(QPointF(i, y));
                }
            }
        } 
        else if (doc.isObject()) {
            QJsonObject obj = doc.object();
            if (obj.contains("data") && obj["data"].isArray()) {
                QJsonArray array = obj["data"].toArray();
                for (int i = 0; i < array.size(); ++i) {
                    double value = array[i].toDouble();
                    m_points.append(QPointF(i, value));
                }
            }
        }
        return !m_points.isEmpty();
    }
};

#endif