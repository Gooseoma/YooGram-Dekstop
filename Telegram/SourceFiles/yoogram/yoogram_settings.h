/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

namespace YooGram {

[[nodiscard]] bool QuickModerationEnabled();
void SetQuickModerationEnabled(bool enabled);

[[nodiscard]] bool GlassMenuEnabled();
void SetGlassMenuEnabled(bool enabled);
void ApplyGlassMenuSetting();

} // namespace YooGram
