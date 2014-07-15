#include "../lib/bb_lib.h"
#include "entry_widgets.h"

#include <QLabel>
#include <QComboBox>
#include <QColorDialog>
#include <QFileDialog>

LineEntry::LineEntry(EntryType type, QWidget *parent)
    : entry_type(type), QLineEdit(parent)
{
    setObjectName("SH_LineEntry");
    setAlignment(Qt::AlignRight);

    if(entry_type == FREQ_ENTRY) {
        setToolTip(tr("e.g. 20MHz = 20m or 20M or 20 mhz"));
    }

    connect(this, SIGNAL(editingFinished()),
            this, SLOT(editChanged()));
}

LineEntry::~LineEntry()
{

}

/*
 * Set the text and update the class variable
 *  Do nothing if the values did not change
 *   Much faster to compare variables than to update the
 *    widget.
 * Returns false if the frequency is not updated
 */
bool LineEntry::SetFrequency(Frequency freq)
{
    frequency = freq;
    setText(freq.GetFreqString());

    return true;
}

bool LineEntry::SetAmplitude(Amplitude amp)
{
    amplitude = amp;
    setText(amp.GetString());
    return true;
}

bool LineEntry::SetTime(Time t)
{
    time = t;
    setText(time.GetString());
    return true;
}

bool LineEntry::SetValue(double val)
{
    value = val;
    QString text;
    text.sprintf("%.3f", value);
    setText(text);
    return true;
}

bool LineEntry::SetValue(double val, QString &units)
{
    value = val;
    QString text;
    text.sprintf("%.6f ", value);
    text += units;
    setText(text);
    return true;
}

void LineEntry::editChanged()
{
    if(entry_type == FREQ_ENTRY) {
        Frequency f = frequency;
        // If not valid frequency, reset entry
        bool valid_freq = Frequency::IsValidFreqString(text(), f);
        if(!valid_freq || f == frequency) {
            SetFrequency(frequency);
            return;
        }
        // Set new frequency, emit signal if different
        SetFrequency(f);
        emit entryUpdated();
    } else if(entry_type == AMP_ENTRY) {

    } else if(entry_type == TIME_ENTRY) {

    } else if(entry_type == VALUE_ENTRY) {
        bool valid;
        double d = text().toDouble(&valid);
        if(!valid || d == value) {
            SetValue(value);
            return;
        }
        // Set new value, emit signal
        SetValue(d);
        emit entryUpdated();
    }
}

FrequencyEntry::FrequencyEntry(const QString &label_text,
                               Frequency f,
                               QWidget *parent)
    : QWidget(parent), freq(f)
{
    move(0, 0);
    resize(ENTRY_WIDTH, ENTRY_H);

    freq = f;

    label = new Label(label_text, this);
    label->move(ENTRY_OFFSET, 0);
    label->resize(QSize(LABEL_W, ENTRY_H));

    entry = new LineEntry(FREQ_ENTRY, this);
    entry->move(QPoint(ENTRY_OFFSET + LABEL_W, 0));
    entry->resize(QSize(ENTRY_WIDTH - LABEL_W - ENTRY_OFFSET, ENTRY_H));
    entry->SetFrequency(freq);

//    connect(entry, SIGNAL(editingFinished()),
//            SLOT(editUpdated()));
    connect(entry, SIGNAL(entryUpdated()),
            this, SLOT(editUpdated()));
}

FrequencyEntry::~FrequencyEntry()
{
    delete label;
    delete entry;
}

void FrequencyEntry::SetFrequency(Frequency &f)
{
    if(f == freq) {
        return;
    }

    freq = f;
    entry->SetFrequency(freq);
}

void FrequencyEntry::editUpdated()
{
    freq = entry->GetFrequency();
    emit freqViewChanged(freq);
}

/*
 * Amplitude Panel Entry widget, no shifts
 */
