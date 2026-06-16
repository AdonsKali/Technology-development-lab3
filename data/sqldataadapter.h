#ifndef SQLDATAADAPTER_H
#define SQLDATAADAPTER_H

#include "interfaces/idata.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDateTime>
#include <QUuid>
#include <algorithm>

class SQLDataAdapter : public IData
{
private:
    QVector<QPointF> m_points;
    QVector<QDateTime> m_dateTimes;
    QString m_type;

public:
    SQLDataAdapter() : m_type("sqlite") {}

    QString getType() const override {
        return m_type;
    }

    QVector<QPointF> getPoints() const override {
        return m_points;
    }

    QVector<QDateTime> getDateTimes() const {
        return m_dateTimes;
    }

    bool load(const QString& source) override {
        m_points.clear();
        m_dateTimes.clear();

        qDebug() << "Loading database:" << source;
        QString connectionName = QUuid::createUuid().toString();

        {
            QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
            db.setDatabaseName(source);

            if (!db.open()) {
                qDebug() << "Failed to open database:" << db.lastError().text();
                QSqlDatabase::removeDatabase(connectionName);
                return false;
            }

            QStringList tables = db.tables();
            qDebug() << "Tables in database:" << tables;

            for (const QString& tableName : tables) {
                if (tableName.startsWith("sqlite_")) continue;

                qDebug() << "Loading from table:" << tableName;
                QString queryStr = QString("SELECT Time, Value FROM %1 ORDER BY Time LIMIT 10000")
                                       .arg(tableName);

                qDebug() << "Executing query:" << queryStr;
                QSqlQuery query(db);
                if (query.exec(queryStr)) {
                    QVector<QPointF> tempPoints;
                    QVector<QDateTime> tempDateTimes;
                    int index = 0;

                    while (query.next()) {
                        double y = query.value(1).toDouble();
                        QString timeStr = query.value(0).toString();

                        QDateTime dt;
                        dt = QDateTime::fromString(timeStr, "dd.MM.yyyy hh:mm");
                        if (!dt.isValid()) {
                            dt = QDateTime::fromString(timeStr, "dd.MM.yyyy hh");
                        }
                        if (!dt.isValid()) {
                            dt = QDateTime::fromString(timeStr, "yyyy-MM-dd hh:mm:ss");
                        }
                        if (!dt.isValid()) {
                            dt = QDateTime::fromString(timeStr, "yyyy-MM-dd");
                        }

                        if (dt.isValid()) {
                            tempDateTimes.append(dt);
                            tempPoints.append(QPointF(dt.toSecsSinceEpoch(), y));
                        } else {
                            tempDateTimes.append(QDateTime());
                            tempPoints.append(QPointF(index, y));
                        }
                        index++;
                    }

                    struct PointWithDateTime {
                        QPointF point;
                        QDateTime dt;
                    };
                    QVector<PointWithDateTime> combined;
                    for (int i = 0; i < tempPoints.size(); ++i) {
                        combined.append({tempPoints[i], tempDateTimes[i]});
                    }
                    std::sort(combined.begin(), combined.end(),
                              [](const PointWithDateTime& a, const PointWithDateTime& b) {
                                  return a.dt < b.dt;
                              });

                    m_points.clear();
                    m_dateTimes.clear();
                    for (const auto& item : combined) {
                        m_points.append(item.point);
                        m_dateTimes.append(item.dt);
                    }

                    if (!m_points.isEmpty()) {
                        qDebug() << "Loaded" << m_points.size() << "points from" << tableName;
                        break;
                    }
                } else {
                    qDebug() << "Query failed:" << query.lastError().text();
                }
            }

            db.close();
        }
        QSqlDatabase::removeDatabase(connectionName);

        if (m_points.isEmpty()) {
            qDebug() << "No data found in database:" << source;
            return false;
        }

        qDebug() << "Total loaded points:" << m_points.size();
        if (!m_dateTimes.isEmpty() && m_dateTimes.first().isValid()) {
            qDebug() << "Date range:" << m_dateTimes.first().toString()
            << "to" << m_dateTimes.last().toString();
        }
        return true;
    }
};

#endif
