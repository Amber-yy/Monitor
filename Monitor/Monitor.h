#pragma once

#include <QWidget>

class Monitor : public QWidget
{
	Q_OBJECT
public:
	Monitor(QWidget *parent = nullptr);
	~Monitor();
protected:
	void mousePressEvent(QMouseEvent *e)override;
	void mouseMoveEvent(QMouseEvent *e)override;
	void mouseReleaseEvent(QMouseEvent *e)override;
	void paintEvent(QPaintEvent *e)override;
	void refresh();
protected:
	struct Data;
	Data *data;
};
