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

#include "Shibetranslator.h"
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QGroupBox>
#include <QDoubleSpinBox>
#include <QFile>
#include "main.h"

int ShibeTranslator::loadFromFile(const QString &fileName)
{
	clearMaps();
	QFile loadFile(fileName);
	if(loadFile.open(QIODevice::ReadOnly))
	{
		fillMapsFromList(QString::fromUtf8(loadFile.readAll().replace("\r","")).split("\n"));
		loadFile.close();
		lastLangFile=fileName;
		emit languageChanged();
		return 0;
	}
	return 1;
}

void ShibeTranslator::fillMapsFromList(const QStringList &list)
{
	for(int n=0;n<list.count();n++)
	{
		QString currentRow=list.at(n);
		if(currentRow.isEmpty()||!currentRow.at(0).isLetter())continue;
		if(fillMapsFromLine(&buttonMap,currentRow,"Button_"))
			if(fillMapsFromLine(&labelMap,currentRow,"Label_"))
				if(fillMapsFromLine(&checkBoxMap,currentRow,"CheckBox_"))
					if(fillMapsFromLine(&spinBoxMap,currentRow,"SpinBox_"))
						if(fillMapsFromLine(&stringMap,currentRow,"String_"))
							fillMapsFromLine(&groupBoxMap,currentRow,"GroupBox_");
	}
}

bool ShibeTranslator::fillMapsFromLine(QMap<QString,QString> *map, QString line, const QString &prefix)
{
	if(!line.startsWith(prefix))return true;
	line.remove(0,prefix.length());
	int splitPos=line.indexOf('=');
	if(splitPos==-1||splitPos+1>=line.length())return true;
	QString currentTid=line.left(splitPos);
	line.remove(0,splitPos+1);
	(*map)[currentTid]=line;
	return false;
}

int ShibeTranslator::saveToFile(const QString &fileName)
{
	QStringList resultList;
	resultList.append(getMapList(&buttonMap,"Button_"));
	resultList.append(getMapList(&labelMap,"Label_"));
	resultList.append(getMapList(&checkBoxMap,"CheckBox_"));
	resultList.append(getMapList(&groupBoxMap,"GroupBox_"));
	resultList.append(getMapList(&spinBoxMap,"SpinBox_"));
	resultList.append(getMapList(&stringMap,"String_"));
	if(resultList.isEmpty())return 1;
	resultList.sort();
	QFile writeFile(fileName);
	if(writeFile.open(QIODevice::WriteOnly|QIODevice::Truncate))
	{
		writeFile.write(QString(resultList.join("\r\n")+"\r\n").toUtf8());
		writeFile.close();
		return 0;
	}
	return 2;
}

QStringList ShibeTranslator::getMapList(QMap<QString,QString> *map, QString prefix)
{
	QStringList mapTids=map->keys();
	for(int n=0;n<mapTids.count();n++)
	{
		mapTids[n]=prefix+mapTids.at(n)+"="+map->value(mapTids.at(n),"");
		mapTids[n].replace("\n","<br>");
		mapTids[n].replace("\r","");
		mapTids[n].replace("\t"," ");
	}
	return mapTids;
}

QString ShibeTranslator::translateButton(const QString &tid, const QString &defaultText){return buttonMap.value(tid,defaultText);}
QString ShibeTranslator::translateLabel(const QString &tid, const QString &defaultText){return labelMap.value(tid,defaultText);}
QString ShibeTranslator::translateCheckBox(const QString &tid, const QString &defaultText){return checkBoxMap.value(tid,defaultText);}
QString ShibeTranslator::translateGroupBox(const QString &tid, const QString &defaultText){return groupBoxMap.value(tid,defaultText);}
QString ShibeTranslator::translateSpinBox(const QString &tid, const QString &defaultText){return spinBoxMap.value(tid,defaultText);}

