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

#include <algorithm>
#include <atomic>

namespace YooGram {
namespace {

[[nodiscard]] bool Read(std::string_view key, bool fallback) {
	return Core::App().settings().readPref<bool>(key, fallback);
}

void Write(std::string_view key, bool value) {
	Core::App().settings().writePref<bool>(key, value);
	Core::App().saveSettingsDelayed();
}

[[nodiscard]] int ReadInt(std::string_view key, int fallback, int min, int max) {
	const auto saved = Core::App().settings().readPref<QByteArray>(key);
	auto ok = false;
	const auto value = saved.toInt(&ok);
	return ok ? std::clamp(value, min, max) : fallback;
}

void WriteInt(std::string_view key, int value) {
	Core::App().settings().writePref<QByteArray>(
		key,
		QByteArray::number(value));
	Core::App().saveSettingsDelayed();
}

// A flag saved in the settings and cached for hot code paths.
class CachedFlag final {
public:
	constexpr CachedFlag(std::string_view key, bool fallback)
	: _key(key)
	, _fallback(fallback) {
	}

	[[nodiscard]] bool value() const {
		return _value.load(std::memory_order_relaxed);
	}
	void set(bool enabled) {
		Write(_key, enabled);
		_value.store(enabled, std::memory_order_relaxed);
	}
	void load() {
		_value.store(Read(_key, _fallback), std::memory_order_relaxed);
	}

private:
	const std::string_view _key;
	const bool _fallback;
	std::atomic<bool> _value = false;

};

constexpr auto kAvatarRadiusKey = std::string_view("yoogram-avatar-radius");
constexpr auto kStickerSizeKey = std::string_view("yoogram-sticker-size");

CachedFlag FullNumbersFlag("yoogram-full-numbers", false);
CachedFlag TimeSecondsFlag("yoogram-time-seconds", false);
CachedFlag HideWelcomeFlag("yoogram-hide-welcome", false);
CachedFlag HideTailFlag("yoogram-hide-tail", false);
CachedFlag EditedIconFlag("yoogram-edited-icon", false);
CachedFlag HideStickerTimeFlag("yoogram-hide-sticker-time", false);
CachedFlag AlwaysHDFlag("yoogram-always-hd", false);
CachedFlag UnifiedRoundingFlag("yoogram-unified-rounding", false);
CachedFlag ForceSnowFlag("yoogram-force-snow", false);
CachedFlag HideStoriesFlag("yoogram-hide-stories", false);
std::atomic<int> StickerSize = kStickerSizeDefault;
std::atomic<int> AvatarRadius = kAvatarRadiusMax;

} // namespace

bool QuickModerationEnabled() {
	return Read("yoogram-quick-moderation", true);
}

void SetQuickModerationEnabled(bool enabled) {
	Write("yoogram-quick-moderation", enabled);
}

bool GlassMenuEnabled() {
	return Read("yoogram-glass-menu", false);
}

void SetGlassMenuEnabled(bool enabled) {
	Write("yoogram-glass-menu", enabled);
	ApplyGlassMenuSetting();
}

void ApplyGlassMenuSetting() {
	Ui::PopupMenu::SetGlassEnabled(GlassMenuEnabled());
}

bool FullNumbers() {
	return FullNumbersFlag.value();
}

void SetFullNumbers(bool enabled) {
	FullNumbersFlag.set(enabled);
}

bool TimeWithSeconds() {
	return TimeSecondsFlag.value();
}

void SetTimeWithSeconds(bool enabled) {
	TimeSecondsFlag.set(enabled);
}

bool HidePhoneNumber() {
	return Read("yoogram-hide-phone", false);
}

void SetHidePhoneNumber(bool enabled) {
	Write("yoogram-hide-phone", enabled);
}

bool CommaAfterMention() {
	return Read("yoogram-comma-mention", false);
}

void SetCommaAfterMention(bool enabled) {
	Write("yoogram-comma-mention", enabled);
}

bool HideWelcomeSticker() {
	return HideWelcomeFlag.value();
}

void SetHideWelcomeSticker(bool enabled) {
	HideWelcomeFlag.set(enabled);
}

bool HideMessageTail() {
	return HideTailFlag.value();
}

void SetHideMessageTail(bool enabled) {
	HideTailFlag.set(enabled);
}

bool EditedIcon() {
	return EditedIconFlag.value();
}

void SetEditedIcon(bool enabled) {
	EditedIconFlag.set(enabled);
}

bool HideStickerTime() {
	return HideStickerTimeFlag.value();
}

void SetHideStickerTime(bool enabled) {
	HideStickerTimeFlag.set(enabled);
}

bool AlwaysHDPhotos() {
	return AlwaysHDFlag.value();
}

void SetAlwaysHDPhotos(bool enabled) {
	AlwaysHDFlag.set(enabled);
}

int AvatarRadiusPercent() {
	return AvatarRadius.load(std::memory_order_relaxed);
}

void SetAvatarRadiusPercent(int percent) {
	percent = std::clamp(percent, kAvatarRadiusMin, kAvatarRadiusMax);
	WriteInt(kAvatarRadiusKey, percent);
	AvatarRadius.store(percent, std::memory_order_relaxed);
}

bool CustomAvatarRadius() {
	return AvatarRadiusPercent() < kAvatarRadiusMax;
}

double UserpicRadiusMultiplier() {
	return AvatarRadiusPercent() / 100.;
}

bool ForceSnow() {
	return ForceSnowFlag.value();
}

void SetForceSnow(bool enabled) {
	ForceSnowFlag.set(enabled);
}

bool HideStories() {
	return HideStoriesFlag.value();
}

void SetHideStories(bool enabled) {
	HideStoriesFlag.set(enabled);
}

int StickerSizeStep() {
	return StickerSize.load(std::memory_order_relaxed);
}

void SetStickerSizeStep(int step) {
	step = std::clamp(step, kStickerSizeMin, kStickerSizeMax);
	WriteInt(kStickerSizeKey, step);
	StickerSize.store(step, std::memory_order_relaxed);
}

bool UnifiedRounding() {
	return UnifiedRoundingFlag.value();
}

void SetUnifiedRounding(bool enabled) {
	UnifiedRoundingFlag.set(enabled);
}

const QImage &AppLogo() {
	static const auto result = QImage(u":/gui/art/icon_round512@2x.png"_q);
	return result;
}

void LoadRuntimeSettings() {
	FullNumbersFlag.load();
	TimeSecondsFlag.load();
	HideWelcomeFlag.load();
	HideTailFlag.load();
	EditedIconFlag.load();
	HideStickerTimeFlag.load();
	AlwaysHDFlag.load();
	UnifiedRoundingFlag.load();
	AvatarRadius.store(
		ReadInt(
			kAvatarRadiusKey,
			kAvatarRadiusMax,
			kAvatarRadiusMin,
			kAvatarRadiusMax),
		std::memory_order_relaxed);
	StickerSize.store(
		ReadInt(
			kStickerSizeKey,
			kStickerSizeDefault,
			kStickerSizeMin,
			kStickerSizeMax),
		std::memory_order_relaxed);
	ForceSnowFlag.load();
	HideStoriesFlag.load();
	ApplyGlassMenuSetting();
}

} // namespace YooGram