AmpEntry::AmpEntry(const QString &label_text,
                   Amplitude a, QWidget *parent)
    : QWidget(parent)
{
    move(0, 0);
    resize(ENTRY_WIDTH, ENTRY_H);

    amplitude = a;

    label = new Label(label_text, this);
    label->move(ENTRY_OFFSET, 0);
    label->resize(LABEL_W, ENTRY_H);

    entry = new LineEntry(VALUE_ENTRY, this);
    entry->move(ENTRY_OFFSET + LABEL_W, 0);
    entry->resize(ENTRY_WIDTH - LABEL_W - ENTRY_OFFSET - 60, ENTRY_H);

    units = new ComboBox(this);
    units->move(ENTRY_OFFSET + label->width() + entry->width(), 0);
    units->resize(60, ENTRY_H);

    QStringList unit_list;
    unit_list << "dBm" << "dBmV" << "dBuV" << "mV";
    units->insertItems(0, unit_list);
    last_unit_index = 0;

    SetAmplitude(amplitude);

    connect(entry, SIGNAL(entryUpdated()), this, SLOT(editUpdated()));
    connect(units, SIGNAL(activated(int)), this, SLOT(unitsUpdated(int)));
}

AmpEntry::~AmpEntry()
{

}

void AmpEntry::SetAmplitude(Amplitude a)
{
    amplitude = a;
    entry->SetValue(amplitude.Val());
    units->setCurrentIndex(amplitude.Units());
}

void AmpEntry::editUpdated()
{
    emit amplitudeChanged(Amplitude(entry->GetValue(), (AmpUnits)units->currentIndex()));
}

/*
 * When the units change, update the associated value to
 *   the new unit type
 */
void AmpEntry::unitsUpdated(int)
{
    if(units->currentIndex() != last_unit_index) {
        entry->SetValue(unit_convert(entry->GetValue(), (AmpUnits)last_unit_index,
                                     (AmpUnits)units->currentIndex()));
        last_unit_index = units->currentIndex();
    }
    emit amplitudeChanged(Amplitude(entry->GetValue(), (AmpUnits)units->currentIndex()));
}

/*
 * Amplitude Panel Entry widget
 */
AmplitudeEntry::AmplitudeEntry(const QString &label_text,
                               Amplitude a, QWidget *parent)
    : QWidget(parent)
{
    move(0, 0);
    resize(ENTRY_WIDTH, ENTRY_H);

    amplitude = a;

    label = new Label(label_text, this);
    label->move(ENTRY_OFFSET, 0);
    label->resize(LABEL_W, ENTRY_H);

    up_btn = new QPushButton(this); //QIcon(":icons/plus.png"), "", this);
    up_btn->move(ENTRY_OFFSET + LABEL_W, 0);
    up_btn->resize(25, ENTRY_H);
    up_btn->setObjectName("Increment");

    down_btn = new QPushButton(this); //QIcon(":icons/minus.png"), "", this);
    down_btn->move(ENTRY_OFFSET + LABEL_W + 25, 0);
    down_btn->resize(25, ENTRY_H);
    down_btn->setObjectName("Decrement");

    entry = new LineEntry(VALUE_ENTRY, this);
    entry->move(ENTRY_OFFSET + LABEL_W + 50, 0);
    entry->resize(ENTRY_WIDTH - LABEL_W - ENTRY_OFFSET - 60 - 50, ENTRY_H);

    units = new ComboBox(this);
    units->move(ENTRY_OFFSET + label->width() + entry->width() + 50, 0);
    units->resize(60, ENTRY_H);

    QStringList unit_list;
    unit_list << "dBm" << "dBmV" << "dBuV" << "mV";
    units->insertItems(0, unit_list);
    last_unit_index = 0;

    SetAmplitude(amplitude);

    connect(up_btn, SIGNAL(clicked()), this, SLOT(clickedUp()));
    connect(down_btn, SIGNAL(clicked()), this, SLOT(clickedDown()));
    connect(entry, SIGNAL(entryUpdated()), this, SLOT(editUpdated()));
    connect(units, SIGNAL(activated(int)), this, SLOT(unitsUpdated(int)));
}

AmplitudeEntry::~AmplitudeEntry()
{

}

void AmplitudeEntry::SetAmplitude(Amplitude a)
{
    amplitude = a;
    entry->SetValue(amplitude.Val());
    units->setCurrentIndex(amplitude.Units());
}

void AmplitudeEntry::editUpdated()
{
    emit amplitudeChanged(Amplitude(entry->GetValue(), (AmpUnits)units->currentIndex()));
}

