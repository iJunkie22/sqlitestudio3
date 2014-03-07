#ifndef CLICOMMANDOPEN_H
#define CLICOMMANDOPEN_H

#include "clicommand.h"

class CliCommandOpen : public CliCommand
{
    public:
        void execute(const QStringList& args);
        QString shortHelp() const;
        QString fullHelp() const;
        void defineSyntax();
};

#endif // CLICOMMANDOPEN_H
