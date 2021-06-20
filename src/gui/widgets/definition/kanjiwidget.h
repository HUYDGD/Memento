////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2021 Ripose
//
// This file is part of Memento.
//
// Memento is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2 of the License.
//
// Memento is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Memento.  If not, see <https://www.gnu.org/licenses/>.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef KANJIWIDGET_H
#define KANJIWIDGET_H

#include <QWidget>

#include "../../../dict/expression.h"

class QLabel;
class QFrame;
class QVBoxLayout;
class QToolButton;

class KanjiWidget : public QWidget
{
    Q_OBJECT

public:
    KanjiWidget(const Kanji *kanji, QWidget *parent = nullptr);
    ~KanjiWidget();

Q_SIGNALS:
    void backPressed();

private Q_SLOTS:
    void addKanji();
    void openAnki();

private:
    const Kanji *m_kanji;
    QToolButton *m_buttonAnkiAdd;
    QToolButton *m_buttonAnkiOpen;

    void buildDefinitionLabel(const KanjiDefinition &def, QVBoxLayout *layout);
    QFrame *createLine();
    QLabel *createLabel(const QString &text,
                        const bool bold = false,
                        const Qt::AlignmentFlag alignment = Qt::AlignLeft);
    QLayout *createKVLabel(const QString &key, const QString &value);
    void addKVSection(const QString &title, const QList<QPair<Tag, QString>> &pairs, QVBoxLayout *layout);
};

#endif // KANJIWIDGET_H