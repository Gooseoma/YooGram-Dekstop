/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "settings/sections/settings_yoogram.h"

#include "core/click_handler_types.h"
#include "lang/lang_keys.h"
#include "settings/sections/settings_main.h"
#include "settings/settings_builder.h"
#include "settings/settings_common.h"
#include "ui/platform/ui_platform_utility.h"
#include "ui/vertical_list.h"
#include "ui/widgets/buttons.h"
#include "ui/wrap/vertical_layout.h"
#include "window/window_session_controller.h"
#include "yoogram/yoogram_settings.h"
#include "styles/style_menu_icons.h"
#include "styles/style_settings.h"

namespace Settings {
namespace {

using namespace Builder;

constexpr auto kGitHubUrl = "https://github.com/Gooseoma/M-YooGram";

void AddToggle(
		SectionBuilder &builder,
		const QString &id,
		rpl::producer<QString> title,
		bool value,
		Fn<void(bool)> set,
		QStringList keywords,
		rpl::producer<bool> shown = nullptr) {
	const auto button = builder.addButton({
		.id = id,
		.title = std::move(title),
		.st = &st::settingsButtonNoIcon,
		.toggled = rpl::single(value),
		.keywords = std::move(keywords),
		.shown = std::move(shown),
	});
	if (button) {
		button->toggledChanges(
		) | rpl::on_next(std::move(set), button->lifetime());
	}
}

void BuildHub(SectionBuilder &builder) {
	builder.addSkip();
	builder.addSectionButton({
		.title = tr::lng_yoogram_main(),
		.targetSection = YooGramSection<YooGramPage::Main>::Id(),
		.icon = { &st::menuIconSettings },
		.keywords = { u"numbers"_q, u"time"_q, u"phone"_q },
	});
	builder.addSectionButton({
		.title = tr::lng_yoogram_appearance(),
		.targetSection = YooGramSection<YooGramPage::Appearance>::Id(),
		.icon = { &st::menuIconPalette },
		.keywords = { u"glass"_q, u"blur"_q, u"material"_q },
	});
	builder.addSectionButton({
		.title = tr::lng_yoogram_chats(),
		.targetSection = YooGramSection<YooGramPage::Chats>::Id(),
		.icon = { &st::menuIconChatBubble },
		.keywords = { u"moderation"_q, u"mention"_q, u"messages"_q },
	});
	builder.addSectionButton({
		.title = tr::lng_yoogram_other(),
		.targetSection = YooGramSection<YooGramPage::Other>::Id(),
		.icon = { &st::menuIconInfo },
		.keywords = { u"support"_q, u"github"_q },
	});
}

void BuildMain(SectionBuilder &builder) {
	builder.addSkip();
	builder.addSubsectionTitle({
		.id = u"yoogram/numbers_time"_q,
		.title = tr::lng_yoogram_numbers_time(),
		.keywords = { u"numbers"_q, u"time"_q, u"seconds"_q },
	});
	AddToggle(
		builder,
		u"yoogram/full_numbers"_q,
		tr::lng_yoogram_full_numbers(),
		YooGram::FullNumbers(),
		[](bool value) { YooGram::SetFullNumbers(value); },
		{ u"round"_q, u"numbers"_q, u"views"_q, u"counters"_q });
	AddToggle(
		builder,
		u"yoogram/time_seconds"_q,
		tr::lng_yoogram_time_seconds(),
		YooGram::TimeWithSeconds(),
		[](bool value) { YooGram::SetTimeWithSeconds(value); },
		{ u"time"_q, u"seconds"_q, u"clock"_q });
	builder.addSkip();
	builder.addDividerText(tr::lng_yoogram_numbers_time_about());

	builder.addSkip();
	builder.addSubsectionTitle({
		.id = u"yoogram/profile"_q,
		.title = tr::lng_yoogram_profile(),
		.keywords = { u"phone"_q, u"number"_q },
	});
	AddToggle(
		builder,
		u"yoogram/hide_phone"_q,
		tr::lng_yoogram_hide_phone(),
		YooGram::HidePhoneNumber(),
		[](bool value) { YooGram::SetHidePhoneNumber(value); },
		{ u"phone"_q, u"number"_q, u"hide"_q });
	builder.addSkip();
	builder.addDividerText(tr::lng_yoogram_hide_phone_about());
}

void BuildAppearance(SectionBuilder &builder) {
	builder.addSkip();
	builder.addSubsectionTitle({
		.id = u"yoogram/material"_q,
		.title = tr::lng_yoogram_material(),
		.keywords = { u"glass"_q, u"blur"_q, u"frosted"_q, u"menu"_q },
	});
	AddToggle(
		builder,
		u"yoogram/glass_menu"_q,
		tr::lng_yoogram_glass_menu(),
		YooGram::GlassMenuEnabled(),
		[](bool value) { YooGram::SetGlassMenuEnabled(value); },
		{ u"glass"_q, u"blur"_q, u"acrylic"_q, u"menu"_q },
		rpl::single(Ui::Platform::GlassBackdropSupported()));
	builder.addSkip();
	builder.addDividerText(tr::lng_yoogram_glass_menu_about());
}

void BuildChats(SectionBuilder &builder) {
	builder.addSkip();
	builder.addSubsectionTitle({
		.id = u"yoogram/moderation"_q,
		.title = tr::lng_yoogram_moderation(),
		.keywords = { u"mute"_q, u"ban"_q, u"admin"_q },
	});
	AddToggle(
		builder,
		u"yoogram/quick_moderation"_q,
		tr::lng_yoogram_quick_moderation(),
		YooGram::QuickModerationEnabled(),
		[](bool value) { YooGram::SetQuickModerationEnabled(value); },
		{ u"mute"_q, u"ban"_q, u"moderation"_q, u"admin"_q });
	builder.addSkip();
	builder.addDividerText(tr::lng_yoogram_quick_moderation_about());

	builder.addSkip();
	builder.addSubsectionTitle({
		.id = u"yoogram/messages"_q,
		.title = tr::lng_yoogram_messages(),
		.keywords = { u"mention"_q, u"comma"_q },
	});
	AddToggle(
		builder,
		u"yoogram/comma_mention"_q,
		tr::lng_yoogram_comma_mention(),
		YooGram::CommaAfterMention(),
		[](bool value) { YooGram::SetCommaAfterMention(value); },
		{ u"mention"_q, u"comma"_q, u"username"_q });
	builder.addSkip();
	builder.addDividerText(tr::lng_yoogram_comma_mention_about());
}

void BuildOther(SectionBuilder &builder) {
	builder.addSkip();
	builder.addSubsectionTitle({
		.id = u"yoogram/support"_q,
		.title = tr::lng_yoogram_support(),
		.keywords = { u"github"_q, u"support"_q, u"source"_q },
	});
	builder.addButton({
		.id = u"yoogram/github"_q,
		.title = tr::lng_yoogram_github(),
		.icon = { &st::menuIconLink },
		.label = rpl::single(u"Gooseoma/M-YooGram"_q),
		.onClick = [] { UrlClickHandler::Open(QString::fromLatin1(kGitHubUrl)); },
		.keywords = { u"github"_q, u"source"_q, u"bugs"_q },
	});
	builder.addSkip();
	builder.addDividerText(tr::lng_yoogram_github_about());
}

template <YooGramPage Page>
void BuildPage(SectionBuilder &builder) {
	if constexpr (Page == YooGramPage::Hub) {
		BuildHub(builder);
	} else if constexpr (Page == YooGramPage::Main) {
		BuildMain(builder);
	} else if constexpr (Page == YooGramPage::Appearance) {
		BuildAppearance(builder);
	} else if constexpr (Page == YooGramPage::Chats) {
		BuildChats(builder);
	} else {
		BuildOther(builder);
	}
}

[[nodiscard]] Type PageParent(YooGramPage page) {
	return (page == YooGramPage::Hub)
		? MainId()
		: YooGramSection<YooGramPage::Hub>::Id();
}

template <YooGramPage Page>
[[nodiscard]] const tr::phrase<> &PageTitle() {
	if constexpr (Page == YooGramPage::Hub) {
		return tr::lng_settings_yoogram;
	} else if constexpr (Page == YooGramPage::Main) {
		return tr::lng_yoogram_main;
	} else if constexpr (Page == YooGramPage::Appearance) {
		return tr::lng_yoogram_appearance;
	} else if constexpr (Page == YooGramPage::Chats) {
		return tr::lng_yoogram_chats;
	} else {
		return tr::lng_yoogram_other;
	}
}

template <YooGramPage Page>
[[nodiscard]] const style::icon &PageIcon() {
	if constexpr (Page == YooGramPage::Hub) {
		return st::menuIconYooGram;
	} else if constexpr (Page == YooGramPage::Main) {
		return st::menuIconSettings;
	} else if constexpr (Page == YooGramPage::Appearance) {
		return st::menuIconPalette;
	} else if constexpr (Page == YooGramPage::Chats) {
		return st::menuIconChatBubble;
	} else {
		return st::menuIconInfo;
	}
}

template <YooGramPage Page>
[[nodiscard]] BuildHelper MakeMeta() {
	return BuildHelper({
		.id = YooGramSection<Page>::Id(),
		.parentId = PageParent(Page),
		.title = &PageTitle<Page>(),
		.icon = &PageIcon<Page>(),
	}, [](SectionBuilder &builder) {
		BuildPage<Page>(builder);
	});
}

const auto kHubMeta = MakeMeta<YooGramPage::Hub>();
const auto kMainMeta = MakeMeta<YooGramPage::Main>();
const auto kAppearanceMeta = MakeMeta<YooGramPage::Appearance>();
const auto kChatsMeta = MakeMeta<YooGramPage::Chats>();
const auto kOtherMeta = MakeMeta<YooGramPage::Other>();

} // namespace

template <YooGramPage Page>
YooGramSection<Page>::YooGramSection(
	QWidget *parent,
	not_null<Window::SessionController*> controller)
: Section<YooGramSection<Page>>(parent, controller) {
	setupContent(controller);
}

template <YooGramPage Page>
rpl::producer<QString> YooGramSection<Page>::title() {
	return PageTitle<Page>()();
}

template <YooGramPage Page>
void YooGramSection<Page>::setupContent(
		not_null<Window::SessionController*> controller) {
	const auto container = Ui::CreateChild<Ui::VerticalLayout>(this);

	const SectionBuildMethod buildMethod = [](
			not_null<Ui::VerticalLayout*> container,
			not_null<Window::SessionController*> controller,
			Fn<void(Type)> showOther,
			rpl::producer<> showFinished) {
		auto &lifetime = container->lifetime();
		const auto highlights
			= lifetime.make_state<HighlightRegistry>();

		auto builder = SectionBuilder(WidgetContext{
			.container = container,
			.controller = controller,
			.showOther = std::move(showOther),
			.isPaused = Window::PausedIn(
				controller,
				Window::GifPauseReason::Layer),
			.highlights = highlights,
		});
		BuildPage<Page>(builder);

		std::move(showFinished) | rpl::on_next([=] {
			for (const auto &[id, entry] : *highlights) {
				if (entry.widget) {
					controller->checkHighlightControl(
						id,
						entry.widget,
						base::duplicate(entry.args));
				}
			}
		}, lifetime);
	};

	this->build(container, buildMethod);

	Ui::ResizeFitChild(this, container);
}

template class YooGramSection<YooGramPage::Hub>;
template class YooGramSection<YooGramPage::Main>;
template class YooGramSection<YooGramPage::Appearance>;
template class YooGramSection<YooGramPage::Chats>;
template class YooGramSection<YooGramPage::Other>;

Type YooGramId() {
	return YooGramSection<YooGramPage::Hub>::Id();
}

} // namespace Settings
