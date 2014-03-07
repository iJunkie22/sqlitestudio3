#include "clicommandexit.h"
#include "cli.h"
#include "unused.h"

void CliCommandExit::execute(const QStringList& args)
{
    UNUSED(args);
    cli->exit();
}

QString CliCommandExit::shortHelp() const
{
    return tr("quits the application");
}

QString CliCommandExit::fullHelp() const
{
    return tr(
                "Quits the application. Settings are stored in configuration file and will be restored on next startup."
             );
}

void CliCommandExit::defineSyntax()
{
    syntax.setName("exit");
    syntax.addAlias("quit");
}
