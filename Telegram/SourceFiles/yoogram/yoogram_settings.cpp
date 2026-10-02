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

#include <QtGui/QImage>

#include <atomic>

namespace YooGram {
namespace {

constexpr auto kQuickModerationKey = std::string_view(
	"yoogram-quick-moderation");
constexpr auto kGlassMenuKey = std::string_view("yoogram-glass-menu");
constexpr auto kFullNumbersKey = std::string_view("yoogram-full-numbers");
constexpr auto kTimeSecondsKey = std::string_view("yoogram-time-seconds");
constexpr auto kHidePhoneKey = std::string_view("yoogram-hide-phone");
constexpr auto kCommaMentionKey = std::string_view("yoogram-comma-mention");

std::atomic<bool> FullNumbersCache = false;
std::atomic<bool> TimeSecondsCache = false;

[[nodiscard]] bool Read(std::string_view key, bool fallback) {
	return Core::App().settings().readPref<bool>(key, fallback);
}

void Write(std::string_view key, bool value) {
	Core::App().settings().writePref<bool>(key, value);
	Core::App().saveSettingsDelayed();
}

} // namespace

bool QuickModerationEnabled() {
	return Read(kQuickModerationKey, true);
}

void SetQuickModerationEnabled(bool enabled) {
	Write(kQuickModerationKey, enabled);
}

bool GlassMenuEnabled() {
	return Read(kGlassMenuKey, false);
}

void SetGlassMenuEnabled(bool enabled) {
	Write(kGlassMenuKey, enabled);
	ApplyGlassMenuSetting();
}

void ApplyGlassMenuSetting() {
	Ui::PopupMenu::SetGlassEnabled(GlassMenuEnabled());
}

bool FullNumbers() {
	return FullNumbersCache.load(std::memory_order_relaxed);
}

void SetFullNumbers(bool enabled) {
	Write(kFullNumbersKey, enabled);
	FullNumbersCache.store(enabled, std::memory_order_relaxed);
}

bool TimeWithSeconds() {
	return TimeSecondsCache.load(std::memory_order_relaxed);
}

void SetTimeWithSeconds(bool enabled) {
	Write(kTimeSecondsKey, enabled);
	TimeSecondsCache.store(enabled, std::memory_order_relaxed);
}

bool HidePhoneNumber() {
	return Read(kHidePhoneKey, false);
}

void SetHidePhoneNumber(bool enabled) {
	Write(kHidePhoneKey, enabled);
}

bool CommaAfterMention() {
	return Read(kCommaMentionKey, false);
}

void SetCommaAfterMention(bool enabled) {
	Write(kCommaMentionKey, enabled);
}

const QImage &AppLogo() {
	static const auto result = QImage(u":/gui/art/icon_round512@2x.png"_q);
	return result;
}

void LoadRuntimeSettings() {
	FullNumbersCache.store(
		Read(kFullNumbersKey, false),
		std::memory_order_relaxed);
	TimeSecondsCache.store(
		Read(kTimeSecondsKey, false),
		std::memory_order_relaxed);
	ApplyGlassMenuSetting();
}

} // namespace YooGram
