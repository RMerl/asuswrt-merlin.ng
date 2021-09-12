#include <QApplication>
#include <QPlainTextEdit>
#include <QPalette>
#include <QFontDatabase>
#include <QVBoxLayout>
#include <QPushButton>

extern "C" {
#include "../highlighter.h"
}

static QColor colormap[] = {
	[HighlightSection] = QColor("#ababab"),
	[HighlightField] = QColor("#70c0b1"),
	[HighlightPrivateKey] = QColor("#7aa6da"),
	[HighlightPublicKey] = QColor("#7aa6da"),
	[HighlightPresharedKey] = QColor("#7aa6da"),
	[HighlightIP] = QColor("#b9ca4a"),
	[HighlightCidr] = QColor("#e78c45"),
	[HighlightHost] = QColor("#b9ca4a"),
	[HighlightPort] = QColor("#e78c45"),
	[HighlightMTU] = QColor("#c397d8"),
	[HighlightKeepalive] = QColor("#c397d8"),
	[HighlightComment] = QColor("#969896"),
	[HighlightDelimiter] = QColor("#7aa6da"),
#ifndef MOBILE_WGQUICK_SUBSET
	[HighlightTable] = QColor("#c397d8"),
	[HighlightFwMark] = QColor("#c397d8"),
	[HighlightSaveConfig] = QColor("#c397d8"),
	[HighlightCmd] = QColor("#969896"),
#endif
	[HighlightError] = QColor("#d54e53")
};

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QWidget w;
	w.setWindowTitle(QObject::tr("WireGuard Configuration Highlighter"));
	QVBoxLayout v;
	w.setLayout(&v);
	QPlainTextEdit e;
	v.addWidget(&e);
	QPalette p(e.palette());
	p.setColor(QPalette::Base, QColor("#010101"));
	p.setColor(QPalette::Text, QColor("#eaeaea"));
	e.setPalette(p);
	QFont f(QFontDatabase::systemFont(QFontDatabase::FixedFont));
	f.setPointSize(16);
	e.setFont(f);
	e.setMinimumSize(400, 500);
	bool guard = false;
	QObject::connect(&e, &QPlainTextEdit::textChanged, [&]() {
		if (guard)
			return;
		struct highlight_span *spans = highlight_config(e.toPlainText().toLatin1().data());
		if (!spans)
			return;
		QTextCursor cursor(e.document());
		QTextCharFormat format;
		cursor.beginEditBlock();
		cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
		cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
		format.setForeground(p.color(QPalette::Text));
		format.setUnderlineStyle(QTextCharFormat::NoUnderline);
		cursor.mergeCharFormat(format);
		for (struct highlight_span *span = spans; span->type != HighlightEnd; ++span) {
			cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
			cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, span->start);
			cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, span->len);
			format.setForeground(colormap[span->type]);
			format.setUnderlineStyle(span->type == HighlightError ? QTextCharFormat::SpellCheckUnderline : QTextCharFormat::NoUnderline);
			cursor.mergeCharFormat(format);
		}
		free(spans);
		guard = true;
		cursor.endEditBlock();
		guard = false;
	});
	QPushButton b;
	v.addWidget(&b);
	b.setText(QObject::tr("&Randomize colors"));
	QObject::connect(&b, &QPushButton::clicked, [&]() {
		for (size_t i = 0; i < sizeof(colormap) / sizeof(colormap[0]); ++i)
			colormap[i] = QColor::fromHsl(qrand() % 360, qrand() % 192 + 64, qrand() % 128 + 128);
		e.setPlainText(e.toPlainText());
	});
	w.show();
	return a.exec();
}
