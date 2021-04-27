/****************************************************************************
**
** Copyright (C) 2012-2015,2019 Jolla Ltd.
** Copyright (C) 2019 Open Mobile Platform LLC.
** Contact: http://jolla.com/
**
** This file is part of Qt Creator.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** Other Usage
**
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Digia.
**
****************************************************************************/

#ifndef MERPLUGIN_H
#define MERPLUGIN_H

#include <extensionsystem/iplugin.h>

#include <QMap>

QT_BEGIN_NAMESPACE
class QVBoxLayout;
QT_END_NAMESPACE

namespace Sfdk {
class VirtualMachine;
}

namespace Mer {
namespace Internal {

class MerBuildEngineOptionsPage;
class MerEmulatorOptionsPage;

class MerPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "plugins/mer/SailfishOS.json")

public:
    MerPlugin();
    ~MerPlugin() override;

    bool initialize(const QStringList &arguments, QString *errorMessage) override;
    void extensionsInitialized() override;
    ShutdownFlag aboutToShutdown() override;

    static void saveSettings();

    static MerBuildEngineOptionsPage *buildEngineOptionsPage();
    static MerEmulatorOptionsPage *emulatorOptionsPage();

    static void workaround_ensureVirtualMachinesAreInitialized();

private slots:
    void handlePromptClosed(int result);

private:
    void onStopListEmpty();
    static void addInfoOnBuildEngineEnvironment(QVBoxLayout *vbox);
    static void ensureCustomRunConfigurationIsTheDefaultOnCompilationDatabaseProjects();
    void ensureDailyUpdateCheckForEarlyAccess();

private:
    QMap<QString, Sfdk::VirtualMachine *> m_stopList;

#ifdef WITH_TESTS
    void testMerSshOutputParsers_data();
    void testMerSshOutputParsers();
#endif
};

} // Internal
} // Mer

#endif // MERPLUGIN_H
