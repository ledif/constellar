#include <QApplication>
#include <QIcon>
#include <QQmlApplicationEngine>

using namespace Qt::StringLiterals;

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QApplication::setWindowIcon(QIcon::fromTheme(u"applications-games"_s));

    QQmlApplicationEngine engine;
    engine.loadFromModule("io.github.ledif.constellar", "Main");
    if (engine.rootObjects().isEmpty())
        return -1;

    return QApplication::exec();
}
