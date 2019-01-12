#include "Monitor.h"

#include <QTimer>
#include <QMouseEvent>
#include <QLabel>
#include <QPainter>
#include <QDebug>

#include <time.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#pragma comment(lib, "IPHLPAPI")

struct Monitor::Data
{
	~Data()
	{
		free(table);
	}
	bool isPress;
	bool isTop;
	DWORD in;
	DWORD out;
	DWORD size;
	FILETIME preidleTime = { 0,0 };
	FILETIME prekernelTime = { 0,0 };
	FILETIME preuserTime = { 0,0 };
	int index;
	int width;
	int height;
	clock_t lastclick;
	QPoint last;
	QPixmap img;
	QTimer *timer;
	QLabel *us;
	QLabel *ds;
	QLabel *ms;
	QLabel *cs;
	MIB_IFTABLE *table;
};

__int64 CompareFileTime2(FILETIME time1, FILETIME time2)
{
	__int64 a = static_cast<__int64>(time1.dwHighDateTime) << 32 | time1.dwLowDateTime;
	__int64 b = static_cast<__int64>(time2.dwHighDateTime) << 32 | time2.dwLowDateTime;
	return b - a;
}

Monitor::Monitor(QWidget *parent):QWidget(parent)
{
	data = new Data;
	data->isPress = false;
	data->isTop = true;
	data->lastclick = 0;
	data->in = data->out = 0;
	data->width = GetSystemMetrics(SM_CXSCREEN);
	data->height = GetSystemMetrics(SM_CYSCREEN);
	data->table = (MIB_IFTABLE *)malloc(sizeof(MIB_IFTABLE));
	data->size = sizeof(MIB_IFTABLE);
	if (GetIfTable(data->table, &data->size, false) == ERROR_INSUFFICIENT_BUFFER)
	{
		free(data->table);
		data->table = (MIB_IFTABLE *)malloc(data->size);
	}

	data->us = new QLabel(this);
	data->ds = new QLabel(this);
	data->ms = new QLabel(this);
	data->cs = new QLabel(this);

	setAutoFillBackground(false);
	setAttribute(Qt::WA_TranslucentBackground, true);
	QFont font;
	font.setFamily(u8"微软雅黑");

	if (data->width >= 2560)
	{
		data->us->setGeometry(110, 22, 105, 25);
		data->ds->setGeometry(110, 42, 105, 25);
		data->ms->setGeometry(15, 30, 105, 25);
		data->cs->setGeometry(185, 41, 105, 25);
		setMaximumSize(226, 88);
		setMinimumSize(226, 88);
		font.setPixelSize(30);
	}
	else if (data->width >= 1920)
	{
		QFont f;
		f.setPixelSize(12 / 1.33);
		data->us->setFont(f);
		data->us->setGeometry(110 / 1.33, 22 / 1.33, 105 / .133, 25 / 1.33);
		data->ds->setFont(f);
		data->ds->setGeometry(110 / 1.33, 42 / 1.33, 105 / 1.33, 25 / 1.33);
		data->ms->setGeometry(15 / 1.33, 30 / 1.33, 105 / 1.33, 25 / 1.33);
		data->cs->setFont(f);
		data->cs->setGeometry(185 / 1.33, 42 / 1.33, 105 / 1.33, 25 / 1.33);
		setMaximumSize(226 / 1.33, 88 / 1.33);
		setMinimumSize(226 / 1.33, 88 / 1.33);
		font.setPixelSize(30 / 1.33);
	}
	else
	{
		QFont f;
		f.setPixelSize(12 / 1.87);
		data->us->setFont(f);
		data->us->setGeometry(110 / 1.87, 22 / 1.87, 105 / .133, 25 / 1.87);
		data->ds->setFont(f);
		data->ds->setGeometry(110 / 1.87, 42 / 1.87, 105 / 1.87, 25 / 1.87);
		data->ms->setGeometry(15 / 1.87, 30 / 1.87, 105 / 1.87, 25 / 1.87);
		data->cs->setFont(f);
		data->cs->setGeometry(185 / 1.87, 42 / 1.87, 105 / 1.87, 25 / 1.87);
		setMaximumSize(226 / 1.87, 88 / 1.87);
		setMinimumSize(226 / 1.87, 88 / 1.87);
		font.setPixelSize(30 / 1.87);
	}

	data->ms->setFont(font);
	data->ms->setStyleSheet("color:white");
	data->img.load("mask.png");

	data->timer = new QTimer(this);
	connect(data->timer, &QTimer::timeout, this, &Monitor::refresh);
	setWindowFlags(Qt::Tool|Qt::WindowStaysOnTopHint|Qt::FramelessWindowHint);
	data->timer->start(1000);
}

Monitor::~Monitor()
{
	delete data;
}

