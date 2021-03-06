#include "bugreporter.h"
#include "services/config.h"
#include "services/notifymanager.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrlQuery>

BugReporter::BugReporter(QObject *parent) :
    QObject(parent)
{
    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished(QNetworkReply*)));
}

QUrl BugReporter::getReporterEmailHelpUrl() const
{
    return QUrl(QString::fromLatin1(reporterEmailHelpUrl));
}

QUrl BugReporter::getReporterUserAndPasswordHelpUrl() const
{
    return QUrl(QString::fromLatin1(reporterUserPassHelpUrl));
}

void BugReporter::validateBugReportCredentials(const QString& login, const QString& password)
{
    if (credentialsValidationInProgress)
    {
        credentialsValidationInProgress->abort();
        credentialsValidationInProgress->deleteLater();
    }

    QUrlQuery query;
    query.addQueryItem("validateUser", login);
    query.addQueryItem("password", password);

    QUrl url = QUrl(QString::fromLatin1(bugReportServiceUrl) + "?" + query.query(QUrl::FullyEncoded));
    QNetworkRequest request(url);
    credentialsValidationInProgress = networkManager->get(request);
    replyToHandler[credentialsValidationInProgress] = [this](bool success, const QString& data)
    {
        if (success && data.trimmed() != "OK")
        {
            success = false;
            emit credentialsValidationResult(success, tr("Invalid login or password"));
        }
        else
        {
            emit credentialsValidationResult(success, success ? QString() : data);
        }
    };
}

void BugReporter::abortCredentialsValidation()
{
    if (credentialsValidationInProgress)
    {
        credentialsValidationInProgress->abort();
        credentialsValidationInProgress->deleteLater();
        credentialsValidationInProgress = nullptr;
    }
}

void BugReporter::useBugReportCredentials(const QString& login, const QString& password)
{
    CFG_CORE.Internal.BugReportUser.set(login);
    CFG_CORE.Internal.BugReportPassword.set(password);
}

void BugReporter::clearBugReportCredentials()
{
    CFG_CORE.Internal.BugReportUser.set(QString());
    CFG_CORE.Internal.BugReportPassword.set(QString());
}

void BugReporter::reportBug(const QString& title, const QString& details, const QString& version, const QString& os, const QString& plugins, BugReporter::ResponseHandler responseHandler, const QString& urlSuffix)
{
    static_qstring(contentsTpl, "%1\n\n<b>Plugins loaded:</b>\n%2\n\n<b>Version:</b>\n%3\n\n<b>Operating System:</b>\n%4");
    QString contents = contentsTpl.arg(escapeParam(details), plugins, version, os);

    QUrlQuery query;
    query.addQueryItem("brief", escapeParam(title));
    query.addQueryItem("contents", contents);
    query.addQueryItem("os", os);
    query.addQueryItem("version", version);
    query.addQueryItem("featureRequest", "0");

    QUrl url = QUrl(QString::fromLatin1(bugReportServiceUrl) + "?" + escapeUrl(query.query(QUrl::FullyEncoded) + urlSuffix));
    QNetworkRequest request(url);
    QNetworkReply* reply = networkManager->get(request);
    if (responseHandler)
        replyToHandler[reply] = responseHandler;

    replyToTypeAndTitle[reply] = QPair<bool,QString>(false, title);
}

void BugReporter::requestFeature(const QString& title, const QString& details, BugReporter::ResponseHandler responseHandler, const QString& urlSuffix)
{
    QUrlQuery query;
    query.addQueryItem("brief", escapeParam(title));
    query.addQueryItem("contents", escapeParam(details));
    query.addQueryItem("featureRequest", "1");

    QUrl url = QUrl(QString::fromLatin1(bugReportServiceUrl) + "?" + escapeUrl(query.query(QUrl::FullyEncoded) + urlSuffix));
    QNetworkRequest request(url);
    QNetworkReply* reply = networkManager->get(request);
    if (responseHandler)
        replyToHandler[reply] = responseHandler;

    replyToTypeAndTitle[reply] = QPair<bool,QString>(true, title);
}

QString BugReporter::escapeParam(const QString &input)
{
    return input.toHtmlEscaped();
}

QString BugReporter::escapeUrl(const QString &input)
{
    // For some reason the ";" character is not encodedy by QUrlQuery when using FullEncoded. Pity. We have to do it manually.
    QString copy = input;
    return copy.replace(";", "%3B");
}

void BugReporter::finished(QNetworkReply* reply)
{
    if (reply == credentialsValidationInProgress)
        credentialsValidationInProgress = nullptr;

    if (!replyToHandler.contains(reply))
    {
        reply->deleteLater();
        return;
    }

    bool success = (reply->error() == QNetworkReply::NoError);
    QString data;
    if (success)
        data = QString::fromLatin1(reply->readAll());
    else
        data = reply->errorString();

    replyToHandler[reply](success, data);
    replyToHandler.remove(reply);

    if (replyToTypeAndTitle.contains(reply))
    {
        if (success)
            CFG->addReportHistory(replyToTypeAndTitle[reply].first, replyToTypeAndTitle[reply].second, data);

        replyToTypeAndTitle.remove(reply);
    }

    reply->deleteLater();
}

void BugReporter::reportBug(const QString& email, const QString& title, const QString& details, const QString& version, const QString& os, const QString& plugins,
                            ResponseHandler responseHandler)
{
    QUrlQuery query;
    query.addQueryItem("byEmail", email);
    QString urlSuffix = "&" + query.query(QUrl::FullyEncoded);

    reportBug(title, details, version, os, plugins, responseHandler,  urlSuffix);
}

void BugReporter::reportBug(const QString& title, const QString& details, const QString& version, const QString& os,
                            const QString& plugins, ResponseHandler responseHandler)
{
    QString user = CFG_CORE.Internal.BugReportUser.get();
    QString pass = CFG_CORE.Internal.BugReportPassword.get();

    QUrlQuery query;
    query.addQueryItem("byUser", user);
    query.addQueryItem("password", pass);
    QString urlSuffix = "&" + query.query(QUrl::FullyEncoded);

    reportBug(title, details, version, os, plugins, responseHandler, urlSuffix);
}

void BugReporter::requestFeature(const QString& email, const QString& title, const QString& details, ResponseHandler responseHandler)
{
    QUrlQuery query;
    query.addQueryItem("byEmail", email);
    QString urlSuffix = "&" + query.query(QUrl::FullyEncoded);

    requestFeature(title, details, responseHandler, urlSuffix);
}

void BugReporter::requestFeature(const QString& title, const QString& details, ResponseHandler responseHandler)
{
    QString user = CFG_CORE.Internal.BugReportUser.get();
    QString pass = CFG_CORE.Internal.BugReportPassword.get();

    QUrlQuery query;
    query.addQueryItem("byUser", user);
    query.addQueryItem("password", pass);
    QString urlSuffix = "&" + query.query(QUrl::FullyEncoded);

    requestFeature(title, details, responseHandler, urlSuffix);
}
