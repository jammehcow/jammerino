#include "appearancepage.hpp"

#include <QFontDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QSlider>
#include <QVBoxLayout>

#include "util/layoutcreator.hpp"
#include "util/removescrollareabackground.hpp"

#define THEME_ITEMS "White", "Light", "Dark", "Black"

#define TAB_X "Show close button"
#define TAB_PREF "Hide preferences button (ctrl+p to show)"
#define TAB_USER "Hide user button"

#define SCROLL_SMOOTH "Enable smooth scrolling"
#define SCROLL_NEWMSG "Enable smooth scrolling for new messages"

#define LAST_MSG "Mark the last message you read (dotted line)"

// clang-format off
#define TIMESTAMP_FORMATS "hh:mm a", "h:mm a", "hh:mm:ss a", "h:mm:ss a", "HH:mm", "H:mm", "HH:mm:ss", "H:mm:ss"
// clang-format on

namespace chatterino {
namespace widgets {
namespace settingspages {

AppearancePage::AppearancePage()
    : SettingsPage("Look", ":/images/theme.svg")
{
    singletons::SettingManager &settings = singletons::SettingManager::getInstance();
    util::LayoutCreator<AppearancePage> layoutCreator(this);

    auto scroll = layoutCreator.emplace<QScrollArea>();
    auto widget = scroll.emplaceScrollAreaWidget();
    util::removeScrollAreaBackground(*scroll, *widget);

    auto layout = widget.setLayoutType<QVBoxLayout>();

    auto application =
        layout.emplace<QGroupBox>("Application").emplace<QVBoxLayout>().withoutMargin();
    {
        auto form = application.emplace<QFormLayout>();

        // clang-format off
            form->addRow("Theme:",       this->createComboBox({THEME_ITEMS}, singletons::ThemeManager::getInstance().themeName));
            form->addRow("Theme color:", this->createThemeColorChanger());
            form->addRow("Font:",        this->createFontChanger());

            form->addRow("Tabs:",        this->createCheckBox(TAB_X, settings.showTabCloseButton));
    #ifndef USEWINSDK
            form->addRow("",             this->createCheckBox(TAB_PREF, settings.hidePreferencesButton));
            form->addRow("",             this->createCheckBox(TAB_USER, settings.hideUserButton));
    #endif

            form->addRow("Scrolling:",   this->createCheckBox(SCROLL_SMOOTH, settings.enableSmoothScrolling));
            form->addRow("",			 this->createCheckBox(SCROLL_NEWMSG, settings.enableSmoothScrollingNewMessages));
        // clang-format on
    }

    auto messages = layout.emplace<QGroupBox>("Messages").emplace<QVBoxLayout>();
    {
        messages.append(this->createCheckBox("Show timestamp", settings.showTimestamps));
        auto tbox = messages.emplace<QHBoxLayout>().withoutMargin();
        {
            tbox.emplace<QLabel>("timestamp format (a = am/pm):");
            tbox.append(this->createComboBox({TIMESTAMP_FORMATS}, settings.timestampFormat));
            tbox->addStretch(1);
        }

        messages.append(this->createCheckBox("Show badges", settings.showBadges));
        auto checkbox = this->createCheckBox("Seperate messages", settings.seperateMessages);
        checkbox->setEnabled(false);
        messages.append(checkbox);
        messages.append(
            this->createCheckBox("Show message length while typing", settings.showMessageLength));

        messages.append(this->createCheckBox(LAST_MSG, settings.showLastMessageIndicator));
    }

    auto emotes = layout.emplace<QGroupBox>("Emotes").setLayoutType<QVBoxLayout>();
    {
        emotes.append(this->createCheckBox("Enable Twitch emotes", settings.enableTwitchEmotes));
        emotes.append(
            this->createCheckBox("Enable BetterTTV emotes for Twitch", settings.enableBttvEmotes));
        emotes.append(this->createCheckBox("Enable FrankerFaceZ emotes for Twitch",
                                           settings.enableFfzEmotes));
        emotes.append(this->createCheckBox("Enable emojis", settings.enableEmojis));
        emotes.append(this->createCheckBox("Enable animations", settings.enableGifAnimations));
    }

    layout->addStretch(1);
}

QLayout *AppearancePage::createThemeColorChanger()
{
    QHBoxLayout *layout = new QHBoxLayout;

    auto &themeHue = singletons::ThemeManager::getInstance().themeHue;

    // SLIDER
    QSlider *slider = new QSlider(Qt::Horizontal);
    layout->addWidget(slider);
    slider->setValue(std::min(std::max(themeHue.getValue(), 0.0), 1.0) * 100);

    // BUTTON
    QPushButton *button = new QPushButton;
    layout->addWidget(button);
    button->setFlat(true);
    button->setFixedWidth(64);

    auto setButtonColor = [button](int value) mutable {
        double newValue = value / 100.0;
        singletons::ThemeManager::getInstance().themeHue.setValue(newValue);

        QPalette pal = button->palette();
        QColor color;
        color.setHsvF(newValue, 1.0, 1.0, 1.0);
        pal.setColor(QPalette::Button, color);
        button->setAutoFillBackground(true);
        button->setPalette(pal);
        button->update();
    };

    // SIGNALS
    QObject::connect(slider, &QSlider::valueChanged, this, setButtonColor);

    setButtonColor(themeHue * 100);

    return layout;
}

QLayout *AppearancePage::createFontChanger()
{
    QHBoxLayout *layout = new QHBoxLayout;

    auto &fontManager = singletons::FontManager::getInstance();

    // LABEL
    QLabel *label = new QLabel();
    layout->addWidget(label);

    auto updateFontFamilyLabel = [label, &fontManager](auto) {
        label->setText(QString::fromStdString(fontManager.currentFontFamily.getValue()) + ", " +
                       QString::number(fontManager.currentFontSize) + "pt");
    };

    fontManager.currentFontFamily.connectSimple(updateFontFamilyLabel, this->managedConnections);
    fontManager.currentFontSize.connectSimple(updateFontFamilyLabel, this->managedConnections);

    // BUTTON
    QPushButton *button = new QPushButton("Select");
    layout->addWidget(button);
    button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Policy::Fixed);

    QObject::connect(button, &QPushButton::clicked, []() {
        auto &fontManager = singletons::FontManager::getInstance();
        QFontDialog dialog(fontManager.getFont(singletons::FontManager::Medium, 1.));

        dialog.connect(&dialog, &QFontDialog::fontSelected, [](const QFont &font) {
            auto &fontManager = singletons::FontManager::getInstance();
            fontManager.currentFontFamily = font.family().toStdString();
            fontManager.currentFontSize = font.pointSize();
        });

        dialog.show();
        dialog.exec();
    });

    return layout;
}

}  // namespace settingspages
}  // namespace widgets
}  // namespace chatterino
