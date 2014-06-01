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

#include "exchange_cryptsy.h"

Exchange_cryptsy::Exchange_cryptsy(QByteArray pRestSign, QByteArray pRestKey)
	: Exchange()
{
	exchangeDisplayOnlyCurrentPairOpenOrders=true;
	buySellAmountExcludedFee=true;
	clearHistoryOnCurrencyChanged=false;
	isLastTradesTypeSupported=true;
	balanceDisplayAvailableAmount=false;
	minimumRequestIntervalAllowed=600;
	calculatingFeeMode=1;
	baseValues.exchangeName="BTC China";
	depthAsks=0;
	depthBids=0;
	forceDepthLoad=false;
	ShibeHttpAuth=0;
	ShibeHttpPublic=0;
	tickerOnly=false;
	privateRestSign=pRestSign;
	privateRestKey=pRestKey;

	currencyMapFile="cryptsy";
	baseValues.currentPair.name="BTC/CNY";
	baseValues.currentPair.setSymbol("BTCCNY");
	baseValues.currentPair.currRequestPair="BTCCNY";
	baseValues.currentPair.priceDecimals=2;
	baseValues.currentPair.priceMin=qPow(0.1,baseValues.currentPair.priceDecimals);
	baseValues.currentPair.tradeVolumeMin=0.001;
	baseValues.currentPair.tradePriceMin=0.1;

	defaultCurrencyParams.currADecimals=3;
	defaultCurrencyParams.currBDecimals=2;
	defaultCurrencyParams.currABalanceDecimals=8;
	defaultCurrencyParams.currBBalanceDecimals=5;
	defaultCurrencyParams.priceDecimals=2;
	defaultCurrencyParams.currABalanceDecimals=8;
	defaultCurrencyParams.currBBalanceDecimals=5;
	defaultCurrencyParams.priceMin=qPow(0.1,baseValues.currentPair.priceDecimals);

	supportsLoginIndicator=true;
	supportsAccountVolume=false;
	supportsExchangeLag=false;

	moveToThread(this);
	authRequestTime.restart();
}

Exchange_cryptsy::~Exchange_cryptsy()
{
}

void Exchange_cryptsy::clearVariables()
{
	isFirstTicker=true;
	cancelingOrderIDs.clear();
	Exchange::clearVariables();
	secondPart=0;
	apiDownCounter=0;
	historyLastDate.clear();
	historyLastID.clear();
	lastHistory.clear();
	lastOrders.clear();
	historyLastTradesRequest="historydata?market="+baseValues.currentPair.currRequestPair;
	reloadDepth();
	lastInfoReceived=false;
	lastFetchTid.clear();
}

void Exchange_cryptsy::clearValues()
{
	if(ShibeHttpAuth)ShibeHttpAuth->clearPendingData();
	if(ShibeHttpPublic)ShibeHttpPublic->clearPendingData();
	clearVariables();
}

QByteArray Exchange_cryptsy::getMidData(QString a, QString b,QByteArray *data)
{
	QByteArray rez;
	if(b.isEmpty())b="\",";
	int startPos=data->indexOf(a,0);
	if(startPos>-1)
	{
		int endPos=data->indexOf(b,startPos+a.length());
		if(endPos>-1)rez=data->mid(startPos+a.length(),endPos-startPos-a.length());
	}
	return rez;
}

