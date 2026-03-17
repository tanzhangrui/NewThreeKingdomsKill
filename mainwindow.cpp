#include "mainwindow.h"
#include "Hero.h"
#include "Card.h"
#include <QPainter>
#include <QPainterPath>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QMessageBox>
#include <QRandomGenerator>
#include <QFont>
#include <QTimer>
#include <QtMath>
#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLayout>
#include <QStackedLayout>
#include <QScrollArea>
#include <QScrollBar>
#include <QFile>
#include <QFileInfo>
#include <QEvent>
#include <QUrl>

static const QColor C_GOLD(0xD4, 0xAF, 0x37);
static const QColor C_RED(0xC0, 0x20, 0x2A);
static const QColor C_JADE(0x00, 0xB3, 0x7A);
static const QColor C_FROST(0xA0, 0xD8, 0xEF);
static const QColor C_PURPLE(0xCC, 0x99, 0xFF);
static const QColor C_ORANGE(0xFF, 0x88, 0x00);
static const QColor C_CYAN(0x00, 0xDD, 0xDD);

static QColor cardColor(CardType t) {
    switch (t) {
        case CardType::Kill:  return QColor(0xC0, 0x20, 0x2A);
        case CardType::Dodge: return QColor(0x20, 0x80, 0xC0);
        case CardType::Peach: return QColor(0xDD, 0x60, 0x90);
        case CardType::Trick: return QColor(0xA0, 0x55, 0xD0);
    }
    return Qt::gray;
}

static QColor heroThemeColor(const QString& heroName) {
    if (heroName.contains(QString::fromUtf8("曹操")))       return QColor(0xC0, 0x20, 0x2A);
    if (heroName.contains(QString::fromUtf8("司马懿"))) return QColor(0x20, 0x50, 0xC0);
    return QColor(0x20, 0xB0, 0x50);
}

CenterEffectWidget::CenterEffectWidget(QWidget* parent) : QWidget(parent) {
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_TranslucentBackground);
    m_animTimer = new QTimer(this);
    m_animTimer->setInterval(16);
    connect(m_animTimer, &QTimer::timeout, this, [this]() {
        if (!m_active) return;
        m_phase += 0.05f;
        update();
    });
}

void CenterEffectWidget::showEffect(const QString& text, const QString& type, int durationMs) {
    m_text = text;
    m_type = type;
    m_alpha = 1.0f;
    m_scale = 0.5f;
    m_phase = 0.0f;
    m_active = true;
    m_animTimer->start();
    
    QTimer::singleShot(durationMs, this, [this]() {
        m_active = false;
        m_animTimer->stop();
        m_alpha = 0.0f;
        update();
    });
    
    update();
}

void CenterEffectWidget::paintEvent(QPaintEvent*) {
    if (!m_active || m_alpha <= 0.0f) return;
    
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);
    
    p.fillRect(rect(), QColor(0, 0, 0, 150));
    
    QPointF center(width() / 2.0, height() / 2.0);
    
    float pulseScale = 1.0f + 0.15f * qSin(m_phase * 4);
    
    p.save();
    p.translate(center);
    p.scale(m_scale * pulseScale, m_scale * pulseScale);
    
    QColor glowColor = C_GOLD;
    if (m_type == QString::fromUtf8("kill")) glowColor = C_RED;
    else if (m_type == QString::fromUtf8("dodge")) glowColor = C_CYAN;
    else if (m_type == QString::fromUtf8("peach")) glowColor = C_JADE;
    else if (m_type == QString::fromUtf8("skill")) glowColor = C_PURPLE;
    else if (m_type == QString::fromUtf8("trick")) glowColor = C_ORANGE;
    
    for (int i = 8; i >= 1; --i) {
        QColor glow = glowColor;
        glow.setAlpha(40 * i);
        p.setPen(QPen(glow, i * 5));
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(QPointF(0, 0), 150 + i * 35, 150 + i * 35);
    }
    
    QRadialGradient rg(0, 0, 200);
    QColor rgColor = glowColor;
    rgColor.setAlpha(80);
    rg.setColorAt(0, rgColor);
    rg.setColorAt(1, QColor(0, 0, 0, 0));
    p.setBrush(rg);
    p.setPen(Qt::NoPen);
    p.drawEllipse(QPointF(0, 0), 200, 200);
    
    QFont f;
    f.setPixelSize(120);
    f.setBold(true);
    f.setFamily(QString::fromUtf8("Microsoft YaHei"));
    p.setFont(f);
    
    for (int i = 3; i >= 1; --i) {
        p.setPen(QPen(QColor(0, 0, 0, 150), 12 + i * 3));
        p.drawText(QRect(-350, -80, 700, 160), Qt::AlignCenter, m_text);
    }
    
    p.setPen(glowColor);
    p.drawText(QRect(-350, -80, 700, 160), Qt::AlignCenter, m_text);
    
    QFont subF;
    subF.setPixelSize(36);
    subF.setBold(true);
    subF.setFamily(QString::fromUtf8("Microsoft YaHei"));
    p.setFont(subF);
    QColor subColor = glowColor.lighter(130);
    subColor.setAlpha(200);
    p.setPen(subColor);
    p.drawText(QRect(-350, 100, 700, 50), Qt::AlignCenter, QString::fromUtf8("✦ ✦ ✦"));
    
    p.restore();
}

AnimOverlay::AnimOverlay(QWidget* parent) : QWidget(parent) {
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_TranslucentBackground);
    m_timer = new QTimer(this);
    m_timer->setInterval(16);
    connect(m_timer, &QTimer::timeout, this, [this]() {
        bool any = false;
        
        for (auto& fc : m_flyCards) {
            if (!fc.active) continue;
            fc.progress += 0.035f;
            float t = qMin(fc.progress, 1.0f);
            float e = 1.f - (1.f-t)*(1.f-t)*(1.f-t);
            float arc = -4.f * t * (t-1.f) * 80.f;
            QPointF straight = fc.start + (fc.end - fc.start) * (double)e;
            fc.pos = QPointF(straight.x(), straight.y() - arc);
            fc.rotation += 3.0f;
            fc.scale = 1.0f + 0.2f * qSin(t * 3.14159f);
            if (fc.progress >= 1.0f) fc.active = false;
            any = true;
        }
        
        for (auto& fl : m_flashes) {
            if (!fl.active) continue;
            fl.alpha -= 0.018f;
            fl.scale += 0.025f;
            fl.glowPhase += 0.15f;
            if (fl.alpha <= 0.f) fl.active = false;
            any = true;
        }
        
        for (auto& fn : m_numbers) {
            if (!fn.active) continue;
            fn.alpha -= 0.015f;
            fn.pos.setY(fn.pos.y() + fn.velY);
            fn.velY *= 0.98f;
            fn.scale += 0.008f;
            if (fn.alpha <= 0.f) fn.active = false;
            any = true;
        }
        
        for (auto& ts : m_turnStarts) {
            if (!ts.active) continue;
            ts.progress += 0.02f;
            if (ts.progress >= 1.0f) ts.active = false;
            any = true;
        }
        
        if (!any) m_timer->stop();
        update();
    });
}

void AnimOverlay::playCardFly(const QPoint& from, const QPoint& to,
                               const QString& label, const QColor& col) {
    FlyCard fc;
    fc.start = from; fc.end = to; fc.pos = from;
    fc.label = label; fc.color = col;
    fc.progress = 0.f; fc.active = true;
    fc.rotation = 0.f; fc.scale = 1.0f;
    m_flyCards.append(fc);
    if (!m_timer->isActive()) m_timer->start();
    update();
}

