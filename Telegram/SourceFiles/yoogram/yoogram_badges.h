/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include "base/basic_types.h"

#include <rpl/producer.h>

#include <QtCore/QString>
#include <QtGui/QColor>

#include <optional>

namespace YooGram {

// A badge granted by the YooGram badge server: either an emoji or a short
// text label shown as a pill.
struct CustomBadge {
	QString text;
	QString tooltip;
	QColor color;
	bool label = false;

	friend inline bool operator==(
		const CustomBadge &,
		const CustomBadge &) = default;
};

// Loads the cached list and starts periodic refreshes from the badge server.
// Every list is checked against the embedded Ed25519 public key first.
void StartBadges();

// Called when the "show badges" setting is switched.
void BadgesSettingChanged();

[[nodiscard]] std::optional<CustomBadge> LookupBadge(uint64 userId);

// Increments every time the list (or the setting) changes.
[[nodiscard]] rpl::producer<int> BadgesVersionValue();

} // namespace YooGram
