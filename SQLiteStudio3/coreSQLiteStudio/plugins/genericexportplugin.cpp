#include "genericexportplugin.h"
#include "common/utils.h"
#include "services/notifymanager.h"
#include "common/unused.h"
#include "config_builder.h"
#include <QTextCodec>

bool GenericExportPlugin::initBeforeExport(Db* db, QIODevice* output, const ExportManager::StandardExportConfig& config)
{
    this->db = db;
    this->output = output;
    this->config = &config;

    if (standardOptionsToEnable().testFlag(ExportManager::CODEC))
    {
        codec = codecForName(this->config->codec);
        if (!codec)
        {
            codec = defaultCodec();
            notifyWarn(tr("Could not initialize text codec for exporting. Using default codec: %1").arg(QString::fromLatin1(codec->name())));
        }
    }

    return beforeExport();
}

ExportManager::ExportModes GenericExportPlugin::getSupportedModes() const
{
    return ExportManager::CLIPBOARD|ExportManager::DATABASE|ExportManager::TABLE|ExportManager::QUERY_RESULTS;
}

ExportManager::ExportProviderFlags GenericExportPlugin::getProviderFlags() const
{
    return ExportManager::NONE;
}

QString GenericExportPlugin::getExportConfigFormName() const
{
    return QString();
}

CfgMain* GenericExportPlugin::getConfig()
{
    return nullptr;
}

QString GenericExportPlugin::getConfigFormName(ExportManager::ExportMode mode) const
{
    UNUSED(mode);
    return QString::null;
}

QString GenericExportPlugin::getMimeType() const
{
    return QString::null;
}

bool GenericExportPlugin::isBinaryData() const
{
    return false;
}

void GenericExportPlugin::setExportMode(ExportManager::ExportMode mode)
{
    this->exportMode = mode;
}

bool GenericExportPlugin::afterExportQueryResults()
{
    return true;
}

bool GenericExportPlugin::afterExportTable()
{
    return true;
}

bool GenericExportPlugin::initBeforeExport()
{
    return true;
}

void GenericExportPlugin::write(const QString& str)
{
    output->write(codec->fromUnicode(str));
}

void GenericExportPlugin::writeln(const QString& str)
{
    write(str + "\n");
}

bool GenericExportPlugin::isTableExport() const
{
    return exportMode == ExportManager::TABLE;
}

bool GenericExportPlugin::beforeExportTables()
{
    return true;
}

bool GenericExportPlugin::afterExportTables()
{
    return true;
}

bool GenericExportPlugin::beforeExportIndexes()
{
    return true;
}

bool GenericExportPlugin::afterExportIndexes()
{
    return true;
}

bool GenericExportPlugin::beforeExportTriggers()
{
    return true;
}

bool GenericExportPlugin::afterExportTriggers()
{
    return true;
}

bool GenericExportPlugin::beforeExportViews()
{
    return true;
}

bool GenericExportPlugin::afterExportViews()
{
    return true;
}

bool GenericExportPlugin::afterExportDatabase()
{
    return true;
}

bool GenericExportPlugin::afterExport()
{
    return true;
}

bool GenericExportPlugin::beforeExport()
{
    return true;
}