/*
 * When the units change, update the associated value to
 *   the new unit type
 */
void AmplitudeEntry::unitsUpdated(int)
{
    if(units->currentIndex() != last_unit_index) {
        entry->SetValue(unit_convert(entry->GetValue(), (AmpUnits)last_unit_index,
                                     (AmpUnits)units->currentIndex()));
        last_unit_index = units->currentIndex();
    }
    emit amplitudeChanged(Amplitude(entry->GetValue(), (AmpUnits)units->currentIndex()));
}

/*
 * Time Entry Line Widget
 */
TimeEntry::TimeEntry(const QString &label_text, Time t,
                     TimeUnit tu, QWidget *parent)
    : QWidget(parent)
{
    time = t;
    units = tu;

    move(0, 0);
    resize(ENTRY_WIDTH, ENTRY_H);

    label = new Label(label_text, this);
    label->move(ENTRY_OFFSET, 0);
    label->resize(LABEL_W, ENTRY_H);

    entry = new LineEntry(VALUE_ENTRY, this);
    entry->move(ENTRY_OFFSET + label->width(), 0);
    entry->resize(ENTRY_WIDTH - LABEL_W - ENTRY_OFFSET - 30, ENTRY_H);

    units_label = new Label(g_time_unit_map[(int)tu].unit_str, this);
    units_label->move(ENTRY_OFFSET + label->width() + entry->width() + 3, 0);
    units_label->resize(30, ENTRY_H);

    connect(entry, SIGNAL(entryUpdated()),
            this, SLOT(entryChanged()));
}

TimeEntry::~TimeEntry()
{

}

void TimeEntry::SetTime(Time t)
{
    time = t;
    entry->SetValue(time.ChangeUnit(units));
}

void TimeEntry::entryChanged()
{
    time = Time(entry->GetValue(), units);
    emit timeChanged(time);
}

///
// Generic entry widget
//
NumericEntry::NumericEntry(const QString &label_text,
                           double starting_value,
                           const QString &units_text,
                           QWidget *parent) :
    QWidget(parent)

{
    value = starting_value;

    move(0, 0);
    resize(ENTRY_WIDTH, ENTRY_H);

    label = new Label(label_text, this);
    label->move(ENTRY_OFFSET, 0);
    label->resize(LABEL_W, ENTRY_H);

    entry = new LineEntry(VALUE_ENTRY, this);
    entry->move(ENTRY_OFFSET + label->width(), 0);
    entry->resize(ENTRY_WIDTH - LABEL_W - ENTRY_OFFSET - 30, ENTRY_H);

    units_label = new Label(units_text, this);
    if(units_text.length() == 0) {
        units_label->resize(0, ENTRY_H);
    } else {
        units_label->resize(30, ENTRY_H);
    }
    units_label->move(ENTRY_OFFSET + label->width() + entry->width() + 3, 0);
    units_label->setAlignment(Qt::AlignCenter);

    entry->SetValue(value);

    connect(entry, SIGNAL(entryUpdated()),
            this, SLOT(entryChanged()));
}

NumericEntry::~NumericEntry()
{

}

void NumericEntry::resizeEvent(QResizeEvent *)
{
    // Units label does not resize just moves

    units_label->move(width() - 30, 0);


    entry->move(width() - units_label->width() - 180, 0);
    label->resize(entry->x(), ENTRY_H);
}

void NumericEntry::entryChanged()
{
    value = entry->GetValue();
    emit valueChanged(value);
}

/*
 * Frequency Shift Entry Widget
 */
