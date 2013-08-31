#include "crossword.h"

crossword::crossword(QWidget *parent) :
    QWidget(parent)
{
    cell_size = 20;
    border = 15;
    pixmap = NULL;
    matrix = NULL;
    active = false;
    can_edit = true;
}

void crossword::paintEvent(QPaintEvent *e){
    if(pixmap!=NULL){
        QPainter p(this);
        p.drawPixmap(0,0,width(),height(), *pixmap);
        p.end();
    }
}

void crossword::setSize(short wCells, short hCells){
    active=true;
    v_numbers.clear();
    h_numbers.clear();
    v_numbers_count=0;
    h_numbers_count=0;
    w_cells=wCells;
    h_cells=hCells;
    delete []matrix;
    matrix = new bool[w_cells*h_cells];
    for(int i=0;i<w_cells*h_cells;i++) matrix[i]=false;
    draw_layout();
    repaint();
}

void crossword::draw_layout(){
    left_border = border + cell_size*v_numbers_count+1;
    top_border = border + cell_size*h_numbers_count+1;
    int w = w_cells*cell_size+left_border + border+2;
    int h = h_cells*cell_size+top_border + border+2;
    this->setFixedSize(w, h);
    delete pixmap;
    pixmap = new QPixmap(width(), height());
    pixmap->fill(Qt::white);
    QPainter p(pixmap);
    p.setPen(QPen(Qt::black, 2));
    p.drawRect(1,1,width()-2,height()-2);

    p.setPen(QPen(Qt::lightGray, 1));
    int i, j, a, b=h_cells*cell_size;
    for(i=1;i<v_numbers_count;i++){
        a=border+cell_size*i;
        p.drawLine(a, top_border, a, b+top_border);
    }
    b=w_cells*cell_size;
    for(i=1;i<h_numbers_count;i++){
        a=border+cell_size*i;
        p.drawLine(left_border, a, b+left_border, a);
    }
    b=h_cells*cell_size;
    for(i=1;i<w_cells;i++){
        a=left_border+cell_size*i;
        p.drawLine(a, border, a, b+top_border);
    }
    b=w_cells*cell_size;
    for(i=1;i<h_cells;i++){
        a=top_border+cell_size*i;
        p.drawLine(border, a, b+left_border, a);
    }
    p.setPen(QPen(Qt::darkGray, 1));
    b=h_cells*cell_size;
    for(i=5;i<w_cells;i+=5){
        a=left_border+cell_size*i;
        p.drawLine(a, border, a, b+top_border);
    }
    b=w_cells*cell_size;
    for(i=5;i<h_cells;i+=5){
        a=top_border+cell_size*i;
        p.drawLine(border, a, b+left_border, a);
    }
    p.setPen(QPen(Qt::darkGray, 2));
    p.drawLine(left_border, border+1, left_border, top_border+cell_size*h_cells);
    p.drawLine(left_border+cell_size*w_cells+1, border+1, left_border+cell_size*w_cells+1, top_border+cell_size*h_cells);
    p.drawLine(border+1, top_border, left_border+cell_size*w_cells, top_border);
    p.drawLine(border+1, top_border+cell_size*h_cells+1, left_border+cell_size*w_cells+1, top_border+cell_size*h_cells+1);
    if(v_numbers_count){
        p.drawLine(left_border, border, left_border+cell_size*w_cells+1, border);
        p.drawLine(border, top_border, border, top_border+cell_size*h_cells+1);

        p.setPen(QPen(Qt::black, 1));
        p.setFont(QFont("Times", cell_size/2+2, QFont::Bold, true));
        for(i=0;i<h_cells;i++)
            for(j=0;j<v_numbers[i].size();j++)
                p.drawText(left_border-cell_size*(j+1)+2, top_border+cell_size*(i+1)-cell_size/5, QString::number(v_numbers[i][j]));
        for(i=0;i<w_cells;i++)
            for(j=0;j<h_numbers[i].size();j++)
                p.drawText(left_border+cell_size*i+2, top_border-cell_size*j-cell_size/5, QString::number(h_numbers[i][j]));
    }

    p.setPen(Qt::gray);
    p.setBrush(Qt::gray);
    for(int i=0;i<h_cells*w_cells;i++)
        if(matrix[i]){
            int x, y;
            x = (i - (i/w_cells)*w_cells)*cell_size+left_border+1;
            y = i/w_cells*cell_size+top_border+1;
            p.drawRect(x, y, cell_size-2, cell_size-2);
        }

    p.end();
}

