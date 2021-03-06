#ifndef BUGDIALOG_H
#define BUGDIALOG_H

#include "guiSQLiteStudio_global.h"
#include <QDialog>

namespace Ui {
    class BugDialog;
}

class GUI_API_EXPORT BugDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit BugDialog(QWidget *parent = 0);
        ~BugDialog();

        void setFeatureRequestMode(bool feature);

    private:
        void init();

        static QString getMessageAboutReportHistory();
        static void finishedBugReport(bool success, const QString& errorMsg);
        static void finishedFeatureRequest(bool success, const QString& errorMsg);

        Ui::BugDialog *ui = nullptr;
        bool bugMode = true;
        QString user;

    private slots:
        void updateState();
        void validate();
        void help();
        void logIn();

    public slots:
        void accept();
};

#endif // BUGDIALOG_H
