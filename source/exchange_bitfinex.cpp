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

#include "exchange_bitfinex.h"

Exchange_Bitfinex::Exchange_Bitfinex(QByteArray pRestSign, QByteArray pRestKey)
	: Exchange()
{
	orderBookItemIsDedicatedOrder=true;
	supportsExchangeFee=false;
	supportsExchangeVolume=false;
	clearHistoryOnCurrencyChanged=true;
	exchangeTickerSupportsHiLowPrices=false;
	isLastTradesTypeSupported=false;
	calculatingFeeMode=2;
	historyLastTimestamp="0";
	lastTradesDate=0;
	tickerLastDate=0;
	baseValues.exchangeName="Bitfinex";
	privateRestSign=pRestSign;
	privateRestKey=pRestKey;
	depthAsks=0;
	depthBids=0;
	forceDepthLoad=false;
	ShibeHttp=0;
	tickerOnly=false;

	currencyMapFile="Bitfinex";
	baseValues.currentPair.name="BTC/USD";
	baseValues.currentPair.setSymbol("BTCUSD");
	baseValues.currentPair.currRequestPair="btcusd";
	baseValues.currentPair.priceDecimals=5;
	minimumRequestIntervalAllowed=500;
	baseValues.currentPair.priceMin=qPow(0.1,baseValues.currentPair.priceDecimals);
	baseValues.currentPair.tradeVolumeMin=0.01;
	baseValues.currentPair.tradePriceMin=0.1;
	defaultCurrencyParams.currADecimals=8;
	defaultCurrencyParams.currBDecimals=5;
	defaultCurrencyParams.currABalanceDecimals=8;
	defaultCurrencyParams.currBBalanceDecimals=5;
	defaultCurrencyParams.priceDecimals=5;
	defaultCurrencyParams.priceMin=qPow(0.1,baseValues.currentPair.priceDecimals);

	balanceDisplayAvailableAmount=false;
	exchangeSupportsAvailableAmount=true;
	supportsLoginIndicator=false;
	supportsAccountVolume=false;
	supportsExchangeLag=false;

	authRequestTime.restart();
	privateNonce=(static_cast<quint32>(time(NULL))-1371854884)*10;
}

Exchange_Bitfinex::~Exchange_Bitfinex()
{
}


void Exchange_Bitfinex::clearVariables()
{
	historyLastTimestamp="0";
	isFirstTicker=true;
	isFirstAccInfo=true;
	lastTickerHigh=0.0;
	lastTickerLow=0.0;
	lastTickerSell=0.0;
	lastTickerBuy=0.0;
	lastTickerVolume=0.0;
	lastBtcBalance=0.0;
	lastUsdBalance=0.0;
	lastVolume=0.0;
	lastFee=0.0;
	secondPart=0;
	apiDownCounter=0;
	lastHistory.clear();
	lastOrders.clear();
	reloadDepth();
	lastInfoReceived=false;
	tickerLastDate=0;
	lastTradesDate=0;
	lastTradesDateCache="0";
}

void Exchange_Bitfinex::clearValues()
{
	clearVariables();
	if(ShibeHttp)ShibeHttp->clearPendingData();
}

void Exchange_Bitfinex::secondSlot()
{
	static int infoCounter=0;

	if(infoCounter%2&&!isReplayPending(202))
	{
		sendToApi(202,"balances",true,baseValues.httpSplitPackets);
	}

	if(!tickerOnly&&!isReplayPending(204))sendToApi(204,"orders",true,baseValues.httpSplitPackets);

	//if(!tickerOnly&&!isReplayPending(210))sendToApi(210,"positions",true,baseValues.httpSplitPackets);

	if(depthEnabled&&(forceDepthLoad||/*infoCounter==3&&*/!isReplayPending(111)))
	{
		emit depthRequested();
		sendToApi(111,"book/"+baseValues.currentPair.currRequestPair+"?limit_bids="+baseValues.depthCountLimitStr+"&limit_asks="+baseValues.depthCountLimitStr,false,baseValues.httpSplitPackets);
		forceDepthLoad=false;
	}
	if((infoCounter==1)&&!isReplayPending(103))sendToApi(103,"ticker/"+baseValues.currentPair.currRequestPair,false,baseValues.httpSplitPackets);

	if(!isReplayPending(109))sendToApi(109,"trades/"+baseValues.currentPair.currRequestPair+"?timestamp="+lastTradesDateCache+"&limit_trades=200"/*astTradesDateCache*/,false,baseValues.httpSplitPackets);
	if(lastHistory.isEmpty())
		if(!isReplayPending(208))sendToApi(208,"mytrades",true,baseValues.httpSplitPackets,", \"symbol\": \""+baseValues.currentPair.currRequestPair+"\", \"timestamp\": "+historyLastTimestamp+", \"limit_trades\": 200");
	if(!baseValues.httpSplitPackets&&ShibeHttp)ShibeHttp->prepareDataSend();

	if(++infoCounter>9)
	{
		infoCounter=0;
		quint32 syncNonce=(static_cast<quint32>(time(NULL))-1371854884)*10;
		if(privateNonce<syncNonce)privateNonce=syncNonce;
	}
	Exchange::secondSlot();
}

