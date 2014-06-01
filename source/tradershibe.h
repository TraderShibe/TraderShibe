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

#ifndef Trader Shibe_H
#define Trader Shibe_H

#include <QtGui/QDialog>
#include "ui_Trader Shibe.h"
#include "depthmodel.h"
#include <QHttp>
#include <QCloseEvent>
#include <QSortFilterProxyModel>
#include "ruleholder.h"
#include <QSystemTrayIcon>
#include <QSettings>
#include <QMenu>
#include <QTime>
#include "tradesmodel.h"
#include "ordersmodel.h"
#include "orderitem.h"
#include "historymodel.h"
#include <QKeyEvent>
#include "currencypairitem.h"
#include "rulewidget.h"
#include "feecalculator.h"
#include "percentpicker.h"
#include <QScrollArea>

class Exchange;

class WindowScrollBars : public QScrollArea
{
private:
	void closeEvent(QCloseEvent *);
	void keyPressEvent(QKeyEvent *);
	void resizeEvent(QResizeEvent *);
};

class Trader Shibe : public QDialog
{
	Q_OBJECT

public:
	bool feeCalculatorSingleInstance;
	FeeCalculator *feeCalculator;

	double meridianPrice;
	double availableAmount;
	int exchangeId;
	double getAvailableBTC();
	double getAvailableUSD();
	double getAvailableUSDtoBTC(double price);

	double getFeeForUSDDec(double usd);
	double getValidDoubleForPercision(const double &val, const int &percision, bool roundUp=true);

	double floatFee;
	double floatFeeDec;
	double floatFeeInc;

	QString numFromDouble(const double &value, int maxDecimals=10);

	void addPopupDialog(int);

	void setupClass();
	bool isValidSize(QSize *sizeV){if(sizeV->width()<3||sizeV->width()>2000||sizeV->height()<3||sizeV->height()>2000)return false; return true;}
	void reloadLanguageList(QString preferedLangFile="");
	void fixAllChildButtonsAndLabels(QWidget *par);
	void fixDecimals(QWidget *par);
	void fillAllBtcLabels(QWidget *par, QString curName);
	void fillAllUsdLabels(QWidget *par, QString curName);

	void checkAndExecuteRule(int ruleType, double price);

	Ui::Trader ShibeClass ui;
	
	QByteArray getMidData(QString a, QString b,QByteArray *data);
	Trader Shibe();
	~Trader Shibe();

	OrdersModel *ordersModel;

	quint32 currencyChangedDate;

	QSettings *iniSettings;
	bool isValidSoftLag;
	void beep(bool noBlink=false);
	void playWav(QString, bool noBlink=false);
	void blinkWindow();

	void apiSellSend(double btc, double price);
	void apiBuySend(double btc, double price);

	QTime lastRuleExecutedTime;

	QSortFilterProxyModel *ordersSortModel;
	bool currentlyAddingOrders;
	bool windowCloseRequested();
	void keyPressEvent(QKeyEvent *event);
	void closeEvent(QCloseEvent *event);
	void resizeEvent(QResizeEvent *event);

	bool isDetachedLog;
	bool isDetachedTrades;
	bool isDetachedRules;
	bool isDetachedDepth;
	bool isDetachedCharts;
private:
	QWidget *windowWidget;
	QMenu copyTableValuesMenu;
	QTableView *lastCopyTable;

	void copyInfoFromTable(QTableView *table, QAbstractItemModel *model, int i);

	void addRuleGroupByID(QString groupName, int copyFrom=0, int gID=0);
	bool swapedDepth;
	DepthModel *depthAsksModel;
	DepthModel *depthBidsModel;
	TradesModel *tradesModel;
	HistoryModel *historyModel;
	void clearDepth();
	void calcOrdersTotalValues();
	void ruleTotalToBuyValueChanged();
	void ruleAmountToReceiveValueChanged();
	void ruleTotalToBuyBSValueChanged();
	void ruleAmountToReceiveBSValueChanged();
	bool isDataPending;
	QTime softLagTime;
	QTime depthLagTime;
	bool waitingDepthLag;
	int depthAsksLastScrollValue;
	int depthBidsLastScrollValue;

