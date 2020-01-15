/****************************************************************************
**
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

#include "sdk_p.h"

#include "asynchronous_p.h"
#include "buildengine_p.h"
#include "device_p.h"
#include "emulator_p.h"
#include "sfdkconstants.h"
#include "sfdk_version_p.h"
#include "usersettings_p.h"
#include "utils_p.h"
#include "vboxvirtualmachine_p.h"

#include <utils/algorithm.h>
#include <utils/fileutils.h>
#include <utils/qtcassert.h>

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>

using namespace Utils;

namespace Sfdk {

namespace {
const char SDK_MAINTENANCE_TOOL_DATA_FILE[] = "SDKMaintenanceTool.dat";
const char GENERAL_SETTINGS_GROUP[] = "General";
const char VBOXMANAGE_PATH[] = "VBoxManagePath";
}

/*!
 * \class Sdk
 */

Sdk *Sdk::s_instance = nullptr;

Sdk::Sdk(Options options)
    : d_ptr(std::make_unique<SdkPrivate>(this))
{
    Q_ASSERT(!s_instance);
    s_instance = this;

    Q_D(Sdk);
    QTC_ASSERT(!(options & VersionedSettings) || !(options & SystemSettingsOnly), return);
    QTC_ASSERT(!(options & SystemSettingsOnly) || !(options & CachedVmInfo),
            options &= ~CachedVmInfo);

    qCDebug(lib) << "Initializing SDK. Options:" << options;

    d->options_ = options;

    d->readGeneralSettings();

    d->commandQueue_ = std::make_unique<CommandQueue>(this);

    d->virtualMachineFactory = std::make_unique<VirtualMachineFactory>(this);
    d->virtualMachineFactory->registerType<VBoxVirtualMachine>();

    d->buildEngineManager = std::make_unique<BuildEngineManager>(this);

    connect(d->buildEngineManager.get(), &BuildEngineManager::buildEngineAdded,
            this, &Sdk::buildEngineAdded);
    connect(d->buildEngineManager.get(), &BuildEngineManager::aboutToRemoveBuildEngine,
            this, &Sdk::aboutToRemoveBuildEngine);

    d->emulatorManager = std::make_unique<EmulatorManager>(this);

    connect(d->emulatorManager.get(), &EmulatorManager::emulatorAdded,
            this, &Sdk::emulatorAdded);
    connect(d->emulatorManager.get(), &EmulatorManager::aboutToRemoveEmulator,
            this, &Sdk::aboutToRemoveEmulator);
    connect(d->emulatorManager.get(), &EmulatorManager::deviceModelsChanged,
            this, &Sdk::deviceModelsChanged);

    d->deviceManager = std::make_unique<DeviceManager>(this);

    connect(d->deviceManager.get(), &DeviceManager::deviceAdded,
            this, &Sdk::deviceAdded);
    connect(d->deviceManager.get(), &DeviceManager::aboutToRemoveDevice,
            this, &Sdk::aboutToRemoveDevice);

    if (!d->isVersionedSettingsEnabled())
        emit d->updateOnceRequested();
}

Sdk::~Sdk()
{
    Q_D(Sdk);
    d->commandQueue_->wait();
    s_instance = nullptr;
}

Sdk *Sdk::instance()
{
    return s_instance;
}

QString Sdk::osVariant(TextStyle textStyle)
{
    return tr(Constants::VARIANT_NAME) + separator(textStyle) + tr("OS");
}

QString Sdk::sdkVariant(TextStyle textStyle)
{
    return tr(Constants::VARIANT_NAME) + separator(textStyle) + tr("SDK");
}

QString Sdk::ideVariant(TextStyle textStyle)
{
    return tr(Constants::VARIANT_NAME) + separator(textStyle) + tr("IDE");
}

void Sdk::enableUpdates()
{
    QTC_ASSERT(SdkPrivate::isVersionedSettingsEnabled(), return);
    qCDebug(lib) << "Begin enable udates";
    s_instance->d_func()->updatesEnabled = true;
    emit s_instance->d_func()->enableUpdatesRequested();
    qCDebug(lib) << "End enable updates";
}

