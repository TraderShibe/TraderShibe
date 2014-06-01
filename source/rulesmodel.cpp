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

#include "rulesmodel.h"
#include "main.h"

RulesModel::RulesModel()
	: QAbstractItemModel()
{
	allDisabled=false;
	stateWidth=80;
	lastQueringHolder=0;
	firstQueringHolder=0;
	isConcurrentMode=false;
	columnsCount=5;
}

RulesModel::~RulesModel()
{
	clear();
}

QString RulesModel::saveRulesToString()
{
	QStringList savableList;
	for(int n=0;n<holderList.count();n++)
		savableList<<holderList.at(n)->generateSavableData();
	return savableList.join("@");
}

void RulesModel::restoreRulesFromString(QString strData)
{
	if(strData.isEmpty())return;
	QStringList restorableList=strData.split("@");
	if(restorableList.count()==0)return;
	clear();
	for(int n=0;n<restorableList.count();n++)
	{
		RuleHolder *restoredHolder=new RuleHolder(restorableList.at(n));
		if(!restoredHolder->invalidHolder)addRule(restoredHolder);
		else delete restoredHolder;
	}
	allDisabled=true;
}

bool RulesModel::haveAnyTradingRules()
{
	foreach(RuleHolder *holder, holderList)if(holder->isTrading())return true;
	return false;
}

bool RulesModel::haveWorkingRule()
{
	foreach(RuleHolder *holder, holderList)if(holder->getRuleState()==1)return true;
	return false;
}

QList<RuleHolder *> RulesModel::getAchievedRules(int type, double val)
{
	QList<RuleHolder*> achievedRules;
	for(int n=0;n<holderList.count();n++)
	{
		RuleHolder *holder=holderList.at(n);
		if(holder&&holder->getRuleState()==1)
		{
			if(holder->getRulePriceType()==type&&holder->isAchieved(val))achievedRules<<holder;
			if(!isConcurrentMode)
			{
				firstQueringHolder=holder;
				if(lastQueringHolder!=firstQueringHolder)
				{
					lastQueringHolder=firstQueringHolder;
					emit dataChanged(index(0,0),index(0,columnsCount-1));
				}
				return achievedRules;
			}
			else
			{
				firstQueringHolder=0;
				if(lastQueringHolder!=firstQueringHolder)
				{
					lastQueringHolder=firstQueringHolder;
					emit dataChanged(index(0,0),index(0,columnsCount-1));
				}
			}
		}
	}
	return achievedRules;
}

void RulesModel::updateHolderByRow(int row, RuleHolder *holder)
{
	if(row<0||row>=holderList.count())return;
	delete holderList[row];
	holderList[row]=new RuleHolder(*holder);
	emit dataChanged(index(row,0),index(row,columnsCount-1));
}

RuleHolder *RulesModel::getRuleHolderByRow(int row)
{
	if(row<0||row>=holderList.count())return 0;
	return holderList.at(row);
}

void RulesModel::moveRowUp(int row)
{
	if(row<1||row>=holderList.count())return;
	holderList.swap(row,row-1);
	emit dataChanged(index(row-1,0),index(row,columnsCount-1));
}

void RulesModel::moveRowDown(int row)
{
	if(row<0||row>=holderList.count()-1)return;
	holderList.swap(row,row+1);
	emit dataChanged(index(row,0),index(row+1,columnsCount-1));
}

void RulesModel::setRuleStateByRow(int curRow, int state)
{
	if(curRow<0||curRow>=holderList.count())return;
	holderList.at(curRow)->setRuleState(state);
	if(state!=0)allDisabled=false;
	emit dataChanged(index(curRow,0),index(curRow,columnsCount-1));
}

void RulesModel::setRuleStateByHolder(RuleHolder *holder, int state)
{
	for(int n=0;n<holderList.count();n++)
		if(holderList.at(n)==holder)
		{
			setRuleStateByRow(n,state);
			return;
		}
}

void RulesModel::clear()
{
	beginResetModel();
	qDeleteAll(holderList.begin(), holderList.end());
	holderList.clear();
	endResetModel();
}

int RulesModel::rowCount(const QModelIndex &) const
{
	return holderList.count();
}

int RulesModel::columnCount(const QModelIndex &) const
{
	return columnsCount;
}

void RulesModel::addRule(RuleHolder *holder)
{
	beginInsertRows(QModelIndex(),holderList.count(),holderList.count());
	holderList<<holder;
	endInsertRows();
}

