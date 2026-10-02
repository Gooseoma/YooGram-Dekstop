/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

class QImage;

namespace YooGram {

[[nodiscard]] bool QuickModerationEnabled();
void SetQuickModerationEnabled(bool enabled);

[[nodiscard]] bool GlassMenuEnabled();
void SetGlassMenuEnabled(bool enabled);
void ApplyGlassMenuSetting();

[[nodiscard]] bool FullNumbers();
void SetFullNumbers(bool enabled);

[[nodiscard]] bool TimeWithSeconds();
void SetTimeWithSeconds(bool enabled);

[[nodiscard]] bool HidePhoneNumber();
void SetHidePhoneNumber(bool enabled);

[[nodiscard]] bool CommaAfterMention();
void SetCommaAfterMention(bool enabled);

[[nodiscard]] const QImage &AppLogo();

// Reads the saved values into the caches used by hot formatting paths.
void LoadRuntimeSettings();

} // namespace YooGram
