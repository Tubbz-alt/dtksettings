/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "widgetfactory.h"

#include <QDebug>
#include <QMap>
#include <QFrame>
#include <QLabel>
#include <QEvent>
#include <QCheckBox>
#include <QLineEdit>
#include <QSlider>
#include <QSpinBox>
#include <QGridLayout>

#include <option.h>

#include "widget/shortcutedit.h"
#include "widget/buttongroup.h"
#include "widget/combobox.h"

DWIDGET_BEGIN_NAMESPACE

QWidget *WidgetFactory::createTwoColumHandle(Option *option, QWidget *rightWidget)
{
    auto optionFrame = new QFrame;
    optionFrame->setMinimumHeight(30);
    optionFrame->setObjectName("OptionFrame");
//    optionFrame->setStyleSheet("QFrame{border: 1px solid red;}");

    auto optionLayout = new QGridLayout(optionFrame);
    optionLayout->setContentsMargins(0, 0, 0, 0);
    optionLayout->setSpacing(0);


    rightWidget->setMinimumWidth(240);
    rightWidget->setStyleSheet("QWidget{font-size: 12px;}");

    if (!option->name().isEmpty()) {
        optionLayout->setColumnMinimumWidth(0, 110);
        optionLayout->setHorizontalSpacing(20);
        optionLayout->setColumnStretch(0, 10);
        optionLayout->setColumnStretch(1, 100);

        auto trName = QObject::tr(option->name().toStdString().c_str());
        auto labelWidget = new QLabel(trName);
        labelWidget->setContentsMargins(5, 0, 0, 0);
        labelWidget->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        labelWidget->setMinimumWidth(150);
        labelWidget->setFixedWidth(160);
        labelWidget->setObjectName("OptionLabel");
        labelWidget->setStyleSheet("#OptionLabel{font-size: 12px; }");
        optionLayout->addWidget(labelWidget, 0, 0, Qt::AlignLeft | Qt::AlignVCenter);
        optionLayout->addWidget(rightWidget, 0, 1, Qt::AlignLeft | Qt::AlignVCenter);
    } else {
        optionLayout->setColumnMinimumWidth(0, 5);
        optionLayout->addWidget(rightWidget, 0, 1, Qt::AlignLeft | Qt::AlignVCenter);
    }

    return  optionFrame;
}

QWidget *createShortcutEditOptionHandle(QObject *opt)
{
    auto option = qobject_cast<Option *>(opt);

    auto rightWidget = new ShortcutEdit();
    rightWidget->setObjectName("OptionShortcutEdit");

    QStringList keyseqs = option->value().toStringList();
    if (keyseqs.length() == 2) {
        auto modifier = static_cast<Qt::KeyboardModifiers>(keyseqs.value(0).toInt());
        auto key = static_cast<Qt::Key>(keyseqs.value(1).toInt());
        rightWidget->setShortCut(modifier, key);
    }

    auto optionWidget = WidgetFactory::createTwoColumHandle(option, rightWidget);

    option->connect(rightWidget, &ShortcutEdit::shortcutChanged,
    option, [ = ](Qt::KeyboardModifiers modifier, Qt::Key key) {
        QStringList keyseqs;
        keyseqs << QString("%1").arg(modifier) << QString("%1").arg(key);
        option->setValue(keyseqs);
    });
    option->connect(option, &Option::valueChanged,
    rightWidget, [ = ](const QVariant & value) {
        QStringList keyseqs = value.toStringList();
        if (keyseqs.length() == 2) {
            auto modifier = static_cast<Qt::KeyboardModifiers>(keyseqs.value(0).toInt());
            auto key = static_cast<Qt::Key>(keyseqs.value(1).toInt());
            rightWidget->setShortCut(modifier, key);
            rightWidget->update();
        }
    });

    return  optionWidget;
}

