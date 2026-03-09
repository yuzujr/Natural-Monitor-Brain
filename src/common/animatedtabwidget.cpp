#include "animatedtabwidget.h"

#include <QTabBar>
#include <QWidget>

AnimatedTabWidget::AnimatedTabWidget(QWidget *parent)
    : QTabWidget(parent)
{
    setDocumentMode(true);
    if (tabBar()) {
        tabBar()->setDrawBase(false);
    }

    connect(this, &QTabWidget::currentChanged, this, &AnimatedTabWidget::onCurrentChanged);
}

void AnimatedTabWidget::onCurrentChanged(int index)
{
    if (isAnimating_ || index < 0) {
        return;
    }

    if (previousIndex_ < 0) {
        previousIndex_ = index;
        return;
    }

    if (previousIndex_ == index) {
        return;
    }

    isAnimating_ = true;
    int fromIndex = previousIndex_;
    int toIndex = index;

    previousIndex_ = index;

    QWidget *fromWidget = widget(fromIndex);
    QWidget *toWidget = widget(toIndex);

    if (!fromWidget || !toWidget) {
        isAnimating_ = false;
        return;
    }

    fadeInEffect_ = new QGraphicsOpacityEffect(toWidget);
    fadeInEffect_->setOpacity(0.0);
    toWidget->setGraphicsEffect(fadeInEffect_);

    fadeInAnimation_ = new QPropertyAnimation(fadeInEffect_, "opacity", this);
    fadeInAnimation_->setDuration(ANIMATION_DURATION);
    fadeInAnimation_->setStartValue(0.0);
    fadeInAnimation_->setEndValue(1.0);
    fadeInAnimation_->setEasingCurve(QEasingCurve::InOutQuad);

    connect(fadeInAnimation_, &QPropertyAnimation::finished, this, [this, toWidget]() {
        if (toWidget) {
            toWidget->setGraphicsEffect(nullptr);
        }
        isAnimating_ = false;
    });

    fadeInAnimation_->start(QAbstractAnimation::DeleteWhenStopped);
}
