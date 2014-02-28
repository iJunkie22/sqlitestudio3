#include "db/dbplugin.h"
#include "pluginmanager.h"
#include "sqlformatterplugin.h"
#include "config.h"
#include "sqlitestudio.h"
#include <QProcessEnvironment>
#include <QCoreApplication>
#include <QDir>
#include <QMetaClassInfo>

PluginManager::PluginManager()
{
}

PluginManager::~PluginManager()
{
}

void PluginManager::init()
{
    SQLiteStudio* sqliteStudio = SQLiteStudio::getInstance();
    pluginDirs += qApp->applicationDirPath() + "/plugins";
    pluginDirs += QDir(sqliteStudio->getConfig()->getConfigDir()).absoluteFilePath("plugins");

    QString envDirs = sqliteStudio->getEnv("SQLITESTUDIO_PLUGINS");
    if (!envDirs.isNull())
        pluginDirs += envDirs.split(PATH_LIST_SEPARATOR);

#ifdef PLUGINS_DIR
    pluginDirs += PLUGINS_DIR;
#endif

    loadPlugins();
}

void PluginManager::deinit()
{
    // Types
    foreach (PluginType* type, registeredPluginTypes)
        delete type;

    registeredPluginTypes.clear();
    pluginCategories.clear();

    // Plugin containers and their plugins
    foreach (PluginContainer* container, pluginContainer.values())
    {
        unload(container->name);
        delete container;
    }

    pluginContainer.clear();
}

QList<PluginType*> PluginManager::getPluginTypes() const
{
    return registeredPluginTypes;
}

QStringList PluginManager::getPluginDirs() const
{
    return pluginDirs;
}

QString PluginManager::getFilePath(Plugin* plugin) const
{
    if (!pluginContainer.contains(plugin->getName()))
        return QString::null;

    return pluginContainer[plugin->getName()]->filePath;
}

bool PluginManager::loadBuiltInPlugin(Plugin* plugin)
{
    return initPlugin(nullptr, QString::null, plugin);
}

PluginType* PluginManager::getPluginType(Plugin* plugin) const
{
    if (!pluginContainer.contains(plugin->getName()))
        return nullptr;

    return pluginContainer[plugin->getName()]->type;
}

void PluginManager::loadPlugins()
{
    QStringList nameFilters;
    nameFilters << "*.so" << "*.dll";

    QPluginLoader* loader;
    Plugin* plugin;
    foreach (QString pluginDirPath, pluginDirs)
    {
        QDir pluginDir(pluginDirPath);
        foreach (QString fileName, pluginDir.entryList(nameFilters, QDir::Files))
        {
            fileName = pluginDir.absoluteFilePath(fileName);
            loader = new QPluginLoader(fileName);

            plugin = loadPluginFromFile(fileName, loader);
            if (!plugin)
            {
                delete loader;
                continue;
            }

            if (!initPlugin(loader, fileName, plugin))
            {
                qDebug() << "File" << fileName << "was loaded as plugin, but SQLiteStudio couldn't initialize plugin.";
                loader->unload();
                delete loader;
            }
        }
    }

    emit pluginsInitiallyLoaded();
}

Plugin* PluginManager::loadPluginFromFile(const QString& fileName, QPluginLoader* loader)
{
    if (!loader->load())
    {
        qDebug() << "File" << fileName << "is in plugins location but SQLiteStudio was unable to load it:"
                 << loader->errorString();
        return nullptr;
    }

    QObject* obj = loader->instance();
    if (!obj)
    {
        qDebug() << "File" << fileName << "was loaded as plugin, but SQLiteStudio couldn't create an instance of it:"
                 << loader->errorString();
        loader->unload();
        return nullptr;
    }

    Plugin* plugin = dynamic_cast<Plugin*>(obj);
    if (!plugin)
    {
        qDebug() << "File" << fileName << "was loaded as plugin, but SQLiteStudio couldn't cast it to Plugin class.";
        loader->unload();
        return nullptr;
    }

    return plugin;
}

bool PluginManager::initPlugin(QPluginLoader* loader, const QString& fileName, Plugin* plugin)
{
    PluginContainer* container;
    foreach (PluginType* type, registeredPluginTypes)
    {
        if (type->test(plugin))
        {
            container = new PluginContainer;
            container->type = type;
            container->filePath = fileName;
            if (loader)
            {
                container->loaded = false;
                container->loader = loader;
            }
            else
            {
                container->loaded = true;
                container->builtIn = true;
                container->plugin = plugin;
            }
            readMetadata(plugin, container);
            pluginCategories[type] << container;
            pluginContainer[plugin->getName()] = container;

            if (shouldAutoLoad(plugin))
                load(plugin->getName());
            else if (!container->builtIn)
                loader->unload();

            return true;
        }
    }
    return false;
}

bool PluginManager::shouldAutoLoad(Plugin* plugin)
{
    QStringList loadedPlugins = CFG_CORE.General.LoadedPlugins.get().split(",", QString::SkipEmptyParts);
    QStringList pair;
    foreach (const QString& loadedPlugin, loadedPlugins)
    {
        pair = loadedPlugin.split("=");
        if (pair.size() != 2)
        {
            qWarning() << "Invalid entry in config General.LoadedPlugins:" << loadedPlugin;
            continue;
        }

        if (pair[0] == plugin->getName())
            return (bool)pair[1].toInt();
    }

    return true;
}

