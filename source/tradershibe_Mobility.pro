#
# Basic stuff
#
TEMPLATE	= app
LANGUAGE        = C++
TARGET 		= Trader Shibe
DEPENDPATH 	+= .
QT		+= network
INCLUDEPATH 	+= .
INCLUDEPATH += $$[QT_INSTALL_PREFIX]/src/3rdparty/zlib
INCLUDEPATH    += /usr/include/QtMultimediaKit
INCLUDEPATH    += /usr/include/QtMobility
MOBILITY       = multimedia

win32 {
LIBS		+= -lcrypt32 -llibeay32 -lssleay32 -luser32 -lgdi32 -ladvapi32
}
!win32 {
LIBS		+= -lcrypto -lz -lQtMultimediaKit
}

CONFIG		+= qt warn_off release mobility

#
# Headers
#

HEADERS += aboutdialog.h \
           addrulegroup.h \
           addrulewindow.h \
           audioplayer.h \
           currencyinfo.h \
           currencypairitem.h \
           datafolderchusedialog.h \
           debugviewer.h \
           depthitem.h \
           depthmodel.h \
           exchange.h \
           exchange_bitfinex.h \
           exchange_cryptsy.h \
           exchange_cryptsy.h \
           exchange_btce.h \
           exchange_cryptsy.h \
           feecalculator.h \
           historyitem.h \
           historymodel.h \
           Shibeaes256.h \
           Shibehttp.h \
           Shibelightchanges.h \
           Shibersa.h \
           Shibescrolluponidle.h \
           Shibespinboxfix.h \
           Shibespinboxpicker.h \
           Shibetranslator.h \
           logthread.h \
           main.h \
           newpassworddialog.h \
           orderitem.h \
           ordersmodel.h \
           orderstablecancelbutton.h \
           passworddialog.h \
           percentpicker.h \
           Trader Shibe.h \
           ruleholder.h \
           rulesmodel.h \
           rulewidget.h \
           thisfeatureunderdevelopment.h \
           tradesitem.h \
           tradesmodel.h \
           translationdialog.h \
           translationline.h \
           updaterdialog.h \
           apptheme.h \
           logobutton.h
FORMS += addrulegroup.ui \
         addrulewindow.ui \
         datafolderchusedialog.ui \
         debugviewer.ui \
         feecalculator.ui \
         newpassworddialog.ui \
         passworddialog.ui \
         percentpicker.ui \
         Trader Shibe.ui \
         rulewidget.ui \
         thisfeatureunderdevelopment.ui \
         translationabout.ui \
         translationdialog.ui \
         updaterdialog.ui \
         logobutton.ui
SOURCES += aboutdialog.cpp \
           addrulegroup.cpp \
           addrulewindow.cpp \
           audioplayer.cpp \
           currencypairitem.cpp \
           datafolderchusedialog.cpp \
           debugviewer.cpp \
           depthitem.cpp \
           depthmodel.cpp \
           exchange.cpp \
           exchange_bitfinex.cpp \
           exchange_cryptsy.cpp \
           exchange_cryptsy.cpp \
           exchange_btce.cpp \
           exchange_cryptsy.cpp \
           feecalculator.cpp \
           historyitem.cpp \
           historymodel.cpp \
           Shibeaes256.cpp \
           Shibehttp.cpp \
           Shibelightchanges.cpp \
           Shibersa.cpp \
           Shibescrolluponidle.cpp \
           Shibespinboxfix.cpp \
           Shibespinboxpicker.cpp \
           Shibetranslator.cpp \
           logthread.cpp \
           main.cpp \
           newpassworddialog.cpp \
           orderitem.cpp \
           ordersmodel.cpp \
           orderstablecancelbutton.cpp \
           passworddialog.cpp \
           percentpicker.cpp \
           Trader Shibe.cpp \
           ruleholder.cpp \
           rulesmodel.cpp \
           rulewidget.cpp \
           thisfeatureunderdevelopment.cpp \
           tradesitem.cpp \
           tradesmodel.cpp \
           translationdialog.cpp \
           translationline.cpp \
           updaterdialog.cpp \
           apptheme.cpp \
           logobutton.cpp
#
# Resources
# 
RESOURCES += QtResource.qrc

#
# Platform dependent stuff
#
unix:!macx {
UI_DIR = .ui
MOC_DIR = .moc
OBJECTS_DIR = .obj
isEmpty( PREFIX ) {
    PREFIX=/usr
}
isEmpty( DESKTOPDIR ) {
    DESKTOPDIR=/usr/share/applications
}
isEmpty( ICONDIR ) {
    ICONDIR=/usr/share/pixmaps
}

target.path = $${PREFIX}/bin

INSTALLS = target

desktop.path = $${DESKTOPDIR}

desktop.files = Trader Shibe.desktop
INSTALLS += desktop

icon.path = $${ICONDIR}

icon.files = Trader Shibe.png
INSTALLS += icon
}
################################
win32 {
RC_FILE = WinResource.rc
}

macx:ICON = $${PWD}/Trader Shibe.icns