FreqShiftEntry::FreqShiftEntry(const QString &label_text,
                               Frequency f,
                               QWidget *parent)
    : QWidget(parent)
{
    move(0, 0);
    resize(ENTRY_WIDTH, ENTRY_H);

    freq = f;

    label = new Label(label_text, this);
    label->move(ENTRY_OFFSET, 0);
    label->resize(LABEL_W, ENTRY_H);

    up_btn = new QPushButton(this); //QIcon(":icons/plus.png"), "", this);
    up_btn->move(ENTRY_OFFSET + LABEL_W, 0);
    up_btn->resize(25, ENTRY_H);
    up_btn->setObjectName("Increment");

    down_btn = new QPushButton(this); //QIcon(":icons/minus.png"), "", this);
    down_btn->move(ENTRY_OFFSET + LABEL_W + 25, 0);
    down_btn->resize(25, ENTRY_H);
    down_btn->setObjectName("Decrement");

    entry = new LineEntry(FREQ_ENTRY, this);
    entry->move(ENTRY_OFFSET + LABEL_W + 50, 0);
    entry->resize(ENTRY_WIDTH - LABEL_W - 50 - ENTRY_OFFSET, ENTRY_H);
    entry->SetFrequency(freq);

    connect(up_btn, SIGNAL(clicked()), this, SLOT(clickedUp()));
    connect(down_btn, SIGNAL(clicked()), this, SLOT(clickedDown()));
    //connect(entry, SIGNAL(editingFinished()),
    //        this, SLOT(editUpdated()));
    connect(entry, SIGNAL(entryUpdated()),
            this, SLOT(editUpdated()));

}

FreqShiftEntry::~FreqShiftEntry()
{
    delete label;
    delete up_btn;
    delete down_btn;
    delete entry;
}

void FreqShiftEntry::SetFrequency(Frequency f)
{
    if(f == freq) {
        return;
    }

    freq = f;
    entry->SetFrequency(freq);
}

void FreqShiftEntry::editUpdated()
{
    freq = entry->GetFrequency();
    emit freqViewChanged(freq);
}

/*
 * Combo entry
 */
ComboEntry::ComboEntry(const QString &label_text, QWidget *parent)
    : QWidget(parent)
{
    move(0, 0);
    resize(ENTRY_WIDTH, ENTRY_H);

    label = new Label(label_text, this);
    label->move(ENTRY_OFFSET, 0);
    label->resize(LABEL_W, ENTRY_H);

    combo_box = new ComboBox(this);
    combo_box->move(ENTRY_OFFSET + LABEL_W, 0);
    combo_box->resize(ENTRY_WIDTH - LABEL_W - ENTRY_OFFSET, ENTRY_H);

    connect(combo_box, SIGNAL(activated(int)),
            this, SIGNAL(comboIndexChanged(int)));
}

ComboEntry::~ComboEntry()
{
    delete label;
    delete combo_box;
}

void ComboEntry::setComboIndex(int ix)
{
    combo_box->setCurrentIndex(ix);
}

void ComboEntry::setComboText(const QStringList &list)
{
    combo_box->clear();
    combo_box->insertItems(0, list);
}

/*
 * Standalone color button
 */
ColorButton::ColorButton(QWidget *parent)
    : QPushButton(parent)
{
    setObjectName("SH_ColorButton");
    color = QColor(0, 0, 0);

    connect(this, SIGNAL(clicked(bool)),
            this, SLOT(onClick(bool)));
}

void ColorButton::SetColor(QColor c)
{
    color = c;

    QString styleColor;
    styleColor.sprintf("background: rgb(%d,%d,%d);",
                       color.red(), color.green(), color.blue());

    setStyleSheet(styleColor);
}

void ColorButton::onClick(bool)
{
    QColor newColor = QColorDialog::getColor(color);

    if(!newColor.isValid())
        return;

    SetColor(newColor);
    emit colorChanged(color);
}

/*
 * Line Entry widget for Color Button
 */
ColorEntry::ColorEntry(const QString &label_text, QWidget *parent)
    : QWidget(parent)
{
    move(0, 0);
    resize(ENTRY_WIDTH, ENTRY_H);

    label = new Label(label_text, this);
    label->move(ENTRY_OFFSET, 0);
    label->resize(LABEL_W, ENTRY_H);

    color_button = new ColorButton(this);
    color_button->move(ENTRY_OFFSET + LABEL_W, 0);
    color_button->resize(ENTRY_WIDTH - LABEL_W - ENTRY_OFFSET, ENTRY_H);

    connect(color_button, SIGNAL(colorChanged(QColor&)),
            this, SIGNAL(colorChanged(QColor&)));
}

ColorEntry::~ColorEntry()
{
    delete label;
    delete color_button;
}

