QT += webkit \
    network
CONFIG += qt \
    windows \
    embed_manifest_exe
INCLUDEPATH = inc/
DESTDIR = ../bin/
OTHER_FILES += todo.txt \
    doc/License.txt
HEADERS += inc/irc/Connection.h \
    inc/irc/ChannelUser.h \
    inc/irc/Session.h \
    inc/irc/Parser.h \
    inc/cv/definitions.h \
    inc/cv/qext.h \
    inc/cv/Client.h \
    inc/cv/Window.h \
    inc/cv/WindowManager.h \
    inc/cv/WindowContainer.h \
    inc/cv/AltWindowContainer.h \
    inc/cv/CLineEdit.h \
    inc/cv/SearchBar.h \
    inc/cv/ConfigOption.h \
    inc/cv/ConfigManager.h \
    inc/cv/irc/types.h \
    inc/cv/irc/OutputWindow.h \
    inc/cv/irc/OutputWindowScrollBar.h \
    inc/cv/irc/StatusWindow.h \
    inc/cv/irc/QueryWindow.h \
    inc/cv/irc/ChannelWindow.h \
    inc/cv/irc/ChannelTopicDelegate.h \
    inc/cv/irc/ChannelListWindow.h
SOURCES += src/irc/Connection.cpp \
    src/irc/ChannelUser.cpp \
    src/irc/Parser.cpp \
    src/irc/Session.cpp \
    src/cv/qext.cpp \
    src/cv/main.cpp \
    src/cv/Client.cpp \
    src/cv/Window.cpp \
    src/cv/WindowManager.cpp \
    src/cv/WindowContainer.cpp \
    src/cv/AltWindowContainer.cpp \
    src/cv/CLineEdit.cpp \
    src/cv/SearchBar.cpp \
    src/cv/ConfigManager.cpp \
    src/cv/irc/OutputWindow.cpp \
    src/cv/irc/OutputWindowScrollBar.cpp \
    src/cv/irc/StatusWindow.cpp \
    src/cv/irc/QueryWindow.cpp \
    src/cv/irc/ChannelWindow.cpp \
    src/cv/irc/ChannelTopicDelegate.cpp \
    src/cv/irc/ChannelListWindow.cpp
