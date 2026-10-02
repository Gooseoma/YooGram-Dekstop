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
#include "yoogram/yoogram_settings.h"

#include <unordered_set>

namespace YooGram {
namespace {

[[nodiscard]] std::unordered_set<const HistoryItem*> &Kept() {
	static auto result = std::unordered_set<const HistoryItem*>();
	return result;
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

} // namespace YooGram
