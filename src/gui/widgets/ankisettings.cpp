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

#include "ankisettings.h"
#include "ui_ankisettings.h"

#include "../../util/iconfactory.h"
#include "../../util/globalmediator.h"

#include <QTableWidgetItem>

#define DUPLICATE_POLICY_NONE       "None"
#define DUPLICATE_POLICY_DIFFERENT  "Allowed in Different Decks"
#define DUPLICATE_POLICY_SAME       "Allowed in Same Deck"

#define SCREENSHOT_PNG              "PNG"
#define SCREENSHOT_JPG              "JPG"
#define SCREENSHOT_WEBP             "WebP"

#define DEFAULT_PROFILE             "Default"
#define DEFAULT_HOST                "localhost"
#define DEFAULT_PORT                "8765"
#define DEFAULT_SCREENSHOT          AnkiConfig::FileType::jpg
#define DEFAULT_DUPLICATE_POLICY    AnkiConfig::DuplicatePolicy::DifferentDeck
#define DEFAULT_TAGS                "memento"

#define REGEX_REMOVE_SPACES_COMMAS "[, ]+"

AnkiSettings::AnkiSettings(QWidget *parent)
    : QWidget(parent),
      m_ui(new Ui::AnkiSettings),
      m_ankiSettingsHelp(new AnkiSettingsHelp),
      m_configs(0)
{
    m_ui->setupUi(this);

    IconFactory *factory = IconFactory::create(this);
    m_ui->buttonAdd->setIcon(factory->getIcon(IconFactory::Icon::plus));
    m_ui->buttonDelete->setIcon(factory->getIcon(IconFactory::Icon::minus));
    delete factory;

    connect(m_ui->checkBoxEnabled, &QCheckBox::stateChanged,
            this, &AnkiSettings::enabledStateChanged);
    connect(m_ui->comboBoxProfile, &QComboBox::currentTextChanged,
            this, &AnkiSettings::changeProfile);
    connect(m_ui->buttonConnect, &QPushButton::clicked,
            [=] { connectToClient(true); });
    connect(m_ui->termCardBuilder, &CardBuilder::modelTextChanged,
            this, &AnkiSettings::updateModelFields);

    // Dialog Buttons
    connect(m_ui->buttonBox->button(QDialogButtonBox::StandardButton::Reset),
            &QPushButton::clicked, this, &AnkiSettings::restoreSaved);
    connect(m_ui->buttonBox->button(
                QDialogButtonBox::StandardButton::RestoreDefaults),
            &QPushButton::clicked, this, &AnkiSettings::restoreDefaults);
    connect(m_ui->buttonBox->button(QDialogButtonBox::StandardButton::Apply),
            &QPushButton::clicked, this, &AnkiSettings::applyChanges);
    connect(m_ui->buttonBox->button(QDialogButtonBox::StandardButton::Close),
            &QPushButton::clicked, this, &AnkiSettings::hide);
    connect(m_ui->buttonBox->button(QDialogButtonBox::StandardButton::Help),
        &QPushButton::clicked, m_ankiSettingsHelp, &AnkiSettingsHelp::show);

    // Profile actions
    connect(m_ui->buttonAdd, &QToolButton::clicked,
            this, &AnkiSettings::addProfile);
    connect(m_ui->buttonDelete, &QToolButton::clicked,
            this, &AnkiSettings::deleteProfile);
    connect(m_ui->comboBoxProfile, &QComboBox::currentTextChanged,
            this, &AnkiSettings::changeProfile);
}

AnkiSettings::~AnkiSettings()
{
    delete m_ui;
    delete m_ankiSettingsHelp;
    clearConfigs();
}

void AnkiSettings::clearConfigs()
{
    if (m_configs)
    {
        for (AnkiConfig *config : *m_configs)
        {
            delete config;
        }
        delete m_configs;
        m_configs = 0;
    }
}

void AnkiSettings::showEvent(QShowEvent *event)
{
    AnkiClient *client = GlobalMediator::getGlobalMediator()->getAnkiClient();
    m_ui->checkBoxEnabled->setChecked(client->isEnabled());
    m_configs        = client->getConfigs();
    m_currentProfile = client->getProfile();
    populateFields(client->getProfile(), m_configs->value(client->getProfile()));
    connectToClient(false);
}

void AnkiSettings::hideEvent(QHideEvent *event)
{
    AnkiClient       *client = GlobalMediator::getGlobalMediator()->getAnkiClient();
    const AnkiConfig *config = client->getConfig(client->getProfile());
    client->setServer(config->address, config->port);
    clearConfigs();
}

