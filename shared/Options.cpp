/*
 * SPDX license identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2021 Alin Popa
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * \author Alin Popa <alin.popa@fxdata.ro>
 */

#include "Options.h"
#include "Defaults.h"

using namespace std;

namespace tkm::monitor
{

Options::Options(const string &configFile)
{
    m_configFile = std::make_shared<bswi::kf::KeyFile>(configFile);
    if (m_configFile->parseFile() != 0) {
        logWarn() << "Fail to parse config file: " << configFile;
        m_configFile.reset();
    }
}

auto Options::getFor(Key key) -> string const
{
    switch (key) {
    case Key::EnablePSI:
        if (hasConfigFile()) {
            const optional<string> prop
                = m_configFile->getPropertyValue("monitor", -1, "EnablePSI");
            return prop.value_or(tkmDefaults.getFor(Defaults::Default::EnablePSI));
        }
        return tkmDefaults.getFor(Defaults::Default::EnablePSI);
    default:
        logError() << "Unknown option key";
        break;
    }

    throw std::runtime_error("Cannot provide option for key");
}

} // namespace tkm::monitor