QWidget *createCheckboxOptionHandle(QObject *opt)
{
    auto option = qobject_cast<Option *>(opt);
    auto value = option->data("text").toString();
    auto trName = QObject::tr(value.toStdString().c_str());

    auto checkboxFrame = new QWidget;
    auto checkboxLayout = new QHBoxLayout(checkboxFrame);
    checkboxLayout->setSpacing(0);
    checkboxLayout->setContentsMargins(0,0,0,0);
    auto rightWidget = new QCheckBox("");
    auto checkboxLabel = new QLabel(trName);
    checkboxLabel->setWordWrap(true);
    checkboxLabel->setMinimumWidth(320);
    checkboxLayout->addWidget(rightWidget);
    checkboxLayout->addSpacing(5);
    checkboxLayout->addWidget(checkboxLabel);
    checkboxLayout->addStretch();

    rightWidget->setObjectName("OptionCheckbox");
    rightWidget->setChecked(option->value().toBool());

    auto optionWidget = WidgetFactory::createTwoColumHandle(option, checkboxFrame);

    option->connect(rightWidget, &QCheckBox::stateChanged,
    option, [ = ](int status) {
        option->setValue(status == Qt::Checked);
    });
    option->connect(option, &Option::valueChanged,
    rightWidget, [ = ](const QVariant & value) {
        rightWidget->setChecked(value.toBool());
        rightWidget->update();
    });

    return  optionWidget;
}

QWidget *createLineEditOptionHandle(QObject *opt)
{
    auto option = qobject_cast<Option *>(opt);
    auto value = option->data("text").toString();
    auto trName = QObject::tr(value.toStdString().c_str());
    auto rightWidget = new QLineEdit(trName);
    rightWidget->setFixedHeight(24);
    rightWidget->setObjectName("OptionLineEdit");
    rightWidget->setText(option->value().toString());

    auto optionWidget = WidgetFactory::createTwoColumHandle(option, rightWidget);

    option->connect(rightWidget, &QLineEdit::editingFinished,
    option, [ = ]() {
        option->setValue(rightWidget->text());
    });
    option->connect(option, &Option::valueChanged,
    rightWidget, [ = ](const QVariant & value) {
        rightWidget->setText(value.toString());
        rightWidget->update();
    });

    return  optionWidget;
}

QWidget *createComboBoxOptionHandle(QObject *opt)
{
    auto option = qobject_cast<Option *>(opt);
    auto rightWidget = new ComboBox();
    rightWidget->setFocusPolicy(Qt::StrongFocus);
    rightWidget->setFixedHeight(24);
    rightWidget->setObjectName("OptionLineEdit");

    auto data = option->data("items").toStringList();
    for (auto item : data) {
        auto trName = QObject::tr(item.toStdString().c_str());
        rightWidget->addItem(trName);
    }

    auto current = option->value().toInt();
    rightWidget->setCurrentIndex(current);

    auto optionWidget = WidgetFactory::createTwoColumHandle(option, rightWidget);

    option->connect(rightWidget, static_cast<void (QComboBox::*)(int index)>(&QComboBox::currentIndexChanged),
    option, [ = ](int index) {
        option->setValue(index);
    });
    option->connect(option, &Option::valueChanged,
    rightWidget, [ = ](const QVariant & value) {
        rightWidget->setCurrentIndex(value.toInt());
    });
    option->connect(option, &Option::dataChanged,
    rightWidget, [ = ](const QString & dataType, const QVariant & value) {
        if ("items" != dataType) {
            return;
        }

        rightWidget->clear();
        auto data = value.toStringList();
        for (auto item : data) {
            rightWidget->addItem(item);
        }

        auto current = option->value().toInt();
        rightWidget->setCurrentIndex(current);
        rightWidget->update();
    });

    return  optionWidget;
}

QWidget *createButtonGroupOptionHandle(QObject *opt)
{
    auto option = qobject_cast<Option *>(opt);
    auto rightWidget = new ButtonGroup();
    rightWidget->setFixedHeight(24);
    rightWidget->setObjectName("OptionButtonGroup");

    auto items = option->data("items").toStringList();
    rightWidget->setButtons(items);
    rightWidget->setCheckedButton(0);

    auto optionWidget = WidgetFactory::createTwoColumHandle(option, rightWidget);
    rightWidget->setParent(optionWidget);

    option->connect(rightWidget, &ButtonGroup::buttonChecked,
    option, [ = ](int id) {
        option->setValue(id);
    });
    option->connect(option, &Option::valueChanged,
    rightWidget, [ = ](const QVariant & value) {
        rightWidget->setCheckedButton(value.toInt());
        rightWidget->update();
    });
    return  optionWidget;
}

