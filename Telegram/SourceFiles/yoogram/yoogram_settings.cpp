/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "yoogram/yoogram_settings.h"

#include "core/application.h"
#include "core/core_settings.h"

namespace YooGram {
namespace {

constexpr auto kQuickModerationKey = std::string_view(
	"yoogram-quick-moderation");

} // namespace

bool QuickModerationEnabled() {
	return Core::App().settings().readPref<bool>(kQuickModerationKey, true);
}

void SetQuickModerationEnabled(bool enabled) {
	Core::App().settings().writePref<bool>(kQuickModerationKey, enabled);
	Core::App().saveSettingsDelayed();
}

} // namespace YooGram
