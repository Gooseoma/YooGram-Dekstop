/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

class HistoryItem;

namespace Ui {
class PopupMenu;
class Show;
} // namespace Ui

namespace YooGram {

void AddQuickModerationActions(
	not_null<Ui::PopupMenu*> menu,
	not_null<HistoryItem*> item,
	std::shared_ptr<Ui::Show> show);

} // namespace YooGram