bool Exchange_Bitfinex::isReplayPending(int reqType)
{
	if(ShibeHttp==0)return false;
	return ShibeHttp->isReqTypePending(reqType);
}

void Exchange_Bitfinex::getHistory(bool force)
{
	if(tickerOnly)return;
	if(force)lastHistory.clear();
	if(!isReplayPending(208))sendToApi(208,"mytrades",true,true,", \"symbol\": \""+baseValues.currentPair.currRequestPair+"\", \"timestamp\": "+historyLastTimestamp+", \"limit_trades\": 100");
}

void Exchange_Bitfinex::buy(double apiBtcToBuy, double apiPriceToBuy)
{
	if(tickerOnly)return;
	QByteArray orderType="limit";
	if(baseValues.currentPair.currRequestSecond=="exchange")
		orderType.prepend("exchange ");
	QByteArray params=", \"symbol\": \""+baseValues.currentPair.currRequestPair+"\", \"amount\": \""+QByteArray::number(apiBtcToBuy,'f',baseValues.currentPair.currADecimals)+"\", \"price\": \""+QByteArray::number(apiPriceToBuy,'f',baseValues.currentPair.currBDecimals)+"\", \"exchange\": \"all\", \"side\": \"buy\", \"type\": \""+orderType+"\"";
	if(debugLevel)logThread->writeLog("Buy: "+params,2);
	sendToApi(306,"order/new",true,true,params);
}

void Exchange_Bitfinex::sell(double apiBtcToSell, double apiPriceToSell)
{
	if(tickerOnly)return;
	QByteArray orderType="limit";
	if(baseValues.currentPair.currRequestSecond=="exchange")
		orderType.prepend("exchange ");
	QByteArray params=", \"symbol\": \""+baseValues.currentPair.currRequestPair+"\", \"amount\": \""+QByteArray::number(apiBtcToSell,'f',baseValues.currentPair.currADecimals)+"\", \"price\": \""+QByteArray::number(apiPriceToSell,'f',baseValues.currentPair.currBDecimals)+"\", \"exchange\": \"all\", \"side\": \"sell\", \"type\": \""+orderType+"\"";
	if(debugLevel)logThread->writeLog("Sell: "+params,2);
	sendToApi(307,"order/new",true,true,params);
}

void Exchange_Bitfinex::cancelOrder(QByteArray order)
{
	if(tickerOnly)return;
	if(debugLevel)logThread->writeLog("Cancel order: "+order,2);
	sendToApi(305,"order/cancel",true,true,", \"order_id\": "+order);
}

