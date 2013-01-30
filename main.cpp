#include <QDebug>
#include <QDir>
#include <QProcess>
#include <QTest>

bool runFullySynchronous(const QString &workingDirectory,
                         const QString &binary,
                         const QStringList &arguments,
                         QByteArray* outputText)
{
    if (binary.isEmpty())
        return false;

    QProcess process;
    process.setWorkingDirectory(workingDirectory);

    process.start(binary, arguments);
    process.closeWriteChannel();
    process.waitForFinished();

    *outputText = process.readAllStandardOutput();

    return process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0;
}

QString oldFindRepositoryForDirectory(const QString &dir)
{
    // Find a directory to run git in:
    const QString root = QDir::rootPath();
    const QString home = QDir::homePath();

    QDir directory(dir);
    do {
        const QString absDirPath = directory.absolutePath();
        if (absDirPath == root || absDirPath == home)
            break;

        if (directory.exists())
            break;
    } while (directory.cdUp());

    QByteArray outputText;
    QStringList arguments;
    arguments << QLatin1String("rev-parse") << QLatin1String("--show-toplevel");
    runFullySynchronous(directory.absolutePath(), QLatin1String("git"), arguments, &outputText);
    return QString::fromLocal8Bit(outputText.trimmed());
}

QString newFindRepositoryForDirectory(const QString &dir)
{
    QDir directory(dir);
    QString dotGit = QLatin1String(".git");
    do {
        if (directory.exists(dotGit))
            return directory.absolutePath();
    } while (directory.cdUp());
    return QString();
}

class GitTest : public QObject
{
    Q_OBJECT
private slots:
    void process();
    void traverse();
};

void GitTest::process()
{
    QString directory = QLatin1String("/home/shaneh/dev/qt-creator/src/plugins/git/gerrit");
    QBENCHMARK {
        for (int i = 0; i < 300; ++i)
            oldFindRepositoryForDirectory(directory);
    }
}

void GitTest::traverse()
{
    QString directory = QLatin1String("/home/shaneh/dev/qt-creator/src/plugins/git/gerrit");
    QBENCHMARK {
        for (int i = 0; i < 300; ++i)
            newFindRepositoryForDirectory(directory);
    }
}

QTEST_MAIN(GitTest)

#include "main.moc"
