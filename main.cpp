#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QDesktopServices>
#include <QUrl>
#include <QPainter>
#include <QIntValidator>
#include <QScrollArea>
#include <QListView>
#include <QKeyEvent>
#include <cmath>

class GradientCard : public QFrame {
public:
    GradientCard(QWidget *parent = nullptr, QColor startColor = QColor(45, 45, 65), QColor endColor = QColor(25, 25, 35))
        : QFrame(parent), m_startColor(startColor), m_endColor(endColor) {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        setStyleSheet("border: none; border-radius: 15px;");
    }

    void setColors(QColor start, QColor end) {
        m_startColor = start;
        m_endColor = end;
        update();
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        QLinearGradient gradient(0, 0, width(), height());
        gradient.setColorAt(0, m_startColor);
        gradient.setColorAt(1, m_endColor);

        painter.setPen(Qt::NoPen);
        painter.setBrush(gradient);
        painter.drawRoundedRect(rect(), 15, 15);

        QFrame::paintEvent(event);
    }

private:
    QColor m_startColor;
    QColor m_endColor;
};

// Modern styled combobox with custom animation and no arrow
class ModernComboBox : public QComboBox {
public:
    ModernComboBox(QWidget *parent = nullptr) : QComboBox(parent) {
        setView(new QListView());
        view()->setStyleSheet("background-color: #2d2d3d; border-radius: 6px; padding: 5px;");

        // Remove the arrow
        setStyleSheet(R"(
            QComboBox {
                border-radius: 6px;
                padding: 6px;
                font-size: 14px;
                background-color: #2d2d3d;
                color: white;
                border: 1px solid #3d3d4a;
            }
            QComboBox::drop-down {
                width: 0;
                border: none;
            }
            QComboBox:hover {
                border: 1px solid #5A10F0;
            }
            QComboBox QAbstractItemView {
                background-color: #2d2d3d;
                color: white;
                selection-background-color: #4A00E0;
                selection-color: white;
                border-radius: 6px;
            }
        )");
    }
};

class InstallmentCalculator : public QMainWindow {
    Q_OBJECT

public:
    InstallmentCalculator(QWidget *parent = nullptr) : QMainWindow(parent) {
        setWindowTitle("UIU Tuition Installment Calculator");
        setMinimumSize(480, 620);  // Increased minimum size to accommodate content
        setupUI();
        applyDarkTheme();
    }

protected:
    bool eventFilter(QObject *obj, QEvent *event) override {
        if (obj == amountInput && event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
                calculateInstallments();
                return true;
            }
        }
        return QMainWindow::eventFilter(obj, event);
    }

private slots:
    void calculateInstallments() {
    int total = amountInput->text().toInt();
    int roundTo = roundingComboBox->currentText().toInt();
    double waiver = waiverComboBox->currentData().toDouble();
    const int trimesterFee = 6500;

    // Calculate tuition excluding trimester fee
    int tuition = total - trimesterFee;

    // Apply waiver to tuition only
    int waived = std::round(tuition * waiver);
    int payable = tuition - waived + trimesterFee;

    // Update credit display
    int credits = tuition / 6500;
    creditLabel->setText(QString("Credits: %1 (%2 Tk)").arg(credits).arg(credits * 6500));
    feeLabel->setText(QString("Trimester Fee: %1 Tk").arg(trimesterFee));
    waiverLabel->setText(QString("Waiver Applied: %1 Tk").arg(waived));

    // Fixed calculation based on waiver percentage
    if (waiver >= 1.0) {
        // 100% waiver - only pay trimester fee in the 3rd installment
        animateValue(installmentAmounts[0], 0);
        animateValue(installmentAmounts[1], 0);
        animateValue(installmentAmounts[2], trimesterFee);
        animateValue(totalAmount, trimesterFee);
    } else if (waiver >= 0.5) {
        // 50% waiver - split payment equally between 2nd and 3rd installments
        int halfPayment = payable / 2;

        // Round to next higher value based on roundTo
        int second = ((halfPayment + roundTo - 1) / roundTo) * roundTo;
        int third = payable - second;

        animateValue(installmentAmounts[0], 0);
        animateValue(installmentAmounts[1], second);
        animateValue(installmentAmounts[2], third);
        animateValue(totalAmount, payable);
    } else {
        // Less than 50% waiver - at least 40% first installment, rest split equally
        int minFirst = std::ceil(payable * 0.4);

        // Round UP to the next value based on roundTo (not just nearest)
        int first = ((minFirst + roundTo - 1) / roundTo) * roundTo;

        int remainder = payable - first;
        int halfRemainder = remainder / 2;

        // Round second installment to nearest value
        int second = ((halfRemainder + roundTo/2) / roundTo) * roundTo;

        // Calculate third installment as the remaining amount
        int third = payable - first - second;

        animateValue(installmentAmounts[0], first);
        animateValue(installmentAmounts[1], second);
        animateValue(installmentAmounts[2], third);
        animateValue(totalAmount, payable);
    }
}

    void changeTheme(int index) {
        if (index == 0) applyDarkTheme();
        else applyMidnightBlueTheme();
    }

