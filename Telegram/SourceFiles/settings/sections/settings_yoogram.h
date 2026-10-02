/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include "settings/settings_common_session.h"

namespace Settings {

enum class YooGramPage {
	Hub,
	Main,
	Appearance,
	Chats,
	Other,
};

template <YooGramPage Page>
class YooGramSection : public Section<YooGramSection<Page>> {
public:
	YooGramSection(
		QWidget *parent,
		not_null<Window::SessionController*> controller);

	[[nodiscard]] rpl::producer<QString> title() override;

private:
	void setupContent(not_null<Window::SessionController*> controller);

};

[[nodiscard]] Type YooGramId();

} // namespace Settings