void Exchange_cryptsy::secondSlot()
{
	static int infoCounter=0;
	switch(infoCounter)
	{
	case 0: if(!tickerOnly&&!isReplayPending(204))sendToApi(204,"getOrders",true,true,"true,\"ALL\""); break;
	case 1: if(!isReplayPending(202))sendToApi(202,"getAccountInfo",true,true); break;
	case 2: if(lastHistory.isEmpty()&&!isReplayPending(208))
			{
				if(historyLastDate.isEmpty())
					sendToApi(208,"getTransactions",true,true,"\"all\",100");
				else
					sendToApi(208,"getTransactions",true,true,"\"all\",1000,0,"+historyLastDate);
			}
			break;
	default: break;
	}

	if(depthEnabled&&(forceDepthLoad||/*infoCounter==3&&*/!isReplayPending(111)))
	{
		if(!isReplayPending(111))
		{
			emit depthRequested();
			sendToApi(111,"getMarketDepth2",true,true,baseValues.depthCountLimitStr+",\""+baseValues.currentPair.currRequestPair+"\"");
		}
		forceDepthLoad=false;
	}

	if(!isReplayPending(103))sendToApi(103,"ticker?market="+baseValues.currentPair.currRequestPair,false,true);//

	if(infoCounter==3&&!isReplayPending(109))
	{
		if(!lastFetchTid.isEmpty())historyLastTradesRequest="historydata?market="+baseValues.currentPair.currRequestPair+"&since="+lastFetchTid;
		else "historydata?market="+baseValues.currentPair.currRequestPair;
		sendToApi(109,historyLastTradesRequest,false,true);
	}

	if(infoCounter++==3)
	{
		infoCounter=0;
	}

	Exchange::secondSlot();
}

bool Exchange_cryptsy::isReplayPending(int reqType)
{
	if(reqType<200)
	{
		if(ShibeHttpPublic==0)return false;
		return ShibeHttpPublic->isReqTypePending(reqType);
	}

	if(ShibeHttpAuth==0)return false;
	return ShibeHttpAuth->isReqTypePending(reqType);
}

void Exchange_cryptsy::getHistory(bool force)
{
	if(tickerOnly)return;
	if(force)lastHistory.clear();
	if(!isReplayPending(208))
	{
		if(historyLastDate.isEmpty())
			sendToApi(208,"getTransactions",true,true,"\"all\",100");
		else
			sendToApi(208,"getTransactions",true,true,"\"all\",1000,0,"+historyLastDate);
	}
}


QByteArray Exchange_cryptsy::numForBuySellFromDouble(double val, int maxDecimals)
{
	val=mainWindow.getValidDoubleForPercision(val,3,false);
	QByteArray numberText=QByteArray::number(val,'f',maxDecimals);
	int curPos=numberText.size()-1;
	while(curPos>0&&numberText.at(curPos)=='0')numberText.remove(curPos--,1);
	if(numberText.size()&&numberText.at(numberText.size()-1)=='.')numberText.remove(numberText.size()-1,1);
	return numberText;
}

void Exchange_cryptsy::buy(double apiBtcToBuy, double apiPriceToBuy)
{
	if(tickerOnly)return;
	QByteArray data=numForBuySellFromDouble(apiPriceToBuy,baseValues.currentPair.priceDecimals)+","+numForBuySellFromDouble(apiBtcToBuy,baseValues.currentPair.currADecimals)+",\""+baseValues.currentPair.currRequestPair+"\"";
	if(debugLevel)logThread->writeLog("Buy: "+data,2);
	sendToApi(306,"buyOrder2",true,true,data);
}

void Exchange_cryptsy::sell(double apiBtcToSell, double apiPriceToSell)
{
	if(tickerOnly)return;
	QByteArray data=numForBuySellFromDouble(apiPriceToSell,baseValues.currentPair.priceDecimals)+","+numForBuySellFromDouble(apiBtcToSell,baseValues.currentPair.currADecimals)+",\""+baseValues.currentPair.currRequestPair+"\"";
	if(debugLevel)logThread->writeLog("Sell: "+data,2);
	sendToApi(307,"sellOrder2",true,true,data);
}

void Exchange_cryptsy::cancelOrder(QByteArray order)
{
	if(tickerOnly)return;
	cancelingOrderIDs<<order;
	if(debugLevel)logThread->writeLog("Cancel order: "+order,2);
	sendToApi(305,"cancelOrder",true,true,order+",\""+baseValues.currentPair.currRequestPair+"\"");
}