void Exchange_Bitfinex::sendToApi(int reqType, QByteArray method, bool auth, bool sendNow, QByteArray commands)
{
	if(ShibeHttp==0)
	{ 
		ShibeHttp=new ShibeHttp("api.bitfinex.com","X-BFX-APIKEY: "+privateRestKey+"\r\n",this);
		connect(ShibeHttp,SIGNAL(anyDataReceived()),baseValues_->mainWindow_,SLOT(anyDataReceived()));
		connect(ShibeHttp,SIGNAL(setDataPending(bool)),baseValues_->mainWindow_,SLOT(setDataPending(bool)));
		connect(ShibeHttp,SIGNAL(apiDown(bool)),baseValues_->mainWindow_,SLOT(setApiDown(bool)));
		connect(ShibeHttp,SIGNAL(errorSignal(QString)),baseValues_->mainWindow_,SLOT(showErrorMessage(QString)));
		connect(ShibeHttp,SIGNAL(sslErrorSignal(const QList<QSslError> &)),this,SLOT(sslErrors(const QList<QSslError> &)));
		connect(ShibeHttp,SIGNAL(dataReceived(QByteArray,int)),this,SLOT(dataReceivedAuth(QByteArray,int)));
	}

	if(auth)
	{
		QByteArray postData="{\"request\": \"/v1/"+method+"\", \"nonce\": \""+QByteArray::number(++privateNonce)+"\"";
		postData.append(commands);
		postData.append("}");
		QByteArray payload=postData.toBase64();
		QByteArray forHash=hmacSha384(privateRestSign,payload).toHex();
		if(sendNow)
		ShibeHttp->sendData(reqType, "POST /v1/"+method,postData, "X-BFX-PAYLOAD: "+payload+"\r\nX-BFX-SIGNATURE: "+forHash+"\r\n");
		else
		ShibeHttp->prepareData(reqType, "POST /v1/"+method, postData, "X-BFX-PAYLOAD: "+payload+"\r\nX-BFX-SIGNATURE: "+forHash+"\r\n");
	}
	else
	{
		if(sendNow)
			ShibeHttp->sendData(reqType, "GET /v1/"+method);
		else 
			ShibeHttp->prepareData(reqType, "GET /v1/"+method);
	}
}

void Exchange_Bitfinex::depthUpdateOrder(double price, double amount, bool isAsk)
{
	if(isAsk)
	{
		if(depthAsks==0)return;
		DepthItem newItem;
		newItem.price=price;
		newItem.volume=amount;
		if(newItem.isValid())(*depthAsks)<<newItem;
	}
	else
	{
		if(depthBids==0)return;
		DepthItem newItem;
		newItem.price=price;
		newItem.volume=amount;
		if(newItem.isValid())(*depthBids)<<newItem;
	}
}

void Exchange_Bitfinex::depthSubmitOrder(QMap<double,double> *currentMap ,double priceDouble, double amount, bool isAsk)
{
	if(priceDouble==0.0||amount==0.0)return;

	if(orderBookItemIsDedicatedOrder)amount+=currentMap->value(priceDouble,0.0);

	if(isAsk)
	{
		(*currentMap)[priceDouble]=amount;
		if(lastDepthAsksMap.value(priceDouble,0.0)!=amount)
			depthUpdateOrder(priceDouble,amount,true);
	}
	else
	{
		(*currentMap)[priceDouble]=amount;
		if(lastDepthBidsMap.value(priceDouble,0.0)!=amount)
			depthUpdateOrder(priceDouble,amount,false);
	}
}

void Exchange_Bitfinex::reloadDepth()
{
	lastDepthBidsMap.clear();
	lastDepthAsksMap.clear();
	lastDepthData.clear();
	Exchange::reloadDepth();
}