bool Sdk::isApplyingUpdates()
{
    QTC_ASSERT(SdkPrivate::isVersionedSettingsEnabled(), return false);
    return UserSettings::isApplyingUpdates();
}

bool Sdk::saveSettings(QStringList *errorStrings)
{
    QTC_ASSERT(errorStrings, return false);
    QTC_ASSERT(!SdkPrivate::useSystemSettingsOnly(), return false);
    qCDebug(lib) << "Begin save settings";
    emit s_instance->d_func()->saveSettingsRequested(errorStrings);
    qCDebug(lib) << "End save settings. Success:" << errorStrings->isEmpty();
    return errorStrings->isEmpty();
}

// FIXME Describe when this becomes initialized
QString Sdk::installationPath()
{
    return BuildEngineManager::installDir();
}

void Sdk::unusedVirtualMachines(const QObject *context,
        const Functor<const QList<VirtualMachineDescriptor> &, bool> &functor)
{
    VirtualMachineFactory::unusedVirtualMachines(context, functor);
}

QList<BuildEngine *> Sdk::buildEngines()
{
    return BuildEngineManager::buildEngines();
}

BuildEngine *Sdk::buildEngine(const QUrl &uri)
{
    return BuildEngineManager::buildEngine(uri);
}

void Sdk::createBuildEngine(const QUrl &virtualMachineUri, const QObject *context,
    const Functor<std::unique_ptr<BuildEngine> &&> &functor)
{
    BuildEngineManager::createBuildEngine(virtualMachineUri, context, functor);
}

int Sdk::addBuildEngine(std::unique_ptr<BuildEngine> &&buildEngine)
{
    return BuildEngineManager::addBuildEngine(std::move(buildEngine));
}

void Sdk::removeBuildEngine(const QUrl &uri)
{
    BuildEngineManager::removeBuildEngine(uri);
}

QList<Emulator *> Sdk::emulators()
{
    return EmulatorManager::emulators();
}

Emulator *Sdk::emulator(const QUrl &uri)
{
    return EmulatorManager::emulator(uri);
}

void Sdk::createEmulator(const QUrl &virtualMachineUri, const QObject *context,
    const Functor<std::unique_ptr<Emulator> &&> &functor)
{
    EmulatorManager::createEmulator(virtualMachineUri, context, functor);
}

int Sdk::addEmulator(std::unique_ptr<Emulator> &&emulator)
{
    return EmulatorManager::addEmulator(std::move(emulator));
}

void Sdk::removeEmulator(const QUrl &uri)
{
    EmulatorManager::removeEmulator(uri);
}

QList<DeviceModelData> Sdk::deviceModels()
{
    return EmulatorManager::deviceModels();
}

DeviceModelData Sdk::deviceModel(const QString &name)
{
    return EmulatorManager::deviceModel(name);
}

void Sdk::setDeviceModels(const QList<DeviceModelData> &deviceModels,
        const QObject *context, const Functor<bool> &functor)
{
    EmulatorManager::setDeviceModels(deviceModels, context, functor);
}

QList<Device *> Sdk::devices()
{
    return DeviceManager::devices();
}

Device *Sdk::device(const QString &id)
{
    return DeviceManager::device(id);
}

Device *Sdk::device(const Emulator &emulator)
{
    return DeviceManager::device(emulator);
}

int Sdk::addDevice(std::unique_ptr<Device> &&device)
{
    return DeviceManager::addDevice(std::move(device));
}

void Sdk::removeDevice(const QString &id)
{
    DeviceManager::removeDevice(id);
}

/*!
 * \class SdkPrivate
 */

SdkPrivate::SdkPrivate(Sdk *q)
    : QObject(q)
{
}

SdkPrivate::~SdkPrivate() = default;

