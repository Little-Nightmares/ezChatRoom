#pragma once

#include <QObject>
#include <QColor>
#include <QString>
#include <QStringList>
#include <QSettings>

namespace chatroom::client {

struct ThemePalette {
    QString name;
    QColor primary;
    QColor primaryLight;
    QColor primaryHover;
    QColor primaryBg;
    QColor background;
    QColor surface;
    QColor surfaceAlt;
    QColor border;
    QColor divider;
    QColor bubbleMine;
    QColor bubbleOther;
    QColor bubbleMineText;
    QColor bubbleOtherText;
    QColor textPrimary;
    QColor textSecondary;
    QColor textTertiary;
    QColor textInverse;
    QColor success;
    QColor warning;
    QColor warningLight;
    QColor danger;
    QColor dangerLight;
    QColor headerBg;
    QColor headerText;
    QColor tabActive;
    QColor tabInactive;
    QColor tabHoverBg;
    QColor tabActiveBg;
};

struct FontConfig {
    QString name;
    QString family;
    QString fallback;
    int bodySize;
    int smallSize;
    int bubbleSize;
    int headerSize;
    int messageSpacing;
    int bubbleRadius;
};

class ThemeManager : public QObject {
    Q_OBJECT

    Q_PROPERTY(int themeIndex READ themeIndex WRITE setThemeIndex NOTIFY themeChanged)
    Q_PROPERTY(int fontIndex READ fontIndex WRITE setFontIndex NOTIFY fontChanged)

    Q_PROPERTY(QColor primary READ primary NOTIFY themeChanged)
    Q_PROPERTY(QColor primaryLight READ primaryLight NOTIFY themeChanged)
    Q_PROPERTY(QColor primaryHover READ primaryHover NOTIFY themeChanged)
    Q_PROPERTY(QColor primaryBg READ primaryBg NOTIFY themeChanged)
    Q_PROPERTY(QColor background READ background NOTIFY themeChanged)
    Q_PROPERTY(QColor surface READ surface NOTIFY themeChanged)
    Q_PROPERTY(QColor surfaceAlt READ surfaceAlt NOTIFY themeChanged)
    Q_PROPERTY(QColor borderColor READ borderColor NOTIFY themeChanged)
    Q_PROPERTY(QColor divider READ divider NOTIFY themeChanged)
    Q_PROPERTY(QColor bubbleMine READ bubbleMine NOTIFY themeChanged)
    Q_PROPERTY(QColor bubbleOther READ bubbleOther NOTIFY themeChanged)
    Q_PROPERTY(QColor bubbleMineText READ bubbleMineText NOTIFY themeChanged)
    Q_PROPERTY(QColor bubbleOtherText READ bubbleOtherText NOTIFY themeChanged)
    Q_PROPERTY(QColor textPrimary READ textPrimary NOTIFY themeChanged)
    Q_PROPERTY(QColor textSecondary READ textSecondary NOTIFY themeChanged)
    Q_PROPERTY(QColor textTertiary READ textTertiary NOTIFY themeChanged)
    Q_PROPERTY(QColor textInverse READ textInverse NOTIFY themeChanged)
    Q_PROPERTY(QColor success READ success NOTIFY themeChanged)
    Q_PROPERTY(QColor warning READ warning NOTIFY themeChanged)
    Q_PROPERTY(QColor warningLight READ warningLight NOTIFY themeChanged)
    Q_PROPERTY(QColor danger READ danger NOTIFY themeChanged)
    Q_PROPERTY(QColor dangerLight READ dangerLight NOTIFY themeChanged)
    Q_PROPERTY(QColor headerBg READ headerBg NOTIFY themeChanged)
    Q_PROPERTY(QColor headerText READ headerText NOTIFY themeChanged)
    Q_PROPERTY(QColor tabActive READ tabActive NOTIFY themeChanged)
    Q_PROPERTY(QColor tabInactive READ tabInactive NOTIFY themeChanged)
    Q_PROPERTY(QColor tabHoverBg READ tabHoverBg NOTIFY themeChanged)
    Q_PROPERTY(QColor tabActiveBg READ tabActiveBg NOTIFY themeChanged)

    Q_PROPERTY(QString fontFamily READ fontFamily NOTIFY fontChanged)
    Q_PROPERTY(int bodySize READ bodySize NOTIFY fontChanged)
    Q_PROPERTY(int smallSize READ smallSize NOTIFY fontChanged)
    Q_PROPERTY(int bubbleSize READ bubbleSize NOTIFY fontChanged)
    Q_PROPERTY(int headerSize READ headerSize NOTIFY fontChanged)
    Q_PROPERTY(int messageSpacing READ messageSpacing NOTIFY fontChanged)
    Q_PROPERTY(int bubbleRadius READ bubbleRadius NOTIFY fontChanged)

    Q_PROPERTY(QString themeName READ themeName NOTIFY themeChanged)

public:
    explicit ThemeManager(QObject* parent = nullptr);

    int themeIndex() const;
    void setThemeIndex(int index);
    int fontIndex() const;
    void setFontIndex(int index);

    QString themeName() const;
    QColor primary() const;
    QColor primaryLight() const;
    QColor primaryHover() const;
    QColor primaryBg() const;
    QColor background() const;
    QColor surface() const;
    QColor surfaceAlt() const;
    QColor borderColor() const;
    QColor divider() const;
    QColor bubbleMine() const;
    QColor bubbleOther() const;
    QColor bubbleMineText() const;
    QColor bubbleOtherText() const;
    QColor textPrimary() const;
    QColor textSecondary() const;
    QColor textTertiary() const;
    QColor textInverse() const;
    QColor success() const;
    QColor warning() const;
    QColor warningLight() const;
    QColor danger() const;
    QColor dangerLight() const;
    QColor headerBg() const;
    QColor headerText() const;
    QColor tabActive() const;
    QColor tabInactive() const;
    QColor tabHoverBg() const;
    QColor tabActiveBg() const;

    QString fontFamily() const;
    int bodySize() const;
    int smallSize() const;
    int bubbleSize() const;
    int headerSize() const;
    int messageSpacing() const;
    int bubbleRadius() const;

    Q_INVOKABLE QStringList themeNames() const;
    Q_INVOKABLE QStringList fontNames() const;

signals:
    void themeChanged();
    void fontChanged();

private:
    static const ThemePalette& theme(int index);
    static const FontConfig& fontCfg(int index);
    const ThemePalette& currentTheme() const;
    const FontConfig& currentFont() const;
    void saveSettings();

    int m_themeIndex = 0;
    int m_fontIndex = 0;
};

} // namespace chatroom::client
