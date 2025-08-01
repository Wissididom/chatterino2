#pragma once

#include "Application.hpp"
#include "common/ChatterinoSetting.hpp"
#include "singletons/WindowManager.hpp"
#include "widgets/buttons/SignalLabel.hpp"

#include <boost/variant.hpp>
#include <pajlada/signals/signalholder.hpp>
#include <QComboBox>
#include <QDebug>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

#include <utility>

class QScrollArea;

namespace chatterino {

class ColorButton;

class Space : public QLabel
{
    Q_OBJECT
};

class TitleLabel : public QLabel
{
    Q_OBJECT

public:
    TitleLabel(const QString &text)
        : QLabel(text)
    {
    }
};

class SubtitleLabel : public QLabel
{
    Q_OBJECT

public:
    SubtitleLabel(const QString &text)
        : QLabel(text)
    {
    }
};

class NavigationLabel : public SignalLabel
{
    Q_OBJECT

public:
    NavigationLabel(const QString &text)
        : SignalLabel()
    {
        this->setText(text);
    }
};

class DescriptionLabel : public QLabel
{
    Q_OBJECT

public:
    DescriptionLabel(const QString &text)
        : QLabel(text)
    {
    }
};

class ComboBox : public QComboBox
{
    Q_OBJECT

protected:
    void wheelEvent(QWheelEvent *event) override
    {
        event->ignore();
    }
};

class SpinBox : public QSpinBox
{
    Q_OBJECT

public:
    SpinBox(QWidget *parent = nullptr)
        : QSpinBox(parent)
    {
        // QAbstractSpinBox defaults to Qt::WheelFocus
        this->setFocusPolicy(Qt::StrongFocus);
    }

protected:
    void wheelEvent(QWheelEvent *event) override
    {
        event->ignore();
    }
};

struct DropdownArgs {
    QString value;
    int index;
    QComboBox *combobox;
};

class GeneralPageView : public QWidget
{
    Q_OBJECT
    GeneralPageView(QWidget *parent = nullptr);

public:
    static GeneralPageView *withNavigation(QWidget *parent);
    static GeneralPageView *withoutNavigation(QWidget *parent);

    void addWidget(QWidget *widget, const QStringList &keywords = {});

    /// Register the widget with the given keywords.
    /// This assumes that the widget is being held by a layout that has been added previously
    void registerWidget(QWidget *widget, const QStringList &keywords,
                        QWidget *parentElement);

    /// Pushes the widget into the current layout
    void pushWidget(QWidget *widget);

    void addLayout(QLayout *layout);
    void addStretch();

    TitleLabel *addTitle(const QString &text);
    SubtitleLabel *addSubtitle(const QString &text);

    ComboBox *addDropdown(const QString &text, const QStringList &items,
                          QString toolTipText = {});
    void addNavigationSpacing();

    template <typename OnClick>
    QPushButton *makeButton(const QString &text, OnClick onClick)
    {
        auto *button = new QPushButton(text);
        this->groups_.back().widgets.push_back({button, {text}});
        QObject::connect(button, &QPushButton::clicked, onClick);
        return button;
    }

    template <typename OnClick>
    QPushButton *addButton(const QString &text, OnClick onClick)
    {
        auto button = makeButton(text, onClick);
        auto *layout = new QHBoxLayout();
        layout->addWidget(button);
        layout->addStretch(1);
        this->addLayout(layout);
        return button;
    }

    template <typename T>
    ComboBox *addDropdown(
        const QString &text, const QStringList &items,
        pajlada::Settings::Setting<T> &setting,
        std::function<boost::variant<int, QString>(T)> getValue,
        std::function<T(DropdownArgs)> setValue, bool editable = true,
        QString toolTipText = {}, bool listenToActivated = false)
    {
        auto items2 = items;
        auto selected = getValue(setting.getValue());

        if (selected.which() == 1)
        {
            // QString
            if (!editable && !items2.contains(boost::get<QString>(selected)))
            {
                items2.insert(0, boost::get<QString>(selected));
            }
        }

        auto *combo = this->addDropdown(text, items2, toolTipText);
        if (editable)
        {
            combo->setEditable(true);
        }

        if (selected.which() == 0)
        {
            // int
            auto value = boost::get<int>(selected);
            if (value >= 0 && value < items2.size())
            {
                combo->setCurrentIndex(value);
            }
        }
        else if (selected.which() == 1)
        {
            // QString
            combo->setEditText(boost::get<QString>(selected));
        }

        setting.connect(
            [getValue = std::move(getValue), combo](const T &value, auto) {
                auto var = getValue(value);
                if (var.which() == 0)
                {
                    combo->setCurrentIndex(boost::get<int>(var));
                }
                else
                {
                    combo->setCurrentText(boost::get<QString>(var));
                    combo->setEditText(boost::get<QString>(var));
                }
            },
            this->managedConnections_);

        auto updateSetting = [combo, &setting, setValue = std::move(setValue)](
                                 const int newIndex) {
            setting = setValue(DropdownArgs{
                .value = combo->itemText(newIndex),
                .index = combo->currentIndex(),
                .combobox = combo,
            });
            getApp()->getWindows()->forceLayoutChannelViews();
        };

        if (listenToActivated)
        {
            QObject::connect(combo, &QComboBox::activated, updateSetting);
        }
        else
        {
            QObject::connect(
                combo,
                QOverload<const int>::of(&QComboBox::currentIndexChanged),
                updateSetting);
        }

        return combo;
    }

    DescriptionLabel *addDescription(const QString &text);

    void addSeparator();
    bool filterElements(const QString &query);

protected:
    void resizeEvent(QResizeEvent *event) override
    {
        (void)event;
    }

private:
    void updateNavigationHighlighting();
    void addToolTip(QWidget &widget, QString text) const;

    struct Widget {
        /// The element of the register widget
        /// This can point to the label of the widget, or the action widget (e.g. the spinbox)
        QWidget *element{};
        QStringList keywords;

        /// The optional parent element of the widget (usually pointing at a SettingWidget)
        QWidget *parentElement{};
    };

    struct Group {
        QString name;
        QWidget *title{};
        QWidget *navigationLink{};
        Space *space{};
        std::vector<Widget> widgets;
    };

    QScrollArea *contentScrollArea_ = nullptr;
    QVBoxLayout *contentLayout_ = nullptr;
    QVBoxLayout *navigationLayout_ = nullptr;

    std::vector<Group> groups_;
    pajlada::Signals::SignalHolder managedConnections_;
};

}  // namespace chatterino
