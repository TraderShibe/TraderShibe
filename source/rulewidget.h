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

#ifndef RULEWIDGET_H
#define RULEWIDGET_H

#include <QWidget>
#include "ui_rulewidget.h"
#include "rulesmodel.h"
#include <QTime>

class RuleWidget : public QWidget
{
	Q_OBJECT

public:
	void setRuleGroupId(int id);
	int getRuleGroupId();
	QString getRuleGroupIdStr();
	void updateStyleSheets();
	void saveRulesData();
	bool haveWorkingRules();
	bool haveAnyRules();
	bool haveAnyTradingRules();
	void removeGroup();
	void languageChanged();
	void checkAndExecuteRule(int ruleType, double price);
	Ui::RuleWidget ui;
	RulesModel *rulesModel;
	RuleWidget(int gID, QString groupName, RuleWidget *copyFrom=0, QString restorableString="");
	~RuleWidget();

private:
	QString ruleGroupIdStr;
	QTime ordersCancelTime;
	QMenu *rulesEnableDisableMenu;
	QString groupName;
public slots:
	void on_ruleUp_clicked();
	void on_ruleDown_clicked();
	void rulesMenuRequested(const QPoint&);
	void ruleDisableEnableMenuFix();
	void on_ruleConcurrentMode_toggled(bool);
	void ruleEnableSelected();
	void ruleDisableSelected();
	void ruleEnableAll();
	void ruleDisableAll();
	void on_ruleAddButton_clicked();
	void on_ruleEditButton_clicked();
	void on_ruleRemoveAll_clicked();
	void checkValidRulesButtons();
	void on_ruleRemove_clicked();
	void on_ruleSave_clicked();
};

#endif // RULEWIDGET_H
