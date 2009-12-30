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
    inc/IrcTypes.h \
    inc/IrcTextBlockData.h \
    inc/IrcStatusWindow.h \
    inc/IrcPrivWindow.h \
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
    inc/AltWindowContainer.h \
    inc/OutputWindowScrollBar.h \
    inc/Parser.h \
    inc/Session.h
SOURCES += src/Window.cpp \
    src/WindowManager.cpp \
    src/WindowContainer.cpp \
    src/SearchBar.cpp \
    src/qext.cpp \
    src/main.cpp \
    src/IrcTextBlockData.cpp \
    src/IrcStatusWindow.cpp \
    src/IrcPrivWindow.cpp \
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
    src/AltWindowContainer.cpp \
    src/OutputWindowScrollBar.cpp \
    src/Parser.cpp \
    src/Session.cpp
