#ifndef CLICOMMANDSQL_H
#define CLICOMMANDSQL_H

#include "clicommand.h"

class CliCommandSql : public CliCommand
{
    public:
        bool execute(QStringList args);
        bool validate(QStringList args);
};

#endif // CLICOMMANDSQL_H
