/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "settings/sections/settings_yoogram.h"

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

void BuildYooGramSectionContent(SectionBuilder &builder) {
	builder.addSkip();
	builder.addSubsectionTitle({
		.id = u"yoogram/moderation"_q,
		.title = tr::lng_yoogram_moderation(),
		.keywords = { u"mute"_q, u"ban"_q, u"admin"_q },
	});

	const auto toggle = builder.addButton({
		.id = u"yoogram/quick_moderation"_q,
		.title = tr::lng_yoogram_quick_moderation(),
		.st = &st::settingsButtonNoIcon,
		.toggled = rpl::single(YooGram::QuickModerationEnabled()),
		.keywords = { u"mute"_q, u"ban"_q, u"moderation"_q },
	});
	if (toggle) {
		toggle->toggledChanges(
		) | rpl::on_next([](bool enabled) {
			YooGram::SetQuickModerationEnabled(enabled);
		}, toggle->lifetime());
	}

	builder.addSkip();
	builder.addDividerText(tr::lng_yoogram_quick_moderation_about());

	const auto glassSupported = Ui::Platform::GlassBackdropSupported();
	builder.addSkip();
	builder.addSubsectionTitle({
		.id = u"yoogram/appearance"_q,
		.title = tr::lng_yoogram_appearance(),
		.keywords = { u"glass"_q, u"blur"_q, u"menu"_q },
	});

	const auto glass = builder.addButton({
		.id = u"yoogram/glass_menu"_q,
		.title = tr::lng_yoogram_glass_menu(),
		.st = &st::settingsButtonNoIcon,
		.toggled = rpl::single(YooGram::GlassMenuEnabled()),
		.keywords = { u"glass"_q, u"blur"_q, u"acrylic"_q, u"menu"_q },
		.shown = rpl::single(glassSupported),
	});
	if (glass) {
		glass->toggledChanges(
		) | rpl::on_next([](bool enabled) {
			YooGram::SetGlassMenuEnabled(enabled);
		}, glass->lifetime());
	}

	builder.addSkip();
	builder.addDividerText(tr::lng_yoogram_glass_menu_about());
}

const auto kMeta = BuildHelper({
	.id = YooGramSection::Id(),
	.parentId = MainId(),
	.title = &tr::lng_settings_yoogram,
	.icon = &st::menuIconManage,
}, [](SectionBuilder &builder) {
	BuildYooGramSectionContent(builder);
});

} // namespace

YooGramSection::YooGramSection(
	QWidget *parent,
	not_null<Window::SessionController*> controller)
: Section(parent, controller) {
	setupContent(controller);
}

rpl::producer<QString> YooGramSection::title() {
	return tr::lng_settings_yoogram();
}

void YooGramSection::setupContent(
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
		BuildYooGramSectionContent(builder);

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

	build(container, buildMethod);

	Ui::ResizeFitChild(this, container);
}

Type YooGramId() {
	return YooGramSection::Id();
}

} // namespace Settings
