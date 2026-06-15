#ifndef DATAFACTORY_H
#define DATAFACTORY_H

#include "interfaces/idatafactory.h"
#include "data/jsondataadapter.h"
#include "data/sqldataadapter.h"
#include "ioc/container.h"
#include <QFileInfo>
#include <QMap>
#include <functional>

class DataFactory : public IDataFactory {
private:
    std::shared_ptr<IOCContainer> m_container;
    QMap<QString, std::function<bool(const QString&)>> m_matchers;

public:
    DataFactory(std::shared_ptr<IOCContainer> container) : m_container(container) {
        // Регистрация матчеров - теперь по любым расширениям
        m_matchers["db"] = [](const QString& path) {
            return path.endsWith(".db", Qt::CaseInsensitive);
        };
        m_matchers["sqlite"] = [](const QString& path) {
            return path.endsWith(".sqlite", Qt::CaseInsensitive);
        };
        m_matchers["json"] = [](const QString& path) {
            return path.endsWith(".json", Qt::CaseInsensitive);
        };
    }

    std::shared_ptr<IData> create(const QString& filePath) override {
        QFileInfo info(filePath);
        QString suffix = info.suffix().toLower();

        if (suffix == "db" || suffix == "sqlite") {
            auto adapter = m_container->GetObject<SQLDataAdapter>();
            if (!adapter) {
                qDebug() << "DataFactory: Failed to get SQLDataAdapter from container";
                return nullptr;
            }
            qDebug() << "DataFactory: Created SQLDataAdapter for" << filePath;
            return adapter;
        } else if (suffix == "json") {
            auto adapter = m_container->GetObject<JSONDataAdapter>();
            if (!adapter) {
                qDebug() << "DataFactory: Failed to get JSONDataAdapter from container";
                return nullptr;
            }
            qDebug() << "DataFactory: Created JSONDataAdapter for" << filePath;
            return adapter;
        }

        qDebug() << "DataFactory: Unsupported suffix:" << suffix;
        return nullptr;
    }

    bool supports(const QString& filePath) const override {
        QFileInfo info(filePath);
        QString suffix = info.suffix().toLower();
        bool supported = m_matchers.contains(suffix);
        qDebug() << "DataFactory::supports:" << filePath << "suffix=" << suffix << "supported=" << supported;
        return supported;
    }
};

#endif
