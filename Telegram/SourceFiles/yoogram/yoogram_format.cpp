/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "yoogram/yoogram_format.h"

#include "yoogram/yoogram_settings.h"

#include <QtCore/QLocale>

namespace YooGram {

QString FormatTime(const QTime &time) {
	const auto locale = QLocale();
	if (!TimeWithSeconds()) {
		return locale.toString(time, QLocale::ShortFormat);
	}
	auto format = locale.timeFormat(QLocale::ShortFormat);
	if (!format.contains(u"ss"_q)) {
		const auto minutes = format.indexOf(u"mm"_q);
		if (minutes < 0) {
			return locale.toString(time, QLocale::ShortFormat);
		}
		format.insert(minutes + 2, u":ss"_q);
	}
	return locale.toString(time, format);
}

} // namespace YooGram