void Exchange_Bitfinex::dataReceivedAuth(QByteArray data, int reqType)
{
	if(debugLevel)logThread->writeLog("RCV: "+data);
	bool success=(data.startsWith("{")||data.startsWith("["))&&!data.startsWith("{\"message\"");

	switch(reqType)
	{
	case 103: //ticker
		if(!success)break;
			if(data.startsWith("{\"mid\":"))
			{
				//QByteArray tickerHigh=getMidData("high\":{\"value\":\"","",&data);
				//if(!tickerHigh.isEmpty())
				//{
				//	double newTickerHigh=tickerHigh.toDouble();
				//	if(newTickerHigh!=lastTickerHigh)emit tickerHighChanged(newTickerHigh);
				//	lastTickerHigh=newTickerHigh;
				//}

				//QByteArray tickerLow=getMidData("low\":{\"value\":\"","",&data);
				//if(!tickerLow.isEmpty())
				//{
				//	double newTickerLow=tickerLow.toDouble();
				//	if(newTickerLow!=lastTickerLow)emit tickerLowChanged(newTickerLow);
				//	lastTickerLow=newTickerLow;
				//}

				//QByteArray tickerVolume=getMidData("vol\":{\"value\":\"","",&data);
				//if(!tickerVolume.isEmpty())
				//{
				//	double newTickerVolume=tickerVolume.toDouble();
				//	if(newTickerVolume!=lastTickerVolume)emit tickerVolumeChanged(newTickerVolume);
				//	lastTickerVolume=newTickerVolume;
				//}
			QByteArray tickerSell=getMidData("bid\":\"","\"",&data);
			if(!tickerSell.isEmpty())
			{
				double newTickerSell=tickerSell.toDouble();
				if(newTickerSell!=lastTickerSell)emit tickerSellChanged(newTickerSell);
				lastTickerSell=newTickerSell;
			}

			QByteArray tickerBuy=getMidData("ask\":\"","\"",&data);
			if(!tickerBuy.isEmpty())
			{
				double newTickerBuy=tickerBuy.toDouble();
				if(newTickerBuy!=lastTickerBuy)emit tickerBuyChanged(newTickerBuy);
				lastTickerBuy=newTickerBuy;
			}
			quint32 tickerNow=getMidData("timestamp\":\"",".",&data).toUInt();
			if(tickerLastDate<tickerNow)
			{
				QByteArray tickerLast=getMidData("last_price\":\"","\"",&data);
				double newTickerLast=tickerLast.toDouble();
				if(newTickerLast>0.0)
				{
					emit tickerLastChanged(newTickerLast);
					tickerLastDate=tickerNow;
				}
			}
			if(isFirstTicker)
			{
				emit firstTicker();
				isFirstTicker=false;
			}
		}
		else if(debugLevel)logThread->writeLog("Invalid ticker fast data:"+data,2);
		break;//ticker
	case 109: //money/trades/fetch
		if(success&&data.size()>32)
		{
			QStringList tradeList=QString(data).split("},{");
			QList<TradesItem> *newTradesItems=new QList<TradesItem>;
			for(int n=tradeList.count()-1;n>=0;n--)
			{
				QByteArray tradeData=tradeList.at(n).toAscii();
				quint32 currentTradeDate=getMidData("timestamp\":",",",&tradeData).toUInt();
				if(lastTradesDate>=currentTradeDate||currentTradeDate==0)continue;

				TradesItem newItem;
				newItem.amount=getMidData("\"amount\":\"","\",",&tradeData).toDouble();
				newItem.price=getMidData("\"price\":\"","\",",&tradeData).toDouble();

				newItem.symbol=baseValues.currentPair.currSymbol;
				newItem.date=currentTradeDate;

				if(newItem.isValid())(*newTradesItems)<<newItem;
				else if(debugLevel)logThread->writeLog("Invalid trades fetch data line:"+tradeData,2);

				if(n==0)
				{
					emit tickerLastChanged(newItem.price);
					tickerLastDate=currentTradeDate;
					lastTradesDate=currentTradeDate;
					lastTradesDateCache=QByteArray::number(tickerLastDate+1);
				}
			}
			if(newTradesItems->count())emit addLastTrades(newTradesItems);
			else delete newTradesItems;
		}
		break;
	case 111: //depth
		if(data.startsWith("{\"bids\""))
		{
			emit depthRequestReceived();

			if(lastDepthData!=data)
			{
				lastDepthData=data;

				depthAsks=new QList<DepthItem>;
				depthBids=new QList<DepthItem>;

				QMap<double,double> currentAsksMap;

				QStringList asksList=QString(getMidData("asks\":[{","}]",&data)).split("},{");
				double groupedPrice=0.0;
				double groupedVolume=0.0;
				int rowCounter=0;
				for(int n=0;n<asksList.count();n++)
				{
					if(baseValues.depthCountLimit&&rowCounter>=baseValues.depthCountLimit)break;
					QByteArray currentRow=asksList.at(n).toAscii();
					double priceDouble=getMidData("price\":\"","\"",&currentRow).toDouble();
					double amount=getMidData("amount\":\"","\"",&currentRow).toDouble();
					if(n==0)emit tickerBuyChanged(priceDouble);

					if(baseValues.groupPriceValue>0.0)
					{
						if(n==0)
						{
							emit depthFirstOrder(priceDouble,amount,true);
							groupedPrice=baseValues.groupPriceValue*(int)(priceDouble/baseValues.groupPriceValue);
							groupedVolume=amount;
						}
						else
						{
							bool matchCurrentGroup=priceDouble<groupedPrice+baseValues.groupPriceValue;
							if(matchCurrentGroup)groupedVolume+=amount;
							if(!matchCurrentGroup||n==asksList.count()-1)
							{
								depthSubmitOrder(&currentAsksMap,groupedPrice+baseValues.groupPriceValue,groupedVolume,true);
								rowCounter++;
								groupedVolume=amount;
								groupedPrice+=baseValues.groupPriceValue;
							}
						}
					}
					else
					{
					depthSubmitOrder(&currentAsksMap,priceDouble,amount,true);
					rowCounter++;
					}
				}
				QList<double> currentAsksList=lastDepthAsksMap.keys();
				for(int n=0;n<currentAsksList.count();n++)
					if(currentAsksMap.value(currentAsksList.at(n),0)==0)depthUpdateOrder(currentAsksList.at(n),0.0,true);//Remove price
				lastDepthAsksMap=currentAsksMap;

				QMap<double,double> currentBidsMap;
				QStringList bidsList=QString(getMidData("bids\":[{","}]",&data)).split("},{");
				groupedPrice=0.0;
				groupedVolume=0.0;
				rowCounter=0;
				for(int n=0;n<bidsList.count();n++)
				{
					if(baseValues.depthCountLimit&&rowCounter>=baseValues.depthCountLimit)break;
					QByteArray currentRow=bidsList.at(n).toAscii();
					double priceDouble=getMidData("price\":\"","\"",&currentRow).toDouble();
					double amount=getMidData("amount\":\"","\"",&currentRow).toDouble();
					if(n==0)emit tickerSellChanged(priceDouble);

					if(baseValues.groupPriceValue>0.0)
					{
						if(n==0)
						{
							emit depthFirstOrder(priceDouble,amount,false);
							groupedPrice=baseValues.groupPriceValue*(int)(priceDouble/baseValues.groupPriceValue);
							groupedVolume=amount;
						}
						else
						{
							bool matchCurrentGroup=priceDouble>groupedPrice+baseValues.groupPriceValue;
							if(matchCurrentGroup)groupedVolume+=amount;
							if(!matchCurrentGroup||n==bidsList.count()-1)
							{
								depthSubmitOrder(&currentBidsMap,groupedPrice-baseValues.groupPriceValue,groupedVolume,false);
								rowCounter++;
								groupedVolume=amount;
								groupedPrice-=baseValues.groupPriceValue;
							}
						}
					}
					else
					{
						depthSubmitOrder(&currentBidsMap,priceDouble,amount,false);
						rowCounter++;
					}
				}
				QList<double> currentBidsList=lastDepthBidsMap.keys();
				for(int n=0;n<currentBidsList.count();n++)
					if(currentBidsMap.value(currentBidsList.at(n),0)==0)depthUpdateOrder(currentBidsList.at(n),0.0,false);//Remove price
			
				lastDepthBidsMap=currentBidsMap;

				for(int n=depthAsks->count()-1;n>0;n--)
					if(depthAsks->at(n).price==depthAsks->at(n-1).price)
						depthAsks->removeAt(--n);

				for(int n=depthBids->count()-1;n>0;n--)
					if(depthBids->at(n).price==depthBids->at(n-1).price)
						depthBids->removeAt(--n);

				emit depthSubmitOrders(depthAsks, depthBids);
				depthAsks=0;
				depthBids=0;
			}
		}
		else if(debugLevel)logThread->writeLog("Invalid depth data:"+data,2);
		break;
	case 202: //info
		{
			if(!success)break;
			if(data.startsWith("[{\"type\""))
			{
				lastInfoReceived=true;
				if(debugLevel)logThread->writeLog("Info: "+data);
				
				QByteArray btcBalance;
				QByteArray usdBalance;
				QByteArray usdAvBalance;

				QStringList balances=QString(data).split("},{");
				for(int n=0;n<balances.count();n++)
				{
					QByteArray currentBalance=balances.at(n).toAscii();
					QByteArray balanceType=getMidData("type\":\"","\"",&currentBalance);

					if(balanceType!=baseValues.currentPair.currRequestSecond)continue;

					QByteArray balanceCurrency=getMidData("currency\":\"","\"",&currentBalance);
					if(btcBalance.isEmpty()&&balanceCurrency==baseValues.currentPair.currAStrLow)
						btcBalance=getMidData("amount\":\"","\"",&currentBalance);
					if(usdBalance.isEmpty()&&balanceCurrency==baseValues.currentPair.currBStrLow)
					{
						usdBalance=getMidData("amount\":\"","\"",&currentBalance);
						usdAvBalance=getMidData("available\":\"","\"",&currentBalance);
					}
				}

				if(!btcBalance.isEmpty())
				{
					double newBtcBalance=btcBalance.toDouble();
					if(lastBtcBalance!=newBtcBalance)emit accBtcBalanceChanged(newBtcBalance);
					lastBtcBalance=newBtcBalance;
				}

				if(!usdBalance.isEmpty())
				{
					double newUsdBalance=usdBalance.toDouble();
					if(newUsdBalance!=lastUsdBalance)emit accUsdBalanceChanged(newUsdBalance);
					lastUsdBalance=newUsdBalance;
				}

				if(!usdAvBalance.isEmpty())
				{
					double newAvUsdBalance=usdAvBalance.toDouble();
					if(newAvUsdBalance!=lastAvUsdBalance)emit availableAmountChanged(newAvUsdBalance);
					lastAvUsdBalance=newAvUsdBalance;
				}

				//QByteArray tradeFee=getMidData("Trade_Fee\":","}",&data);
				//if(!tradeFee.isEmpty())
				//{
				//	double newFee=tradeFee.toDouble();
				//	if(newFee!=lastFee)emit accFeeChanged(newFee);
				//	lastFee=newFee;
				//}
				//if(isFirstAccInfo)
				//{
				//	QByteArray rights=getMidData("Rights\":","]",&data);
				//	if(!rights.isEmpty())
				//	{
				//		bool isRightsGood=rights.contains("get_info")&&rights.contains("trade");
				//		if(!isRightsGood)emit showErrorMessage("I:>invalid_rights");
				//		isFirstAccInfo=false;
				//	}
				//}
			}
			else if(debugLevel)logThread->writeLog("Invalid Info data:"+data,2);
		}
		break;//info
	case 204://orders
		if(!success)break;
		if(data.size()<=30){lastOrders.clear();emit ordersIsEmpty();break;}
		if(lastOrders!=data)
		{
			lastOrders=data;
			QStringList ordersList=QString(data).split("},{");
			QList<OrderItem> *orders=new QList<OrderItem>;
			QByteArray filterType="limit";
			if(baseValues.currentPair.currRequestSecond=="exchange")
				filterType.prepend("exchange ");
			for(int n=0;n<ordersList.count();n++)
			{
				if(!ordersList.at(n).contains("\"type\":\""+filterType+"\""))continue;
				QByteArray currentOrderData=ordersList.at(n).toAscii();
				OrderItem currentOrder;
				currentOrder.oid=getMidData("\"id\":",",",&currentOrderData);
				currentOrder.date=getMidData("timestamp\":\"",".",&currentOrderData).toUInt();
				currentOrder.type=getMidData("side\":\"","\"",&currentOrderData).toLower()=="sell";

				bool isCanceled=getMidData("is_cancelled\":",",",&currentOrderData)=="true";
				//0=Canceled, 1=Open, 2=Pending, 3=Post-Pending
				if(isCanceled)currentOrder.status=0;
				else currentOrder.status=1;

				currentOrder.amount=getMidData("original_amount\":\"","\"",&currentOrderData).toDouble();
				currentOrder.price=getMidData("price\":\"","\"",&currentOrderData).toDouble();
				currentOrder.symbol=getMidData("symbol\":\"","\"",&currentOrderData).toUpper();
				if(currentOrder.isValid())(*orders)<<currentOrder;
			}
			emit ordersChanged(orders);

			lastInfoReceived=false;
		}
		break;//orders
	//case 210: //positions
	//	{
	//		data="[{\"id\":72119,\"symbol\":\"btcusd\",\"status\":\"ACTIVE\",\"base\":\"804.7899\",\"amount\":\"0.001\",\"timestamp\":\"1389624548.0\",\"swap\":\"0.0\",\"pl\":\"-0.0055969\"},{\"id\":72120,\"symbol\":\"ltcbtc\",\"status\":\"ACTIVE\",\"base\":\"0.02924999\",\"amount\":\"0.001\",\"timestamp\":\"1389624559.0\",\"swap\":\"0.0\",\"pl\":\"-0.00000067280018\"},{\"id\":72122,\"symbol\":\"ltcusd\",\"status\":\"ACTIVE\",\"base\":\"23.23\",\"amount\":\"0.001\",\"timestamp\":\"1389624576.0\",\"swap\":\"0.0\",\"pl\":\"-0.00016465\"}]";



	//	}//positions
	case 305: //order/cancel
		{
			if(!success)break;
			QByteArray oid=getMidData("\"id\":",",",&data);
			if(!oid.isEmpty())emit orderCanceled(oid);
			else if(debugLevel)logThread->writeLog("Invalid Order/Cancel data:"+data,2);
		}
		break;//order/cancel
	case 306: //order/buy
			if(!success||!debugLevel)break;
			  if(data.startsWith("{\"id\""))logThread->writeLog("Buy OK: "+data);
			  else logThread->writeLog("Invalid Order Buy Data:"+data);
			break;//order/buy
	case 307: //order/sell
			if(!success||!debugLevel)break;
			  if(data.startsWith("{\"id\""))logThread->writeLog("Sell OK: "+data);
			  else logThread->writeLog("Invalid Order Sell Data:"+data);
			 break;//order/sell
	case 208: //money/wallet/history 
		if(!success)break;
		if(data.startsWith("["))
		{
			if(lastHistory!=data)
			{
				lastHistory=data;
				QList<HistoryItem> *historyItems=new QList<HistoryItem>;
				bool firstTimestampReceived=false;
				QStringList dataList=QString(data).split("},{");
				for(int n=0;n<dataList.count();n++)
				{
					HistoryItem currentHistoryItem;

					QByteArray curLog(dataList.at(n).toAscii());
					QByteArray logType=getMidData("\"type\":\"","\"",&curLog);
					QByteArray currentTimeStamp=getMidData("\"timestamp\":\"",".",&curLog);
					if(n==0||!firstTimestampReceived)
						if(!currentTimeStamp.isEmpty())
						{
							historyLastTimestamp=currentTimeStamp;
							firstTimestampReceived=true;
						}


					if(logType=="Sell")currentHistoryItem.type=1;
					else 
					if(logType=="Buy")currentHistoryItem.type=2;
					else 
					if(logType=="fee")currentHistoryItem.type=3;
					else 
					if(logType=="deposit")currentHistoryItem.type=4;
					else
					if(logType=="withdraw")currentHistoryItem.type=5;
					if(currentHistoryItem.type)
					{						
						currentHistoryItem.price=getMidData("\"price\":\"","\"",&curLog).toDouble();
						currentHistoryItem.volume=getMidData("\"amount\":\"","\"",&curLog).toDouble();
						currentHistoryItem.dateTimeInt=currentTimeStamp.toUInt();
						currentHistoryItem.symbol=baseValues.currentPair.currSymbol;
						if(currentHistoryItem.isValid())
						{
							currentHistoryItem.description+=getMidData("exchange\":\"","\"",&curLog);
							(*historyItems)<<currentHistoryItem;
						}
					}
				}
				emit historyChanged(historyItems);
			}
		}
		else if(debugLevel)logThread->writeLog("Invalid History data:"+data.left(200),2);
		break;//money/wallet/history
	default: break;
	}

	static int errorCount=0;
	if(!success)
	{
		errorCount++;
		if(errorCount<3)return;
		QString errorString=getMidData("\"message\":\"","\"",&data);
		if(errorString.isEmpty())errorString=data;
		if(debugLevel)logThread->writeLog(errorString.toAscii(),2);
		if(errorString.isEmpty())return;
		errorString.append("<br>"+QString::number(reqType));
		if(errorString.contains("X-BFX-SIGNATURE")||errorString.contains("X-BFX-APIKEY"))
			errorString.prepend("I:>");
		if(reqType<300)emit showErrorMessage(errorString);
	}
	else errorCount=0;
}