void RulesModel::removeRuleByRow(int row)
{
	beginRemoveRows(QModelIndex(),row,row);
	delete holderList.at(row);
	holderList.removeAt(row);
	endRemoveRows();
}

QVariant RulesModel::data(const QModelIndex &index, int role) const
{
	if(!index.isValid())return QVariant();
	int currentRow=index.row();
	if(currentRow<0||currentRow>=holderList.count())return QVariant();

	if(role!=Qt::DisplayRole&&role!=Qt::ToolTipRole&&role!=Qt::ForegroundRole&&role!=Qt::BackgroundRole&&role!=Qt::TextAlignmentRole)return QVariant();

	int indexColumn=index.column();

	if(role==Qt::TextAlignmentRole)return 0x0084;

	if(role==Qt::ForegroundRole)return baseValues.appTheme.black;

	if(role==Qt::BackgroundRole)
	{
		switch(holderList.at(currentRow)->getRuleState())
		{
			case 1:
			{
				if(isConcurrentMode||firstQueringHolder&&firstQueringHolder==holderList.at(currentRow))
					return QVariant();
				return baseValues.appTheme.lightRedGreen;
			}
			break;
			case 2: return baseValues.appTheme.lightGreen; break;
			default: return baseValues.appTheme.lightRed;break;
		}
		return QVariant();
	}

	switch(indexColumn)
	{
	case 0://State
		switch(holderList.at(currentRow)->getRuleState())
		{
		case 1:
			{
				if(isConcurrentMode||firstQueringHolder&&firstQueringHolder==holderList.at(currentRow))
					return ShibeTr("RULE_STATE_PROCESSING","processing");
				return ShibeTr("RULE_STATE_PENDING","pending");
			}
			break;
		case 2: return ShibeTr("RULE_STATE_DONE","done"); break;
		default: return ShibeTr("RULE_STATE_DISABLED","disabled");break;
		}
	case 1://Description
			return holderList.at(currentRow)->getDescriptionString(); 
		break;
	case 2://Action
			return holderList.at(currentRow)->getSellOrBuyString();
		break;
	case 3://Amount
		return holderList.at(currentRow)->getBitcoinsString();
		break;
	case 4://Price
			return holderList.at(currentRow)->getPriceText();
		break;
	default: break;
	}
	return QVariant();
}

void RulesModel::disableAll()
{
	for(int n=0;n<holderList.count();n++)
		setRuleStateByRow(n,0);
	allDisabled=true;
}

void RulesModel::enableAll()
{
	for(int n=0;n<holderList.count();n++)
		setRuleStateByRow(n,1);
}

QVariant RulesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if(role==Qt::TextAlignmentRole)return 0x0084;

	if(role==Qt::SizeHintRole&&orientation==Qt::Horizontal)
	{
		switch(section)
		{
		case 0: return QSize(stateWidth,defaultHeightForRow);//State
		//case 2: return QSize(typeWidth,defaultSectionSize);//Type
		}
		return QVariant();
	}

	if(role!=Qt::DisplayRole)return QVariant();
	if(orientation==Qt::Vertical)return section;
	if(headerLabels.count()!=columnsCount)return QVariant();

	return headerLabels.at(section);
}

Qt::ItemFlags RulesModel::flags(const QModelIndex &) const
{
	return Qt::ItemIsSelectable|Qt::ItemIsEnabled;
}

void RulesModel::setHorizontalHeaderLabels(QStringList list)
{
	if(list.count()!=columnsCount)return;
	headerLabels=list;
	stateWidth=qMax(textFontWidth(headerLabels.first()),textFontWidth(ShibeTr("RULE_STATE_PROCESSING","processing")));
	stateWidth=qMax(stateWidth,textFontWidth(ShibeTr("RULE_STATE_PENDING","pending")));
	stateWidth=qMax(stateWidth,textFontWidth(ShibeTr("RULE_STATE_DONE","done")));
	stateWidth=qMax(stateWidth,textFontWidth(ShibeTr("RULE_STATE_DISABLED","disabled")));
	stateWidth+=12;
	emit headerDataChanged(Qt::Horizontal, 0, columnsCount-1);
}

QModelIndex RulesModel::index(int row, int column, const QModelIndex &parent) const
{
	if(!hasIndex(row, column, parent))return QModelIndex();
	return createIndex(row,column);
}

QModelIndex RulesModel::parent(const QModelIndex &) const
{
	return QModelIndex();
}