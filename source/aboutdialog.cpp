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

#include "aboutdialog.h"
#include "main.h"
#include "translationdialog.h"

TranslationAbout::TranslationAbout(QWidget *par)
	: QDialog()
{
	ui.setupUi(this);
	setWindowFlags(Qt::WindowCloseButtonHint|par->windowFlags());
	setWindowModality(Qt::ApplicationModal);
	setAttribute(Qt::WA_DeleteOnClose,true);
	//setFixedSize(size());
	ui.aboutTextLabel->setStyleSheet("QLabel {color: "+baseValues.appTheme.black.name()+"; border: 1px solid "+baseValues.appTheme.gray.name()+"; background: "+baseValues.appTheme.white.name()+"; padding:6px}");
	ui.translationAuthor->setStyleSheet(ui.aboutTextLabel->styleSheet());
}

TranslationAbout::~TranslationAbout()
{
	if(baseValues.mainWindow_)mainWindow.addPopupDialog(-1);
}

void TranslationAbout::showWindow()
{
	ShibeTranslator.translateUi(this);
	ui.languageField->setText(ShibeTr("LANGUAGE_NAME","Invalid Language"));
	ui.translationAuthor->setText(ShibeTr("LANGUAGE_AUTHOR","Invalid About"));
	ui.aboutTrader ShibeGroupBox->setTitle(ShibeTr("ABOUT_QT_BITCOIN_TRADER","About %1").arg(windowTitle()));
	ui.aboutTextLabel->setText(ShibeTr("ABOUT_QT_BITCOIN_TRADER_TEXT","Qt Bitcoin Trader is a free Open Source project<br>developed on C++ Qt and OpenSSL.<br>If you want to help make project better please donate.<br>Feel free to send me recommendations and fixes to: %1").arg("<a href=\"mailto:ShibeShibe@gmail.com\">ShibeShibe@gmail.com</a>"));
	if(baseValues.mainWindow_)mainWindow.addPopupDialog(1);
	show();
}

void TranslationAbout::createTranslation()
{
	accept();
	TranslationDialog *translationDialog=new TranslationDialog;
	translationDialog->setWindowFlags(windowFlags());
	translationDialog->show();
}

void TranslationAbout::buttonCheckUpdates()
{
	mainWindow.checkUpdate();
}