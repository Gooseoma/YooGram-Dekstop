/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

class HistoryItem;

namespace YooGram {

// When the "save deleted messages" setting is on, a message that the server
// reports as deleted (by the other side, an admin or a timer) is not destroyed
// but marked and kept in the chat until the chat is unloaded or the app is
// closed. Returns true if the item was kept (and must not be destroyed).
[[nodiscard]] bool KeepDeletedMessage(HistoryItem *item);

[[nodiscard]] bool IsKeptDeletedMessage(const HistoryItem *item);

// Called from ~HistoryItem.
void ForgetKeptDeletedMessage(const HistoryItem *item);

} // namespace YooGram
