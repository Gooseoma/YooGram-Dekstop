/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "yoogram/yoogram_deleted.h"

#include "data/data_session.h"
#include "history/history.h"
#include "history/history_item.h"
#include "main/main_session.h"
#include "data/data_peer.h"
#include "settings.h"
#include "yoogram/yoogram_settings.h"

#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

#include <unordered_set>

namespace YooGram {
namespace {

constexpr auto kMaxLogSize = qint64(2 * 1024 * 1024);

[[nodiscard]] std::unordered_set<const HistoryItem*> &Kept() {
	static auto result = std::unordered_set<const HistoryItem*>();
	return result;
}

[[nodiscard]] QString LogPath() {
	return cWorkingDir() + u"tdata/yoogram-deleted.jsonl"_q;
}

[[nodiscard]] QByteArray Serialize(const DeletedRecord &record) {
	auto object = QJsonObject();
	object.insert(u"a"_q, QString::number(record.account));
	object.insert(u"p"_q, QString::number(record.peer));
	object.insert(u"m"_q, record.msgId);
	object.insert(u"c"_q, record.chatName);
	object.insert(u"f"_q, record.fromName);
	object.insert(u"d"_q, QString::number(record.date));
	object.insert(u"x"_q, QString::number(record.deletedAt));
	object.insert(u"t"_q, record.text);
	return QJsonDocument(object).toJson(QJsonDocument::Compact) + '\n';
}

[[nodiscard]] std::vector<QByteArray> ReadLines() {
	auto file = QFile(LogPath());
	if (!file.open(QIODevice::ReadOnly)) {
		return {};
	}
	auto result = std::vector<QByteArray>();
	while (!file.atEnd()) {
		auto line = file.readLine().trimmed();
		if (!line.isEmpty()) {
			result.push_back(std::move(line));
		}
	}
	return result;
}

void WriteLines(const std::vector<QByteArray> &lines) {
	QDir().mkpath(cWorkingDir() + u"tdata"_q);
	auto file = QFile(LogPath());
	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		return;
	}
	for (const auto &line : lines) {
		file.write(line);
		file.write("\n");
	}
}

void AppendToLog(const DeletedRecord &record) {
	QDir().mkpath(cWorkingDir() + u"tdata"_q);
	auto file = QFile(LogPath());
	if (file.size() > kMaxLogSize) {
		// Keep the newer half.
		auto lines = ReadLines();
		lines.erase(lines.begin(), lines.begin() + lines.size() / 2);
		WriteLines(lines);
	}
	if (file.open(QIODevice::WriteOnly | QIODevice::Append)) {
		file.write(Serialize(record));
	}
}

[[nodiscard]] DeletedRecord MakeRecord(not_null<HistoryItem*> item) {
	const auto history = item->history();
	auto text = item->originalText().text;
	if (text.isEmpty()) {
		text = item->notificationText().text;
	}
	return {
		.account = history->session().userId().bare,
		.peer = history->peer->id.value,
		.msgId = int(item->id.bare),
		.chatName = history->peer->name(),
		.fromName = item->from()->name(),
		.date = qint64(item->date()),
		.deletedAt = QDateTime::currentSecsSinceEpoch(),
		.text = std::move(text),
	};
}

} // namespace

bool KeepDeletedMessage(HistoryItem *item) {
	if (!item || !SaveDeletedMessages()) {
		return false;
	} else if (Kept().contains(item)) {
		return true;
	} else if (!item->isRegular()
		|| !item->isHistoryEntry()
		|| item->isService()
		|| item->isEphemeral()) {
		return false;
	}
	Kept().emplace(item);
	AppendToLog(MakeRecord(item));
	const auto owner = &item->history()->owner();
	owner->notifyItemDataChange(item);
	item->invalidateChatListEntry();
	return true;
}

bool IsKeptDeletedMessage(const HistoryItem *item) {
	return item && !Kept().empty() && Kept().contains(item);
}

void ForgetKeptDeletedMessage(const HistoryItem *item) {
	if (!Kept().empty()) {
		Kept().erase(item);
	}
}

std::vector<DeletedRecord> LoadDeletedLog(uint64 account) {
	auto result = std::vector<DeletedRecord>();
	const auto lines = ReadLines();
	result.reserve(lines.size());
	for (auto i = lines.rbegin(); i != lines.rend(); ++i) {
		const auto object = QJsonDocument::fromJson(*i).object();
		if (object.isEmpty()
			|| object.value(u"a"_q).toString().toULongLong() != account) {
			continue;
		}
		result.push_back({
			.account = account,
			.peer = object.value(u"p"_q).toString().toULongLong(),
			.msgId = object.value(u"m"_q).toInt(),
			.chatName = object.value(u"c"_q).toString(),
			.fromName = object.value(u"f"_q).toString(),
			.date = object.value(u"d"_q).toString().toLongLong(),
			.deletedAt = object.value(u"x"_q).toString().toLongLong(),
			.text = object.value(u"t"_q).toString(),
		});
	}
	return result;
}

void ClearDeletedLog(uint64 account) {
	auto kept = std::vector<QByteArray>();
	for (const auto &line : ReadLines()) {
		const auto object = QJsonDocument::fromJson(line).object();
		if (object.value(u"a"_q).toString().toULongLong() != account) {
			kept.push_back(line);
		}
	}
	WriteLines(kept);
}

} // namespace YooGram
