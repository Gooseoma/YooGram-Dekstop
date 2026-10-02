/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include "base/basic_types.h"

#include <QtCore/QString>

#include <vector>

class HistoryItem;

namespace YooGram {

// A line of the persistent log of messages deleted on the server.
struct DeletedRecord {
	uint64 account = 0;
	uint64 peer = 0;
	int msgId = 0;
	QString chatName;
	QString fromName;
	qint64 date = 0;
	qint64 deletedAt = 0;
	QString text;
};

// When the "save deleted messages" setting is on, a message that the server
// reports as deleted (by the other side, an admin or a timer) is not destroyed
// but marked and kept in the chat until the chat is unloaded or the app is
// closed. Returns true if the item was kept (and must not be destroyed).
[[nodiscard]] bool KeepDeletedMessage(HistoryItem *item);

[[nodiscard]] bool IsKeptDeletedMessage(const HistoryItem *item);

// Called from ~HistoryItem.
void ForgetKeptDeletedMessage(const HistoryItem *item);

// Records of the account, newest first.
[[nodiscard]] std::vector<DeletedRecord> LoadDeletedLog(uint64 account);
void ClearDeletedLog(uint64 account);

} // namespace YooGram
