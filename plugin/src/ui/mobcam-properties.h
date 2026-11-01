/*
 * MobCam - OBS Studio Plugin
 * Copyright (C) 2026 Yash Rayjada (YBR Digital)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef MOBCAM_PROPERTIES_H
#define MOBCAM_PROPERTIES_H

#include <obs-module.h>

#ifdef __cplusplus
extern "C" {
#endif

obs_properties_t *mobcam_get_properties(void *data);
void mobcam_get_defaults(obs_data_t *settings);

#ifdef __cplusplus
}
#endif

#endif // MOBCAM_PROPERTIES_H
