#ifndef POPULATERANDOM_H
#define POPULATERANDOM_H

#include "genericplugin.h"
#include "populateplugin.h"
#include "config_builder.h"

CFG_CATEGORIES(PopulateRandomConfig,
    CFG_CATEGORY(PopulateRandom,
        CFG_ENTRY(int,     MinValue, 0)
        CFG_ENTRY(int,     MaxValue, 99999999)
        CFG_ENTRY(QString, Prefix,   QString())
        CFG_ENTRY(QString, Suffix,   QString())
    )
)

class PopulateRandom : public GenericPlugin, public PopulatePlugin
{
        Q_OBJECT

        SQLITESTUDIO_PLUGIN_TITLE("Random")
        SQLITESTUDIO_PLUGIN_DESC("Support for populating tables with random numbers.")
        SQLITESTUDIO_PLUGIN_VERSION(10000)
        SQLITESTUDIO_PLUGIN_AUTHOR("sqlitestudio.pl")

    public:
        PopulateRandom();

        QString getTitle() const;
        PopulateEngine* createEngine();
};

class PopulateRandomEngine : public PopulateEngine
{
    public:
        bool beforePopulating();
        QVariant nextValue();
        void afterPopulating();
        CfgMain* getConfig();
        QString getPopulateConfigFormName() const;
        bool validateOptions();

    private:
        CFG_LOCAL(PopulateRandomConfig, cfg)
        int range;
};
#endif // POPULATERANDOM_H