void AnkiSettings::enabledStateChanged(int state)
{
    bool enabled = state == Qt::CheckState::Checked;

    // Buttons
    m_ui->buttonConnect->setEnabled(enabled);
    m_ui->buttonAdd->setEnabled(enabled);
    m_ui->buttonDelete->setEnabled(enabled);

    // Combo Boxes
    m_ui->comboBoxProfile->setEnabled(enabled);
    m_ui->comboBoxDuplicates->setEnabled(enabled);
    m_ui->comboBoxScreenshot->setEnabled(enabled);

    // Labels
    m_ui->labelHostName->setEnabled(enabled);
    m_ui->labelPort->setEnabled(enabled);
    m_ui->labelTags->setEnabled(enabled);
    m_ui->labelProfile->setEnabled(enabled);
    m_ui->labelProfileName->setEnabled(enabled);
    m_ui->labelDuplicates->setEnabled(enabled);
    m_ui->labelScreenshot->setEnabled(enabled);

    // Line Edits
    m_ui->lineEditHost->setEnabled(enabled);
    m_ui->lineEditPort->setEnabled(enabled);
    m_ui->lineEditTags->setEnabled(enabled);
    m_ui->lineEditProfileName->setEnabled(enabled);

    // Tabs
    m_ui->tabTermKanji->setEnabled(enabled);
    m_ui->termCardBuilder->setEnabled(enabled);
    m_ui->kanjiCardBuilder->setEnabled(enabled);
}

void AnkiSettings::addProfile()
{
    QString profileName = m_ui->lineEditProfileName->text();
    if (m_configs->contains(profileName))
    {
        Q_EMIT GlobalMediator::getGlobalMediator()->showInformation(
            "Failed",
            "Profile with name " + profileName + " already exists."
        );
    }
    else
    {
        m_configs->insert(
            profileName,
            new AnkiConfig(
                *m_configs->value(m_ui->comboBoxProfile->currentText())));

        m_ui->comboBoxProfile->blockSignals(true);
        m_ui->comboBoxProfile->addItem(profileName);
        m_ui->comboBoxProfile->setCurrentText(profileName);
        m_ui->comboBoxProfile->model()->sort(0);
        m_ui->comboBoxProfile->blockSignals(false);

        m_currentProfile = profileName;
    }
}

void AnkiSettings::deleteProfile()
{
    QString profile = m_ui->comboBoxProfile->currentText();
    if (profile == DEFAULT_PROFILE)
    {
        Q_EMIT GlobalMediator::getGlobalMediator()->showInformation(
            "Failed",
            "The Default profile cannot be deleted"
        );
    }
    else
    {
        delete m_configs->value(profile);
        m_configs->remove(profile);

        m_ui->comboBoxProfile->blockSignals(true);
        m_ui->comboBoxProfile->removeItem(
            m_ui->comboBoxProfile->currentIndex());
        populateFields(
            m_ui->comboBoxProfile->currentText(),
            m_configs->value(m_ui->comboBoxProfile->currentText()));
        m_ui->comboBoxProfile->blockSignals(false);

        m_currentProfile = m_ui->comboBoxProfile->currentText();
    }
}

void AnkiSettings::changeProfile(const QString &text)
{
    applyToConfig(m_currentProfile);
    if (m_currentProfile != m_ui->lineEditProfileName->text())
        renameProfile(m_currentProfile, m_ui->lineEditProfileName->text());
    populateFields(text, m_configs->value(text));
    m_currentProfile = text;
}

void AnkiSettings::connectToClient(const bool showErrors)
{
    m_ui->buttonConnect->setEnabled(false);

    AnkiClient *client = GlobalMediator::getGlobalMediator()->getAnkiClient();
    client->setServer(m_ui->lineEditHost->text(), m_ui->lineEditPort->text());

    AnkiReply *reply = client->testConnection();
    connect(reply, &AnkiReply::finishedBool,
        [=] (const bool val, const QString &error) {
        if (val)
        {
            AnkiReply *reply = client->getDeckNames();
            connect(reply, &AnkiReply::finishedStringList,
                [=] (const QStringList &decks, const QString &error) {
                    if (error.isEmpty())
                    {
                        m_ui->termCardBuilder->setDecks(decks, client->getConfig(client->getProfile())->deck);
                    }
                    else if (showErrors)
                    {
                        Q_EMIT GlobalMediator::getGlobalMediator()->showCritical("Error", error);
                    }
                });
            
            reply = client->getModelNames();
            connect(reply, &AnkiReply::finishedStringList,
                [=] (const QStringList &models, const QString &error) {
                    if (error.isEmpty())
                    {
                        m_ui->termCardBuilder->blockSignals(true);
                        m_ui->termCardBuilder->setModels(models, client->getConfig(client->getProfile())->model);
                        m_ui->termCardBuilder->blockSignals(false);
                    }
                    else if (showErrors)
                    {
                        Q_EMIT GlobalMediator::getGlobalMediator()->showCritical("Error", error);
                    }
                });
        }
        else if (showErrors)
        {
            Q_EMIT GlobalMediator::getGlobalMediator()->showCritical("Error", error);
        }

        m_ui->buttonConnect->setEnabled(m_ui->checkBoxEnabled->isChecked());
    });
}