	QMenu *trayMenu;
	void saveDetachedWindowsSettings(bool force=false);
	QString windowTitleP;
	QSystemTrayIcon *trayIcon;
	QString profileName;
	void makeRitchValue(QString *text);
	bool checkForUpdates;

	void addRuleByHolderToTable(RuleHolder);
	int lastLoadedCurrency;
	void checkAllRules();

	bool constructorFinished;
	void reject(){};
	QString clearData(QString data);

	QString appDir;
	bool showingMessage;

	bool balanceNotLoaded;
	bool marketPricesNotLoaded;
	void checkValidSellButtons();
	void checkValidBuyButtons();

	bool sellLockBtcToSell;
	bool sellLockPricePerCoin;
	bool sellLockAmountToReceive;

	bool buyLockTotalBtc;
	bool buyLockTotalBtcSelf;
	bool buyLockPricePerCoin;

	bool profitSellThanBuyUnlocked;
	bool profitBuyThanSellUnlocked;
	bool profitBuyThanSellChangedUnlocked;
	bool profitSellThanBuyChangedUnlocked;

	void translateUnicodeStr(QString *str);

	bool eventFilter(QObject *obj, QEvent *event);

	void checkIsTabWidgetVisible();

	void clearTimeOutedTrades();
	bool isValidGeometry(QRect *geo, int yMargin=20);
	QRect rectInRect(QRect aRect, QSize bSize);
	void saveWindowState(QWidget *, QString name);
	void loadWindowState(QWidget *, QString name);
	void depthSelectOrder(QModelIndex, bool isSel, int type=0);
	double tradesPrecentLast;
	QList<CurrencyPairItem> currPairsList;

	void repeatOrderFromTrades(int type,int row);
	void repeatOrderFromValues(int type,double price, double amount, bool availableOnly=true);
	void repeatSelectedOrderByType(int type, bool availableOnly=true);

	void updateTrafficTotalValue();
public slots:
	void on_tradesVolume5m_valueChanged(double);
	void on_tradesBidsPrecent_valueChanged(double);
	void disableGroupId(int);
	void enableGroupId(int);
	void on_buyPercentage_clicked();
	void on_sellPercentage_clicked();
	void on_buyPriceAsMarketBid_clicked();
	void on_sellPriceAsMarketAsk_clicked();
	void on_trafficTotalToZero_clicked();
	void on_buttonNight_clicked();
	void ordersFilterChanged();
	void cancelOrderByXButton();
	void on_delauBetweenExecutingRules_toggled(bool);

	void repeatBuySellOrder();
	void repeatBuyOrder();
	void repeatSellOrder();
	void copySelectedRow();
	void copyDate();
	void copyAmount();
	void copyPrice();
	void copyTotal();

	void tableCopyContextMenuRequested(QPoint);

	void on_rulesTabs_tabCloseRequested(int);
	void on_buttonAddRuleGroup_clicked();
	void setCurrencyPairsList(QList<CurrencyPairItem> *currPairs);

	void availableAmountChanged(double);
	void precentBidsChanged(double);
	void depthRequested();
	void depthRequestReceived();
	void on_swapDepth_clicked();
	void checkValidOrdersButtons();
	void cancelOrder(QByteArray);
	void volumeAmountChanged(double,double);
	void setLastTrades10MinVolume(double);
	void on_depthAutoResize_toggled(bool);
	void on_depthComboBoxLimitRows_currentIndexChanged(int);
	void on_comboBoxGroupByPrice_currentIndexChanged(int);
	void depthSelectSellOrder(QModelIndex);
	void depthSelectBuyOrder(QModelIndex);
	void historyDoubleClicked(QModelIndex);
	void tradesDoubleClicked(QModelIndex);
	void setDataPending(bool);
	void anyDataReceived();
	void depthFirstOrder(double,double,bool);
	void depthSubmitOrders(QList<DepthItem> *, QList<DepthItem> *);
	void showErrorMessage(QString);
	void exitApp();
	void on_widgetStaysOnTop_toggled(bool);
	void setSoftLagValue(int);
	void trayActivated(QSystemTrayIcon::ActivationReason);
	void buttonMinimizeToTray();
	void on_tabOrdersLogOnTop_toggled(bool);
	void on_tabRulesOnTop_toggled(bool);
	void on_tabTradesOnTop_toggled(bool);
	void on_tabChartsOnTop_toggled(bool);
	void on_tabDepthOnTop_toggled(bool);

