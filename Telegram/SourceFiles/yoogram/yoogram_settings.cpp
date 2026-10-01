/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "yoogram/yoogram_settings.h"

#include "core/application.h"
#include "core/core_settings.h"
#include "ui/widgets/popup_menu.h"

namespace YooGram {
namespace {

constexpr auto kQuickModerationKey = std::string_view(
	"yoogram-quick-moderation");
constexpr auto kGlassMenuKey = std::string_view("yoogram-glass-menu");

} // namespace

bool QuickModerationEnabled() {
	return Core::App().settings().readPref<bool>(kQuickModerationKey, true);
}

void SetQuickModerationEnabled(bool enabled) {
	Core::App().settings().writePref<bool>(kQuickModerationKey, enabled);
	Core::App().saveSettingsDelayed();
}

bool GlassMenuEnabled() {
	return Core::App().settings().readPref<bool>(kGlassMenuKey, false);
}

void SetGlassMenuEnabled(bool enabled) {
	Core::App().settings().writePref<bool>(kGlassMenuKey, enabled);
	Core::App().saveSettingsDelayed();
	ApplyGlassMenuSetting();
}

void ApplyGlassMenuSetting() {
	Ui::PopupMenu::SetGlassEnabled(GlassMenuEnabled());
}

} // namespace YooGram
