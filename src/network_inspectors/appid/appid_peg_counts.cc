//--------------------------------------------------------------------------
// Copyright (C) 2014-2017 Cisco and/or its affiliates. All rights reserved.
// Copyright (C) 2005-2013 Sourcefire, Inc.
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License Version 2 as published
// by the Free Software Foundation.  You may not use, modify or distribute
// this program under any other version of the GNU General Public License.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//--------------------------------------------------------------------------

// appid_peg_counts.cc author davis mcpherson <davmcphe@cisco.com>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "appid_peg_counts.h"
#include "app_info_table.h"

#include <algorithm>
#include <string>

bool AppIdPegCounts::detectors_configured = false;
std::map<AppId, uint32_t> AppIdPegCounts::appid_detector_pegs_idx;
std::vector<PegInfo> AppIdPegCounts::appid_detectors_peg_info;
std::vector<PegInfo> AppIdPegCounts::appid_pegs =
{
    { CountType::SUM, "packets", "count of packets received" },
    { CountType::SUM, "processed_packets", "count of packets processed" },
    { CountType::SUM, "ignored_packets", "count of packets ignored" },
    { CountType::SUM, "total_sessions", "count of sessions created" },
};

THREAD_LOCAL std::vector<PegCount>* AppIdPegCounts::appid_peg_counts;

void AppIdPegCounts::init_pegs()
{
    appid_peg_counts = new std::vector<PegCount>(appid_detectors_peg_info.size() + NUM_APPID_GLOBAL_PEGS, 0);
}

void AppIdPegCounts::cleanup_pegs()
{
    delete appid_peg_counts;
}

void AppIdPegCounts::init_detector_peg_info(const std::string& app_name, const std::string& name_suffix,
    const std::string& help_suffix)
{
    std::string name = app_name + name_suffix;
    std::string help = "count of ";
    help += app_name + help_suffix;
    appid_detectors_peg_info.push_back({CountType::SUM, snort_strdup(name.c_str()), snort_strdup(help.c_str())});
}

void AppIdPegCounts::add_app_peg_info(AppInfoTableEntry& entry, AppId app_id )
{
    std::string app_name = entry.app_name_key;
    std::replace(app_name.begin(), app_name.end(), ' ', '_');

    appid_detector_pegs_idx[app_id] = appid_detectors_peg_info.size() + NUM_APPID_GLOBAL_PEGS;
    init_detector_peg_info(app_name, "_flows", " services detected");
    init_detector_peg_info(app_name, "_clients", " clients detected");
    init_detector_peg_info(app_name, "_users", " users detected");
    init_detector_peg_info(app_name, "_payloads", " payloads detected");
    init_detector_peg_info(app_name, "_misc", " misc detected");
}

PegCount* AppIdPegCounts::get_peg_counts()
{
    if ( detectors_configured )
        return appid_peg_counts->data();
    else
        return nullptr;
}

PegInfo* AppIdPegCounts::get_peg_info()
{
    if ( detectors_configured )
    {
        if ( appid_detectors_peg_info.size() )
            appid_pegs.insert( appid_pegs.end(), appid_detectors_peg_info.begin(), appid_detectors_peg_info.end());

        // add the sentinel entry at the end
        appid_pegs.push_back({ CountType::END, nullptr, nullptr });
        return appid_pegs.data();
    }
    else
        return nullptr;
}

