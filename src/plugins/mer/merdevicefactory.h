/****************************************************************************
**
** Copyright (C) 2012 - 2014 Jolla Ltd.
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

#ifndef MERDEVICEFACTORY_H
#define MERDEVICEFACTORY_H

#include <projectexplorer/devicesupport/idevicefactory.h>

namespace Mer {
namespace Internal {

class MerDeviceFactory : public ProjectExplorer::IDeviceFactory
{
    Q_OBJECT

public:
    static MerDeviceFactory *instance();
    MerDeviceFactory();
    ~MerDeviceFactory();

    QString displayNameForId(Core::Id type) const override;
    QList<Core::Id> availableCreationIds() const override;
    QIcon iconForId(Core::Id type) const override;
    static bool canCreate(Core::Id id);
    ProjectExplorer::IDevice::Ptr create(Core::Id id) const override;
    bool canRestore(const QVariantMap &map) const override;
    ProjectExplorer::IDevice::Ptr restore(const QVariantMap &map) const override;

signals:
    void deviceCreated(const ProjectExplorer::IDevice::Ptr &device);

private:
    static MerDeviceFactory *s_instance;
};

} // Internal
} // Mer

#endif // MERDEVICEFACTORY_H