private:
    void setupUI() {
        QWidget *centralWidget = new QWidget(this);
        QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
        mainLayout->setContentsMargins(12, 12, 12, 5);
        mainLayout->setSpacing(8);

        // Create a scroll area to handle content overflow
        QScrollArea *scrollArea = new QScrollArea();
        scrollArea->setWidgetResizable(true);
        scrollArea->setFrameShape(QFrame::NoFrame);

        QWidget *scrollContent = new QWidget();
        QVBoxLayout *scrollLayout = new QVBoxLayout(scrollContent);
        scrollLayout->setContentsMargins(0, 0, 0, 0);
        scrollLayout->setSpacing(8);

        // Header
        QHBoxLayout *headerLayout = new QHBoxLayout();
        headerLayout->setContentsMargins(0, 0, 0, 0);

        QLabel *logo = new QLabel();
        QPixmap logoPixmap(":/images/logo.png");
        if (logoPixmap.isNull()) {
            // Fallback if logo can't be loaded
            logo->setText("UIU");
            logo->setStyleSheet("font-size: 24px; font-weight: bold; background: linear-gradient(#8E2DE2, #4A00E0); -webkit-background-clip: text; color: transparent;");
        } else {
            // Scale logo to appropriate size
            logoPixmap = logoPixmap.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            logo->setPixmap(logoPixmap);
            logo->setFixedSize(48, 48);
        }
        QVBoxLayout *titleLayout = new QVBoxLayout();
        titleLayout->setSpacing(2);
        QLabel *title = new QLabel("United International University");
        title->setStyleSheet("font-size: 14px; font-weight: bold;");
        QLabel *subtitle = new QLabel("Tuition Installment Calculator");
        subtitle->setStyleSheet("font-size: 12px; color: #9f9fb7;");

        titleLayout->addWidget(title);
        titleLayout->addWidget(subtitle);
        headerLayout->addWidget(logo);
        headerLayout->addLayout(titleLayout);
        headerLayout->addStretch();

        themeComboBox = new ModernComboBox();
        themeComboBox->addItems({"Dark", "Midnight"});
        themeComboBox->setFixedWidth(100);
        connect(themeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &InstallmentCalculator::changeTheme);
        headerLayout->addWidget(themeComboBox);

        mainLayout->addLayout(headerLayout);

        // Separator
        QFrame *line = new QFrame();
        line->setFrameShape(QFrame::HLine);
        line->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #8E2DE2, stop:1 #4A00E0); border: none; height: 1px;");
        mainLayout->addWidget(line);

        // Input Section
        GradientCard *inputCard = new GradientCard();
        QVBoxLayout *inputLayout = new QVBoxLayout(inputCard);
        inputLayout->setContentsMargins(12, 12, 12, 12);

        QGridLayout *inputGrid = new QGridLayout();
        inputGrid->setVerticalSpacing(8);

        amountInput = new QLineEdit();
        amountInput->setValidator(new QIntValidator(6500, 1000000, this));
        amountInput->setPlaceholderText("Total Amount (BDT)");
        amountInput->setStyleSheet("font-size: 14px; padding: 6px;");
        amountInput->installEventFilter(this);  // Install event filter for Enter key

        roundingComboBox = new ModernComboBox();
        roundingComboBox->addItems({"500", "1000"});
        roundingComboBox->setCurrentIndex(0);

        waiverComboBox = new ModernComboBox();
        waiverComboBox->addItem("No Waiver", 0.0);
        waiverComboBox->addItem("25% Waiver", 0.25);
        waiverComboBox->addItem("50% Waiver", 0.50);
        waiverComboBox->addItem("100% Waiver", 1.0);

        creditLabel = new QLabel("Credits: 0 (0 Tk)");
        feeLabel = new QLabel("Trimester Fee: 6500 Tk");
        waiverLabel = new QLabel("Waiver Applied: 0 Tk");

        inputGrid->addWidget(new QLabel("Total Amount:"), 0, 0);
        inputGrid->addWidget(amountInput, 0, 1);
        inputGrid->addWidget(new QLabel("Round to:"), 1, 0);
        inputGrid->addWidget(roundingComboBox, 1, 1);
        inputGrid->addWidget(new QLabel("Waiver:"), 2, 0);
        inputGrid->addWidget(waiverComboBox, 2, 1);
        inputGrid->addWidget(creditLabel, 3, 0, 1, 2);
        inputGrid->addWidget(feeLabel, 4, 0, 1, 2);
        inputGrid->addWidget(waiverLabel, 5, 0, 1, 2);

        QPushButton *calculateBtn = new QPushButton("Calculate");
        calculateBtn->setCursor(Qt::PointingHandCursor);
        calculateBtn->setStyleSheet(
            "QPushButton {"
            "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #8E2DE2, stop:1 #4A00E0);"
            "  color: white; border-radius: 6px; padding: 8px 16px;"
            "}"
            "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #9E3DF2, stop:1 #5A10F0); }"
        );
        connect(calculateBtn, &QPushButton::clicked, this, &InstallmentCalculator::calculateInstallments);

        inputLayout->addLayout(inputGrid);
        inputLayout->addWidget(calculateBtn, 0, Qt::AlignCenter);
        scrollLayout->addWidget(inputCard);

        // Results Section
        GradientCard *resultCard = new GradientCard();
        QVBoxLayout *resultLayout = new QVBoxLayout(resultCard);
        resultLayout->setContentsMargins(12, 12, 12, 12);
        resultLayout->setSpacing(8);

        QLabel *resultTitle = new QLabel("Installment Schedule");
        resultTitle->setStyleSheet("font-size: 14px; font-weight: bold;");
        resultLayout->addWidget(resultTitle);

        for(int i=0; i<3; i++) {
            GradientCard *card = new GradientCard();
            card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            QVBoxLayout *cardLayout = new QVBoxLayout(card);

            QHBoxLayout *header = new QHBoxLayout();
            QLabel *title = new QLabel(QString("Installment %1").arg(i+1));
            QLabel *percent = new QLabel(i==0 ? "40%" : "30%");

            installmentAmounts[i] = new QLabel("0 BDT");
            installmentAmounts[i]->setAlignment(Qt::AlignRight);

            header->addWidget(title);
            header->addStretch();
            header->addWidget(percent);

            cardLayout->addLayout(header);
            cardLayout->addWidget(installmentAmounts[i]);
            resultLayout->addWidget(card);
            resultCards.append(card);
        }

        // Total
        GradientCard *totalCard = new GradientCard();
        QHBoxLayout *totalLayout = new QHBoxLayout(totalCard);
        totalLayout->addWidget(new QLabel("Total:"));
        totalAmount = new QLabel("0 BDT");
        totalAmount->setAlignment(Qt::AlignRight);
        totalLayout->addStretch();
        totalLayout->addWidget(totalAmount);
        resultLayout->addWidget(totalCard);
        resultCards.append(totalCard);

        scrollLayout->addWidget(resultCard, 1);

        // Footer
        QHBoxLayout *footer = new QHBoxLayout();
        footer->setContentsMargins(0, 5, 0, 0);

        QLabel *watermark = new QLabel("Made by Raihan | UIU CSE 243");
        watermark->setStyleSheet("color: #9f9fb7; font-size: 11px;");

        QPushButton *github = new QPushButton("Github");
        github->setCursor(Qt::PointingHandCursor);
        github->setStyleSheet(
            "QPushButton {"
            "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #8E2DE2, stop:1 #4A00E0);"
            "  color: white; border-radius: 6px; padding: 4px 8px;"
            "  font-size: 11px;"
            "}"
            "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #9E3DF2, stop:1 #5A10F0); }"
        );
        connect(github, &QPushButton::clicked, []{ QDesktopServices::openUrl(QUrl("https://github.com/Enmilo-dev")); });

        footer->addWidget(watermark);
        footer->addStretch();
        footer->addWidget(github);
        scrollLayout->addLayout(footer);

        // Set up the scroll area
        scrollArea->setWidget(scrollContent);
        mainLayout->addWidget(scrollArea);

        setCentralWidget(centralWidget);
    }

    void applyDarkTheme() {
        QColor cardStart(45, 45, 65);
        QColor cardEnd(25, 25, 35);
        setStyleSheet(QString(
            "QMainWindow, QScrollArea { background: #1a1a2e; }"
            "QLabel { color: white; }"
            "QLineEdit {"
            "  background: %1; color: white; border: 1px solid #3d3d4a;"
            "  border-radius: 6px; padding: 6px; font-size: 14px;"
            "}"
            "QScrollBar:vertical {"
            "  border: none;"
            "  background: #1a1a2e;"
            "  width: 8px;"
            "  margin: 0px;"
            "}"
            "QScrollBar::handle:vertical {"
            "  background: #3d3d4a;"
            "  min-height: 20px;"
            "  border-radius: 4px;"
            "}"
            "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
            "  height: 0px;"
            "}"
        ).arg(cardStart.name()));
        updateCardColors(cardStart, cardEnd);
    }

    void applyMidnightBlueTheme() {
        QColor cardStart(15, 26, 43);
        QColor cardEnd(10, 16, 33);
        setStyleSheet(QString(
            "QMainWindow, QScrollArea { background: #0a0f1a; }"
            "QLabel { color: white; }"
            "QLineEdit {"
            "  background: %1; color: white; border: 1px solid #1a2a3a;"
            "  border-radius: 6px; padding: 6px; font-size: 14px;"
            "}"
            "QScrollBar:vertical {"
            "  border: none;"
            "  background: #0a0f1a;"
            "  width: 8px;"
            "  margin: 0px;"
            "}"
            "QScrollBar::handle:vertical {"
            "  background: #1a2a3a;"
            "  min-height: 20px;"
            "  border-radius: 4px;"
            "}"
            "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
            "  height: 0px;"
            "}"
        ).arg(cardStart.name()));
        updateCardColors(cardStart, cardEnd);
    }

    void updateCardColors(QColor start, QColor end) {
        for(GradientCard *card : resultCards) {
            card->setColors(start, end);
        }
    }

    void animateValue(QLabel *label, int value) {
        QPropertyAnimation *anim = new QPropertyAnimation(label, "number");
        anim->setDuration(500);
        anim->setEasingCurve(QEasingCurve::OutCubic);
        anim->setStartValue(label->text().replace(" BDT", "").replace(",", "").toInt());
        anim->setEndValue(value);
        connect(anim, &QPropertyAnimation::valueChanged, [label](const QVariant &val) {
            label->setText(QString("%L1 BDT").arg(val.toInt()));
        });
        anim->start(QPropertyAnimation::DeleteWhenStopped);
    }

    QLineEdit *amountInput;
    ModernComboBox *roundingComboBox;
    ModernComboBox *waiverComboBox;
    ModernComboBox *themeComboBox;
    QLabel *creditLabel;
    QLabel *feeLabel;
    QLabel *waiverLabel;
    QLabel *installmentAmounts[3];
    QLabel *totalAmount;
    QList<GradientCard*> resultCards;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setStyle("Fusion");

    app.setWindowIcon(QIcon(":/images/icon.png"));

    InstallmentCalculator calculator;
    calculator.show();

    return app.exec();
}

#include "main.moc"