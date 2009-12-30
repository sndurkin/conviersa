QT += webkit \
    network
CONFIG += qt \
    windows \
    embed_manifest_exe
INCLUDEPATH = inc/
DESTDIR = ../bin/
OTHER_FILES += todo.txt \
    doc/License.txt
HEADERS += inc/WindowManager.h \
    inc/WindowContainer.h \
    inc/SearchBar.h \
    inc/qext.h \
    inc/OutputWindowScrollBar.h \
    inc/IrcTypes.h \
    inc/IrcTextBlockData.h \
    inc/IrcStatusWindow.h \
    inc/irc/Session.h \
    inc/IrcPrivWindow.h \
    inc/irc/Parser.h \
    inc/IrcChanWindow.h \
    inc/IrcChanUser.h \
    inc/IrcChanTopicDelegate.h \
    inc/IrcChanListWindow.h \
    inc/IIrcWindow.h \
    inc/IChatWindow.h \
    inc/definitions.h \
    inc/irc/Connection.h \
    inc/ConfigOption.h \
    inc/ConfigManager.h \
    inc/CLineEdit.h \
    inc/Client.h \
    inc/AltWindowContainer.h \
    inc/Window.h \
    inc/irc/Channel.h
SOURCES += src/WindowManager.cpp \
    src/WindowContainer.cpp \
    src/SearchBar.cpp \
    src/qext.cpp \
    src/main.cpp \
    src/OutputWindowScrollBar.cpp \
    src/IrcTextBlockData.cpp \
    src/IrcStatusWindow.cpp \
    src/irc/Session.cpp \
    src/IrcPrivWindow.cpp \
    src/irc/Parser.cpp \
    src/IrcChanWindow.cpp \
    src/IrcChanUser.cpp \
    src/IrcChanTopicDelegate.cpp \
    src/IrcChanListWindow.cpp \
    src/IIrcWindow.cpp \
    src/IChatWindow.cpp \
    src/irc/Connection.cpp \
    src/ConfigManager.cpp \
    src/CLineEdit.cpp \
    src/Client.cpp \
    src/AltWindowContainer.cpp \
    src/Window.cpp \