void AnkiSettings::updateModelFields(const QString &model)
{
    m_mutexUpdateModelFields.lock();
    AnkiClient *client = GlobalMediator::getGlobalMediator()->getAnkiClient();
    AnkiReply *reply = client->getFieldNames(model);
    connect(reply, &AnkiReply::finishedStringList,
        [=] (const QStringList &fields, const QString error) {
            if (error.isEmpty())
            {
                m_ui->termCardBuilder->setFields(fields);
            }
            else
            {
                Q_EMIT GlobalMediator::getGlobalMediator()->showCritical("Error", error);
            }
            m_mutexUpdateModelFields.unlock();
        });
}

void AnkiSettings::applyChanges()
{
    // Renaming profile if changed
    if (m_ui->comboBoxProfile->currentText() !=
        m_ui->lineEditProfileName->text())
    {
        renameProfile(
            m_ui->comboBoxProfile->currentText(),
            m_ui->lineEditProfileName->text()
        );
    }

    applyToConfig(m_ui->comboBoxProfile->currentText());

    // Apply changes to the client
    AnkiClient *client = GlobalMediator::getGlobalMediator()->getAnkiClient();
    client->setEnabled(m_ui->checkBoxEnabled->isChecked());
    client->clearProfiles();
    for (auto it = m_configs->constKeyValueBegin(); it != m_configs->constKeyValueEnd(); ++it)
    {
        client->addProfile(it->first, *it->second);
    }
    client->setProfile(m_ui->comboBoxProfile->currentText());

    // Write the changes to the config file
    client->writeChanges();
}

void AnkiSettings::restoreDefaults()
{
    AnkiConfig defaultConfig;
    defaultConfig.address         = DEFAULT_HOST;
    defaultConfig.port            = DEFAULT_PORT;
    defaultConfig.duplicatePolicy = DEFAULT_DUPLICATE_POLICY;
    defaultConfig.screenshotType  = DEFAULT_SCREENSHOT;
    defaultConfig.tags.append(DEFAULT_TAGS);
    defaultConfig.deck            = m_ui->termCardBuilder->getDeckText();
    defaultConfig.model           = m_ui->termCardBuilder->getModelText();
    QStringList fields            = m_configs->value(m_ui->comboBoxProfile->currentText())->fields.keys();
    for (const QString &field : fields)
        defaultConfig.fields[field] = "";
    populateFields(m_ui->comboBoxProfile->currentText(), &defaultConfig);
}

void AnkiSettings::restoreSaved()
{
    clearConfigs();
    AnkiClient *client = GlobalMediator::getGlobalMediator()->getAnkiClient();
    m_configs = client->getConfigs();

    m_ui->comboBoxProfile->blockSignals(true);
    m_ui->comboBoxProfile->clear();
    for (auto it = m_configs->keyBegin(); it != m_configs->keyEnd(); ++it)
    {
        m_ui->comboBoxProfile->addItem(*it);
    }
    m_ui->comboBoxProfile->model()->sort(0);
    m_ui->comboBoxProfile->blockSignals(false);

    populateFields(client->getProfile(), client->getConfig(client->getProfile()));

    m_currentProfile = client->getProfile();
}

void AnkiSettings::populateFields(const QString &profile,
                                  const AnkiConfig *config)
{
    AnkiClient *client = GlobalMediator::getGlobalMediator()->getAnkiClient();

    m_ui->comboBoxProfile->blockSignals(true);
    m_ui->comboBoxProfile->clear();
    for (auto it = m_configs->keyBegin(); it != m_configs->keyEnd(); ++it)
        m_ui->comboBoxProfile->addItem(*it);
    m_ui->comboBoxProfile->setCurrentText(profile);
    m_ui->comboBoxProfile->model()->sort(0);
    m_ui->comboBoxProfile->blockSignals(false);

    m_ui->lineEditProfileName->setText(profile);

    m_ui->lineEditHost->setText(config->address);
    m_ui->lineEditPort->setText(config->port);

    client->setServer(config->address, config->port);

    m_ui->comboBoxDuplicates->setCurrentText(
        duplicatePolicyToString(config->duplicatePolicy));
    
    m_ui->comboBoxScreenshot->setCurrentText(
        fileTypeToString(config->screenshotType));

    QString tags;
    for (auto it = config->tags.begin(); it != config->tags.end(); ++it)
        tags += it->toString() + ",";
    tags.chop(1);
    m_ui->lineEditTags->setText(tags);

    m_ui->termCardBuilder->blockSignals(true);
    m_ui->termCardBuilder->setDeckCurrentText(config->deck);
    m_ui->termCardBuilder->setModelCurrentText(config->model);
    m_ui->termCardBuilder->blockSignals(false);

    m_ui->termCardBuilder->setFields(config->fields);
}

