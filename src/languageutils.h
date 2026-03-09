#ifndef LANGUAGEUTILS_H
#define LANGUAGEUTILS_H

#include <QObject>
#include <QTranslator>

class AppTranslator : public QTranslator
{
    Q_OBJECT

public:
    explicit AppTranslator(QObject *parent = nullptr);

    QString translate(const char *context, const char *sourceText,
                      const char *disambiguation = nullptr, int n = -1) const override;
};

class LanguageManager : public QObject
{
    Q_OBJECT

public:
    enum class Language {
        Chinese,
        English
    };

    explicit LanguageManager(QObject *parent = nullptr);

    void initialize();
    Language language() const;
    QString languageKey() const;
    QString currentLanguageDisplayName() const;
    QString nextLanguageButtonText() const;
    void setLanguage(Language language);
    void toggleLanguage();

    static Language languageFromKey(const QString &key);

signals:
    void languageChanged();

private:
    void applyLanguage();

    AppTranslator translator_;
    Language currentLanguage_ = Language::Chinese;
};

#endif // LANGUAGEUTILS_H