void ColorEntry::resizeEvent(QResizeEvent *)
{
    // Entry widget moves to end of length
    // Label resizes to fill space up to widget
    int minY = LABEL_W + ENTRY_OFFSET;

    color_button->move(qMax<int>(minY, width() - 210), 0);
    label->resize(width() - (color_button->width() + ENTRY_OFFSET), ENTRY_H);
}

/*
 * Line Entry for check box
 */
CheckBoxEntry::CheckBoxEntry(const QString &label_text, QWidget *parent)
    : QWidget(parent)
{
    move(0, 0);
    resize(ENTRY_WIDTH, ENTRY_H);

    label = new Label(label_text, this);
    label->move(ENTRY_OFFSET, 0);
    label->resize(100, ENTRY_H);

    check_box = new QCheckBox(this);
    check_box->setObjectName("SH_CheckBox");
    check_box->setLayoutDirection(Qt::RightToLeft);
    check_box->move(ENTRY_OFFSET + LABEL_W, 0);
    check_box->resize(ENTRY_WIDTH - LABEL_W - ENTRY_OFFSET, ENTRY_H);

    connect(check_box, SIGNAL(clicked(bool)),
            this, SIGNAL(clicked(bool)));
}

CheckBoxEntry::~CheckBoxEntry()
{
    delete check_box;
}

void CheckBoxEntry::resizeEvent(QResizeEvent *)
{
    check_box->resize(width() - LABEL_W - ENTRY_OFFSET, ENTRY_H);
}

/*
 * Dual side-by-side check boxes
 */
DualCheckBox::DualCheckBox(const QString &left_text,
                           const QString &right_text,
                           QWidget *parent) :
    QWidget(parent),
    left(new CheckBoxEntry(left_text, this)),
    right(new CheckBoxEntry(right_text, this))
{
    move(0, 0);
    resize(ENTRY_WIDTH, ENTRY_H);

    left->move(0, 0);
    left->resize(ENTRY_WIDTH / 2, ENTRY_H);
    right->move(ENTRY_WIDTH / 2, 0);
    right->resize(ENTRY_WIDTH / 2, ENTRY_H);

//    left = new QCheckBox(left_text, this);
//    left->setObjectName("SH_CheckBox");
//    left->move(ENTRY_OFFSET, 0);
//    left->resize((ENTRY_WIDTH - ENTRY_OFFSET)/2, ENTRY_H);

//    right = new QCheckBox(right_text, this);
//    right->setObjectName("SH_CheckBox");
//    right->move(ENTRY_OFFSET + (ENTRY_WIDTH - ENTRY_OFFSET)/2, 0);
//    right->resize((ENTRY_WIDTH - ENTRY_OFFSET)/2, ENTRY_H);

    connect(left, SIGNAL(clicked(bool)),
            this, SIGNAL(leftClicked(bool)));
    connect(right, SIGNAL(clicked(bool)),
            this, SIGNAL(rightClicked(bool)));
}

DualCheckBox::~DualCheckBox()
{
    delete left;
    delete right;
}

/*
 * Dual Button Line Entry
 */
DualButtonEntry::DualButtonEntry(const QString &left_button_title,
                                 const QString &right_button_title,
                                 QWidget *parent)
    : QWidget(parent)
{
    move(0, 0);
    resize(ENTRY_WIDTH, ENTRY_H);

    left_button = new QPushButton(left_button_title, this);
    left_button->setObjectName("BBPushButton");
    left_button->move(ENTRY_OFFSET, 0);
    left_button->resize((ENTRY_WIDTH - ENTRY_OFFSET)/2, ENTRY_H);

    right_button = new QPushButton(right_button_title, this);
    right_button->setObjectName("BBPushButton");
    right_button->move(ENTRY_OFFSET + (ENTRY_WIDTH - ENTRY_OFFSET)/2, 0);
    right_button->resize((ENTRY_WIDTH - ENTRY_OFFSET)/2, ENTRY_H);

    connect(left_button, SIGNAL(clicked()),
            this, SIGNAL(leftPressed()));
    connect(right_button, SIGNAL(clicked()),
            this, SIGNAL(rightPressed()));
}

DualButtonEntry::~DualButtonEntry()
{
    delete left_button;
    delete right_button;
}