QString AnkiSettings::duplicatePolicyToString(
    AnkiConfig::DuplicatePolicy policy)
{
    switch(policy)
    {
    case AnkiConfig::DuplicatePolicy::None:
        return DUPLICATE_POLICY_NONE;
    case AnkiConfig::DuplicatePolicy::DifferentDeck:
        return DUPLICATE_POLICY_DIFFERENT;
    case AnkiConfig::DuplicatePolicy::SameDeck:
        return DUPLICATE_POLICY_SAME;
    default:
        return DUPLICATE_POLICY_DIFFERENT;
    }
}

AnkiConfig::DuplicatePolicy AnkiSettings::stringToDuplicatePolicy(
    const QString &str)
{
    if (str == DUPLICATE_POLICY_NONE)
    {
        return AnkiConfig::DuplicatePolicy::None;
    }
    else if (str == DUPLICATE_POLICY_DIFFERENT)
    {
        return AnkiConfig::DuplicatePolicy::DifferentDeck;
    }
    else if (str == DUPLICATE_POLICY_SAME)
    {
        return AnkiConfig::DuplicatePolicy::SameDeck;
    }

    qDebug() << "Invalid duplicate policy string:" << str;
    return DEFAULT_DUPLICATE_POLICY;
}

QString AnkiSettings::fileTypeToString(AnkiConfig::FileType type)
{
    switch(type)
    {
    case AnkiConfig::FileType::png:
        return SCREENSHOT_PNG;
    case AnkiConfig::FileType::jpg:
        return SCREENSHOT_JPG;
    case AnkiConfig::FileType::webp:
        return SCREENSHOT_WEBP;
    default:
        return SCREENSHOT_JPG;
    }
}

AnkiConfig::FileType AnkiSettings::stringToFileType(const QString &str)
{
    if (str == SCREENSHOT_JPG)
    {
        return AnkiConfig::FileType::jpg;
    }
    else if (str == SCREENSHOT_PNG)
    {
        return AnkiConfig::FileType::png;
    }
    else if (str == SCREENSHOT_WEBP)
    {
        return AnkiConfig::FileType::webp;
    }

    qDebug() << "Invalid file type string:" << str;
    return DEFAULT_SCREENSHOT;
}

void AnkiSettings::applyToConfig(const QString &profile)
{
    AnkiConfig *config = m_configs->value(profile);

    config->address = m_ui->lineEditHost->text();
    config->port = m_ui->lineEditPort->text();

    config->duplicatePolicy =
        stringToDuplicatePolicy(m_ui->comboBoxDuplicates->currentText());
    config->screenshotType =
        stringToFileType(m_ui->comboBoxScreenshot->currentText());

    config->tags = QJsonArray();
    QStringList splitTags =
        m_ui->lineEditTags->text().split(QRegExp(REGEX_REMOVE_SPACES_COMMAS));
    for (auto it = splitTags.begin(); it != splitTags.end(); ++it)
        config->tags.append(*it);

    if (!m_ui->termCardBuilder->getDeckText().isEmpty())
    {
        config->deck = m_ui->termCardBuilder->getDeckText();
    }
    if (!m_ui->termCardBuilder->getModelText().isEmpty())
    {
        config->model = m_ui->termCardBuilder->getModelText();
    }

    config->fields = m_ui->termCardBuilder->getFields();
}

void AnkiSettings::renameProfile(const QString &oldName, const QString &newName)
{

    if (oldName == DEFAULT_PROFILE)
    {
        Q_EMIT GlobalMediator::getGlobalMediator()->showInformation(
            "Info",
            "Default profile cannnot be renamed"
        );
        m_ui->lineEditProfileName->setText(DEFAULT_PROFILE);
    }
    else if (newName.isEmpty())
    {
        Q_EMIT GlobalMediator::getGlobalMediator()->showInformation(
            "Info",
            "Profile must have a name"
        );
        m_ui->lineEditProfileName->setText(oldName);
    }
    else
    {
        m_configs->insert(newName, m_configs->value(oldName));
        m_configs->remove(oldName);

        m_ui->comboBoxProfile->blockSignals(true);
        m_ui->comboBoxProfile->clear();
        for (auto it = m_configs->keyBegin(); it != m_configs->keyEnd(); ++it)
        {
            m_ui->comboBoxProfile->addItem(*it);
        }
        m_ui->comboBoxProfile->setCurrentText(newName);
        m_ui->comboBoxProfile->model()->sort(0);
        m_ui->comboBoxProfile->blockSignals(false);
    }
}
