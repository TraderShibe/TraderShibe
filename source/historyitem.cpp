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

#include "historyitem.h"
#include "main.h"

HistoryItem::HistoryItem()
{
	displayFullDate=false;
	dateTimeInt=0;
	volume=0.0;
	price=0.0;
	total=0.0;
	type=0;
}

void HistoryItem::cacheStrings()
{
	QDateTime cachedDateTime=QDateTime::fromTime_t(dateTimeInt);
	dateTimeStr=cachedDateTime.toString(baseValues.dateTimeFormat);
	timeStr=cachedDateTime.toString(baseValues.timeFormat);
	cachedDateTime.setTime(QTime(0,0,0,0));
	dateInt=cachedDateTime.toTime_t();

	QString usdSign=baseValues.currencyMap.value(symbol.right(3),CurencyInfo("$")).sign;
	if(price>0.0)priceStr=usdSign+mainWindow.numFromDouble(price);
	if(volume>0.0)
	{
		volumeStr=baseValues.currencyMap.value(symbol.left(3),CurencyInfo("BTC")).sign+mainWindow.numFromDouble(volume);
	}
	if(volume>0.0&&price>0.0)
	{
		QString totalStrDown=mainWindow.numFromDouble(mainWindow.getValidDoubleForPercision(price*volume,8,false));
		QString totalStrUp=mainWindow.numFromDouble(mainWindow.getValidDoubleForPercision(price*volume,8,true));
		totalStr=usdSign+(totalStrDown.length()>totalStrUp.length()?totalStrUp:totalStrDown);

		if(!baseValues.forceDotInSpinBoxes)
		{
			priceStr.replace(".",",");
			volumeStr.replace(".",",");
			totalStr.replace(".",",");
		}
	}
}

bool HistoryItem::isValid()
{
	bool valid=dateTimeInt>0&&symbol.size()==6;
	if(valid)cacheStrings();
	return valid;
}