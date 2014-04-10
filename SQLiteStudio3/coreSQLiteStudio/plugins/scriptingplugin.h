#ifndef SCRIPTINGPLUGIN_H
#define SCRIPTINGPLUGIN_H

#include "plugin.h"
#include <QVariant>

class Db;

class ScriptingPlugin : virtual public Plugin
{
    public:
        class Context
        {
            public:
                virtual ~Context() {}
        };

        virtual QString getLanguage() const = 0;
        virtual Context* createContext() = 0;
        virtual void releaseContext(Context* context) = 0;
        virtual void resetContext(Context* context) = 0;
        virtual void setVariable(Context* context, const QString& name, const QVariant& value) = 0;
        virtual QVariant getVariable(Context* context, const QString& name) = 0;
        virtual QVariant evaluate(Context* context, const QString& code, const QList<QVariant>& args) = 0;
        virtual bool hasError(Context* context) const = 0;
        virtual QString getErrorMessage(Context* context) const = 0;
        virtual QVariant evaluate(const QString& code, const QList<QVariant>& args, QString* errorMessage = nullptr) = 0;
        virtual QByteArray getIconData() const = 0;
};

class DbAwareScriptingPlugin : public ScriptingPlugin
{
    public:
        virtual QVariant evaluate(Context* context, const QString& code, const QList<QVariant>& args, Db* db, bool locking) = 0;
        virtual QVariant evaluate(const QString& code, const QList<QVariant>& args, Db* db, bool locking, QString* errorMessage = nullptr) = 0;

        QVariant evaluate(Context* context, const QString& code, const QList<QVariant>& args)
        {
            return evaluate(context, code, args, nullptr, true);
        }

        QVariant evaluate(const QString& code, const QList<QVariant>& args, QString* errorMessage = nullptr)
        {
            return evaluate(code, args, nullptr, true, errorMessage);
        }
};

Q_DECLARE_METATYPE(ScriptingPlugin::Context*)

#endif // SCRIPTINGPLUGIN_H
