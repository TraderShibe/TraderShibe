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

#include "tradesitem.h"
#include "main.h"

TradesItem::TradesItem()
{
	backGray=false;
	displayFullDate=false;
	date=0;
	amount=0.0;
	price=0.0;
	total=0.0;
	orderType=0;
	direction=0;
}

void TradesItem::cacheStrings()
{
	QDateTime itemDate=QDateTime::fromTime_t(date);
	timeStr=itemDate.toString(baseValues.timeFormat);
	dateStr=itemDate.toString(baseValues.dateTimeFormat);
	if(price>0.0)priceStr=mainWindow.numFromDouble(price);
	if(amount>0.0)amountStr=mainWindow.numFromDouble(amount);
	if(amount>0.0&&price>0.0)totalStr=QString::number(price*amount,'f',baseValues.currentPair.currBDecimals);
}

bool TradesItem::isValid()
{
	bool valid=date>0&&price>0.0&&amount>0.0;
	if(valid)cacheStrings();
	return valid;
}