#include <QCoreApplication>
#include <QDateTime>
#include <QTextStream>

#include "LogWatcher.h"
#include "RecordingController.h"

// Dev tool: point LogWatcher at a real WoW Logs directory, print every
// parsed line, and feed it through RecordingController so its start/stop
// decisions can be sanity-checked against a live client before ObsEngine
// exists to act on them. Not part of the shipped daemon.
int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    if (argc < 2) {
        QTextStream(stderr) << "usage: logtail <path-to-Logs-directory> [idle-timeout-ms]\n";
        return 1;
    }

    const QString directory = QString::fromLocal8Bit(argv[1]);
    const int idleTimeoutMs = argc > 2 ? QString::fromLocal8Bit(argv[2]).toInt() : 60'000;

    LogWatcher watcher(directory, idleTimeoutMs);

    // LFR here (rather than the library default of Normal) so this dev tool
    // actually shows a START/STOP round trip regardless of what difficulty
    // is being tested against live.
    RecordingController::Config config;
    config.minDifficulty = RecordingController::RaidDifficulty::LFR;
    RecordingController controller(config);

    QTextStream out(stdout);
    QObject::connect(&watcher, &LogWatcher::lineReceived, [&out, &controller](const LogLine &line) {
        out << "[" << line.dateTime().toString(Qt::ISODateWithMs) << "] " << line.type() << " ("
            << line.argCount() << " args): " << line.raw() << "\n";
        out.flush();
        controller.onLineReceived(line);
    });
    QObject::connect(&watcher, &LogWatcher::idleTimeout, [&out]() {
        out << ">>> idle timeout <<<\n";
        out.flush();
    });
    QObject::connect(
        &controller, &RecordingController::recordingStarted,
        [&out](const RecordingController::RaidEncounter &encounter, const QDateTime &preRollFrom) {
            out << "=== RECORDING START: " << encounter.encounterName << " (difficultyID "
                << encounter.difficultyId
                << ") preRollFrom=" << preRollFrom.toString(Qt::ISODateWithMs) << " ===\n";
            out.flush();
        });
    QObject::connect(&controller, &RecordingController::recordingStopped,
                     [&out](const RecordingController::RaidEncounter &encounter, bool success,
                            const QDateTime &stopTime) {
                         out << "=== RECORDING STOP: " << encounter.encounterName
                             << (success ? " (KILL)" : " (WIPE)")
                             << " stopTime=" << stopTime.toString(Qt::ISODateWithMs) << " ===\n";
                         out.flush();
                     });

    if (!watcher.start()) {
        QTextStream(stderr) << "logtail: failed to watch " << directory << "\n";
        return 1;
    }

    out << "watching " << directory << " ...\n";
    out.flush();
    return app.exec();
}