void AnimOverlay::playSkillFlash(const QPoint& center, const QString& label, const QColor& col) {
    FlashLabel fl;
    fl.center = center; fl.label = label; fl.color = col;
    fl.alpha = 1.0f; fl.scale = 0.5f; fl.active = true;
    fl.glowPhase = 0.f;
    m_flashes.append(fl);
    if (!m_timer->isActive()) m_timer->start();
    update();
}

void AnimOverlay::playDamageNumber(const QPoint& pos, int amount, const QColor& col) {
    FloatingNumber fn;
    fn.pos = pos; fn.text = QString("-%1").arg(amount);
    fn.color = col; fn.alpha = 1.0f; fn.scale = 1.2f;
    fn.velY = -2.5f; fn.active = true; fn.isDamage = true;
    m_numbers.append(fn);
    if (!m_timer->isActive()) m_timer->start();
}

void AnimOverlay::playHealNumber(const QPoint& pos, int amount, const QColor& col) {
    FloatingNumber fn;
    fn.pos = pos; fn.text = QString("+%1").arg(amount);
    fn.color = col; fn.alpha = 1.0f; fn.scale = 1.0f;
    fn.velY = -1.5f; fn.active = true; fn.isDamage = false;
    m_numbers.append(fn);
    if (!m_timer->isActive()) m_timer->start();
}

void AnimOverlay::playTurnStart(const QPoint& center, const QString& name) {
    TurnStartAnim ts;
    ts.center = center; ts.name = name;
    ts.progress = 0.f; ts.active = true;
    m_turnStarts.append(ts);
    if (!m_timer->isActive()) m_timer->start();
    update();
}

void AnimOverlay::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);
    
    for (const auto& fc : m_flyCards) {
        if (!fc.active) continue;
        p.save();
        p.translate(fc.pos);
        p.rotate(fc.rotation);
        p.scale(fc.scale, fc.scale);
        
        for (int g = 5; g >= 1; --g) {
            QColor glow = fc.color.lighter(150 + g*20);
            glow.setAlpha(12 * g);
            p.setPen(QPen(glow, g*4+2));
            p.setBrush(Qt::NoBrush);
            p.drawRoundedRect(QRectF(-38,-26,76,52), 10, 10);
        }
        
        QPainterPath path;
        path.addRoundedRect(QRectF(-38,-26,76,52), 10, 10);
        QLinearGradient grad(QPointF(-38,-26), QPointF(-38,26));
        QColor top = fc.color.lighter(170); top.setAlpha(250);
        QColor bot = fc.color.darker(140);  bot.setAlpha(250);
        grad.setColorAt(0, top); grad.setColorAt(1, bot);
        p.fillPath(path, grad);
        
        p.setPen(QPen(Qt::white, 1.5));
        p.drawPath(path);
        
        QFont f; f.setPixelSize(20); f.setBold(true); p.setFont(f);
        p.setPen(Qt::white);
        p.drawText(QRect(-38,-26,76,52), Qt::AlignCenter, fc.label);
        p.restore();
    }
    
    for (const auto& fl : m_flashes) {
        if (!fl.active) continue;
        p.save();
        p.translate(fl.center);
        p.scale(fl.scale, fl.scale);
        p.setOpacity(qBound(0.0, (double)fl.alpha, 1.0));
        
        float glowSize = 60 + qSin(fl.glowPhase) * 15;
        QRadialGradient rg(0, 0, glowSize);
        QColor glowCol = fl.color;
        glowCol.setAlpha(100);
        rg.setColorAt(0, glowCol);
        rg.setColorAt(1, QColor(0,0,0,0));
        p.setPen(Qt::NoPen);
        p.setBrush(rg);
        p.drawEllipse(QPointF(0,0), glowSize, glowSize);
        
        QFont f; f.setPixelSize(44); f.setBold(true); p.setFont(f);
        p.setPen(QPen(QColor(0,0,0,180), 8));
        p.drawText(QRect(-180,-35,360,70), Qt::AlignCenter, fl.label);
        p.setPen(fl.color);
        p.drawText(QRect(-180,-35,360,70), Qt::AlignCenter, fl.label);
        p.restore();
    }
    
    for (const auto& fn : m_numbers) {
        if (!fn.active) continue;
        p.save();
        p.translate(fn.pos);
        p.scale(fn.scale, fn.scale);
        p.setOpacity(qBound(0.0, (double)fn.alpha, 1.0));
        
        QFont f; f.setPixelSize(fn.isDamage ? 36 : 28); f.setBold(true); p.setFont(f);
        p.setPen(QPen(QColor(0,0,0,200), 5));
        p.drawText(QRect(-50,-25,100,50), Qt::AlignCenter, fn.text);
        p.setPen(fn.color);
        p.drawText(QRect(-50,-25,100,50), Qt::AlignCenter, fn.text);
        p.restore();
    }
    
    for (const auto& ts : m_turnStarts) {
        if (!ts.active) continue;
        p.save();
        p.translate(ts.center);
        
        float t = ts.progress;
        float alpha = t < 0.3f ? t/0.3f : (t > 0.7f ? (1.0f-t)/0.3f : 1.0f);
        p.setOpacity(alpha);
        
        float ringScale = 0.5f + t * 1.5f;
        p.scale(ringScale, ringScale);
        
        for (int i = 0; i < 3; ++i) {
            float ringAlpha = (1.0f - t) * (1.0f - i * 0.25f);
            QColor ringCol = C_GOLD;
            ringCol.setAlphaF(ringAlpha * 0.6f);
            p.setPen(QPen(ringCol, 3 - i * 0.5f));
            p.setBrush(Qt::NoBrush);
            p.drawEllipse(QPointF(0,0), 80 + i * 30, 80 + i * 30);
        }
        
        p.resetTransform();
        p.translate(ts.center);
        p.setOpacity(alpha);
        QFont f; f.setPixelSize(32); f.setBold(true); p.setFont(f);
        p.setPen(QPen(QColor(0,0,0,180), 6));
        p.drawText(QRect(-150,-25,300,50), Qt::AlignCenter, ts.name + QString::fromUtf8(" 的回合"));
        p.setPen(C_GOLD);
        p.drawText(QRect(-150,-25,300,50), Qt::AlignCenter, ts.name + QString::fromUtf8(" 的回合"));
        p.restore();
    }
}

HeroPanel::HeroPanel(int idx, QWidget* parent)
    : QWidget(parent), m_idx(idx) {
    setMinimumSize(320, 220);
    setCursor(Qt::PointingHandCursor);
    m_flashTimer = new QTimer(this);
    m_flashTimer->setInterval(20);
    connect(m_flashTimer, &QTimer::timeout, this, [this]() {
        m_flashAlpha -= 0.07;
        if (m_flashAlpha <= 0.0) { m_flashAlpha = 0.0; m_flashTimer->stop(); }
        update();
    });
    m_glowTimer = new QTimer(this);
    m_glowTimer->setInterval(30);
    connect(m_glowTimer, &QTimer::timeout, this, [this]() {
        m_animPhase += 0.1f;
        m_glowIntensity = 0.5f + 0.5f * qSin(m_animPhase);
        update();
    });
}

void HeroPanel::setPortraitImage(const QString& imagePath) {
    if (QFile::exists(imagePath)) {
        m_portraitPixmap.load(imagePath);
        if (m_portraitMovie) {
            delete m_portraitMovie;
            m_portraitMovie = nullptr;
        }
        update();
    }
}