QDateTime SdkPrivate::lastMaintained()
{
    // It is not available initially, in which case it is verbose, so keep quiet here.
    const QString installationPath = Sdk::installationPath();
    if (installationPath.isEmpty())
        return QDateTime::currentDateTime();

    QFileInfo maintenanceDataInfo(installationPath + '/' + SDK_MAINTENANCE_TOOL_DATA_FILE);
    QTC_ASSERT(maintenanceDataInfo.exists(), return QDateTime::currentDateTime());

    return maintenanceDataInfo.lastModified();
}

Utils::FileName SdkPrivate::libexecPath()
{
    // See ICore::libexecPath()
    return FileName::fromString(QDir::cleanPath(QCoreApplication::applicationDirPath()
                + '/' + RELATIVE_LIBEXEC_PATH));
}

Utils::FileName SdkPrivate::settingsFile(SettingsScope scope, const QString &basename)
{
    const QString prefix = scope == SessionScope
        ? QString::fromLatin1(Constants::LIB_ID) + '-'
        : QString();
    return settingsLocation(scope).appendPath(prefix + basename);
}

Utils::FileName SdkPrivate::settingsLocation(SettingsScope scope)
{
    static FileName systemLocation;
    static FileName userLocation;
    static FileName sessionLocation;

    FileName *location = nullptr;
    switch (scope) {
    case SystemScope:
        location = &systemLocation;
        break;
    case UserScope:
        location = &userLocation;
        break;
    case SessionScope:
        location = &sessionLocation;
        break;
    }
    Q_ASSERT(location);

    if (!location->isNull())
        return *location;

    QTC_CHECK(!QCoreApplication::organizationName().isEmpty());
    QTC_CHECK(!QCoreApplication::applicationName().isEmpty());

    const QSettings::Scope qscope = scope == SystemScope
        ? QSettings::SystemScope
        : QSettings::UserScope;
    const QString applicationName = scope == SessionScope
        ? QCoreApplication::applicationName()
        : QString::fromLatin1(Constants::LIB_ID);

    QSettings settings(QSettings::IniFormat, qscope, QCoreApplication::organizationName(),
            applicationName);

    // See ICore::userResourcePath()
    QTC_CHECK(settings.fileName().endsWith(".ini"));
    const auto iniInfo = QFileInfo(settings.fileName());
    const QString resourceDir = iniInfo.completeBaseName().toLower();
    *location = FileName::fromString(iniInfo.path() + '/' + resourceDir);

    qCDebug(lib) << "Settings location" << scope << *location;

    return *location;
}

Utils::FileName SdkPrivate::cacheFile(const QString &basename)
{
    return cacheLocation().appendPath(basename);
}

Utils::FileName SdkPrivate::cacheLocation()
{
    static FileName cacheLocation;
    if (!cacheLocation.isEmpty())
        return cacheLocation;

    QTC_CHECK(!QCoreApplication::organizationName().isEmpty());
    QTC_CHECK(!QCoreApplication::applicationName().isEmpty());

    const QString genericCacheLocation =
        QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation);

    QTC_CHECK(!genericCacheLocation.isEmpty());
    if (!genericCacheLocation.isEmpty()) {
        cacheLocation = FileName::fromString(genericCacheLocation)
            .appendPath(QCoreApplication::organizationName())
            .appendPath(Constants::LIB_ID);
    } else {
        cacheLocation = FileName::fromString(
                QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
    }

    qCDebug(lib) << "Cache location" << cacheLocation;

    return cacheLocation;
}

void SdkPrivate::readGeneralSettings()
{
    QSettings settings(QSettings::IniFormat, QSettings::SystemScope,
            QCoreApplication::organizationName(), QString::fromLatin1(Constants::LIB_ID));

    qCDebug(lib) << "General settings location" << settings.fileName();

    settings.beginGroup(GENERAL_SETTINGS_GROUP);
    customVBoxManagePath_ = settings.value(VBOXMANAGE_PATH).toString();
    settings.endGroup();
}

} // namespace Sfdk
