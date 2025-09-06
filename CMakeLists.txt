cmake_minimum_required(VERSION 3.5)

project(Notebook VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Xml Qml Concurrent)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Xml Qml Concurrent)
add_definitions(-DQSCINTILLA_DLL)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        SqlFormat/SqlParser.cpp
        SqlFormat/SqlParser.h
        SqlFormat/SqlHighlighter.cpp
        SqlFormat/SqlHighlighter.h
        CodeEditor/CodeEditor.h
        CodeEditor/CodeEditor.cpp
        SqlParserWindow/SqlParserWindow.h
        SqlParserWindow/SqlParserWindow.cpp
        DiffFunction/myersDiff.h
        DiffFunction/myersDiff.cpp
        DiffFunction/DiffWidget.h
        DiffFunction/DiffWidget.cpp
        jsvariablereplacer/jsvariablereplacer.h
        jsvariablereplacer/jsvariablereplacer.cpp
        qrc.qrc

        folderDiff/util/zfile.h
        folderDiff/widget/tree/ztreemodel.h
        folderDiff/widget/tree/ztreeitem.h
        folderDiff/widget/ztopwidget.h
        folderDiff/widget/zcenterwidget.h
        folderDiff/diff/zfilediffmodel.h
        folderDiff/diff/zfilediff.h
        folderDiff/diff/zpathdiffmodel.h
        folderDiff/diff/zpathdiff.h
        folderDiff/ctl/zfolderctl.h
        folderDiff/widget/tree/ztreeitemmodel.h
        folderDiff/env/zcons.h
        folderDiff/widget/folder/zfolderwidget.h
        folderDiff/widget/file/zfilewidget.h
        folderDiff/widget/file/zdiffinfo.h
        folderDiff/widget/file/zlinenumberwidget.h
        folderDiff/widget/file/zdiffareawidget.h
        folderDiff/widget/file/ztextwidget.h
        folderDiff/widget/file/zscrolltextwidget.h
        folderDiff/widget/file/zdiffarea.h
        folderDiff/FolderCompareWidget.h
        folderDiff/CompareDialog.h

        folderDiff/util/zfile.cpp
        folderDiff/widget/tree/ztreemodel.cpp
        folderDiff/widget/tree/ztreeitem.cpp
        folderDiff/widget/ztopwidget.cpp
        folderDiff/widget/zcenterwidget.cpp
        folderDiff/diff/zfilediff.cpp
        folderDiff/diff/zfilediffmodel.cpp
        folderDiff/diff/zpathdiffmodel.cpp
        folderDiff/diff/zpathdiff.cpp
        folderDiff/ctl/zfolderctl.cpp
        folderDiff/widget/tree/ztreeitemmodel.cpp
        folderDiff/widget/folder/zfolderwidget.cpp
        folderDiff/widget/file/zfilewidget.cpp
        folderDiff/widget/file/zdiffinfo.cpp
        folderDiff/widget/file/zlinenumberwidget.cpp
        folderDiff/widget/file/zdiffareawidget.cpp
        folderDiff/widget/file/ztextwidget.cpp
        folderDiff/widget/file/zscrolltextwidget.cpp
        folderDiff/widget/file/zdiffarea.cpp
        folderDiff/FolderCompareWidget.cpp
        folderDiff/CompareDialog.cpp




)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Notebook
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(Notebook SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(Notebook
            ${PROJECT_SOURCES}
        )
    endif()
endif()


#链接qscintilla
set(QScintillaLib_DIR "${CMAKE_SOURCE_DIR}/QScintilla")
set(QScintillaLib_INCLUDE_DIRS "${QScintillaLib_DIR}/include")
include_directories(${QScintillaLib_INCLUDE_DIRS})
link_directories(${QScintillaLib_DIR}/lib)
set(QScintillaLib_LIBS "${QScintillaLib_DIR}/lib/qscintilla2_qt5.lib")

target_include_directories(Notebook PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/folderDiff
)



target_link_libraries(Notebook PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Xml
    Qt${QT_VERSION_MAJOR}::Qml
    Qt${QT_VERSION_MAJOR}::Concurrent
    ${QScintillaLib_LIBS}
)

set_target_properties(Notebook PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS Notebook
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(WIN32)
    set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/notebook.rc")
    target_sources(Notebook PRIVATE ${APP_ICON_RESOURCE_WINDOWS})
endif()

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Notebook)
endif()