void HeroPanel::setPortraitGif(const QString& gifPath) {
    if (QFile::exists(gifPath)) {
        if (!m_portraitLabel) {
            m_portraitLabel = new QLabel(this);
            m_portraitLabel->setAlignment(Qt::AlignCenter);
        }
        if (m_portraitMovie) {
            delete m_portraitMovie;
        }
        m_portraitMovie = new QMovie(gifPath);
        m_portraitLabel->setMovie(m_portraitMovie);
        m_portraitMovie->start();
        m_portraitLabel->show();
        m_portraitLabel->raise();
    }
}

void HeroPanel::setPlayer(Player* p) { m_player = p; update(); }
void HeroPanel::refresh() { update(); }
void HeroPanel::highlight(bool on) { m_highlighted = on; update(); }

void HeroPanel::flashDamage() {
    m_flashAlpha = 1.0;
    m_flashTimer->start();
    update();
}

void HeroPanel::flashHeal() {
    m_healAlpha = 1.0;
    QTimer::singleShot(300, this, [this](){ m_healAlpha = 0.0; update(); });
    update();
}

void HeroPanel::playTurnGlow() {
    m_glowIntensity = 1.0;
    m_animPhase = 0.0f;
    m_glowTimer->start();
    QTimer::singleShot(2000, this, [this](){ m_glowTimer->stop(); m_glowIntensity = 0.0; update(); });
}

void HeroPanel::mousePressEvent(QMouseEvent*) { emit clicked(m_idx); }
void HeroPanel::enterEvent(QEvent*) { m_hovered = true; update(); }
void HeroPanel::leaveEvent(QEvent*) { m_hovered = false; update(); }

void HeroPanel::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);
    QRect r = rect().adjusted(3,3,-3,-3);
    
    QPainterPath bg;
    bg.addRoundedRect(r, 15, 15);
    
    QLinearGradient grad(r.topLeft(), r.bottomLeft());
    grad.setColorAt(0.0, QColor(0x2C,0x1A,0x50,245));
    grad.setColorAt(1.0, QColor(0x0D,0x07,0x22,245));
    p.fillPath(bg, grad);
    
    if (m_glowIntensity > 0.0) {
        QColor glowCol = C_GOLD;
        glowCol.setAlphaF(m_glowIntensity * 0.5f);
        p.fillPath(bg, glowCol);
    }
    
    if (m_highlighted) {
        for (int i = 8; i >= 1; --i) {
            QColor gc = C_GOLD;
            gc.setAlpha(30*i);
            p.setPen(QPen(gc, i*3+4));
            p.setBrush(Qt::NoBrush);
            p.drawPath(bg);
        }
        p.setPen(QPen(C_GOLD, 3));
    } else if (m_hovered) {
        p.setPen(QPen(QColor(0x99, 0x77, 0xBB), 2));
    } else {
        p.setPen(QPen(QColor(0x66,0x44,0x99), 2));
    }
    p.drawPath(bg);
    
    if (!m_player) {
        p.setPen(QColor(140,120,180));
        QFont f; f.setPixelSize(20); p.setFont(f);
        p.drawText(r, Qt::AlignCenter, QString::fromUtf8("等待中..."));
        return;
    }
    
    bool alive = m_player->isAlive();
    if (!alive) p.setOpacity(0.4);
    
    QString heroName   = m_player->getHero() ? m_player->getHero()->getName() : "?";
    QString displayName = m_player->getHero() ? m_player->getHero()->getDisplayName() : "?";
    
    QRect portrait(r.left()+12, r.top()+12, 90, 110);
    QPainterPath pp; pp.addRoundedRect(portrait, 10, 10);
    
    QColor themeCol = heroThemeColor(heroName);
    
    if (!m_portraitPixmap.isNull()) {
        QPainterPath clipPath;
        clipPath.addRoundedRect(portrait, 10, 10);
        p.setClipPath(clipPath);
        QPixmap scaled = m_portraitPixmap.scaled(portrait.size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        p.drawPixmap(portrait, scaled);
        p.setClipping(false);
    } else {
        QLinearGradient pg(portrait.topLeft(), portrait.bottomRight());
        pg.setColorAt(0, themeCol.lighter(150));
        pg.setColorAt(0.5, themeCol);
        pg.setColorAt(1, themeCol.darker(160));
        p.fillPath(pp, pg);
    }
    
    p.setPen(QPen(C_GOLD.darker(120), 2));
    p.drawPath(pp);
    
    if (m_portraitPixmap.isNull() && !m_portraitMovie) {
        QFont hf; hf.setPixelSize(52); hf.setBold(true); p.setFont(hf);
        p.setPen(themeCol.lighter(180));
        QString portChar = m_player->getHero() ? m_player->getHero()->portraitChar() : heroName.left(1);
        p.drawText(portrait, Qt::AlignCenter, portChar);
    }
    
    if (m_portraitLabel) {
        m_portraitLabel->setGeometry(portrait);
    }
    
    int tx = portrait.right() + 16;
    QFont nf; nf.setPixelSize(20); nf.setBold(true); p.setFont(nf);
    p.setPen(C_GOLD);
    p.drawText(tx, r.top()+32, displayName);
    
    if (m_player->getHero()) {
        QFont sf; sf.setPixelSize(14); p.setFont(sf);
        p.setPen(C_PURPLE);
        Hero* h = m_player->getHero();
        QString skillText;
        for (int i = 0; i < h->getSkillCount(); ++i) {
            SkillInfo info = h->getSkillInfo(i);
            QString prefix = info.isActive() ? QString::fromUtf8("[主]") : QString::fromUtf8("[被]");
            if (i > 0) skillText += QString::fromUtf8(" ");
            skillText += prefix + info.name;
        }
        p.drawText(tx, r.top()+56, skillText);
    }
    
    int hp = m_player->getHealth(), maxHp = m_player->getMaxHealth();
    int pipW=20, pipH=16, pipGap=4, startX=tx, startY=r.top()+76;
    
    for (int i = 0; i < maxHp; ++i) {
        QRectF pip(startX+i*(pipW+pipGap), startY, pipW, pipH);
        QPainterPath pp2; pp2.addRoundedRect(pip,4,4);
        QColor fc;
        if (i < hp) {
            float ratio=(float)hp/maxHp;
            if      (ratio>0.6f) fc=QColor(0x22,0xDD,0x66);
            else if (ratio>0.3f) fc=QColor(0xFF,0x99,0x22);
            else                 fc=QColor(0xEE,0x33,0x33);
        } else { fc=QColor(0x25,0x18,0x35); }
        p.fillPath(pp2, fc);
    }
    
    QFont hpf; hpf.setPixelSize(16); p.setFont(hpf);
    p.setPen(QColor(220,220,220));
    p.drawText(startX, startY+pipH+20, QString("%1/%2 HP").arg(hp).arg(maxHp));
    p.setPen(QColor(180,160,220));
    p.drawText(startX, startY+pipH+42, QString::fromUtf8("手牌: ") + QString::number(m_player->getHandCards().size()));
    
    if (m_flashAlpha > 0.0) {
        p.fillPath(bg, QColor(0xFF,0x10,0x10,(int)(m_flashAlpha*140)));
    }
    if (m_healAlpha > 0.0) {
        p.fillPath(bg, QColor(0x20,0xDD,0x60,(int)(m_healAlpha*120)));
    }
    if (!alive) {
        p.fillPath(bg, QColor(0,0,0,180));
        QFont df; df.setPixelSize(32); df.setBold(true); p.setFont(df);
        p.setPen(QColor(200,0,0,240));
        p.drawText(r, Qt::AlignCenter, QString::fromUtf8("阵亡"));
    }
}

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle(QString::fromUtf8("三国杀 · 天意侵蚀"));
    resize(1400, 900);
    setMinimumSize(1200, 800);
    buildUI();
}