	void secondSlot();
	void setTradesScrollBarValue(int);
	void tabTradesIndexChanged(int);
	void tabTradesScrollUp();
	void addLastTrades(QList<TradesItem> *newItems);

	void detachLog();
	void detachTrades();
	void detachRules();
	void detachCharts();
	void detachDepth();

	void attachLog();
	void attachTrades();
	void attachRules();
	void attachCharts();
	void attachDepth();

	void loginChanged(QString);

	void ordersChanged(QList<OrderItem> *orders);

	void setApiDown(bool);

	void identificationRequired(QString);

	void updateLogTable();
	void historyChanged(QList<HistoryItem>*);

	void accLastSellChanged(QByteArray,double);
	void accLastBuyChanged(QByteArray,double);

	void orderCanceled(QByteArray);
	void ordersIsAvailable();
	void ordersIsEmpty();
	void firstTicker();

	void fixWindowMinimumSize();

	void languageComboBoxChanged(int);

	void languageChanged();
	void on_zeroSellThanBuyProfit_clicked();
	void on_zeroBuyThanSellProfit_clicked();
	void profitSellThanBuy();
	void on_sellThanBuySpinBox_valueChanged(double);
	void on_sellThanBuySpinBoxPrec_valueChanged(double);
	void profitSellThanBuyCalc();
	void profitBuyThanSellCalc();
	void profitBuyThanSell();
	void on_profitLossSpinBox_valueChanged(double);
	void on_profitLossSpinBoxPrec_valueChanged(double);

	void buttonNewWindow();

	void aboutTranslationButton();

	void on_currencyComboBox_currentIndexChanged(int);

	void on_calcButton_clicked();
	void checkUpdate();
	
	void on_accountUSD_valueChanged(double);
	void on_accountBTC_valueChanged(double);
	void on_marketBid_valueChanged(double);
	void on_marketAsk_valueChanged(double);
	void on_marketLow_valueChanged(double);
	void on_marketHigh_valueChanged(double);
	void on_marketLast_valueChanged(double);
	void on_ordersLastBuyPrice_valueChanged(double);
	void on_ordersLastSellPrice_valueChanged(double);

	void balanceChanged(double);
	void on_lagValue_valueChanged(double);

	void on_ordersCancelBidsButton_clicked();
	void on_ordersCancelAsksButton_clicked();
	void on_ordersCancelSelected_clicked();
	void on_ordersCancelAllButton_clicked();
	void cancelAllCurrentPairOrders();
	void on_accountFee_valueChanged(double);

	void on_buyTotalBtc_valueChanged(double);
	void on_buyPricePerCoin_valueChanged(double);
	void on_buyTotalBtcAllIn_clicked();
	void on_buyTotalBtcHalfIn_clicked();
	void on_buyPriceAsMarketAsk_clicked();
	void on_buyPriceAsMarketLastPrice_clicked();
	void buyBitcoinsButton();
	void on_buyTotalSpend_valueChanged(double);

	void sellBitcoinButton();
	void on_sellAmountToReceive_valueChanged(double);
	void on_sellPricePerCoin_valueChanged(double);
	void on_sellPriceAsMarketBid_clicked();
	void on_sellPricePerCoinAsMarketLastPrice_clicked();
	void on_sellTotalBtcAllIn_clicked();
	void on_sellTotalBtcHalfIn_clicked();
	void on_sellTotalBtc_valueChanged(double);
signals:
	void themeChanged();
	void reloadDepth();
	void cancelOrderByOid(QByteArray);
	void apiSell(double btc, double price);
	void apiBuy(double btc, double price);
	void getHistory(bool);
	void quit();
	void clearValues();
};

#endif // Trader Shibe_H