void Exchange_cryptsy::sendToApi(int reqType, QByteArray method, bool auth, bool sendNow, QByteArray commands)
{
	if(auth)
	{
		if(ShibeHttpAuth==0)
		{
			ShibeHttpAuth=new ShibeHttp("api.cryptsy.com","",this,true,true,"application/json-rpc");
			connect(ShibeHttpAuth,SIGNAL(anyDataReceived()),baseValues_->mainWindow_,SLOT(anyDataReceived()));
			connect(ShibeHttpAuth,SIGNAL(setDataPending(bool)),baseValues_->mainWindow_,SLOT(setDataPending(bool)));
			connect(ShibeHttpAuth,SIGNAL(apiDown(bool)),baseValues_->mainWindow_,SLOT(setApiDown(bool)));
			connect(ShibeHttpAuth,SIGNAL(errorSignal(QString)),baseValues_->mainWindow_,SLOT(showErrorMessage(QString)));
			connect(ShibeHttpAuth,SIGNAL(sslErrorSignal(const QList<QSslError> &)),this,SLOT(sslErrors(const QList<QSslError> &)));
			connect(ShibeHttpAuth,SIGNAL(dataReceived(QByteArray,int)),this,SLOT(dataReceivedAuth(QByteArray,int)));
		}

		QByteArray signatureParams;

		signatureParams=commands;
		signatureParams.replace("\"","");
		signatureParams.replace("true","1");
		signatureParams.replace("false","");

		QByteArray postData;
		QByteArray appendedHeader;

		static int tonceCounter=0;		
		static quint32 lastTonce=static_cast<quint32>(time(NULL));
		quint32 newTonce=static_cast<quint32>(time(NULL));

		if(lastTonce!=newTonce)
		{
			tonceCounter=0;
			lastTonce=newTonce;
		}
		else
		{
			tonceCounter+=10;
			if(tonceCounter>99)tonceCounter=0;
		}

		QByteArray tonceCounterData=QByteArray::number(tonceCounter);
		if(tonceCounter>9)tonceCounterData.append("0000");
		else tonceCounterData.append("00000");

		QByteArray tonce=QByteArray::number(newTonce)+tonceCounterData;

		QByteArray signatureString="tonce="+tonce+"&accesskey="+privateRestKey+"&requestmethod=post&id=1&method="+method+"&params="+signatureParams;

		signatureString=privateRestKey+":"+hmacSha1(privateRestSign,signatureString).toHex();

		if(debugLevel&&reqType>299)logThread->writeLog(postData);

		postData="{\"method\":\""+method+"\",\"params\":["+commands+"],\"id\":1}";

		appendedHeader="Authorization: Basic "+signatureString.toBase64()+"\r\n";
		appendedHeader+="Json-Rpc-Tonce: "+tonce+"\r\n";

		if(sendNow)
			ShibeHttpAuth->sendData(reqType, "POST /api_trade_v1.php",postData,appendedHeader);
		else
			ShibeHttpAuth->prepareData(reqType, "POST /api_trade_v1.php",postData,appendedHeader);
	}
	else
	{
		if(ShibeHttpPublic==0)
		{
			ShibeHttpPublic=new ShibeHttp("data.cryptsy.com","",this,true,true,"application/json-rpc");
			connect(ShibeHttpPublic,SIGNAL(anyDataReceived()),baseValues_->mainWindow_,SLOT(anyDataReceived()));
			connect(ShibeHttpPublic,SIGNAL(setDataPending(bool)),baseValues_->mainWindow_,SLOT(setDataPending(bool)));
			connect(ShibeHttpPublic,SIGNAL(apiDown(bool)),baseValues_->mainWindow_,SLOT(setApiDown(bool)));
			connect(ShibeHttpPublic,SIGNAL(errorSignal(QString)),baseValues_->mainWindow_,SLOT(showErrorMessage(QString)));
			connect(ShibeHttpPublic,SIGNAL(sslErrorSignal(const QList<QSslError> &)),this,SLOT(sslErrors(const QList<QSslError> &)));
			connect(ShibeHttpPublic,SIGNAL(dataReceived(QByteArray,int)),this,SLOT(dataReceivedAuth(QByteArray,int)));
		}

		if(sendNow)
			ShibeHttpPublic->sendData(reqType, "GET /data/"+method);
		else
			ShibeHttpPublic->prepareData(reqType, "GET /data/"+method);
	}
}

void Exchange_cryptsy::depthUpdateOrder(double price, double amount, bool isAsk)
{
	if(isAsk)
	{
		if(depthAsks==0)return;
		DepthItem newItem;
		newItem.price=price;
		newItem.volume=amount;
		if(newItem.isValid())
			(*depthAsks)<<newItem;
	}
	else
	{
		if(depthBids==0)return;
		DepthItem newItem;
		newItem.price=price;
		newItem.volume=amount;
		if(newItem.isValid())
			(*depthBids)<<newItem;
	}
}

