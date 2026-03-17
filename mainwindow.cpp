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
    if (heroName == QString::fromUtf8("曹操"))       return QColor(0xC0, 0x20, 0x2A);
    if (heroName == QString::fromUtf8("司马懿")) return QColor(0x20, 0x50, 0xC0);
    return QColor(0x20, 0xB0, 0x50);
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
        p.drawText(QRect(-150,-25,300,50), Qt::AlignCenter, ts.name + QString::fromUtf8(" \u7684\u56de\u5408"));
        p.setPen(C_GOLD);
        p.drawText(QRect(-150,-25,300,50), Qt::AlignCenter, ts.name + QString::fromUtf8(" \u7684\u56de\u5408"));
        p.restore();
    }
}

HeroPanel::HeroPanel(int idx, QWidget* parent)
    : QWidget(parent), m_idx(idx) {
    setMinimumSize(240, 150);
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
    QRect r = rect().adjusted(2,2,-2,-2);
    
    QPainterPath bg;
    bg.addRoundedRect(r, 12, 12);
    
    QLinearGradient grad(r.topLeft(), r.bottomLeft());
    grad.setColorAt(0.0, QColor(0x2C,0x1A,0x50,240));
    grad.setColorAt(1.0, QColor(0x0D,0x07,0x22,240));
    p.fillPath(bg, grad);
    
    if (m_glowIntensity > 0.0) {
        QColor glowCol = C_GOLD;
        glowCol.setAlphaF(m_glowIntensity * 0.4f);
        p.fillPath(bg, glowCol);
    }
    
    if (m_highlighted) {
        for (int i = 6; i >= 1; --i) {
            QColor gc = C_GOLD;
            gc.setAlpha(25*i);
            p.setPen(QPen(gc, i*2+3));
            p.setBrush(Qt::NoBrush);
            p.drawPath(bg);
        }
        p.setPen(QPen(C_GOLD, 2.5));
    } else if (m_hovered) {
        p.setPen(QPen(QColor(0x88, 0x66, 0xAA), 1.5));
    } else {
        p.setPen(QPen(QColor(0x55,0x35,0x88), 1.5));
    }
    p.drawPath(bg);
    
    if (!m_player) {
        p.setPen(QColor(120,100,160));
        QFont f; f.setPixelSize(14); p.setFont(f);
        p.drawText(r, Qt::AlignCenter, QString::fromUtf8("\u7b49\u5f85\u4e2d..."));
        return;
    }
    
    bool alive = m_player->isAlive();
    if (!alive) p.setOpacity(0.4);
    
    QString heroName   = m_player->getHero() ? m_player->getHero()->getName() : "?";
    QString playerName = m_player->getName();
    
    QRect portrait(r.left()+10, r.top()+10, 70, 85);
    QPainterPath pp; pp.addRoundedRect(portrait, 8, 8);
    
    QColor themeCol = heroThemeColor(heroName);
    QLinearGradient pg(portrait.topLeft(), portrait.bottomRight());
    pg.setColorAt(0, themeCol.lighter(140));
    pg.setColorAt(0.5, themeCol);
    pg.setColorAt(1, themeCol.darker(160));
    p.fillPath(pp, pg);
    
    p.setPen(QPen(C_GOLD.darker(130), 1.5));
    p.drawPath(pp);
    
    QFont hf; hf.setPixelSize(36); hf.setBold(true); p.setFont(hf);
    p.setPen(themeCol.lighter(160));
    QString portChar = m_player->getHero() ? m_player->getHero()->portraitChar() : heroName.left(1);
    p.drawText(portrait, Qt::AlignCenter, portChar);
    
    int tx = portrait.right() + 14;
    QFont nf; nf.setPixelSize(13); nf.setBold(true); p.setFont(nf);
    p.setPen(C_GOLD);
    p.drawText(tx, r.top()+22, playerName + QString::fromUtf8(" \u00b7 ") + heroName);
    
    if (m_player->getHero()) {
        QFont sf; sf.setPixelSize(9); p.setFont(sf);
        p.setPen(C_PURPLE);
        QString sk = QString::fromUtf8("\u300a") + m_player->getHero()->getSkillName(0)
                   + QString::fromUtf8("\u300b \u300a") + m_player->getHero()->getSkillName(1) + QString::fromUtf8("\u300b");
        p.drawText(tx, r.top()+38, sk);
    }
    
    int hp = m_player->getHealth(), maxHp = m_player->getMaxHealth();
    int pipW=14, pipH=10, pipGap=3, startX=tx, startY=r.top()+54;
    
    for (int i = 0; i < maxHp; ++i) {
        QRectF pip(startX+i*(pipW+pipGap), startY, pipW, pipH);
        QPainterPath pp2; pp2.addRoundedRect(pip,3,3);
        QColor fc;
        if (i < hp) {
            float ratio=(float)hp/maxHp;
            if      (ratio>0.6f) fc=QColor(0x22,0xDD,0x66);
            else if (ratio>0.3f) fc=QColor(0xFF,0x99,0x22);
            else                 fc=QColor(0xEE,0x33,0x33);
        } else { fc=QColor(0x25,0x18,0x35); }
        p.fillPath(pp2, fc);
    }
    
    QFont hpf; hpf.setPixelSize(10); p.setFont(hpf);
    p.setPen(QColor(200,200,200));
    p.drawText(startX, startY+pipH+14, QString("%1/%2 HP").arg(hp).arg(maxHp));
    p.setPen(QColor(160,140,200));
    p.drawText(startX, startY+pipH+26, QString::fromUtf8("\u624b\u724c: ") + QString::number(m_player->getHandCards().size()));
    
    if (m_flashAlpha > 0.0) {
        p.fillPath(bg, QColor(0xFF,0x10,0x10,(int)(m_flashAlpha*120)));
    }
    if (m_healAlpha > 0.0) {
        p.fillPath(bg, QColor(0x20,0xDD,0x60,(int)(m_healAlpha*100)));
    }
    if (!alive) {
        p.fillPath(bg, QColor(0,0,0,150));
        QFont df; df.setPixelSize(22); df.setBold(true); p.setFont(df);
        p.setPen(QColor(180,0,0,220));
        p.drawText(r, Qt::AlignCenter, QString::fromUtf8("\u9635\u4ea1"));
    }
}

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle(QString::fromUtf8("\u4e09\u56fd\u6740 \u00b7 \u5929\u610f\u4fb5\u8680"));
    resize(1280, 800);
    setMinimumSize(1024, 700);
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
        "border:2px solid rgba(255,255,255,30);"
        "border-radius:8px;font-size:14px;font-weight:bold;padding:10px;}"
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
    
    QVBoxLayout* mainLayout = new QVBoxLayout(m_centralWidget);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(8);
    
    QHBoxLayout* topArea = new QHBoxLayout();
    topArea->setSpacing(10);
    
    m_heroPanels[1] = new HeroPanel(1, m_centralWidget);
    m_heroPanels[2] = new HeroPanel(2, m_centralWidget);
    
    QVBoxLayout* topLeft = new QVBoxLayout();
    topLeft->addWidget(m_heroPanels[1]);
    topLeft->setStretch(0, 1);
    
    m_turnLabel = new QLabel(QString::fromUtf8("\u70b9\u51fb\u300c\u5f00\u59cb\u6e38\u620f\u300d\u5f00\u59cb"), m_centralWidget);
    m_turnLabel->setAlignment(Qt::AlignCenter);
    m_turnLabel->setStyleSheet(
        "color:#D4AF37;font-size:20px;font-weight:bold;background:rgba(0,0,0,100);"
        "padding:8px;border-radius:8px;");
    m_turnLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    
    QVBoxLayout* topRight = new QVBoxLayout();
    topRight->addWidget(m_heroPanels[2]);
    topRight->setStretch(0, 1);
    
    topArea->addLayout(topLeft, 1);
    topArea->addWidget(m_turnLabel, 2);
    topArea->addLayout(topRight, 1);
    
    m_handArea = new QWidget(m_centralWidget);
    m_handArea->setMinimumHeight(130);
    m_handArea->setStyleSheet("background:rgba(20,15,35,180);border-radius:10px;border:1px solid #3A2A5A;");
    
    m_log = new QTextBrowser(m_centralWidget);
    m_log->setMaximumWidth(200);
    m_log->setStyleSheet(
        "QTextBrowser{background:rgba(8,6,20,230);color:#C8B8F0;"
        "border:1px solid #3A2A5A;border-radius:8px;"
        "font-family:'Microsoft YaHei';font-size:11px;padding:4px;}");
    
    QWidget* bottomArea = new QWidget(m_centralWidget);
    QHBoxLayout* bottomLayout = new QHBoxLayout(bottomArea);
    bottomLayout->setContentsMargins(0, 0, 0, 0);
    bottomLayout->setSpacing(10);
    
    m_heroPanels[0] = new HeroPanel(0, m_centralWidget);
    
    QVBoxLayout* btnPanel = new QVBoxLayout();
    btnPanel->setSpacing(8);
    m_btnStart   = new QPushButton(QString::fromUtf8("\u2694 \u5f00\u59cb\u6e38\u620f"), m_centralWidget);
    m_btnEndTurn = new QPushButton(QString::fromUtf8("\u23e9 \u7ed3\u675f\u56de\u5408"), m_centralWidget);
    m_btnSkill0  = new QPushButton(QString::fromUtf8("\u2605 \u6280\u80fd\u4e00"), m_centralWidget);
    m_btnSkill1  = new QPushButton(QString::fromUtf8("\u2605 \u6280\u80fd\u4e8c"), m_centralWidget);
    m_btnCancel  = new QPushButton(QString::fromUtf8("\u2716 \u53d6\u6d88"), m_centralWidget);
    
    styleButton(m_btnStart,   "#5A0A00", "#FFD700");
    styleButton(m_btnEndTurn, "#0A2035", "#A0D8EF");
    styleButton(m_btnSkill0,  "#2A0A44", "#CC99FF");
    styleButton(m_btnSkill1,  "#2A0A44", "#CC99FF");
    styleButton(m_btnCancel,  "#1E1E1E", "#CC7777");
    
    m_btnCancel->hide();
    m_btnStart->setMinimumHeight(50);
    m_btnEndTurn->setMinimumHeight(40);
    m_btnSkill0->setMinimumHeight(40);
    m_btnSkill1->setMinimumHeight(40);
    m_btnCancel->setMinimumHeight(40);
    
    btnPanel->addWidget(m_btnStart);
    btnPanel->addWidget(m_btnSkill0);
    btnPanel->addWidget(m_btnSkill1);
    btnPanel->addWidget(m_btnEndTurn);
    btnPanel->addWidget(m_btnCancel);
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
    
    m_overlay = new AnimOverlay(m_centralWidget);
    m_overlay->setGeometry(m_centralWidget->geometry());
    m_overlay->lower();
    
    setGameButtonsEnabled(false);

    connect(m_btnStart,   &QPushButton::clicked, this, &MainWindow::onStartGame);
    connect(m_btnEndTurn, &QPushButton::clicked, this, &MainWindow::onEndTurn);
    connect(m_btnSkill0,  &QPushButton::clicked, this, &MainWindow::onSkill0);
    connect(m_btnSkill1,  &QPushButton::clicked, this, &MainWindow::onSkill1);
    connect(m_btnCancel,  &QPushButton::clicked, this, [this](){ exitTargetSelectionMode(); });

    for (int i = 0; i < 3; ++i) {
        connect(m_heroPanels[i], &HeroPanel::clicked, this, &MainWindow::onTargetSelected);
    }

    m_particleTimer = new QTimer(this);
    m_particleTimer->setInterval(16);
    connect(m_particleTimer, &QTimer::timeout, this, &MainWindow::onParticleTimer);
    
    m_backgroundTimer = new QTimer(this);
    m_backgroundTimer->setInterval(33);
    connect(m_backgroundTimer, &QTimer::timeout, this, &MainWindow::onBackgroundTimer);
    m_backgroundTimer->start();
}

