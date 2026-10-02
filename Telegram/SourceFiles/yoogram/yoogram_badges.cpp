/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "yoogram/yoogram_badges.h"

#include "base/basic_types.h"
#include "base/timer.h"
#include "settings.h"
#include "yoogram/yoogram_settings.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QPointer>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

#include <openssl/evp.h>

#include <map>
#include <memory>

namespace YooGram {
namespace {

constexpr auto kBadgesUrl = "https://api-tg.gooseoma.ru/v1/badges";
// Raw Ed25519 public key of the badge server, base64.
constexpr auto kPublicKey = "2osxSbQGkDvrMiHUMDoMlQ0t0fE9vOyxS1dYbS+KbpE=";
constexpr auto kRefreshEvery = 15 * 60 * crl::time(1000);
constexpr auto kRequestTimeout = 15 * 1000;
constexpr auto kMaxBodySize = 1024 * 1024;
constexpr auto kDefaultColor = "#2a9df4";

void Fetch();

struct State {
	State() : timer([=] { Fetch(); }) {
	}

	std::unique_ptr<QNetworkAccessManager> network;
	base::Timer timer;
	QPointer<QNetworkReply> reply;
	std::map<uint64, CustomBadge> badges;
	qint64 version = -1;
	QByteArray etag;
	int counter = 0;
	rpl::event_stream<int> changed;
	bool started = false;
};

[[nodiscard]] State &Get() {
	// Lives until the process exits, so no destruction order issues with
	// the network stack at shutdown.
	static const auto instance = new State();
	return *instance;
}

[[nodiscard]] QString CachePath() {
	return cWorkingDir() + u"tdata/yoogram-badges.json"_q;
}

[[nodiscard]] bool VerifySignature(
		const QByteArray &payload,
		const QByteArray &signature) {
	const auto key = QByteArray::fromBase64(QByteArray(kPublicKey));
	if (key.size() != 32 || signature.size() != 64) {
		return false;
	}
	const auto pkey = std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>(
		EVP_PKEY_new_raw_public_key(
			EVP_PKEY_ED25519,
			nullptr,
			reinterpret_cast<const unsigned char*>(key.constData()),
			key.size()),
		&EVP_PKEY_free);
	if (!pkey) {
		return false;
	}
	const auto context = std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)>(
		EVP_MD_CTX_new(),
		&EVP_MD_CTX_free);
	if (!context
		|| EVP_DigestVerifyInit(
			context.get(),
			nullptr,
			nullptr,
			nullptr,
			pkey.get()) != 1) {
		return false;
	}
	return EVP_DigestVerify(
		context.get(),
		reinterpret_cast<const unsigned char*>(signature.constData()),
		signature.size(),
		reinterpret_cast<const unsigned char*>(payload.constData()),
		payload.size()) == 1;
}

void NotifyChanged() {
	auto &state = Get();
	++state.counter;
	state.changed.fire_copy(state.counter);
}

// Returns true if the response was valid (signature checked and not older
// than what we already have).
[[nodiscard]] bool Apply(const QByteArray &body) {
	const auto outer = QJsonDocument::fromJson(body);
	if (!outer.isObject()) {
		return false;
	}
	const auto object = outer.object();
	const auto payload = object.value(u"payload"_q).toString().toUtf8();
	const auto signature = QByteArray::fromBase64(
		object.value(u"sig"_q).toString().toUtf8());
	if (payload.isEmpty() || !VerifySignature(payload, signature)) {
		return false;
	}
	const auto inner = QJsonDocument::fromJson(payload);
	if (!inner.isObject()) {
		return false;
	}
	const auto data = inner.object();
	if (data.value(u"v"_q).toInt() != 1) {
		return false;
	}
	auto &state = Get();
	const auto version = qint64(data.value(u"version"_q).toDouble(-1));
	if (version < state.version) {
		return false;
	}
	auto badges = std::map<uint64, CustomBadge>();
	const auto list = data.value(u"badges"_q).toObject();
	for (auto i = list.begin(); i != list.end(); ++i) {
		auto ok = false;
		const auto id = i.key().toULongLong(&ok);
		if (!ok || !id || !i.value().isObject()) {
			continue;
		}
		const auto entry = i.value().toObject();
		const auto label = (entry.value(u"kind"_q).toString() == u"label"_q);
		auto text = entry.value(label ? u"label"_q : u"emoji"_q).toString();
		if (text.isEmpty()) {
			continue;
		}
		auto color = QColor(entry.value(u"color"_q).toString());
		if (!color.isValid()) {
			color = QColor(QLatin1String(kDefaultColor));
		}
		badges.emplace(id, CustomBadge{
			.text = std::move(text),
			.tooltip = entry.value(u"tooltip"_q).toString(),
			.color = color,
			.label = label,
		});
	}
	const auto changed = (state.badges != badges);
	state.version = version;
	if (changed) {
		state.badges = std::move(badges);
		NotifyChanged();
	}
	return true;
}

void SaveCache(const QByteArray &body) {
	QDir().mkpath(cWorkingDir() + u"tdata"_q);
	auto file = QFile(CachePath());
	if (file.open(QIODevice::WriteOnly)) {
		file.write(body);
	}
}

void LoadCache() {
	auto file = QFile(CachePath());
	if (file.size() > 0
		&& file.size() <= kMaxBodySize
		&& file.open(QIODevice::ReadOnly)) {
		[[maybe_unused]] const auto ok = Apply(file.readAll());
	}
}

void Fetch() {
	auto &state = Get();
	if (state.reply || !ShowBadges()) {
		return;
	}
	if (!state.network) {
		state.network = std::make_unique<QNetworkAccessManager>();
	}
	auto request = QNetworkRequest(QUrl(QLatin1String(kBadgesUrl)));
	request.setTransferTimeout(kRequestTimeout);
	request.setRawHeader("Accept", "application/json");
	if (!state.etag.isEmpty()) {
		request.setRawHeader("If-None-Match", state.etag);
	}
	const auto reply = state.network->get(request);
	state.reply = reply;
	QObject::connect(reply, &QNetworkReply::finished, reply, [=] {
		auto &current = Get();
		current.reply = nullptr;
		const auto status = reply->attribute(
			QNetworkRequest::HttpStatusCodeAttribute).toInt();
		if (reply->error() == QNetworkReply::NoError && status == 200) {
			const auto body = reply->read(kMaxBodySize + 1);
			if (body.size() <= kMaxBodySize && Apply(body)) {
				current.etag = reply->rawHeader("ETag");
				SaveCache(body);
			}
		}
		// On any failure keep the last good list.
		reply->deleteLater();
	});
}

} // namespace

void StartBadges() {
	auto &state = Get();
	if (state.started) {
		return;
	}
	state.started = true;
	LoadCache();
	state.timer.callEach(kRefreshEvery);
	Fetch();
}

void BadgesSettingChanged() {
	NotifyChanged();
	if (ShowBadges()) {
		Fetch();
	}
}

std::optional<CustomBadge> LookupBadge(uint64 userId) {
	if (!ShowBadges()) {
		return std::nullopt;
	}
	const auto &badges = Get().badges;
	const auto i = badges.find(userId);
	return (i != badges.end())
		? std::make_optional(i->second)
		: std::nullopt;
}

rpl::producer<int> BadgesVersionValue() {
	auto &state = Get();
	return state.changed.events_starting_with(int(state.counter));
}

} // namespace YooGram