void Exchange_cryptsy::depthSubmitOrder(QMap<double,double> *currentMap ,double priceDouble, double amount, bool isAsk)
{
	if(priceDouble==0.0||amount==0.0)return;
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

void Exchange_cryptsy::reloadDepth()
{
	lastDepthBidsMap.clear();
	lastDepthAsksMap.clear();
	lastDepthData.clear();
	Exchange::reloadDepth();
}

void Exchange_cryptsy::dataReceivedAuth(QByteArray data, int reqType)
{
	if(debugLevel)logThread->writeLog("RCV: "+data);
	bool success=data.startsWith("{")&&!data.startsWith("{\"error\":")||data.startsWith("[{");
	if(success&&data.startsWith("401"))success=false;

	switch(reqType)
	{
	case 103: //ticker
		if(!success)break;
		if(data.startsWith("{\"ticker\":{"))
		{
			QByteArray tickerHigh=getMidData("\"high\":\"","\"",&data);
			if(!tickerHigh.isEmpty())
			{
				double newTickerHigh=tickerHigh.toDouble();
				if(newTickerHigh!=lastTickerHigh)emit tickerHighChanged(newTickerHigh);
				lastTickerHigh=newTickerHigh;
			}

			QByteArray tickerLow=getMidData("\"low\":\"","\"",&data);
			if(!tickerLow.isEmpty())
			{
				double newTickerLow=tickerLow.toDouble();
				if(newTickerLow!=lastTickerLow)emit tickerLowChanged(newTickerLow);
				lastTickerLow=newTickerLow;
			}

			QByteArray tickerVolume=getMidData("\"vol\":\"","\"",&data);
			if(!tickerVolume.isEmpty())
			{
				double newTickerVolume=tickerVolume.toDouble();
				if(newTickerVolume!=lastTickerVolume)emit tickerVolumeChanged(newTickerVolume);
				lastTickerVolume=newTickerVolume;
			}

			QByteArray tickerLast=getMidData("\"last\":\"","\"",&data);
			if(!tickerLast.isEmpty())
			{
				double newTickerLast=tickerLast.toDouble();
				if(newTickerLast!=lastTickerLast)emit tickerLastChanged(newTickerLast);
				lastTickerLast=newTickerLast;
			}

			QByteArray tickerSell=getMidData("\"buy\":\"","\"",&data);
			if(!tickerSell.isEmpty())
			{
				double newTickerSell=tickerSell.toDouble();
				if(newTickerSell!=lastTickerSell)emit tickerSellChanged(newTickerSell);
				lastTickerSell=newTickerSell;
			}

			QByteArray tickerBuy=getMidData("\"sell\":\"","\"",&data);
			if(!tickerBuy.isEmpty())
			{
				double newTickerBuy=tickerBuy.toDouble();
				if(newTickerBuy!=lastTickerBuy)emit tickerBuyChanged(newTickerBuy);
				lastTickerBuy=newTickerBuy;
			}

			if(isFirstTicker)
			{
				emit firstTicker();
				isFirstTicker=false;
			}
		}
		else if(debugLevel)logThread->writeLog("Invalid ticker data:"+data,2);
		break;//ticker
	case 109: //trades
		if(success&&data.size()>32)
		{
			if(data.startsWith("[{"))
			{
				QStringList tradeList=QString(data).split("},{");
				QList<TradesItem> *newTradesItems=new QList<TradesItem>;
				for(int n=0;n<tradeList.count();n++)
				{
					QByteArray tradeData=tradeList.at(n).toAscii();
					QByteArray nextFetchTid=getMidData("\"tid\":\"","\"",&tradeData);
					if(nextFetchTid<=lastFetchTid)continue;
					TradesItem newItem;
					newItem.amount=getMidData("\"amount\":",",",&tradeData).toDouble();
					newItem.price=getMidData("\"price\":",",",&tradeData).toDouble();
					newItem.date=getMidData("\"date\":\"","\"",&tradeData).toUInt();
					newItem.symbol=baseValues.currentPair.currSymbol;
					newItem.orderType=getMidData("\"type\":\"","\"",&tradeData)=="sell"?1:-1;

					if(newItem.isValid())(*newTradesItems)<<newItem;
					else if(debugLevel)logThread->writeLog("Invalid trades fetch data line:"+tradeData,2);

					if(n==tradeList.count()-1&&!nextFetchTid.isEmpty())lastFetchTid=nextFetchTid;
				}
				if(newTradesItems->count())emit addLastTrades(newTradesItems);
				else delete newTradesItems;
			}
			else if(debugLevel)logThread->writeLog("Invalid trades fetch data:"+data,2);
		}
		break;
	case 111: //bc/orderbook
		if(data.startsWith("{\"result\":{\"market_depth"))
		{
			emit depthRequestReceived();

			if(lastDepthData!=data)
			{
				lastDepthData=data;
				depthAsks=new QList<DepthItem>;
				depthBids=new QList<DepthItem>;

				int asksStart=data.indexOf("\"ask\":");
				if(asksStart==-1)return;
				QByteArray bidsData=data.mid(35,asksStart-38);
				data.remove(0,asksStart+8);
				data.remove(data.size()-14,14);

				QMap<double,double> currentBidsMap;
				QStringList bidsList=QString(bidsData).split("},{");
				double groupedPrice=0.0;
				double groupedVolume=0.0;
				int rowCounter=0;
				for(int n=0;n<bidsList.count();n++)
				{
					if(baseValues.depthCountLimit&&rowCounter>=baseValues.depthCountLimit)break;
					QByteArray currentRow=bidsList.at(n).toAscii()+"}";
					double priceDouble=getMidData("price\":",",",&currentRow).toDouble();
					double amount=getMidData("amount\":","}",&currentRow).toDouble();
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
							bool matchCurrentGroup=priceDouble>groupedPrice-baseValues.groupPriceValue;
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

				QMap<double,double> currentAsksMap;
				QStringList asksList=QString(data).split("},{");
				groupedPrice=0.0;
				groupedVolume=0.0;
				rowCounter=0;

				for(int n=0;n<asksList.count();n++)
				{
					if(baseValues.depthCountLimit&&rowCounter>=baseValues.depthCountLimit)break;
					QByteArray currentRow=asksList.at(n).toAscii()+"}";
					double priceDouble=getMidData("price\":",",",&currentRow).toDouble();
					double amount=getMidData("amount\":","}",&currentRow).toDouble();
					if(n==0)emit tickerBuyChanged(priceDouble);

					if(priceDouble>99999)break;

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

				emit depthSubmitOrders(depthAsks, depthBids);
				depthAsks=0;
				depthBids=0;
			}
		}
		else if(debugLevel)logThread->writeLog("Invalid depth data:"+data,2);
		break;
	case 202: //AccountInfo
		{
			if(!success)break;
			if(data.startsWith("{\"result\":{\"profile\""))
			{
				lastInfoReceived=true;
				if(debugLevel)logThread->writeLog("Info: "+data);

				if(apiLogin.isEmpty())
				{
					QByteArray login=getMidData("username\":\"","\"",&data);
					if(!login.isEmpty())
					{
						apiLogin=login;
						translateUnicodeStr(&apiLogin);
						emit loginChanged(apiLogin);
					}
				}

				QByteArray feeData=getMidData("trade_fee\":",",",&data);
				if(!feeData.isEmpty())emit accFeeChanged(feeData.toDouble());

				QByteArray balanceData=getMidData("\"balance\":","}}}",&data)+"}";
				QByteArray btcBalance=getMidData("\""+baseValues.currentPair.currAStrLow+"\":","}",&balanceData);
				if(!btcBalance.isEmpty())
				{
					double newBtcBalance=btcBalance.toDouble()+getMidData("\"amount\":\"","\"",&btcBalance).toDouble();
					if(lastBtcBalance!=newBtcBalance)emit accBtcBalanceChanged(newBtcBalance);
					lastBtcBalance=newBtcBalance;
				}

				QByteArray usdBalance=getMidData("\""+baseValues.currentPair.currBStrLow+"\":","}",&balanceData);
				if(!usdBalance.isEmpty())
				{
					double newUsdBalance=usdBalance.toDouble()+getMidData("\"amount\":\"","\"",&usdBalance).toDouble();
					if(newUsdBalance!=lastUsdBalance)emit accUsdBalanceChanged(newUsdBalance);
					lastUsdBalance=newUsdBalance;
				}
			}
			else if(debugLevel)logThread->writeLog("Invalid Info data:"+data,2);
		}
		break;//balance
	case 204://open_orders
		if(!success)break;
		if(data.startsWith("{\"result\":{\"order"))
		{
			if(lastOrders!=data)
			{
				if(data.size()<100){lastOrders.clear();emit ordersIsEmpty();break;}
				QList<OrderItem> *orders=new QList<OrderItem>;
				QStringList currencyList=QString(data).split("],\"");
				for(int n=0;n<currencyList.count();n++)
				{
					QByteArray currencyByteArray=currencyList[n].toAscii();
					QString curerntSymbol=getMidData("order_","\":[",&currencyByteArray).toUpper();

					QStringList ordersList=currencyList.at(n).split("},{");

					for(int n=0;n<ordersList.count();n++)
					{	
						OrderItem currentOrder;
						QByteArray currentOrderData=ordersList.at(n).toAscii();
						currentOrder.oid=getMidData("\"id\":",",",&currentOrderData);
						currentOrder.date=getMidData("\"date\":",",",&currentOrderData).toUInt();
						currentOrder.type=getMidData("\"type\":\"","\"",&currentOrderData)=="ask";
						QByteArray status=getMidData("\"status\":\"","\"",&currentOrderData);
						//0=Canceled, 1=Open, 2=Pending, 3=Post-Pending
						if(status=="open")currentOrder.status=1;
						else
							if(status=="cancelled")currentOrder.status=0;
							else currentOrder.status=2;

							currentOrder.amount=getMidData("\"amount\":\"","\"",&currentOrderData).toDouble();
							currentOrder.price=getMidData("\"price\":\"","\"",&currentOrderData).toDouble();
							currentOrder.symbol=curerntSymbol.toAscii();
							if(currentOrder.isValid())(*orders)<<currentOrder;
					}
				}
				lastOrders=data;

				emit ordersChanged(orders);
				lastInfoReceived=false;
			}
		}
		else if(debugLevel)logThread->writeLog("Invalid Orders data:"+data,2);
		break;//open_orders
	case 305: //cancelOrder
		{
			if(!success)break;
			if(!cancelingOrderIDs.isEmpty())
			{
				if(data.startsWith("{\"result\":true"))emit orderCanceled(cancelingOrderIDs.first());
				if(debugLevel)logThread->writeLog("Order canceled:"+cancelingOrderIDs.first(),2);
				cancelingOrderIDs.removeFirst();
			}
		}
		break;//cancelOrder
	case 306: //buyOrder
		if(!success||!debugLevel)break;
		if(data.startsWith("{\"result\":"))logThread->writeLog("Buy OK: "+data);
		else logThread->writeLog("Invalid Order Buy Data:"+data);
		break;//buyOrder
	case 307: //sellOrder
		if(!success||!debugLevel)break;
		if(data.startsWith("{\"result\":"))logThread->writeLog("Sell OK: "+data);
		else logThread->writeLog("Invalid Order Sell Data:"+data);
		break;//sellOrder
	case 208: //money/wallet/history 
		if(!success)break;
		if(data.startsWith("{\"result\":{\"transaction"))
		{
			if(lastHistory!=data)
			{
				lastHistory=data;

				QList<HistoryItem> *historyItems=new QList<HistoryItem>;
				QStringList dataList=QString(data).split("},{");
				for(int n=0;n<dataList.count();n++)
				{
					QByteArray curLog(dataList.at(n).toAscii());
					curLog.append("}");

					if(n==0)
					{
						QByteArray currentOrderID=getMidData("id\":",",",&curLog);
						if(!historyLastID.isEmpty()&&currentOrderID>historyLastID)break;
						historyLastID=currentOrderID;
					}

					QByteArray currentOrderDate=getMidData("date\":",",",&curLog);

					if(n==0)
					{
						if(!historyLastDate.isEmpty()&&currentOrderDate>historyLastDate)break;
						historyLastDate=QByteArray::number(currentOrderDate.toUInt()-1);
					}

					QByteArray transactionType=getMidData("type\":\"","\"",&curLog).replace("money","cny");

					HistoryItem currentHistoryItem;

					if(transactionType.startsWith("sell"))currentHistoryItem.type=1;
					if(transactionType.startsWith("buy"))currentHistoryItem.type=2;
					if(transactionType.startsWith("fund"))currentHistoryItem.type=4;
					if(transactionType.startsWith("withdraw"))currentHistoryItem.type=5;

					if(currentHistoryItem.type==0)continue;

					QByteArray currencyA=transactionType.right(3).toUpper();

					double btcAmount=getMidData("btc_amount\":\"","\"",&curLog).toDouble();
					if(btcAmount<0.0)btcAmount=-btcAmount;

					double cnyAmount=getMidData("cny_amount\":\"","\"",&curLog).toDouble();
					if(cnyAmount<0.0)cnyAmount=-cnyAmount;

					double ltcAmount=getMidData("ltc_amount\":\"","\"",&curLog).toDouble();
					if(ltcAmount<0.0)ltcAmount=-ltcAmount;

					QByteArray currencyB;
					if(currencyA=="BTC")
					{
						currentHistoryItem.volume=btcAmount;
						if(cnyAmount!=0.0)
						{
							currentHistoryItem.price=cnyAmount/btcAmount;
							currencyB="CNY";
						}
						else
						if(ltcAmount!=0.0)
						{
							currentHistoryItem.price=ltcAmount/btcAmount;
							currencyB="LTC";
						}
					}

					if(currencyA=="CNY")
					{
						currentHistoryItem.volume=cnyAmount;
						if(btcAmount!=0.0)
						{
							currentHistoryItem.price=btcAmount/cnyAmount;
							currencyB="BTC";
						}
						else
						if(ltcAmount!=0.0)
						{
							currentHistoryItem.price=ltcAmount/cnyAmount;
							currencyB="LTC";
						}
					}

					if(currencyA=="LTC")
					{
						currentHistoryItem.volume=ltcAmount;
						if(btcAmount!=0.0)
						{
							currentHistoryItem.price=btcAmount/ltcAmount;
							currencyB="BTC";
						}
						else
						if(cnyAmount!=0.0)
						{
							currentHistoryItem.price=cnyAmount/ltcAmount;
							currencyB="CNY";
						}
					}
					if(currencyB.isEmpty())currencyB="CNY";

					currentHistoryItem.symbol=currencyA+currencyB;

					currentHistoryItem.dateTimeInt=currentOrderDate.toUInt();

					if(currentHistoryItem.isValid())(*historyItems)<<currentHistoryItem;
				}
				emit historyChanged(historyItems);
			}
		}
	default: break;
	}

	if(reqType<200||reqType==204||reqType==305)return;
	static int errorCount=0;
	if(!success)
	{
		errorCount++;
		QString errorString;
		bool invalidMessage=!data.startsWith("{");
		if(!invalidMessage)
			errorString=getMidData("message\":\"","\",",&data)+" Code:"+getMidData("code\":",",",&data);
		else errorString=data;
		if(debugLevel)logThread->writeLog("API Error: "+errorString.toAscii()+" ReqType:"+QByteArray::number(reqType),2);
		if(errorCount<3&&reqType<300&&!errorString.endsWith("Unauthorized"))return;
		if(errorString.isEmpty())return;
		errorString.append("<br>"+QString::number(reqType));
		if(invalidMessage||reqType<300)emit showErrorMessage("I:>"+errorString);
	}
	else errorCount=0;
}

void Exchange_cryptsy::sslErrors(const QList<QSslError> &errors)
{
	QStringList errorList;
	for(int n=0;n<errors.count();n++)errorList<<errors.at(n).errorString();
	if(debugLevel)logThread->writeLog(errorList.join(" ").toAscii(),2);
	emit showErrorMessage("SSL Error: "+errorList.join(" "));
}
