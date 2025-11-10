#ifndef BASESETTINGS_H
#define BASESETTINGS_H
#include <QWidget>

class BaseSettings : public QWidget
{
    Q_OBJECT
  public:
    BaseSettings(QWidget *parent = nullptr) : QWidget(parent) {}
    virtual ~BaseSettings()    = default;
    virtual void apply()       = 0;
    virtual void setupFields() = 0;
  signals:
    void restartRequired();
    void optionChanged();
};

#endif // BASESETTINGS_H
