/****************************************************************************
**
** Copyright (C) 2017-2019 Jolla Ltd.
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

#include "merdevicedebugsupport.h"

#include "merconstants.h"
#include "meremulatordevice.h"
#include "merrunconfiguration.h"
#include "merrunconfigurationaspect.h"
#include "merqmllivebenchmanager.h"
#include "merqmlrunconfiguration.h"
#include "mersdkkitaspect.h"
#include "mersdkmanager.h"
#include "mertoolchain.h"

#include <sfdk/buildengine.h>
#include <sfdk/sfdkconstants.h>

#include <debugger/debuggerkitinformation.h>
#include <debugger/debuggerplugin.h>
#include <debugger/debuggerruncontrol.h>
#include <projectexplorer/target.h>
#include <qmakeprojectmanager/qmakeproject.h>
#include <qmldebug/qmldebugcommandlinearguments.h>
#include <qtsupport/qtkitinformation.h>
#include <remotelinux/remotelinuxdebugsupport.h>
#include <utils/portlist.h>
#include <utils/qtcassert.h>

using namespace Debugger;
using namespace ProjectExplorer;
using namespace QmakeProjectManager;
using namespace Mer;
using namespace Mer::Internal;
using namespace RemoteLinux;
using namespace Sfdk;

namespace {
const int GDB_SERVER_READY_TIMEOUT_MS = 10000;
}

class GdbServerReadyWatcher : public ProjectExplorer::RunWorker
{
    Q_OBJECT

public:
    explicit GdbServerReadyWatcher(ProjectExplorer::RunControl *runControl,
                             GdbServerPortsGatherer *gdbServerPortsGatherer)
        : RunWorker(runControl)
        , m_gdbServerPortsGatherer(gdbServerPortsGatherer)
    {
        setId("GdbServerReadyWatcher");

        connect(&m_usedPortsGatherer, &DeviceUsedPortsGatherer::error,
                this, &RunWorker::reportFailure);
        connect(&m_usedPortsGatherer, &DeviceUsedPortsGatherer::portListReady,
                this, &GdbServerReadyWatcher::handlePortListReady);
    }
    ~GdbServerReadyWatcher()
    {
    }

private:
    void start() override
    {
        appendMessage(tr("Waiting for gdbserver..."), Utils::NormalMessageFormat);
        m_usedPortsGatherer.start(device());
        m_startTime.restart();
    }

    void handlePortListReady()
    {
        if (!m_usedPortsGatherer.usedPorts().contains(m_gdbServerPortsGatherer->gdbServerPort())) {
            if (m_startTime.elapsed() > GDB_SERVER_READY_TIMEOUT_MS) {
                reportFailure(tr("Timeout waiting for gdbserver to become ready."));
                return;
            }
            m_usedPortsGatherer.start(device());
        }
        reportDone();
    }

    GdbServerPortsGatherer *m_gdbServerPortsGatherer;
    DeviceUsedPortsGatherer m_usedPortsGatherer;
    QTime m_startTime;
};

MerDeviceDebugSupport::MerDeviceDebugSupport(RunControl *runControl)
    : DebuggerRunTool(runControl)
{
    setId("MerDeviceDebugSupport");

    setUsePortsGatherer(isCppDebugging(), isQmlDebugging());

    auto gdbServer = new GdbServerRunner(runControl, portsGatherer());

    addStartDependency(gdbServer);

    setStartMode(AttachToRemoteServer);
    setCloseMode(KillAndExitMonitorAtClose);
    setUseExtendedRemote(true);

    if (isCppDebugging()) {
        auto gdbServerReadyWatcher = new GdbServerReadyWatcher(runControl, portsGatherer());
        gdbServerReadyWatcher->addStartDependency(gdbServer);
        addStartDependency(gdbServerReadyWatcher);
    }

    connect(this, &DebuggerRunTool::inferiorRunning, this, [runControl]() {
        MerQmlLiveBenchManager::notifyInferiorRunning(runControl);
    });
}

void MerDeviceDebugSupport::start()
{
    RunConfiguration *runConfig = runControl()->runConfiguration();

    if (isCppDebugging()) {
        ProjectNode *const root = runConfig->target()->project()->rootProjectNode();
        root->forEachProjectNode([this](const ProjectNode *node) {
            auto qmakeNode = dynamic_cast<const QmakeProFileNode *>(node);
            if (!qmakeNode || !qmakeNode->includedInExactParse())
                return;
            if (qmakeNode->projectType() != ProjectType::SharedLibraryTemplate)
                return;
            const QDir outPwd = QDir(qmakeNode->buildDir());
            addSolibSearchDir(outPwd.absoluteFilePath(qmakeNode->targetInformation().destDir.toString()));
        });
    }

    BuildEngine *const engine = MerSdkKitAspect::buildEngine(runConfig->target()->kit());

    if (engine && !engine->sharedHomePath().isEmpty()) {
        addSourcePathMap(Sfdk::Constants::BUILD_ENGINE_SHARED_HOME_MOUNT_POINT,
                engine->sharedHomePath().toString());
    }
    if (engine && !engine->sharedSrcPath().isEmpty()) {
        addSourcePathMap(Sfdk::Constants::BUILD_ENGINE_SHARED_SRC_MOUNT_POINT,
                engine->sharedSrcPath().toString());
    }

    DebuggerRunTool::start();
}

#include "merdevicedebugsupport.moc"