void Monitor::mousePressEvent(QMouseEvent * e)
{
	if (e->button() == Qt::LeftButton)
	{
		clock_t cur = clock();
		if (cur - data->lastclick < 500)
		{
			data->isTop? setWindowFlags(Qt::Tool | Qt::FramelessWindowHint): setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
			data->isTop = !data->isTop;
			show();
		}
		data->lastclick = cur;
	}

	data->isPress = true;
	data->last = e->globalPos();
}

void Monitor::mouseMoveEvent(QMouseEvent * e)
{
	if (!data->isPress)
	{
		return;
	}

	QPoint pt = e->globalPos();
	int dx = pt.x() - data->last.x();
	int dy = pt.y() - data->last.y();

	move(x() + dx, y() + dy);

	data->last = pt;
}

void Monitor::mouseReleaseEvent(QMouseEvent * e)
{
	data->isPress = false;

	int cx = x();
	int cy = y();
	int cw = width();
	int ch = height();
	bool flag = false;

	if (cx < 0)
	{
		cx = 0;
		flag = true;
	}
	else if (cx + cw > data->width)
	{
		cx = data->width - cw;
		flag = true;
	}

	if (cy < 0)
	{
		cy = 0;
		flag = true;
	}
	else if (cy + ch > data->height)
	{
		cy = data->height- ch;
		flag = true;
	}

	if (flag)
	{
		data->last = QPoint(cx, cy);
		move(cx, cy);
	}
}

void Monitor::paintEvent(QPaintEvent * e)
{
	QPainter painter(this);
	painter.setRenderHints(QPainter::SmoothPixmapTransform);
	painter.drawPixmap(rect(),data->img);
}

void Monitor::refresh()
{
	GetIfTable(data->table,&data->size, false);
	if (data->in == 0&&data->out==0)
	{
		for (int i = 0; i < data->table->dwNumEntries; ++i)
		{
			MIB_IFROW *row = (MIB_IFROW *)&data->table->table[i];
			if (row->dwInOctets > data->in)
			{
				data->in = row->dwInOctets;
				data->out = row->dwOutOctets;
				data->index = i;
			}
		}
	}
	else
	{
		/*网速*/
		int r1 = data->table->table[data->index].dwInOctets-data->in;//下载速率
		int r2 = data->table->table[data->index].dwOutOctets - data->out;//上载速率
		data->in = data->table->table[data->index].dwInOctets;
		data->out = data->table->table[data->index].dwOutOctets;

		/*内存*/
		MEMORYSTATUSEX memsStat;
		memsStat.dwLength = sizeof(memsStat);
		DWORDLONG bRet = GlobalMemoryStatusEx(&memsStat);
		DWORDLONG nMemFree = memsStat.ullAvailPhys;
		DWORDLONG nMemTotal = memsStat.ullTotalPhys;
		DWORDLONG  MemeoryRate = (nMemTotal - nMemFree) * 100 / nMemTotal;//内存利用率

		/*CPU*/
		FILETIME idleTime;
		FILETIME kernelTime;
		FILETIME userTime;

		GetSystemTimes(&idleTime, &kernelTime, &userTime);

		__int64 idle = CompareFileTime2( data->preidleTime, idleTime);
		__int64 kernel = CompareFileTime2(data->prekernelTime, kernelTime);
		__int64 user = CompareFileTime2(data->preuserTime, userTime);
		int  cpu_usage = 0;//CPU利用率
		if (kernel + user == 0)
		{
			 cpu_usage = 0;
		}
		else
		{
			 cpu_usage = static_cast<int>(abs((kernel + user - idle) * 100 / (kernel + user)));
		}
		data-> preidleTime = idleTime;
		data-> prekernelTime = kernelTime;
		data-> preuserTime = userTime;

		QString ds=u8"",us = u8"",ms = u8"",cs = u8"";
		if (r1 < 1024)
		{
			ds += QString::number(r1) + "B/s";
		}
		else if (r1 < 1024 * 1024)
		{
			ds += QString::number(r1/1024) + "K/s";
		}
		else
		{
			char buffer[128];
			sprintf(buffer,"%.2f", (double)r1 / (double)(1024 * 1024));
			char *p = buffer + strlen(buffer) - 1;
			while (p > buffer && *p == '0')
				--p;
			if (*p != '.')
				++p;
			*p = '\0';
			ds += QString(buffer) + "M/s";
		}

		if (r2 < 1024)
		{
			us += QString::number(r2) + "B/s";
		}
		else if (r2 < 1024 * 1024)
		{
			us += QString::number(r2 / 1024) + "K/s";
		}
		else
		{
			char buffer[128];
			sprintf(buffer, "%.2f", (double)r2 / (double)(1024 * 1024));
			char *p = buffer + strlen(buffer) -1;
			while (p > buffer && *p == '0')
				--p;
			if (*p != '.')
				++p;
			*p = '\0';
			us += QString(buffer) + "M/s";
		}

		ms += QString::number(MemeoryRate) + "%";
		cs += QString::number(cpu_usage) + "%";

		data->us->setText(us);
		data->ds->setText(ds);
		data->ms->setText(ms);
		data->cs->setText(cs);

	}
}