QStringList PluginManager::getAllPluginNames(PluginType* type) const
{
    QStringList names;
    if (!pluginCategories.contains(type))
        return names;

    foreach (PluginContainer* container, pluginCategories[type])
        names << container->name;

    return names;
}

QStringList PluginManager::getAllPluginNames() const
{
    return pluginContainer.keys();
}

PluginType* PluginManager::getPluginType(const QString& pluginName) const
{
    if (!pluginContainer.contains(pluginName))
        return nullptr;

    return pluginContainer[pluginName]->type;
}

QString PluginManager::getAuthor(const QString& pluginName) const
{
    if (!pluginContainer.contains(pluginName))
        return QString::null;

    return pluginContainer[pluginName]->author;
}

QString PluginManager::getTitle(const QString& pluginName) const
{
    if (!pluginContainer.contains(pluginName))
        return QString::null;

    return pluginContainer[pluginName]->title;
}

QString PluginManager::getPrintableVersion(const QString& pluginName) const
{
    if (!pluginContainer.contains(pluginName))
        return QString::null;

    return pluginContainer[pluginName]->printableVersion;
}

int PluginManager::getVersion(const QString& pluginName) const
{
    if (!pluginContainer.contains(pluginName))
        return 0;

    return pluginContainer[pluginName]->version;
}

QString PluginManager::getDescription(const QString& pluginName) const
{
    if (!pluginContainer.contains(pluginName))
        return QString::null;

    return pluginContainer[pluginName]->description;
}

void PluginManager::unload(Plugin* plugin)
{
    if (!plugin)
        return;

    unload(plugin->getName());
}

void PluginManager::unload(const QString& pluginName)
{
    if (!pluginContainer.contains(pluginName))
    {
        qWarning() << "No such plugin in containers:" << pluginName << "while trying to unload plugin.";
        return;
    }

    PluginContainer* container = pluginContainer[pluginName];
    if (container->builtIn)
        return;

    if (!container->loaded)
        return;

    emit aboutToUnload(container->plugin, container->type);
    container->plugin->deinit();

    QPluginLoader* loader = container->loader;
    if (!loader->isLoaded())
    {
        qWarning() << "QPluginLoader says the plugin is not loaded. Weird.";
        emit unloaded(container->name, container->type);
        return;
    }

    loader->unload();

    container->plugin = nullptr;
    container->loaded = false;

    emit unloaded(container->name, container->type);

    qDebug() << container->type->getName() << "/" << pluginName << "unloaded:" << container->filePath;
}

bool PluginManager::load(const QString& pluginName)
{
    if (!pluginContainer.contains(pluginName))
    {
        qWarning() << "No such plugin in containers:" << pluginName << "while trying to load plugin.";
        return false;
    }

    PluginContainer* container = pluginContainer[pluginName];
    if (!container->builtIn)
    {
        QPluginLoader* loader = container->loader;
        if (loader->isLoaded())
        {
            pluginLoaded(container);
            return true;
        }

        if (!loader->load())
        {
            qWarning() << "Could not load plugin file:" << loader->errorString();
            return false;
        }

        Plugin* plugin = dynamic_cast<Plugin*>(container->loader->instance());
        if (!plugin->init())
        {
            qWarning() << "Error initializing plugin:" << container->name;
            return false;
        }

        pluginLoaded(container);
    }

    return true;
}

void PluginManager::pluginLoaded(PluginManager::PluginContainer* container)
{
    if (container->builtIn)
        return;

    container->plugin = dynamic_cast<Plugin*>(container->loader->instance());
    container->loaded = true;
    readMetadata(container->plugin, container);


    emit loaded(container->plugin, container->type);
    qDebug() << container->name << "loaded:" << container->filePath;
}

void PluginManager::readMetadata(Plugin* plugin, PluginManager::PluginContainer* container)
{
    container->name = plugin->getName();
    container->version = plugin->getVersion();
    container->printableVersion = plugin->getPrintableVersion();
    container->author = plugin->getAuthor();
    container->description = plugin->getDescription();
    container->title = plugin->getTitle();
}

bool PluginManager::isLoaded(const QString& pluginName) const
{
    if (!pluginContainer.contains(pluginName))
    {
        qWarning() << "No such plugin in containers:" << pluginName << "while trying to get plugin 'loaded' status.";
        return false;
    }

    return pluginContainer[pluginName]->loaded;
}

bool PluginManager::isBuiltIn(const QString& pluginName) const
{
    if (!pluginContainer.contains(pluginName))
    {
        qWarning() << "No such plugin in containers:" << pluginName << "while trying to get plugin 'builtIn' status.";
        return false;
    }

    return pluginContainer[pluginName]->builtIn;
}

Plugin* PluginManager::getLoadedPlugin(const QString& pluginName) const
{
    if (!pluginContainer.contains(pluginName))
        return nullptr;

    if (!pluginContainer[pluginName]->loaded)
        return nullptr;

    return pluginContainer[pluginName]->plugin;
}

QList<Plugin*> PluginManager::getLoadedPlugins(PluginType* type) const
{
    QList<Plugin*> list;
    if (!pluginCategories.contains(type))
        return list;

    foreach (PluginContainer* container, pluginCategories[type])
    {
        if (container->loaded)
            list << container->plugin;
    }

    return list;
}
