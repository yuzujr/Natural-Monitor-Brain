#ifndef ANIMATEDTABWIDGET_H
#define ANIMATEDTABWIDGET_H

#include <QTabWidget>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

class AnimatedTabWidget : public QTabWidget
{
    Q_OBJECT

public:
    explicit AnimatedTabWidget(QWidget *parent = nullptr);

private slots:
    void onCurrentChanged(int index);

private:
    QPropertyAnimation *fadeInAnimation_ = nullptr;
    QGraphicsOpacityEffect *fadeInEffect_ = nullptr;
    int previousIndex_ = -1;
    bool isAnimating_ = false;
    static constexpr int ANIMATION_DURATION = 150;
};

#endif // ANIMATEDTABWIDGET_H