QWidget *createSpinButtonOptionHandle(QObject *opt)
{
    auto option = qobject_cast<Option *>(opt);
    auto rightWidget = new QSpinBox();
    rightWidget->setFixedHeight(24);
    rightWidget->setObjectName("OptionDSpinBox");
    rightWidget->setValue(option->value().toInt());

    auto optionWidget = WidgetFactory::createTwoColumHandle(option, rightWidget);

    option->connect(rightWidget, static_cast<void (QSpinBox::*)(int value)>(&QSpinBox::valueChanged),
    option, [ = ](int value) {
        option->setValue(value);
    });
    option->connect(option, &Option::valueChanged,
    rightWidget, [ = ](const QVariant & value) {
        rightWidget->setValue(value.toInt());
        rightWidget->update();
    });
    return  optionWidget;
}

QWidget *createSliderOptionHandle(QObject *opt)
{
    auto option = qobject_cast<Option *>(opt);
    auto rightWidget = new QSlider();
    rightWidget->setFixedHeight(24);
    rightWidget->setObjectName("OptionQSlider");
    rightWidget->setOrientation(Qt::Horizontal);
    rightWidget->setMaximum(option->data("max").toInt());
    rightWidget->setMinimum(option->data("min").toInt());
    rightWidget->setValue(option->value().toInt());

    auto optionWidget = WidgetFactory::createTwoColumHandle(option, rightWidget);

    option->connect(rightWidget, &QSlider::valueChanged,
    option, [ = ](int value) {
        rightWidget->blockSignals(true);
        option->setValue(value);
        rightWidget->blockSignals(false);
    });
    option->connect(option, &Option::valueChanged,
    rightWidget, [ = ](const QVariant & value) {
        rightWidget->setValue(value.toInt());
        rightWidget->update();
    });
    return  optionWidget;
}

QWidget *createUnsupportHandle(QObject *opt)
{
    auto option = qobject_cast<Option *>(opt);
    auto rightWidget = new QLabel();
    rightWidget->setFixedHeight(24);
    rightWidget->setObjectName("OptionUnsupport");
    rightWidget->setText("Unsupport option type: " + option->viewType());

    auto optionWidget = WidgetFactory::createTwoColumHandle(option, rightWidget);

//    optionWidget->setStyleSheet("QFrame{border: 1px solid red;}");
    return  optionWidget;
}

class WidgetFactoryPrivate
{
public:
    WidgetFactoryPrivate(WidgetFactory *parent) : q_ptr(parent)
    {
        widgetCreateHandles.insert("checkbox", createCheckboxOptionHandle);
        widgetCreateHandles.insert("lineedit", createLineEditOptionHandle);
        widgetCreateHandles.insert("combobox", createComboBoxOptionHandle);
        widgetCreateHandles.insert("shortcut", createShortcutEditOptionHandle);
        widgetCreateHandles.insert("spinbutton", createSpinButtonOptionHandle);
        widgetCreateHandles.insert("buttongroup", createButtonGroupOptionHandle);
        widgetCreateHandles.insert("slider", createSliderOptionHandle);
    }

    QMap<QString, WidgetCreateHandle> widgetCreateHandles;

    WidgetFactory *q_ptr;
    Q_DECLARE_PUBLIC(WidgetFactory)
};

WidgetFactory::WidgetFactory(QObject *parent) :
    QObject(parent), d_ptr(new WidgetFactoryPrivate(this))
{

}

WidgetFactory::~WidgetFactory()
{

}

void WidgetFactory::registerWidget(const QString &viewType, WidgetCreateHandle handle)
{
    Q_D(WidgetFactory);
    d->widgetCreateHandles.insert(viewType, handle);
}

QWidget *WidgetFactory::createWidget(QPointer<Option> option)
{
    Q_D(WidgetFactory);
    auto handle = d->widgetCreateHandles.value(option->viewType());
    if (handle) {
        return handle(option.data());
    } else {
        return createUnsupportHandle(option.data());
    }
}

DWIDGET_END_NAMESPACE
