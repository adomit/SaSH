﻿module;

#include <QString>
#include <QDateTime>
#include <QLocale>
#include <QTimeZone>

export module Compile;

namespace compile
{
	constexpr const char* global_date(__DATE__);
	constexpr const char* global_time(__TIME__);

	static QDateTime g_buildDate;

	static const QString initializeBuildDateTime();

	static const QString g_buildDateTime = initializeBuildDateTime();

	static const QString initializeBuildDateTime()
	{
		QString dateTimeStr(global_date);
		dateTimeStr.replace("  ", " 0");// 注意" "是兩個空格，用於日期為單數時需要轉成“空格+0”

		// 創建 QDateTime 對象以解析日期部分
		const QLocale l(QLocale::English);
		const QDateTime d(l.toDateTime(dateTimeStr, "MMM dd yyyy"));

		// 構建日期時間字符串
		const QString str(QString("%1 %2").arg(d.toString("yyyy-MM-dd")).arg(global_time));

		QDateTime dt(QDateTime::fromString(str, "yyyy-MM-dd hh:mm:ss"));

		static const QTimeZone pacificTimeZone("America/Los_Angeles");
		dt.setTimeZone(pacificTimeZone);

		static const QTimeZone beijingTimeZone("Asia/Shanghai");
		dt = dt.toTimeZone(beijingTimeZone);

		g_buildDate = dt;

		return dt.toString("yyyyMMdd-hh:mm:ss");
	}

	export const QString buildDateTime(QDateTime* date)
	{
		if (date != nullptr && g_buildDate.isValid())
			*date = g_buildDate;
		return g_buildDateTime;
	}
}