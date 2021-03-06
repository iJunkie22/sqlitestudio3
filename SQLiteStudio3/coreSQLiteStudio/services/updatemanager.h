#ifndef UPDATEMANAGER_H
#define UPDATEMANAGER_H

#ifdef PORTABLE_CONFIG

#include "common/global.h"
#include "sqlitestudio.h"
#include <QObject>
#include <functional>
#include <QProcess>

class QNetworkAccessManager;
class QNetworkReply;
class QTemporaryDir;
class QFile;

class API_EXPORT UpdateManager : public QObject
{
        Q_OBJECT
    public:
        typedef std::function<bool(const QString& msg)> RetryFunction;

        struct UpdateEntry
        {
            QString compontent;
            QString version;
            QString url;
        };

        explicit UpdateManager(QObject *parent = 0);
        ~UpdateManager();

        void checkForUpdates(bool force = false);
        void update();
        bool isPlatformEligibleForUpdate() const;
        static bool executeFinalStep(const QString& tempDir, const QString& backupDir, const QString& appDir);
        static bool handleUpdateOptions(const QStringList& argList, int& returnCode);
        static QString getStaticErrorMessage();

        static void setRetryFunction(const RetryFunction &value);

        static_char* UPDATE_OPTION_NAME = "--update-final-step";
        static_char* WIN_INSTALL_FILE = "install.dat";
        static_char* WIN_UPDATE_DONE_FILE = "UpdateFinished.lck";

    private:
        enum class LinuxPermElevator
        {
            KDESU,
            GKSU,
            PKEXEC,
            NONE
        };

        QString getPlatformForUpdate() const;
        QString getCurrentVersions() const;
        void handleAvailableUpdatesReply(QNetworkReply* reply);
        void handleDownloadReply(QNetworkReply* reply);
        void getUpdatesMetadata(QNetworkReply*& replyStoragePointer, bool force = false);
        void handleUpdatesMetadata(QNetworkReply* reply);
        QList<UpdateEntry> readMetadata(const QJsonDocument& doc);
        void downloadUpdates();
        void updatingFailed(const QString& errMsg);
        void installUpdates();
        bool installComponent(const QString& component, const QString& tempDir);
        bool executeFinalStep(const QString& tempDir);
        bool executeFinalStepAsRoot(const QString& tempDir, const QString& backupDir, const QString& appDir);
#if defined(Q_OS_LINUX)
        bool executeFinalStepAsRootLinux(const QString& tempDir, const QString& backupDir, const QString& appDir);
        bool unpackToDirLinux(const QString& packagePath, const QString& outputDir);
#elif defined(Q_OS_MACX)
        bool unpackToDirMac(const QString& packagePath, const QString& outputDir);
        bool executeFinalStepAsRootMac(const QString& tempDir, const QString& backupDir, const QString& appDir);
#elif defined(Q_OS_WIN32)
        bool runAnotherInstanceForUpdate(const QString& tempDir, const QString& backupDir, const QString& appDir, bool reqAdmin);
        bool unpackToDirWin(const QString& packagePath, const QString& outputDir);
#endif
        bool doRequireAdminPrivileges();
        bool unpackToDir(const QString& packagePath, const QString& outputDir);
        void handleStaticFail(const QString& errMsg);
        QString getAppDirPath() const;
        void cleanup();

        static bool moveDir(const QString& src, const QString& dst, bool contentsOnly = false);
        static bool deleteDir(const QString& path);
        static bool execCmd(const QString& cmd, const QStringList& args, QString* errorMsg = nullptr);
        static bool waitForProcess(QProcess& proc);
        static QString readError(QProcess& proc, bool reverseOrder = false);
        static void staticUpdatingFailed(const QString& errMsg);
        static LinuxPermElevator findPermElevatorForLinux();
        static QString wrapCmdLineArgument(const QString& arg);
        static QString escapeCmdLineArgument(const QString& arg);
        static QString getBackupDir(const QString& appDir);
#if defined(Q_OS_WIN32)
        static bool executePreFinalStepWin(const QString& tempDir, const QString& backupDir, const QString& appDir, bool reqAdmin);
        static bool executeFinalStepAsRootWin(const QString& tempDir, const QString& backupDir, const QString& appDir);
        static bool executePostFinalStepWin(const QString& tempDir);
        static bool waitForFileToDisappear(const QString& filePath, int seconds);
        static bool waitForFileToAppear(const QString& filePath, int seconds);
#endif

        QNetworkAccessManager* networkManager = nullptr;
        QNetworkReply* updatesCheckReply = nullptr;
        QNetworkReply* updatesGetUrlsReply = nullptr;
        QNetworkReply* updatesGetReply = nullptr;
        bool updatesInProgress = false;
        QList<UpdateEntry> updatesToDownload;
        QHash<QString,QString> updatesToInstall;
        QTemporaryDir* tempDir = nullptr;
        QFile* currentDownloadFile = nullptr;
        int totalPercent = 0;
        int totalDownloadsCount = 0;
        QString currentJobTitle;
        bool requireAdmin = false;
        static RetryFunction retryFunction;

        static QString staticErrorMessage;
        static_char* WIN_PRE_FINAL_UPDATE_OPTION_NAME = "--update-pre-final-step";
        static_char* WIN_POST_FINAL_UPDATE_OPTION_NAME = "--update-post-final-step";
        static_char* WIN_UPDATER_BINARY = "UpdateSQLiteStudio.exe";
        static_char* updateServiceUrl = "http://sqlitestudio.pl/updates3.rvt";
        static_char* manualUpdatesHelpUrl = "http://wiki.sqlitestudio.pl/index.php/User_Manual#Manual";

    private slots:
        void finished(QNetworkReply* reply);
        void downloadProgress(qint64 bytesReceived, qint64 totalBytes);
        void readDownload();

    signals:
        void updatesAvailable(const QList<UpdateManager::UpdateEntry>& updates);
        void noUpdatesAvailable();
        void updatingProgress(const QString& jobTitle, int jobPercent, int totalPercent);
        void updatingFinished();
        void updatingError(const QString& errorMessage);
};

#define UPDATES SQLITESTUDIO->getUpdateManager()

#endif // PORTABLE_CONFIG
#endif // UPDATEMANAGER_H
