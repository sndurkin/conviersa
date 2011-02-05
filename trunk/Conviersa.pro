QT += webkit \
    network
CONFIG += qt \
    windows \
    embed_manifest_exe
INCLUDEPATH = inc/
DESTDIR = bin/
OTHER_FILES += todo.txt \
    doc/License.txt
HEADERS += inc/cv/qext.h \
    inc/cv/Connection.h \
    inc/cv/ChannelUser.h \
    inc/cv/Session.h \
    inc/cv/Parser.h \
    inc/cv/ConfigOption.h \
    inc/cv/ConfigManager.h \
    inc/cv/FastDelegate.h \
    inc/cv/gui/definitions.h \
    inc/cv/gui/Client.h \
    inc/cv/gui/Window.h \
    inc/cv/gui/WindowManager.h \
    inc/cv/gui/WindowContainer.h \
    inc/cv/gui/AltWindowContainer.h \
    inc/cv/gui/CLineEdit.h \
    inc/cv/gui/SearchBar.h \
    inc/cv/gui/OutputWindow.h \
    inc/cv/gui/OutputWindowScrollBar.h \
    inc/cv/gui/StatusWindow.h \
    inc/cv/gui/QueryWindow.h \
    inc/cv/gui/ChannelWindow.h \
    inc/cv/gui/ChannelTopicDelegate.h \
    inc/cv/gui/ChannelListWindow.h \
    inc/cv/EventManager.h \
    inc/cv/gui/InputOutputWindow.h \
    inc/cv/gui/OutputControl.h \
    inc/cv/gui/OverlayPanel.h \
    inc/cv/gui/ServerConnectionPanel.h \
    inc/cv/gui/DebugWindow.h
SOURCES += src/cv/main.cpp \
    src/cv/qext.cpp \
    src/cv/Connection.cpp \
    src/cv/ChannelUser.cpp \
    src/cv/Parser.cpp \
    src/cv/Session.cpp \
    src/cv/ConfigManager.cpp \
    src/cv/gui/Client.cpp \
    src/cv/gui/Window.cpp \
    src/cv/gui/WindowManager.cpp \
    src/cv/gui/WindowContainer.cpp \
    src/cv/gui/AltWindowContainer.cpp \
    src/cv/gui/CLineEdit.cpp \
    src/cv/gui/SearchBar.cpp \
    src/cv/gui/OutputWindow.cpp \
    src/cv/gui/OutputWindowScrollBar.cpp \
    src/cv/gui/StatusWindow.cpp \
    src/cv/gui/QueryWindow.cpp \
    src/cv/gui/ChannelWindow.cpp \
    src/cv/gui/ChannelTopicDelegate.cpp \
    src/cv/gui/ChannelListWindow.cpp \
    src/cv/EventManager.cpp \
    src/cv/gui/InputOutputWindow.cpp \
    src/cv/gui/OutputControl.cpp \
    src/cv/gui/OverlayPanel.cpp \
    src/cv/gui/ServerConnectionPanel.cpp \
    src/cv/gui/DebugWindow.cpp
