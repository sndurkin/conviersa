QT += webkit \
    network
CONFIG += qt \
    windows \
    embed_manifest_exe
INCLUDEPATH = inc/
DESTDIR = ../bin/
OTHER_FILES += todo.txt \
    doc/License.txt
HEADERS += inc/Window.h \
    inc/WindowManager.h \
    inc/WindowContainer.h \
    inc/SearchBar.h \
    inc/qext.h \
    inc/IrcWindowScrollBar.h \
    inc/IrcTypes.h \
    inc/IrcTextBlockData.h \
    inc/IrcStatusWindow.h \
    inc/IrcServerInfoService.h \
    inc/IrcPrivWindow.h \
    inc/IrcParser.h \
    inc/IrcChanWindow.h \
    inc/IrcChanUser.h \
    inc/IrcChanTopicDelegate.h \
    inc/IrcChanListWindow.h \
    inc/IIrcWindow.h \
    inc/IChatWindow.h \
    inc/definitions.h \
    inc/Connection.h \
    inc/ConfigOption.h \
    inc/ConfigManager.h \
    inc/CLineEdit.h \
    inc/Client.h \
    inc/AltWindowContainer.h
SOURCES += src/Window.cpp \
    src/WindowManager.cpp \
    src/WindowContainer.cpp \
    src/SearchBar.cpp \
    src/qext.cpp \
    src/main.cpp \
    src/IrcWindowScrollBar.cpp \
    src/IrcTextBlockData.cpp \
    src/IrcStatusWindow.cpp \
    src/IrcServerInfoService.cpp \
    src/IrcPrivWindow.cpp \
    src/IrcParser.cpp \
    src/IrcChanWindow.cpp \
    src/IrcChanUser.cpp \
    src/IrcChanTopicDelegate.cpp \
    src/IrcChanListWindow.cpp \
    src/IIrcWindow.cpp \
    src/IChatWindow.cpp \
    src/Connection.cpp \
    src/ConfigManager.cpp \
    src/CLineEdit.cpp \
    src/Client.cpp \
    src/AltWindowContainer.cpp
