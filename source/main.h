//  This file is part of Qt Bitcion Trader
//      https://github.com/ShibeShibe/Trader Shibe
//  Copyright (C) 2013-2014 Shibe Shibe <ShibeShibe@gmail.com>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  In addition, as a special exception, the copyright holders give
//  permission to link the code of portions of this program with the
//  OpenSSL library under certain conditions as described in each
//  individual source file, and distribute linked combinations including
//  the two.
//
//  You must obey the GNU General Public License in all respects for all
//  of the code used other than OpenSSL. If you modify file(s) with this
//  exception, you may extend this exception to your version of the
//  file(s), but you are not obligated to do so. If you do not wish to do
//  so, delete this exception statement from your version. If you delete
//  this exception statement from all source files in the program, then
//  also delete it here.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef MAIN_H
#define MAIN_H
//#include <QDebug>////////////
//#define showDebug(text) qDebug()<<QTime::currentTime().toString(localTimeFormat)+"> "+text//////////
#include "qtextstream.h"
#include <QFontMetrics>
#include "Shibetranslator.h"
#include "logthread.h"

#define USE_QTMULTIMEDIA

#define textFontWidth(text) baseValues_->fontMetrics_->width(text)
#define debugLevel (baseValues_->debugLevel_)
#define appDataDir (baseValues_->appDataDir_)
#define grouped (baseValues_->groupPriceValue>0.0?2:0)
#define mainWindow (*baseValues_->mainWindow_)
#define logThread (baseValues_->logThread_)

#define ShibeTr baseValues_->ShibeTranslator_.translateString
#define ShibeTranslator baseValues_->ShibeTranslator_
#define currentExchange baseValues_->currentExchange_

#define defaultHeightForRow baseValues_->defaultHeightForRow_
#define upArrowStr baseValues_->upArrow
#define downArrowStr baseValues_->downArrow
#define upArrowNoUtfStr baseValues_->upArrowNoUtf8
#define downArrowNoUtfStr baseValues_->downArrowNoUtf8

#include "Trader Shibe.h"

#define hmacSha512(key, baseString) QByteArray(reinterpret_cast<const char *>(HMAC(EVP_sha512(),key.constData(), key.size(), reinterpret_cast<const unsigned char *>(baseString.constData()), baseString.size(), 0, 0)),64)
#define hmacSha384(key, baseString) QByteArray(reinterpret_cast<const char *>(HMAC(EVP_sha384(),key.constData(), key.size(), reinterpret_cast<const unsigned char *>(baseString.constData()), baseString.size(), 0, 0)),48)
#define hmacSha256(key, baseString) QByteArray(reinterpret_cast<const char *>(HMAC(EVP_sha256(),key.constData(), key.size(), reinterpret_cast<const unsigned char *>(baseString.constData()), baseString.size(), 0, 0)),32)
#define hmacSha1(key, baseString) QByteArray(reinterpret_cast<const char *>(HMAC(EVP_sha1(),key.constData(), key.size(), reinterpret_cast<const unsigned char *>(baseString.constData()), baseString.size(), 0, 0)),20)

#include "currencyinfo.h"
#include "apptheme.h"

class Exchange;

#include "currencypairitem.h"

struct BaseValues
{
	void Construct();

	QString osStyle;
	int lastGroupID;
	bool forceDotInSpinBoxes;

	int trafficSpeed;
	qint64 trafficTotal;
	int trafficTotalType;

	CurrencyPairItem currentPair;

	bool nightMode;
	bool rulesSafeMode;
	int rulesSafeModeInterval;
	
	QByteArray upArrow;
	QByteArray downArrow;
	QByteArray upArrowNoUtf8;
	QByteArray downArrowNoUtf8;

	QString customUserAgent;
	QString customCookies;
	bool gzipEnabled;
	AppTheme appThemeLight;
	AppTheme appThemeDark;
	AppTheme appTheme;
	int debugLevel_;//0: Disabled; 1: Debug; 2: Log
	bool supportsUtfUI;
	bool highResolutionDisplay;
	int defaultHeightForRow_;
	double groupPriceValue;
	QFontMetrics *fontMetrics_;
	int apiDownCount;
	int uiUpdateInterval;
	QByteArray depthCountLimitStr;
	int depthCountLimit;
	int httpRetryCount;
	bool httpSplitPackets;
	int httpRequestInterval;
	int httpRequestTimeout;
	Exchange *currentExchange_;
	QString exchangeName;
	QString timeFormat;
	QString dateTimeFormat;
	QString defaultLangFile;
	ShibeTranslator ShibeTranslator_;
	QByteArray appDataDir_;
	QByteArray appVerStr;
	LogThread *logThread_;
	QByteArray restKey;
	QByteArray restSign;
	QByteArray randomPassword;
	Trader Shibe *mainWindow_;
	QString logFileName;
	QString iniFileName;
	double appVerReal;
	double appVerLastReal;
	bool appVerIsBeta;
	QMap<QByteArray,QByteArray> currencyMapSign;
	QMap<QByteArray,CurencyInfo> currencyMap;
};

#define baseValues (*baseValues_)
extern BaseValues *baseValues_;

#endif // MAIN_H
