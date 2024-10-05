#include <QWidget>
#include <QPainter>
#include <QTimer>
#include <QFontMetrics>

class PitchIndicator : public QWidget
{
    Q_OBJECT

public:
    explicit PitchIndicator(QWidget* parent = nullptr)
        : QWidget(parent), pitch(0)
    {

    }

    void setValue(int pitchValue)
    {
        pitch = pitchValue;
    }

protected:
    void paintEvent(QPaintEvent* event) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        int centerX = width() / 2;
        int centerY = height() / 2;

        painter.setPen(QPen(Qt::white, 2));
        // render 2 lines in the middle
        painter.drawLine(centerX - 100, centerY, centerX - 30, centerY);
        painter.drawLine(centerX + 100, centerY, centerX + 30, centerY);

        QFont font = painter.font();
        font.setPointSize(12);
        painter.setFont(font);

        QFontMetrics metrics(font);

        // render degrees between 180 and -180
        for (int i = -180; i <= 180; i += 10) {
            int offsetY = centerY + (i - pitch) * 5 + 5;
            QString text = QString::number(i);

            int textWidth = metrics.horizontalAdvance(text);

            if (offsetY > 0 && offsetY < height()) {
                painter.drawText(centerX - textWidth / 2, offsetY, text);
            }
        }
    }

private slots:
    // for testing
    void updatePitch()
    {
        pitch++;
        if (pitch > 180) {
            pitch = -180;
        }
        update();
    }

private:
    int pitch;
};
