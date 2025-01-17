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

#include "sfdkglobal.h"

namespace Sfdk {
namespace Log {

Q_LOGGING_CATEGORY(lib, "sfdk.lib", QtInfoMsg)
Q_LOGGING_CATEGORY(vms, "sfdk.vms", QtInfoMsg)
Q_LOGGING_CATEGORY(queue, "sfdk.queue", QtInfoMsg)
Q_LOGGING_CATEGORY(device, "sfdk.device", QtInfoMsg)
Q_LOGGING_CATEGORY(emulator, "sfdk.emulator", QtInfoMsg)
Q_LOGGING_CATEGORY(engine, "sfdk.engine", QtInfoMsg)

} // namespace Log
} // namespace Sfdk