void MainWindow::resizeEvent(QResizeEvent* e) {
    QMainWindow::resizeEvent(e);
    if (m_overlay) {
        m_overlay->setGeometry(m_centralWidget->geometry());
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

void MainWindow::onStartGame() {
    delete m_engine; m_engine = nullptr;
    for (auto*& pl : m_players) { delete pl; pl = nullptr; }

    m_players[0] = new Player(QString::fromUtf8("\u73a9\u5bb6"),   4);
    m_players[1] = new Player(QString::fromUtf8("AI-\u9b4f"),  4);
    m_players[2] = new Player(QString::fromUtf8("AI-\u5434"),   4);
    m_players[0]->setHero(new LiuBei());
    m_players[1]->setHero(new CaoCao());
    m_players[2]->setHero(new SimaYi());

    for (int i = 0; i < 3; ++i)
        m_heroPanels[i]->setPlayer(m_players[i]);

    m_engine = new GameEngine(this);
    connect(m_engine, &GameEngine::logMessage,        m_log,  &QTextBrowser::append);
    connect(m_engine, &GameEngine::gameStateChanged,  this,   &MainWindow::onUpdateUI);
    connect(m_engine, &GameEngine::gameOver,          this,   &MainWindow::onGameOver);
    connect(m_engine, &GameEngine::animationRequest,  this,   &MainWindow::onAnimationRequest);

    m_engine->startGame(m_players[0], m_players[1], m_players[2]);
    m_btnStart->hide();
    setGameButtonsEnabled(true);
    onUpdateUI();
}

void MainWindow::setGameButtonsEnabled(bool en) {
    m_btnEndTurn->setEnabled(en);
    m_btnSkill0->setEnabled(en);
    m_btnSkill1->setEnabled(en);
}

void MainWindow::onEndTurn() {
    if (!m_engine) return;
    exitTargetSelectionMode();
    m_engine->playerEndTurn();
}

void MainWindow::onSkill0() {
    if (!m_engine) return;
    m_pendingSkillIndex = 0;
    m_pendingCardIndex  = -1;
    enterTargetSelectionMode([this](int tgt){
        m_engine->playerUseSkill(0, tgt);
    });
}

void MainWindow::onSkill1() {
    if (!m_engine) return;
    m_pendingSkillIndex = 1;
    m_pendingCardIndex  = -1;
    enterTargetSelectionMode([this](int tgt){
        m_engine->playerUseSkill(1, tgt);
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
        m_log->append(QString::fromUtf8("\u3010\u95ea\u3011\u53ea\u80fd\u5e94\u5bf9\u4ed6\u4eba\u7684\u3010\u6740\u3011\uff0c\u4e0d\u80fd\u4e3b\u52a8\u4f7f\u7528"));
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
    m_log->append(QString::fromUtf8("\u8bf7\u70b9\u51fb\u76ee\u6807\u73a9\u5bb6..."));
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
        m_turnLabel->setText(cp->getName() + QString::fromUtf8(" \u7684\u56de\u5408"));
        for (int i = 0; i < 3; ++i) {
            bool isCurrentTurn = (i == cur) && !m_selectingTarget;
            m_heroPanels[i]->highlight(isCurrentTurn);
            if (isCurrentTurn) m_heroPanels[i]->playTurnGlow();
        }
    }

    bool isHuman = m_engine->isHumanTurn();
    m_btnEndTurn->setEnabled(isHuman);
    m_btnSkill0->setEnabled(isHuman);
    m_btnSkill1->setEnabled(isHuman);

    if (m_players[0] && m_players[0]->getHero()) {
        Hero* h = m_players[0]->getHero();
        m_btnSkill0->setText(QString::fromUtf8("\u2605 ") + h->getSkillName(0));
        m_btnSkill1->setText(QString::fromUtf8("\u2605 ") + h->getSkillName(1));
    }
    refreshHandCards();
}

void MainWindow::refreshHandCards() {
    for (auto* btn : m_cardButtons) {
        btn->deleteLater();
    }
    m_cardButtons.clear();
    
    if (!m_engine || !m_players[0]) return;

    bool isHuman = m_engine->isHumanTurn();
    const auto& cards = m_players[0]->getHandCards();
    int n = cards.size();
    
    QLayout* layout = m_handArea->layout();
    if (!layout) {
        layout = new QHBoxLayout(m_handArea);
        layout->setContentsMargins(10, 10, 10, 10);
        layout->setSpacing(10);
    }
    
    if (n == 0) {
        QLabel* emptyLabel = new QLabel(QString::fromUtf8("\u6ca1\u6709\u624b\u724c"), m_handArea);
        emptyLabel->setAlignment(Qt::AlignCenter);
        emptyLabel->setStyleSheet("color:#666;font-size:16px;");
        layout->addWidget(emptyLabel);
        m_cardButtons.append(nullptr);
        return;
    }

    for (int i = 0; i < n; ++i) {
        QColor col = cardColor(cards[i]->getType());
        QString light = col.lighter(170).name();
        QString dark  = col.darker(150).name();
        
        QPushButton* btn = new QPushButton(cards[i]->getName(), m_handArea);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setToolTip(cards[i]->getDescription());
        btn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        btn->setMinimumWidth(80);
        btn->setMinimumHeight(100);
        
        btn->setStyleSheet(QString(
            "QPushButton{"
            "  background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 %1,stop:1 %2);"
            "  color:white;border:2px solid rgba(255,255,255,50);"
            "  border-radius:8px;font-size:16px;font-weight:bold;"
            "}"
            "QPushButton:hover{"
            "  border:3px solid white;"
            "  background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 %3,stop:1 %1);"
            "}"
            "QPushButton:pressed{background:%2;}"
            "QPushButton:disabled{opacity:0.5;}"
        ).arg(light, dark, col.lighter(200).name()));
        
        btn->setEnabled(isHuman);
        
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
    if      (cardName == QString::fromUtf8("\u95ea"))                    t = CardType::Dodge;
    else if (cardName == QString::fromUtf8("\u6843"))                    t = CardType::Peach;
    else if (cardName.contains(QString::fromUtf8("\u987a\u624b"))
          || cardName.contains(QString::fromUtf8("\u8fc7\u6cb3"))
          || cardName.contains(QString::fromUtf8("\u9526\u56ca")))              t = CardType::Trick;
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
    m_overlay->playTurnStart(playerPanelCenter(playerIdx), m_players[playerIdx]->getName());
    spawnParticles(playerPanelCenter(playerIdx), C_GOLD, 15, "spiral");
}

void MainWindow::onAnimationRequest(const QString& type, int from, int to, const QString& extra) {
    if      (type == "card")  playCardAnimation(from, to, extra);
    else if (type == "skill") playSkillAnimation(from, to, extra);
    else if (type == "damage") playDamageAnimation(to, extra.toInt());
    else if (type == "heal")   playHealAnimation(to, extra.toInt());
    else if (type == "turn")   playTurnStartAnimation(from);
}

void MainWindow::onGameOver(const QString& winner) {
    setGameButtonsEnabled(false);
    exitTargetSelectionMode();
    for (int i = 0; i < 3; ++i)
        if (m_heroPanels[i]) m_heroPanels[i]->refresh();

    QString msg = winner + QString::fromUtf8(" \u83b7\u5f97\u80dc\u5229\uff01");
    m_turnLabel->setText(msg);
    showFloatingText(msg, 1, "#FFD700");

    QTimer::singleShot(500, this, [this, msg](){
        QMessageBox mb(this);
        mb.setWindowTitle(QString::fromUtf8("\u6e38\u620f\u7ed3\u675f"));
        mb.setText(msg);
        mb.setStyleSheet(
            "QMessageBox{background:#0D0A1A;}"
            "QLabel{color:#D4AF37;font-size:18px;font-weight:bold;}"
            "QPushButton{background:#3A1A5C;color:#CC99FF;"
            "border:1px solid #6A3A8C;border-radius:8px;"
            "padding:8px 24px;font-size:14px;font-weight:bold;}"
            "QPushButton:hover{background:#5A2A8C;}");
        mb.exec();
        m_btnStart->show();
        m_btnStart->setEnabled(true);
    });
}
