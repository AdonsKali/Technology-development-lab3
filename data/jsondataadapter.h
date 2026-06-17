#ifndef JSONDATAADAPTER_H
#define JSONDATAADAPTER_H

#include "interfaces/idata.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QDebug>
#include <QDateTime>

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
                
                if (obj.contains("Time") && obj.contains("Value")) {
                    QString timeStr = obj["Time"].toString();
                    double y = obj["Value"].toDouble();
                    
                    QDateTime dt;
                    
                    dt = QDateTime::fromString(timeStr, "dd.MM.yyyy");
                    if (!dt.isValid()) {
                        dt = QDateTime::fromString(timeStr, "yyyy-MM-dd");
                    }
                    if (!dt.isValid()) {
                        dt = QDateTime::fromString(timeStr, "dd.MM.yyyy hh:mm");
                    }
                    if (!dt.isValid()) {
                        dt = QDateTime::fromString(timeStr, "yyyy-MM-dd hh:mm:ss");
                    }
                    
                    if (dt.isValid()) {
                        m_points.append(QPointF(dt.toSecsSinceEpoch(), y));
                        qDebug() << "Parsed date:" << timeStr << "->" << dt.toString() 
                                 << "timestamp:" << dt.toSecsSinceEpoch() << "value:" << y;
                    } else {
                        qDebug() << "Failed to parse date:" << timeStr << "using index:" << i;
                        m_points.append(QPointF(i, y));
                    }
                }
                else if (obj.contains("x") && obj.contains("y")) {
                    double x = obj["x"].toDouble();
                    double y = obj["y"].toDouble();
                    m_points.append(QPointF(x, y));
                } 
                else if (obj.contains("value")) {
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
        
        qDebug() << "JSONDataAdapter: loaded" << m_points.size() << "points";
        return !m_points.isEmpty();
    }
};

#endif
