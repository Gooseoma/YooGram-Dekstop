/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

class QImage;

namespace YooGram {

inline constexpr auto kStickerSizeMin = 2;
inline constexpr auto kStickerSizeMax = 20;
inline constexpr auto kStickerSizeDefault = 14;
inline constexpr auto kAvatarRadiusMin = 2;
inline constexpr auto kAvatarRadiusMax = 50;

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

[[nodiscard]] bool HideWelcomeSticker();
void SetHideWelcomeSticker(bool enabled);

[[nodiscard]] bool HideMessageTail();
void SetHideMessageTail(bool enabled);

[[nodiscard]] bool EditedIcon();
void SetEditedIcon(bool enabled);

[[nodiscard]] bool HideStickerTime();
void SetHideStickerTime(bool enabled);

[[nodiscard]] bool AlwaysHDPhotos();
void SetAlwaysHDPhotos(bool enabled);

// Corner radius of avatars in percent of their size: kAvatarRadiusMin is
// almost a square, kAvatarRadiusMax is a circle (the default).
[[nodiscard]] int AvatarRadiusPercent();
void SetAvatarRadiusPercent(int percent);
[[nodiscard]] bool CustomAvatarRadius();
[[nodiscard]] double UserpicRadiusMultiplier();

[[nodiscard]] bool ForceSnow();
void SetForceSnow(bool enabled);

[[nodiscard]] bool HideStories();
void SetHideStories(bool enabled);

[[nodiscard]] bool HideFolderCounters();
void SetHideFolderCounters(bool enabled);

// Show badges issued by the YooGram badge server in profiles.
[[nodiscard]] bool ShowBadges();
void SetShowBadges(bool enabled);

// Keep messages deleted on the server visible (marked) in the chat.
[[nodiscard]] bool SaveDeletedMessages();
void SetSaveDeletedMessages(bool enabled);

// A green "online" dot on the userpics next to messages.
[[nodiscard]] bool MessageOnlineIndicator();
void SetMessageOnlineIndicator(bool enabled);

// Sticker size step like on Android: kStickerSizeDefault is the normal size.
[[nodiscard]] int StickerSizeStep();
void SetStickerSizeStep(int step);

// Seconds to jump on a double click on the left or right side of a video
// in the media viewer, 0 means the feature is off.
[[nodiscard]] int DoubleTapSeekSeconds();
void SetDoubleTapSeekSeconds(int seconds);

[[nodiscard]] bool UnifiedRounding();
void SetUnifiedRounding(bool enabled);

[[nodiscard]] const QImage &AppLogo();

// Reads the saved values into the caches used by hot formatting paths.
void LoadRuntimeSettings();

} // namespace YooGram