QString ShibeTranslator::translateString(const QString &tid, const QString &defaultText)
{
	QString result=stringMap.value(tid,defaultText);
	if(stringMap.values(tid).count()==0)stringMap[tid]=defaultText;
	return result;
}

void ShibeTranslator::loadMapFromUi(QWidget *par)
{
	foreach(QPushButton* curButton, par->findChildren<QPushButton*>())
		if(!curButton->accessibleName().isEmpty())
			buttonMap[curButton->accessibleName()]=curButton->text().replace("\n","<br>").replace("\r","");

	foreach(QToolButton* curButton, par->findChildren<QToolButton*>())
		if(!curButton->accessibleName().isEmpty())
			buttonMap[curButton->accessibleName()]=curButton->text().replace("\n","<br>").replace("\r","");
		
	foreach(QCheckBox* curCheckBox, par->findChildren<QCheckBox*>())
		if(!curCheckBox->accessibleName().isEmpty())
			checkBoxMap[curCheckBox->accessibleName()]=curCheckBox->text().replace("\n","<br>").replace("\r","");

		foreach(QRadioButton* curCheckBox, par->findChildren<QRadioButton*>())
			if(!curCheckBox->accessibleName().isEmpty())
				checkBoxMap[curCheckBox->accessibleName()]=curCheckBox->text().replace("\n","<br>").replace("\r","");

	foreach(QLabel* curLabel, par->findChildren<QLabel*>())
		if(!curLabel->accessibleName().isEmpty())
			labelMap[curLabel->accessibleName()]=curLabel->text().replace("\n","<br>").replace("\r","");
		
	foreach(QGroupBox* curGroupBox, par->findChildren<QGroupBox*>())
		if(!curGroupBox->accessibleName().isEmpty())
			groupBoxMap[curGroupBox->accessibleName()]=curGroupBox->title().replace("\n","<br>").replace("\r","");

	foreach(QDoubleSpinBox* curSpinBox, par->findChildren<QDoubleSpinBox*>())
		if(!curSpinBox->accessibleName().isEmpty())
			spinBoxMap[curSpinBox->accessibleName()]=curSpinBox->suffix();
}

void ShibeTranslator::translateUi(QWidget *par)
{
	foreach(QPushButton* curButton, par->findChildren<QPushButton*>())
		if(!curButton->accessibleName().isEmpty())
			curButton->setText(translateButton(curButton->accessibleName(),curButton->text()));

	foreach(QToolButton* curButton, par->findChildren<QToolButton*>())
		if(!curButton->accessibleName().isEmpty())
			curButton->setText(translateButton(curButton->accessibleName(),curButton->text()));

	foreach(QCheckBox* curCheckBox, par->findChildren<QCheckBox*>())
		if(!curCheckBox->accessibleName().isEmpty())
			curCheckBox->setText(translateCheckBox(curCheckBox->accessibleName(),curCheckBox->text()));

	foreach(QRadioButton* curCheckBox, par->findChildren<QRadioButton*>())
		if(!curCheckBox->accessibleName().isEmpty())
			curCheckBox->setText(translateCheckBox(curCheckBox->accessibleName(),curCheckBox->text()));

	foreach(QLabel* curLabel, par->findChildren<QLabel*>())
		if(!curLabel->accessibleName().isEmpty())
			curLabel->setText(translateLabel(curLabel->accessibleName(),curLabel->text()));

	foreach(QGroupBox* curGroupBox, par->findChildren<QGroupBox*>())
		if(!curGroupBox->accessibleName().isEmpty())
			curGroupBox->setTitle(translateGroupBox(curGroupBox->accessibleName(),curGroupBox->title()));

	foreach(QDoubleSpinBox* curSpinBox, par->findChildren<QDoubleSpinBox*>())
		if(!curSpinBox->accessibleName().isEmpty())
			curSpinBox->setSuffix(translateSpinBox(curSpinBox->accessibleName(),curSpinBox->suffix()));
}