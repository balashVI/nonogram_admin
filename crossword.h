#ifndef CROSSWORD_H
#define CROSSWORD_H

#include <QWidget>
#include <QPixmap>
#include <QPainter>
#include <QSizePolicy>
#include <QMouseEvent>
#include <QDebug>
#include <QVector>

class crossword : public QWidget
{
    Q_OBJECT
public:
    explicit crossword(QWidget *parent = 0);
    ~crossword();
    void setSize(short wCells, short hCells);
    void draw_layout();
    void set_cell_size(short size);
    QString get_crossword();
    int get_width();
    int get_height();
    void set_can_edit(bool state);

private:
    QPixmap *pixmap;
    unsigned  short w_cells, h_cells, cell_size, left_border, top_border, border;
    bool *matrix, changed;
    QVector <QVector <int> > v_numbers, h_numbers;//Числа v-по вертикалі, h-горизонталі
    unsigned short v_numbers_count, h_numbers_count;
    void calculate_numbers();
    bool active;
    bool can_edit;

protected:
    void paintEvent(QPaintEvent* e);
    void mousePressEvent(QMouseEvent *e);
    
signals:
    
public slots:
    void slot_clear();
};

#endif // CROSSWORD_H
