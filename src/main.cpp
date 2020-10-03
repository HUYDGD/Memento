#include <QApplication>
#include <QDebug>

#include "dictionary/dictionarymanager.h"
#include "gui/mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication memento(argc, argv);
    // Qt sets the locale in the QApplication constructor, but libmpv requires
    // the LC_NUMERIC category to be set to "C", so change it back.
    setlocale(LC_NUMERIC, "C");

    qDebug() << "Loading dictionary";
    DictionaryManager::prepareDictionary(DictionaryType::JAPANESE_DEFAULT);
    qDebug() << "Dictionary loaded";

    MainWindow main_window;
    main_window.show();
    return memento.exec();
}