MainWindow::~MainWindow() {
    delete m_engine;
    for (auto* p : m_players) delete p;
}

void MainWindow::styleButton(QPushButton* btn, const QString& bg, const QString& fg) {
    btn->setCursor(Qt::PointingHandCursor);
    btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    btn->setStyleSheet(QString(
        "QPushButton{background:%1;color:%2;"
        "border:2px solid rgba(255,255,255,40);"
        "border-radius:10px;font-size:18px;font-weight:bold;padding:14px;}"
        "QPushButton:hover{background:%3;border:2px solid %2;}"
        "QPushButton:pressed{background:%4;}"
        "QPushButton:disabled{background:#2A2A3E;color:#555;border:2px solid #3A3A4E;}"
    ).arg(bg, fg, QColor(bg).lighter(120).name(), QColor(bg).darker(130).name()));
}

void MainWindow::buildUI() {
    m_centralWidget = new QWidget(this);
    m_centralWidget->setObjectName("centralWidget");
    m_centralWidget->setStyleSheet("#centralWidget{background:transparent;}");
    setCentralWidget(m_centralWidget);
    
    m_stackedWidget = new QStackedWidget(m_centralWidget);
    QVBoxLayout* mainLayout = new QVBoxLayout(m_centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(m_stackedWidget);
    
    buildCharacterSelectUI();
    buildGameUI();
    
    m_stackedWidget->setCurrentWidget(m_characterSelectPage);
    
    installEventFilter(this);
    
    m_particleTimer = new QTimer(this);
    m_particleTimer->setInterval(16);
    connect(m_particleTimer, &QTimer::timeout, this, &MainWindow::onParticleTimer);
    
    m_backgroundTimer = new QTimer(this);
    m_backgroundTimer->setInterval(33);
    connect(m_backgroundTimer, &QTimer::timeout, this, &MainWindow::onBackgroundTimer);
    m_backgroundTimer->start();
}

void MainWindow::buildCharacterSelectUI() {
    m_characterSelectPage = new QWidget();
    m_characterSelectPage->setStyleSheet("background:transparent;");
    
    QVBoxLayout* layout = new QVBoxLayout(m_characterSelectPage);
    layout->setContentsMargins(60, 40, 60, 40);
    layout->setSpacing(25);
    
    QLabel* title = new QLabel(QString::fromUtf8("三国杀 · 天意侵蚀"), m_characterSelectPage);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("color:#D4AF37;font-size:56px;font-weight:bold;");
    layout->addWidget(title);
    
    QLabel* subtitle = new QLabel(QString::fromUtf8("选择你的武将"), m_characterSelectPage);
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setStyleSheet("color:#CC99FF;font-size:28px;");
    layout->addWidget(subtitle);
    
    QHBoxLayout* heroLayout = new QHBoxLayout();
    heroLayout->setSpacing(50);
    
    struct HeroData { QString name; QString desc; QString color; QString skills; };
    QVector<HeroData> heroes = {
        { QString::fromUtf8("曹操"), 
          QString::fromUtf8("技能：\n[被]战无不胜の斗鸡眼\n[被]呱\n[主]汪\n[被]临终关怀"), 
          QString("#C0202A"),
          QString::fromUtf8(
            "【战无不胜の斗鸡眼】被动\n血量≤50%时，对全体敌方造成1点精神伤害\n\n"
            "【呱】被动\n本回合未出牌时，对一名玩家造成1点精神伤害\n\n"
            "【汪】主动\n指定一名玩家，双方轮流出【杀】，最先放弃的受1点伤害\n\n"
            "【临终关怀】被动\n阵亡时，对全体玩家造成1点精神伤害"
          )
        },
        { QString::fromUtf8("刘备"), 
          QString::fromUtf8("技能：\n[被]自刎归天\n[被]无敌の二弟\n[主]仁之剑义之剑"), 
          QString("#20B050"),
          QString::fromUtf8(
            "【自刎归天】被动\n打出【杀】时，无论敌我全员都受到1点伤害\n\n"
            "【无敌の二弟】被动\n受到攻击时，可召唤二弟对攻击者造成1点伤害\n\n"
            "【仁之剑义之剑】主动\n自己恢复1血，对目标玩家造成1点伤害"
          )
        },
        { QString::fromUtf8("司马懿"), 
          QString::fromUtf8("技能：\n[被]天意化骨掌\n[被]天意面瘫"), 
          QString("#2050C0"),
          QString::fromUtf8(
            "【天意化骨掌】被动\n可无限出【杀】，每回合出第二张【杀】时自动发动\n\n"
            "【天意面瘫】被动\n打出【桃】时，自身恢复2点，其余存活玩家恢复1点"
          )
        }
    };
    
    for (const auto& h : heroes) {
        QWidget* heroCard = new QWidget(m_characterSelectPage);
        heroCard->setFixedSize(360, 520);
        heroCard->setCursor(Qt::PointingHandCursor);
        heroCard->setStyleSheet(QString(
            "QWidget{background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 %1,stop:1 #1A0A2A);"
            "border:4px solid #D4AF37;border-radius:18px;}"
        ).arg(h.color));
        
        QVBoxLayout* cardLayout = new QVBoxLayout(heroCard);
        cardLayout->setContentsMargins(25, 25, 25, 25);
        cardLayout->setSpacing(18);
        
        QLabel* nameLabel = new QLabel(QString::fromUtf8("邪·") + h.name, heroCard);
        nameLabel->setAlignment(Qt::AlignCenter);
        nameLabel->setStyleSheet("color:#D4AF37;font-size:36px;font-weight:bold;background:transparent;border:none;");
        cardLayout->addWidget(nameLabel);
        
        QLabel* portraitLabel = new QLabel(h.name.left(1), heroCard);
        portraitLabel->setAlignment(Qt::AlignCenter);
        portraitLabel->setStyleSheet(QString(
            "color:%1;font-size:100px;font-weight:bold;background:rgba(0,0,0,80);"
            "border-radius:15px;border:none;"
        ).arg(QColor(h.color).lighter(160).name()));
        portraitLabel->setFixedHeight(140);
        portraitLabel->setObjectName("portraitLabel");
        cardLayout->addWidget(portraitLabel);
        
        QLabel* skillTitle = new QLabel(QString::fromUtf8("【技能介绍】"), heroCard);
        skillTitle->setAlignment(Qt::AlignCenter);
        skillTitle->setStyleSheet("color:#FFD700;font-size:18px;font-weight:bold;background:transparent;border:none;");
        cardLayout->addWidget(skillTitle);
        
        QLabel* descLabel = new QLabel(h.skills, heroCard);
        descLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
        descLabel->setWordWrap(true);
        descLabel->setStyleSheet("color:#CC99FF;font-size:15px;background:transparent;border:none;line-height:1.5;");
        descLabel->setTextFormat(Qt::PlainText);
        cardLayout->addWidget(descLabel, 1);
        
        heroCard->installEventFilter(this);
        heroCard->setProperty("heroName", h.name);
        
        heroLayout->addWidget(heroCard);
    }
    
    layout->addLayout(heroLayout);
    layout->addStretch();
    
    m_stackedWidget->addWidget(m_characterSelectPage);
}

void MainWindow::buildGameUI() {
    m_gamePage = new QWidget();
    m_gamePage->setStyleSheet("background:transparent;");
    
    QVBoxLayout* mainLayout = new QVBoxLayout(m_gamePage);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(8);
    
    QHBoxLayout* topArea = new QHBoxLayout();
    topArea->setSpacing(10);
    
    m_heroPanels[1] = new HeroPanel(1, m_gamePage);
    m_heroPanels[2] = new HeroPanel(2, m_gamePage);
    
    QVBoxLayout* topLeft = new QVBoxLayout();
    topLeft->addWidget(m_heroPanels[1]);
    topLeft->setStretch(0, 1);
    
    m_turnLabel = new QLabel(QString::fromUtf8("选择角色开始游戏"), m_gamePage);
    m_turnLabel->setAlignment(Qt::AlignCenter);
    m_turnLabel->setStyleSheet(
        "color:#D4AF37;font-size:28px;font-weight:bold;background:rgba(0,0,0,120);"
        "padding:12px;border-radius:10px;");
    m_turnLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    
    QVBoxLayout* topRight = new QVBoxLayout();
    topRight->addWidget(m_heroPanels[2]);
    topRight->setStretch(0, 1);
    
    topArea->addLayout(topLeft, 1);
    topArea->addWidget(m_turnLabel, 2);
    topArea->addLayout(topRight, 1);
    
    m_centerEffect = new CenterEffectWidget(m_gamePage);
    m_centerEffect->lower();
    
    m_handArea = new QWidget(m_gamePage);
    m_handArea->setMinimumHeight(160);
    m_handArea->setStyleSheet("background:rgba(20,15,35,200);border-radius:12px;border:2px solid #3A2A5A;");
    
    m_log = new QTextBrowser(m_gamePage);
    m_log->setMaximumWidth(280);
    m_log->setStyleSheet(
        "QTextBrowser{background:rgba(8,6,20,240);color:#C8B8F0;"
        "border:2px solid #3A2A5A;border-radius:10px;"
        "font-family:'Microsoft YaHei';font-size:16px;padding:8px;}");
    
    QWidget* bottomArea = new QWidget(m_gamePage);
    QHBoxLayout* bottomLayout = new QHBoxLayout(bottomArea);
    bottomLayout->setContentsMargins(0, 0, 0, 0);
    bottomLayout->setSpacing(10);
    
    m_heroPanels[0] = new HeroPanel(0, m_gamePage);
    
    QVBoxLayout* btnPanel = new QVBoxLayout();
    btnPanel->setSpacing(8);
    
    m_btnEndTurn = new QPushButton(QString::fromUtf8("结束回合"), m_gamePage);
    styleButton(m_btnEndTurn, "#0A2035", "#A0D8EF");
    m_btnEndTurn->setMinimumHeight(55);
    
    m_btnCancel = new QPushButton(QString::fromUtf8("取消"), m_gamePage);
    styleButton(m_btnCancel, "#1E1E1E", "#CC7777");
    m_btnCancel->setMinimumHeight(55);
    m_btnCancel->hide();
    
    btnPanel->addWidget(m_btnEndTurn);
    btnPanel->addWidget(m_btnCancel);
    
    for (int i = 0; i < 4; ++i) {
        QPushButton* btn = new QPushButton(m_gamePage);
        styleButton(btn, "#2A0A44", "#CC99FF");
        btn->setMinimumHeight(55);
        btn->hide();
        m_skillButtons.append(btn);
        btnPanel->addWidget(btn);
    }
    
    QWidget* dodgeWidget = new QWidget(m_gamePage);
    QVBoxLayout* dodgeLayout = new QVBoxLayout(dodgeWidget);
    dodgeLayout->setContentsMargins(0, 0, 0, 0);
    dodgeLayout->setSpacing(8);
    
    QLabel* dodgeLabel = new QLabel(QString::fromUtf8("是否使用【闪】？"), m_gamePage);
    dodgeLabel->setAlignment(Qt::AlignCenter);
    dodgeLabel->setStyleSheet("color:#FFD700;font-size:20px;font-weight:bold;");
    dodgeLabel->hide();
    
    m_btnUseDodge = new QPushButton(QString::fromUtf8("使用【闪】"), m_gamePage);
    styleButton(m_btnUseDodge, "#2080C0", "#FFFFFF");
    m_btnUseDodge->setMinimumHeight(50);
    m_btnUseDodge->hide();
    
    m_btnNoDodge = new QPushButton(QString::fromUtf8("不使用"), m_gamePage);
    styleButton(m_btnNoDodge, "#3A2A5A", "#AAAAAA");
    m_btnNoDodge->setMinimumHeight(50);
    m_btnNoDodge->hide();
    
    dodgeLayout->addWidget(dodgeLabel);
    dodgeLayout->addWidget(m_btnUseDodge);
    dodgeLayout->addWidget(m_btnNoDodge);
    
    btnPanel->addWidget(dodgeWidget);
    btnPanel->addStretch();
    
    bottomLayout->addWidget(m_heroPanels[0], 2);
    bottomLayout->addWidget(m_handArea, 3);
    bottomLayout->addLayout(btnPanel, 1);
    
    QHBoxLayout* centerRow = new QHBoxLayout();
    centerRow->addWidget(m_log, 1);
    centerRow->addStretch(3);
    
    mainLayout->addLayout(topArea);
    mainLayout->addLayout(centerRow, 1);
    mainLayout->addWidget(bottomArea);
    
    m_overlay = new AnimOverlay(m_gamePage);
    m_overlay->setGeometry(m_gamePage->geometry());
    m_overlay->lower();
    
    m_videoOverlay = new QWidget(m_gamePage);
    m_videoOverlay->setStyleSheet("background:black;");
    m_videoOverlay->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    m_videoOverlay->hide();
    
    m_videoWidget = new QVideoWidget(m_videoOverlay);
    m_videoPlayer = new QMediaPlayer(this);
    m_videoPlayer->setVideoOutput(m_videoWidget);
    
    m_stackedWidget->addWidget(m_gamePage);
    
    setGameButtonsEnabled(false);
    
    connect(m_btnEndTurn, &QPushButton::clicked, this, &MainWindow::onEndTurn);
    connect(m_btnCancel, &QPushButton::clicked, this, [this](){ exitTargetSelectionMode(); });
    connect(m_btnUseDodge, &QPushButton::clicked, this, [this](){ onDodgeResponse(true); });
    connect(m_btnNoDodge, &QPushButton::clicked, this, [this](){ onDodgeResponse(false); });
    
    for (int i = 0; i < 3; ++i) {
        connect(m_heroPanels[i], &HeroPanel::clicked, this, &MainWindow::onTargetSelected);
    }
    
    connect(m_videoPlayer, static_cast<void(QMediaPlayer::*)(QMediaPlayer::State)>(&QMediaPlayer::stateChanged), 
            this, [this](QMediaPlayer::State state) {
        if (state == QMediaPlayer::StoppedState) {
            m_videoOverlay->hide();
        }
    });
}

void MainWindow::resizeEvent(QResizeEvent* e) {
    QMainWindow::resizeEvent(e);
    if (m_overlay) {
        m_overlay->setGeometry(m_gamePage->geometry());
    }
    if (m_videoOverlay) {
        m_videoOverlay->setGeometry(m_gamePage->geometry());
    }
    if (m_centerEffect) {
        m_centerEffect->setGeometry(m_gamePage->geometry());
        m_centerEffect->lower();
    }
}

void MainWindow::paintEvent(QPaintEvent*) {
    QPainter p(this);
    QRect r = rect();
    
    QLinearGradient bg(r.topLeft(), r.bottomRight());
    bg.setColorAt(0.0, QColor(0x0D,0x08,0x1E));
    bg.setColorAt(0.5, QColor(0x12,0x08,0x28));
    bg.setColorAt(1.0, QColor(0x08,0x04,0x14));
    p.fillRect(r, bg);
    
    p.setPen(QPen(QColor(255,255,255,5), 1));
    for (int x = 0; x < r.width();  x += 48) p.drawLine(x,0,x,r.height());
    for (int y = 0; y < r.height(); y += 48) p.drawLine(0,y,r.width(),y);
    
    QRadialGradient rg(r.center(), r.width() * 0.5);
    int pulseAlpha = 20 + (int)(10 * qSin(m_bgPhase));
    rg.setColorAt(0.0, QColor(0x45,0x10,0x90, pulseAlpha));
    rg.setColorAt(1.0, QColor(0,0,0,0));
    p.fillRect(r, rg);
    
    for (const auto& pt : m_particles) {
        float a = pt.life / pt.maxLife;
        QColor col = pt.color;
        col.setAlphaF(a * 0.9);
        
        p.save();
        p.translate(pt.pos);
        p.rotate(pt.rotation);
        
        QRadialGradient pg(0, 0, pt.size * a);
        pg.setColorAt(0, col);
        pg.setColorAt(1, QColor(col.red(), col.green(), col.blue(), 0));
        
        p.setPen(Qt::NoPen);
        p.setBrush(pg);
        p.drawEllipse(QPointF(0,0), (double)(pt.size * a + 2), (double)(pt.size * a + 2));
        p.restore();
    }
}

void MainWindow::onBackgroundTimer() {
    m_bgPhase += 0.05f;
    if (m_bgPhase > 6.28f) m_bgPhase -= 6.28f;
    update();
}

void MainWindow::onParticleTimer() {
    for (int i = m_particles.size()-1; i >= 0; --i) {
        m_particles[i].pos     += m_particles[i].vel;
        m_particles[i].rotation += m_particles[i].rotSpeed;
        m_particles[i].life    -= 0.02f;
        m_particles[i].vel     *= 0.98f;
        
        if (m_particles[i].style == "spiral") {
            float angle = m_particles[i].rotation * 0.1f;
            m_particles[i].vel.setX(m_particles[i].vel.x() + qCos(angle) * 0.1f);
            m_particles[i].vel.setY(m_particles[i].vel.y() + qSin(angle) * 0.1f);
        }
        
        if (m_particles[i].life <= 0) m_particles.removeAt(i);
    }
    
    if (!m_particles.isEmpty()) update();
    else m_particleTimer->stop();
}

void MainWindow::spawnParticles(const QPoint& pos, const QColor& col, int count, const QString& style) {
    for (int i = 0; i < count; ++i) {
        Particle pt;
        pt.pos = pos;
        
        float angle = QRandomGenerator::global()->bounded(360) * 3.14159f / 180.f;
        float speed = QRandomGenerator::global()->bounded(100) * 0.04f + 0.5f;
        
        if (style == "burst") {
            pt.vel = QPointF(qCos(angle)*speed*2, qSin(angle)*speed*2);
        } else if (style == "spiral") {
            pt.vel = QPointF(qCos(angle)*speed, qSin(angle)*speed);
        } else if (style == "rise") {
            pt.vel = QPointF((QRandomGenerator::global()->bounded(100)-50)*0.01f, -speed);
        } else {
            pt.vel = QPointF(qCos(angle)*speed, qSin(angle)*speed);
        }
        
        pt.maxLife = pt.life = 0.6f + QRandomGenerator::global()->bounded(80)*0.008f;
        pt.color   = col;
        pt.size    = 3.f + QRandomGenerator::global()->bounded(100)*0.04f;
        pt.rotation = QRandomGenerator::global()->bounded(360);
        pt.rotSpeed = (QRandomGenerator::global()->bounded(100)-50)*0.1f;
        pt.style = style;
        
        m_particles.append(pt);
    }
    if (!m_particleTimer->isActive()) m_particleTimer->start();
}

QPoint MainWindow::playerPanelCenter(int idx) const {
    if (idx < 0 || idx > 2 || !m_heroPanels[idx]) return QPoint(width()/2, height()/2);
    return m_heroPanels[idx]->mapTo(this, m_heroPanels[idx]->rect().center());
}

void MainWindow::startGameWithHero(const QString& heroName) {
    m_selectedHero = heroName;
    
    delete m_engine; m_engine = nullptr;
    for (auto*& pl : m_players) { delete pl; pl = nullptr; }

    m_players[0] = new Player(QString::fromUtf8("玩家"), 4);
    m_players[1] = new Player(QString::fromUtf8("AI-邪"), 4);
    m_players[2] = new Player(QString::fromUtf8("AI-邪"), 4);
    
    if (heroName == QString::fromUtf8("曹操")) {
        m_players[0]->setHero(new CaoCao());
        m_players[1]->setHero(new LiuBei());
        m_players[2]->setHero(new SimaYi());
    } else if (heroName == QString::fromUtf8("刘备")) {
        m_players[0]->setHero(new LiuBei());
        m_players[1]->setHero(new CaoCao());
        m_players[2]->setHero(new SimaYi());
    } else {
        m_players[0]->setHero(new SimaYi());
        m_players[1]->setHero(new CaoCao());
        m_players[2]->setHero(new LiuBei());
    }

    for (int i = 0; i < 3; ++i)
        m_heroPanels[i]->setPlayer(m_players[i]);

    m_engine = new GameEngine(this);
    connect(m_engine, &GameEngine::logMessage, this, [this](const QString& msg){
        m_log->append(msg);
        m_log->verticalScrollBar()->setValue(m_log->verticalScrollBar()->maximum());
    });
    connect(m_engine, &GameEngine::gameStateChanged,  this,   &MainWindow::onUpdateUI);
    connect(m_engine, &GameEngine::gameOver,          this,   &MainWindow::onGameOver);
    connect(m_engine, &GameEngine::animationRequest,  this,   &MainWindow::onAnimationRequest);
    connect(m_engine, &GameEngine::showCenterEffect,  this,   &MainWindow::onShowCenterEffect);
    connect(m_engine, &GameEngine::skillEffectRequest,this,   &MainWindow::onSkillEffectRequest);
    connect(m_engine, &GameEngine::requestDodgeResponse, this, &MainWindow::onRequestDodgeResponse);

    m_engine->startGame(m_players[0], m_players[1], m_players[2]);
    setGameButtonsEnabled(true);
    refreshSkillButtons();
    onUpdateUI();
}

void MainWindow::onStartGame() {
    m_stackedWidget->setCurrentWidget(m_characterSelectPage);
}

void MainWindow::onCharacterSelected(const QString& heroName) {
    m_stackedWidget->setCurrentWidget(m_gamePage);
    startGameWithHero(heroName);
}

void MainWindow::setGameButtonsEnabled(bool en) {
    m_btnEndTurn->setEnabled(en);
    for (auto* btn : m_skillButtons) {
        btn->setEnabled(en);
    }
}

void MainWindow::refreshSkillButtons() {
    for (auto* btn : m_skillButtons) {
        btn->hide();
    }
    
    if (!m_players[0] || !m_players[0]->getHero()) return;
    
    Hero* h = m_players[0]->getHero();
    int skillCount = h->getSkillCount();
    
    QVector<int> activeSkillIndices;
    QVector<int> passiveSkillIndices;
    
    for (int i = 0; i < skillCount; ++i) {
        SkillInfo info = h->getSkillInfo(i);
        if (info.isActive()) {
            activeSkillIndices.append(i);
        } else {
            passiveSkillIndices.append(i);
        }
    }
    
    int btnIdx = 0;
    for (int i : activeSkillIndices) {
        if (btnIdx >= m_skillButtons.size()) break;
        SkillInfo info = h->getSkillInfo(i);
        QPushButton* btn = m_skillButtons[btnIdx];
        
        QString prefix = QString::fromUtf8("[主]");
        btn->setText(prefix + info.name);
        btn->setToolTip(QString::fromUtf8("<html><head/><body><p style='white-space:pre-wrap;font-size:16px;'>"
                                          "<b style='font-size:20px;color:#CC99FF;'>%1</b><br><br>%2</p></body></html>")
                        .arg(info.name, info.desc));
        btn->setEnabled(true);
        btn->show();
        
        disconnect(btn, nullptr, nullptr, nullptr);
        connect(btn, &QPushButton::clicked, this, [this, i](){ onSkillClicked(i); });
        btnIdx++;
    }
    
    for (int i : passiveSkillIndices) {
        if (btnIdx >= m_skillButtons.size()) break;
        SkillInfo info = h->getSkillInfo(i);
        QPushButton* btn = m_skillButtons[btnIdx];
        
        QString prefix = QString::fromUtf8("[被]");
        btn->setText(prefix + info.name);
        btn->setToolTip(QString::fromUtf8("<html><head/><body><p style='white-space:pre-wrap;font-size:16px;'>"
                                          "<b style='font-size:20px;color:#AA77DD;'>%1</b><br><br>%2</p></body></html>")
                        .arg(info.name, info.desc));
        btn->setEnabled(false);
        btn->show();
        btnIdx++;
    }
}

void MainWindow::onEndTurn() {
    if (!m_engine) return;
    exitTargetSelectionMode();
    m_engine->playerEndTurn();
}

void MainWindow::onSkillClicked(int skillIndex) {
    if (!m_engine) return;
    m_pendingSkillIndex = skillIndex;
    m_pendingCardIndex  = -1;
    enterTargetSelectionMode([this, skillIndex](int tgt){
        m_engine->playerUseSkill(skillIndex, tgt);
    });
}

void MainWindow::onCardClicked(int index) {
    if (!m_engine || !m_players[0]) return;
    const auto& cards = m_players[0]->getHandCards();
    if (index < 0 || index >= cards.size()) return;
    CardType t = cards[index]->getType();
    if (t == CardType::Peach) {
        m_engine->playerUseCard(index, 0);
        return;
    }
    if (t == CardType::Dodge) {
        m_log->append(QString::fromUtf8("【闪】只能应对他人的【杀】，不能主动使用"));
        return;
    }
    
    TrickCard* trick = dynamic_cast<TrickCard*>(cards[index]);
    if (trick && !trick->requiresTarget()) {
        m_engine->playerUseCard(index, 0);
        return;
    }
    
    m_pendingCardIndex  = index;
    m_pendingSkillIndex = -1;
    enterTargetSelectionMode([this, index](int tgt){
        m_engine->playerUseCard(index, tgt);
    });
}

void MainWindow::enterTargetSelectionMode(std::function<void(int)> cb) {
    m_selectingTarget = true;
    m_targetCallback  = cb;
    for (int i = 0; i < 3; ++i) m_heroPanels[i]->highlight(i != 0);
    m_btnCancel->show();
    m_log->append(QString::fromUtf8("请点击目标玩家..."));
}

void MainWindow::exitTargetSelectionMode() {
    m_selectingTarget   = false;
    m_targetCallback    = nullptr;
    m_pendingCardIndex  = -1;
    m_pendingSkillIndex = -1;
    for (int i = 0; i < 3; ++i) m_heroPanels[i]->highlight(false);
    m_btnCancel->hide();
}

void MainWindow::onTargetSelected(int playerIndex) {
    if (!m_selectingTarget || !m_targetCallback) return;
    auto cb = m_targetCallback;
    exitTargetSelectionMode();
    cb(playerIndex);
}

void MainWindow::onUpdateUI() {
    if (!m_engine) return;
    for (int i = 0; i < 3; ++i)
        if (m_heroPanels[i]) m_heroPanels[i]->refresh();

    int cur     = m_engine->currentPlayerIndex();
    Player* cp  = m_engine->currentPlayer();
    if (cp) {
        m_turnLabel->setText(cp->getDisplayName() + QString::fromUtf8(" 的回合"));
        
        if (!m_selectingTarget) {
            for (int i = 0; i < 3; ++i) {
                bool isCurrentTurn = (i == cur);
                m_heroPanels[i]->highlight(isCurrentTurn);
                if (isCurrentTurn) m_heroPanels[i]->playTurnGlow();
            }
        }
    }

    bool isHuman = m_engine->isHumanTurn();
    m_btnEndTurn->setEnabled(isHuman && !m_selectingTarget);
    for (auto* btn : m_skillButtons) {
        btn->setEnabled(isHuman && !m_selectingTarget);
    }
    
    refreshSkillButtons();
    refreshHandCards();
}

void MainWindow::refreshHandCards() {
    QLayout* oldLayout = m_handArea->layout();
    if (oldLayout) {
        QLayoutItem* item;
        while ((item = oldLayout->takeAt(0)) != nullptr) {
            if (item->widget()) item->widget()->deleteLater();
            delete item;
        }
        delete oldLayout;
    }
    m_cardButtons.clear();
    
    if (!m_engine || !m_players[0]) return;

    bool isHuman = m_engine->isHumanTurn();
    const auto& cards = m_players[0]->getHandCards();
    int n = cards.size();
    
    QHBoxLayout* layout = new QHBoxLayout(m_handArea);
    layout->setContentsMargins(15, 15, 15, 15);
    layout->setSpacing(12);
    
    if (n == 0) {
        QLabel* emptyLabel = new QLabel(QString::fromUtf8("没有手牌"), m_handArea);
        emptyLabel->setAlignment(Qt::AlignCenter);
        emptyLabel->setStyleSheet("color:#666;font-size:22px;");
        layout->addWidget(emptyLabel);
        return;
    }

    for (int i = 0; i < n; ++i) {
        QColor col = cardColor(cards[i]->getType());
        QString light = col.lighter(170).name();
        QString dark  = col.darker(150).name();
        
        QString cardName = cards[i]->getName();
        QString displayName = cardName;
        if (cardName.length() > 4) {
            displayName = cardName.left(4) + QString::fromUtf8("...");
        }
        
        QPushButton* btn = new QPushButton(displayName, m_handArea);
        btn->setCursor(Qt::PointingHandCursor);
        
        QString fullDesc = cards[i]->getDescription();
        btn->setToolTip(QString::fromUtf8("<html><head/><body><p style='white-space:pre-wrap;font-size:16px;'>"
                                          "<b style='font-size:20px;color:%1;'>%2</b><br><br>%3</p></body></html>")
                        .arg(col.lighter(140).name(), cardName, fullDesc));
        
        btn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        btn->setMinimumWidth(110);
        btn->setMinimumHeight(130);
        
        btn->setStyleSheet(QString(
            "QPushButton{"
            "  background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 %1,stop:1 %2);"
            "  color:white;border:3px solid rgba(255,255,255,60);"
            "  border-radius:12px;font-size:22px;font-weight:bold;"
            "}"
            "QPushButton:hover{"
            "  border:4px solid white;"
            "  background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 %3,stop:1 %1);"
            "}"
            "QPushButton:pressed{background:%2;}"
            "QPushButton:disabled{opacity:0.5;}"
        ).arg(light, dark, col.lighter(200).name()));
        
        btn->setEnabled(isHuman && !m_selectingTarget);
        
        int idx = i;
        connect(btn, &QPushButton::clicked, this, [this,idx](){ onCardClicked(idx); });
        
        layout->addWidget(btn);
        m_cardButtons.append(btn);
    }
}

void MainWindow::showFloatingText(const QString& text, int playerIndex, const QString& color) {
    m_overlay->playSkillFlash(playerPanelCenter(playerIndex), text, QColor(color));
}

void MainWindow::playCardAnimation(int fromIdx, int toIdx, const QString& cardName) {
    QPoint from = playerPanelCenter(fromIdx);
    QPoint to   = playerPanelCenter(toIdx);

    CardType t = CardType::Kill;
    if      (cardName == QString::fromUtf8("闪"))                    t = CardType::Dodge;
    else if (cardName == QString::fromUtf8("桃"))                    t = CardType::Peach;
    else if (cardName.contains(QString::fromUtf8("顺手"))
          || cardName.contains(QString::fromUtf8("过河"))
          || cardName.contains(QString::fromUtf8("跳跳虎"))
          || cardName.contains(QString::fromUtf8("天意侵袭"))
          || cardName.contains(QString::fromUtf8("锦囊")))              t = CardType::Trick;
    QColor col = cardColor(t);
    m_overlay->playCardFly(from, to, cardName, col);
    
    QString particleStyle = (t == CardType::Kill) ? "burst" : 
                            (t == CardType::Peach) ? "rise" : "spiral";
    spawnParticles(to, col, 30, particleStyle);
    
    if (toIdx >= 0 && toIdx < 3) {
        if (t == CardType::Kill) {
            m_heroPanels[toIdx]->flashDamage();
            m_overlay->playDamageNumber(to, 1, C_RED);
        } else if (t == CardType::Peach) {
            m_heroPanels[toIdx]->flashHeal();
            m_overlay->playHealNumber(to, 1, C_JADE);
        }
    }
}

void MainWindow::playSkillAnimation(int fromIdx, int toIdx, const QString& skillName) {
    QPoint center = playerPanelCenter(fromIdx);
    QColor col    = C_PURPLE;
    m_overlay->playSkillFlash(center, skillName, col);
    
    if (toIdx != fromIdx && toIdx >= 0 && toIdx < 3) {
        spawnParticles(playerPanelCenter(toIdx), col, 25, "spiral");
        m_heroPanels[toIdx]->flashDamage();
    }
}

void MainWindow::playDamageAnimation(int playerIdx, int amount) {
    if (playerIdx < 0 || playerIdx > 2) return;
    m_heroPanels[playerIdx]->flashDamage();
    m_overlay->playDamageNumber(playerPanelCenter(playerIdx), amount, C_RED);
    spawnParticles(playerPanelCenter(playerIdx), C_RED, 35, "burst");
}

void MainWindow::playHealAnimation(int playerIdx, int amount) {
    if (playerIdx < 0 || playerIdx > 2) return;
    m_heroPanels[playerIdx]->flashHeal();
    m_overlay->playHealNumber(playerPanelCenter(playerIdx), amount, C_JADE);
    spawnParticles(playerPanelCenter(playerIdx), C_JADE, 20, "rise");
}

void MainWindow::playTurnStartAnimation(int playerIdx) {
    if (playerIdx < 0 || playerIdx > 2) return;
    m_overlay->playTurnStart(playerPanelCenter(playerIdx), m_players[playerIdx]->getDisplayName());
    spawnParticles(playerPanelCenter(playerIdx), C_GOLD, 15, "spiral");
}

void MainWindow::showCenterEffectText(const QString& text, const QString& type, int durationMs) {
    if (m_centerEffect) {
        m_centerEffect->showEffect(text, type, durationMs);
    }
}

void MainWindow::playVideoEffect(const QString& videoPath) {
    QFile videoFile(videoPath);
    if (!videoFile.exists()) {
        qDebug() << QString::fromUtf8("视频文件不存在:") << videoPath;
        return;
    }
    
    if (m_videoOverlay && m_videoPlayer) {
        m_videoOverlay->setGeometry(m_gamePage->geometry());
        m_videoOverlay->show();
        m_videoOverlay->raise();
        
        m_videoWidget->setGeometry(m_videoOverlay->rect());
        m_videoWidget->show();
        
        m_videoPlayer->setMedia(QUrl::fromLocalFile(videoPath));
        m_videoPlayer->play();
    }
}

void MainWindow::onAnimationRequest(const QString& type, int from, int to, const QString& extra) {
    if      (type == "card")  playCardAnimation(from, to, extra);
    else if (type == "skill") playSkillAnimation(from, to, extra);
    else if (type == "damage") playDamageAnimation(to, extra.toInt());
    else if (type == "heal")   playHealAnimation(to, extra.toInt());
    else if (type == "turn")   playTurnStartAnimation(from);
}

void MainWindow::onShowCenterEffect(const QString& type, const QString& text, int durationMs) {
    showCenterEffectText(text, type, durationMs);
}

void MainWindow::onSkillEffectRequest(const QString& heroName, const QString& skillName, const QString& videoPath) {
    Q_UNUSED(heroName)
    showCenterEffectText(skillName, QString::fromUtf8("skill"), 2000);
    playVideoEffect(videoPath);
}

void MainWindow::onRequestDodgeResponse(int targetPlayerIndex, int secondsTimeout) {
    Q_UNUSED(targetPlayerIndex)
    Q_UNUSED(secondsTimeout)
    
    m_btnUseDodge->show();
    m_btnNoDodge->show();
    m_btnUseDodge->setEnabled(true);
    m_btnNoDodge->setEnabled(true);
    
    setGameButtonsEnabled(false);
}

void MainWindow::onDodgeResponse(bool useDodge) {
    m_btnUseDodge->hide();
    m_btnNoDodge->hide();
    
    if (m_engine) {
        m_engine->playerRespondDodge(useDodge);
    }
    
    setGameButtonsEnabled(true);
}

void MainWindow::onGameOver(const QString& winner) {
    setGameButtonsEnabled(false);
    exitTargetSelectionMode();
    for (int i = 0; i < 3; ++i)
        if (m_heroPanels[i]) m_heroPanels[i]->refresh();

    QString msg = winner + QString::fromUtf8(" 获得胜利！");
    m_turnLabel->setText(msg);
    showFloatingText(msg, 1, "#FFD700");
    showCenterEffectText(QString::fromUtf8("胜利"), QString::fromUtf8("gameover"), 2500);

    QTimer::singleShot(800, this, [this, msg](){
        QMessageBox mb(this);
        mb.setWindowTitle(QString::fromUtf8("游戏结束"));
        mb.setText(msg);
        mb.setStyleSheet(
            "QMessageBox{background:#0D0A1A;}"
            "QLabel{color:#D4AF37;font-size:26px;font-weight:bold;}"
            "QPushButton{background:#3A1A5C;color:#CC99FF;"
            "border:2px solid #6A3A8C;border-radius:10px;"
            "padding:12px 40px;font-size:20px;font-weight:bold;}"
            "QPushButton:hover{background:#5A2A8C;}");
        mb.exec();
        
        m_stackedWidget->setCurrentWidget(m_characterSelectPage);
    });
}

bool MainWindow::eventFilter(QObject* watched, QEvent* event) {
    if (event->type() == QEvent::MouseButtonPress) {
        if (m_characterSelectPage && m_characterSelectPage->isAncestorOf(qobject_cast<QWidget*>(watched))) {
            QWidget* w = qobject_cast<QWidget*>(watched);
            while (w && w != m_characterSelectPage) {
                QVariant heroProp = w->property("heroName");
                if (heroProp.isValid()) {
                    QString heroName = heroProp.toString();
                    onCharacterSelected(heroName);
                    return true;
                }
                w = w->parentWidget();
            }
        }
    }
    return QMainWindow::eventFilter(watched, event);
}
