/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "yoogram/yoogram_moderation.h"

#include "api/api_chat_participants.h"
#include "base/unixtime.h"
#include "data/data_channel.h"
#include "data/data_chat_participant_status.h"
#include "data/data_user.h"
#include "history/history.h"
#include "history/history_item.h"
#include "lang/lang_keys.h"
#include "ui/boxes/confirm_box.h"
#include "ui/layers/show.h"
#include "ui/widgets/popup_menu.h"
#include "yoogram/yoogram_settings.h"
#include "styles/style_layers.h"
#include "styles/style_menu_icons.h"

namespace YooGram {
namespace {

constexpr auto kMuteTenMinutes = TimeId(10 * 60);
constexpr auto kMuteHour = TimeId(60 * 60);
constexpr auto kMuteDay = TimeId(24 * 60 * 60);

[[nodiscard]] ChatRestrictions MuteFlags() {
	using Flag = ChatRestriction;
	return Flag::SendOther
		| Flag::SendStickers
		| Flag::SendGifs
		| Flag::SendGames
		| Flag::SendInline
		| Flag::SendPolls
		| Flag::SendPhotos
		| Flag::SendVideos
		| Flag::SendVideoMessages
		| Flag::SendMusic
		| Flag::SendVoiceMessages
		| Flag::SendFiles
		| Flag::EmbedLinks;
}

[[nodiscard]] bool IsAdminOrCreator(
		not_null<ChannelData*> channel,
		not_null<UserData*> user) {
	const auto info = channel->mgInfo.get();
	return info
		&& ((info->creator == user)
			|| info->admins.contains(peerToUser(user->id)));
}

void Apply(
		std::shared_ptr<Ui::Show> show,
		not_null<ChannelData*> channel,
		not_null<UserData*> user,
		ChatRestrictionsInfo rights,
		const QString &done) {
	Api::ChatParticipants::Restrict(
		channel,
		user,
		ChatRestrictionsInfo(),
		rights,
		[=] { show->showToast(done); },
		[=](const QString &error) { show->showToast(error); });
}

} // namespace

void AddQuickModerationActions(
		not_null<Ui::PopupMenu*> menu,
		not_null<HistoryItem*> item,
		std::shared_ptr<Ui::Show> show) {
	if (!QuickModerationEnabled()
		|| item->isLocal()
		|| item->isService()) {
		return;
	}
	const auto channel = item->history()->peer->asChannel();
	const auto user = item->from()->asUser();
	if (!channel
		|| !user
		|| !channel->isMegagroup()
		|| !channel->canBanMembers()
		|| user->isSelf()
		|| IsAdminOrCreator(channel, user)) {
		return;
	}

	auto submenu = std::make_unique<Ui::PopupMenu>(menu, menu->st());
	const auto addMute = [&](const QString &label, TimeId duration) {
		submenu->addAction(label, [=] {
			Apply(
				show,
				channel,
				user,
				ChatRestrictionsInfo(
					MuteFlags(),
					base::unixtime::now() + duration),
				tr::lng_yoogram_muted(tr::now, lt_name, user->name()));
		}, &st::menuIconMuteFor);
	};
	addMute(tr::lng_yoogram_mute_10m(tr::now), kMuteTenMinutes);
	addMute(tr::lng_yoogram_mute_1h(tr::now), kMuteHour);
	addMute(tr::lng_yoogram_mute_1d(tr::now), kMuteDay);
	submenu->addAction(tr::lng_yoogram_ban(tr::now), [=] {
		show->show(Ui::MakeConfirmBox({
			.text = tr::lng_yoogram_ban_sure(tr::now, lt_name, user->name()),
			.confirmed = [=](Fn<void()> &&close) {
				close();
				Apply(
					show,
					channel,
					user,
					ChatRestrictionsInfo(
						ChatRestriction::ViewMessages,
						0),
					tr::lng_yoogram_banned(tr::now, lt_name, user->name()));
			},
			.confirmText = tr::lng_yoogram_ban(),
			.confirmStyle = &st::attentionBoxButton,
		}));
	}, &st::menuIconBlock);

	menu->addAction(
		tr::lng_yoogram_moderation(tr::now),
		std::move(submenu),
		&st::menuIconManage);
}

} // namespace YooGram
