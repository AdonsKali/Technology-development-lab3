#ifndef SQLDATAADAPTER_H
#define SQLDATAADAPTER_H

#include "interfaces/idata.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDateTime>
#include <QUuid>

class SQLDataAdapter : public IData
{
private:
    QVector<QPointF> m_points;
    QString m_type;

public:
    SQLDataAdapter() : m_type("sqlite") {}

    QString getType() const override {
        return m_type;
    }

    QVector<QPointF> getPoints() const override {
        return m_points;
    }

    bool load(const QString& source) override {
        m_points.clear();

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

            bool success = false;

            for (const QString& tableName : tables) {
                if (tableName.startsWith("sqlite_")) continue;

                qDebug() << "Loading from table:" << tableName;
                QString queryStr = QString("SELECT Time, Value FROM %1 ORDER BY Time LIMIT 10000")
                                       .arg(tableName);
                
                qDebug() << "Executing query:" << queryStr;
                QSqlQuery query(db);
                if (query.exec(queryStr)) {
                    int rowIndex = 0;
                    
                    while (query.next()) {
                        double x = rowIndex;
                        double y = query.value(1).toDouble();
                        QString timeStr = query.value(0).toString();
                        QDateTime dt = QDateTime::fromString(timeStr, "dd.MM.yyyy hh");
                        
                        if (dt.isValid()) {
                            x = dt.toSecsSinceEpoch() / 86400.0;
                        }

                        m_points.append(QPointF(x, y));
                        rowIndex++;
                    }
                    if (!m_points.isEmpty()) {
                        success = true;
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
        qDebug() << "First 5 points:";
        for (int i = 0; i < qMin(5, m_points.size()); ++i) {
            qDebug() << "  [" << i << "] x=" << m_points[i].x() << "y=" << m_points[i].y();
        }
        return true;
    }
};

#endif 