QString crossword::get_crossword()
{
    if(matrix!=NULL){
        QString result = "";
        for(int i=0;i<w_cells*h_cells;i++)result+=QString::number(matrix[i]);
        return result;
    }
    return "";
}

int crossword::get_width()
{
    return w_cells;
}

int crossword::get_height()
{
    return h_cells;
}

void crossword::set_can_edit(bool state)
{
    can_edit = state;
}

void crossword::clear()
{
    active = false;
    delete pixmap;
    pixmap = new QPixmap(0, 0);
}

void crossword::setCrossword(short wCells, short hCells, QString cr)
{
    active=true;
    v_numbers.clear();
    h_numbers.clear();
    v_numbers_count=0;
    h_numbers_count=0;
    w_cells=wCells;
    h_cells=hCells;
    delete matrix;
    matrix = new bool[w_cells*h_cells];
    for(int i=0;i<w_cells*h_cells;i++) matrix[i]= (cr.at(i) == '0') ? false : true;
    calculate_numbers();
}

void crossword::mousePressEvent(QMouseEvent *e){
    if(can_edit){
        if(e->x()-left_border<0 || e->y()-top_border<0) return;
        int x_cell = (e->x()-left_border)/cell_size;
        int y_cell = (e->y()-top_border)/cell_size;
        if(x_cell>=w_cells || y_cell>=h_cells) return;
        if(matrix[w_cells*y_cell + x_cell]) matrix[w_cells*y_cell + x_cell]=false;
        else matrix[w_cells*y_cell + x_cell]=true;
        calculate_numbers();
    }
}

void crossword::calculate_numbers()
{
    v_numbers.clear();
    h_numbers.clear();
    v_numbers.resize(h_cells);
    h_numbers.resize(w_cells);
    int count=0, i, j;
    //Обчислення вертикальних чисел
    for(i=0;i<h_cells;i++){
        for(j=0;j<w_cells;j++)
            if(matrix[i*w_cells+j])count++;
            else if(count){
                v_numbers[i].push_front(count);
                count = 0;
            }
        if(count){
            v_numbers[i].push_front(count);
            count = 0;
        }
    }

    //Обчислення горизонатальних чисел
    for(i=0;i<w_cells;i++){
        for(j=0;j<h_cells;j++)
            if(matrix[j*w_cells+i])count++;
            else if(count){
                h_numbers[i].push_front(count);
                count = 0;
            }
        if(count){
            h_numbers[i].push_front(count);
            count = 0;
        }
    }

    //Обчислення максимальної кількості чисел
    v_numbers_count=v_numbers[0].size();
    for(i=1;i<h_cells;i++)
        if(v_numbers[i].size()>v_numbers_count)v_numbers_count=v_numbers[i].size();
    h_numbers_count=h_numbers[0].size();
    for(i=1;i<w_cells;i++)
        if(h_numbers[i].size()>h_numbers_count)h_numbers_count=h_numbers[i].size();

    draw_layout();
    repaint();
}

void crossword::slot_clear()
{
    setSize(w_cells, h_cells);
}

void crossword::slot_cell_size(int size)
{
    cell_size=size;
    if(active){
        draw_layout();
    }
}

crossword::~crossword(){
    delete []matrix;
    delete pixmap;
}
