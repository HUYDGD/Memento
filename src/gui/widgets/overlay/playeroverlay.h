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

#ifndef PLAYEROVERLAY_H
#define PLAYEROVERLAY_H

#include <QVBoxLayout>

#include <QPointer>
#include <QPropertyAnimation>
#include <QSharedPointer>
#include <QTimer>

class DefinitionWidget;
class PlayerControls;
class PlayerMenu;
class SubtitleWidget;

struct Term;
typedef QSharedPointer<Term> SharedTerm;
typedef QSharedPointer<QList<SharedTerm>> SharedTermList;

struct Kanji;
typedef QSharedPointer<Kanji> SharedKanji;

/**
 * Widget for overlaying controls over the player.
 */
class PlayerOverlay : public QVBoxLayout
{
    Q_OBJECT

public:
    /**
     * Creates an overlay over the player.
     * @param parent The player widget.
     */
    PlayerOverlay(QWidget *parent);
    ~PlayerOverlay();

public Q_SLOTS:
    /**
     * Returns if the cursor if over the children of the player.
     * @return true if the cursor is over part of the overlay, false otherwise.
     */
    bool underMouse() const;

    /**
     * Hides the overlay if it should be hidden.
     */
    void hideOverlay();

    /**
     * Shows the overlay if it should be shown.
     */
    void showOverlay();

private Q_SLOTS:
    /**
     * Initializes the settings relevant to the overlay.
     */
    void initSettings();

    /**
     * Cycles the OSC visibility.
     */
    void cycleOSCVisibility();

    /**
     * Handles mouse movement events from the player.
     */
    void handleMouseMovement();

    /**
     * Handles updating the mouse movement information when the tick timer
     * fires.
     */
    void handleTickTimeout();

    /**
     * Opens a new DefinitionWidget populated with the terms in the list.
     * @param terms A list of terms to display.
     * @param kanji A kanji if found.
     */
    void setTerms(SharedTermList terms, SharedKanji kanji);

    /**
     * Repositions the subtitles over the player. Used for keeping the subtitle
     * widget in the correct place when showing the overlay.
     */
    void repositionSubtitles();

    /**
     * Increases the size of the subtitle widget.
     */
    void increaseSubScale();

    /**
     * Decreases the size of the subtitle widget.
     */
    void decreaseSubScale();

    /**
     * Moves the subtitle widget up.
     */
    void moveSubsUp();

    /**
     * Moves the subtitle widget down.
     */
    void moveSubsDown();

    /**
     * Moves the DefinitionWidget to a position over the player relative to the
     * cursor.
     */
    void setDefinitionWidgetLocation();

private:
    /**
     * Changes the subtitle scale. Saved in settings.
     * @param inc A value between -1.0 and 1.0 to increment the current scale
     *            by. If the final value is outside of 0.0 or 1.0, it is
     *            rounded.
     */
    void updateSubScale(const double inc);

    /**
     * Changes the subtitle position. Saved in settings.
     * @param inc A value between -1.0 and 1.0 to increment the position by. If
     *            the final value is outside of 0.0 or 1.0, it is rounded.
     */
    void moveSubtitles(const double inc);

    /* Saved pointer to the player. Does not have ownership. */
    QWidget *m_player;

    /* Saved pointer to the current definition widget. Has ownership. */
    DefinitionWidget *m_definition;

    /* The menu widget */
    PlayerMenu *m_menu;

    /* The subtitle widget */
    SubtitleWidget *m_subtitle;

    /* A generic spacer widget for position the subtitles */
    QWidget *m_spacer;

    /* The widget containing the player controls */
    PlayerControls *m_controls;

    /* The amount of time before the OSC should be hidden */
    QTimer m_hideTimer;

    /* Holds all information relating to handling mouse ticks */
    struct MouseMovement
    {
        /* Fires once every tick */
        QTimer tickTimer;

        /* The last point the mouse was moved at */
        QPoint lastPoint;

        /* The point at the last tick */
        QPoint lastTickPoint;
    }
    m_mouseMovement;

    enum class OSCVisibility
    {
        /* Automatically hide and show the OSC */
        Auto,

        /* Keep the OSC hidden */
        Hidden,

        /* Keep the OSC visible */
        Visible
    };

    /* Settings saved to avoid querying QSettings */
    struct SavedSettings
    {
        /* The current OSC visibility */
        OSCVisibility visibility = OSCVisibility::Auto;

        /* Saved setting describing the subtitle offset from the bottom */
        double subOffset;

#if defined(Q_OS_WIN)
        /* true if the menubar should be shown in fullscreen on windows */
        bool showMenuBar = false;
#endif

        /* The saved OSC fade duration */
        int fadeDuration = 250;

        /* The minimum number of pixels the cursor needs to move before the OSC
         * is shown */
        int cursorMinMove = 0;
    }
    m_settings;

    /* A reference to the currently active property animation for the menu */
    QPointer<QPropertyAnimation> m_menuFade = nullptr;

    /* A reference to the currently active property animation for the
     * controls */
    QPointer<QPropertyAnimation> m_controlFade = nullptr;
};

#endif // PLAYEROVERLAY_H
