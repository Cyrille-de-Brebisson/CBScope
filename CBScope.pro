QT += quick quickcontrols2 printsupport multimedia multimedia-private
CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += qtquickcompiler

#QMAKE_EXTRA_TARGETS += builddatehook svnverhook
#PRE_TARGETDEPS +=builddate svnver


# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include(source.pri)
include(language/language.pri)

CONFIG+=sdk_no_version_check

QMAKE_TARGET_BUNDLE_PREFIX = com.brebisson
QMAKE_BUNDLE = CBScope

RESOURCES += \
    resources.qrc

OTHER_FILES += \
    qml/main.qml \
    qml/AppTheme.qml \



# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH += src

VPATH+= src

win32 {
    CONFIG(release, debug|release) {
      CONFIGTYPE = Release
      DEFINES += NDEBUG
      CONFIG += static
    }

    CONFIG(debug, debug|release) {
      CONFIGTYPE = Debug
      DEFINES += _DEBUG
    }

    DEPLOY_COMMAND = windeployqt
    DELETE_COMMAND = rmdir
    DELETE_COMMAND_POST = /s /q
    COPY_COMMAND = xcopy
    COPY_COMMAND_POST = /y
    MAKEDIR_COMMAND = mkdir

    DESTDIR = $$CONFIGTYPE/Win32
    RC_ICONS = Resources/icon.ico
}

android {
    QT += androidextras

    CONFIG(release, debug|release) {
      CONFIGTYPE = Release
      DEFINES += NDEBUG
    }

    CONFIG(debug, debug|release) {
      CONFIGTYPE = Debug
      DEFINES += _DEBUG
    }

    DESTDIR = $$CONFIGTYPE/Android
    DEPLOYMENTFOLDERS += help
}

mac {

    CONFIG(debug, debug|release) {
      CONFIGTYPE = Debug
      ios {
        DEFINES += IOS
        DESTDIR = Debug/IOS
      } else {
        DEFINES += OSX
        DESTDIR = Debug/MAC
      }
      DEFINES += _DEBUG
    } else {
      CONFIGTYPE = Release
      ios {
        DEFINES += IOS
        DESTDIR = Release/IOS
      } else {
        DEFINES += OSX
        DESTDIR = Release/MAC
        OBJECTS_DIR = ./Intermediate/$$DESTDIR
      }
      DEFINES += NDEBUG
    }

    QMAKE_CFLAGS_WARN_ON -= -Wall -W
    QMAKE_CFLAGS_WARN_ON += -Wno-unused-parameter -Wno-null-conversion -Wno-parentheses -Wno-empty-body -Wno-backslash-newline-escape -Wno-c++11-extensions -Wno-missing-braces \
                              -Wno-address-of-temporary -Wno-unused-value -Wno-backslash-newline-escape -Wno-ignored-attributes -Wno-parentheses -Wno-comment -Wno-invalid-offsetof \
                              -Wno-switch -Wno-reorder -Wno-extra-tokens -Wno-c++11-compat-deprecated-writable-strings -Wno-absolute-value -Wno-unsequenced -Wno-unknown-pragmas -Wno-c++11-narrowing
    QMAKE_CXXFLAGS_WARN_ON -= -Wall -W
    QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter -Wno-null-conversion -Wno-parentheses -Wno-empty-body -Wno-backslash-newline-escape -Wno-c++11-extensions -Wno-missing-braces \
                              -Wno-address-of-temporary -Wno-unused-value -Wno-backslash-newline-escape -Wno-ignored-attributes -Wno-parentheses -Wno-comment -Wno-invalid-offsetof \
                              -Wno-switch -Wno-reorder -Wno-extra-tokens -Wno-c++11-compat-deprecated-writable-strings -Wno-absolute-value -Wno-unsequenced -Wno-unknown-pragmas -Wno-c++11-narrowing

    ICON = Resources/icon.icns

    DEPLOY_COMMAND = macdeployqt

    CONFIG(debug, debug|release) {
      DEPLOY_OPTIONS = -always-overwrite -use-debug-libs -libpath=$$PWD/../../FIRConnKit/Core/3rd_party/QtZeroConf/
    } else {
      DEPLOY_OPTIONS = -always-overwrite -libpath=$$PWD/../../FIRConnKit/Core/3rd_party/QtZeroConf/
    }

    CONFIG(debug, debug|release) {
      DEPLOY_OPTIONS = -always-overwrite -use-debug-libs
    } else {
      DEPLOY_OPTIONS = -always-overwrite
    }

    DELETE_COMMAND = rm -f
    COPY_COMMAND = cp
    #COPY_COMMAND_POST =
    MAKEDIR_COMMAND = mkdir -p


}

#QMAKE_CXXFLAGS_WARN_OFF+= -Wpadded
#QMAKE_CFLAGS_WARN_OFF+= -Wpadded

OBJECTS_DIR = ./Intermediate/$$DESTDIR
MOC_DIR= ./Intermediate/$$DESTDIR

DISTFILES += \
    qml/MyMultiText.qml \
    qml/MyOText.qml \
    android/AndroidManifest.xml \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradlew \
    android/res/values/libs.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew.bat \
    android/AndroidManifest.xml \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradlew \
    android/res/values/libs.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew.bat \
    android/AndroidManifest.xml \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradlew \
    android/res/values/libs.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew.bat

HEADERS +=

SOURCES +=

contains(ANDROID_TARGET_ARCH,armeabi-v7a) {
    ANDROID_PACKAGE_SOURCE_DIR = \
        $$PWD/android
}

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
