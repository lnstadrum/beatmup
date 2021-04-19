/*
    Beatmup image and signal processing library
    Copyright (C) 2021, lnstadrum

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
#include "../exception.h"

#include <cstdint>
#include <fcntl.h>
#include <unistd.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <gbm.h>

namespace Beatmup {
    /**
     * Low-level DRM/GBM primitives.
     * This file is not expected to be included in the application code, and only used by the graphic pipeline to get access to the GPU.
     * Following RAII paradigm and enabling move construction to free acquired resources in case of error and move them to persistent storages
     * when everything is set up.
     */
    namespace DRM {
        class DRMError : public Beatmup::Exception {
        public:
            DRMError(const std::string& info) : Exception(info.c_str()) {}
        };

        class Device {
            Device(const Device&) = delete;
        private:
            int device;
        public:
            Device(): device(0) {}

            Device(const char* path) {
                device = open(path, O_RDWR | O_CLOEXEC);
                if (device == 0)
                    throw DRMError(std::string("Cannot open DRM device") + path);
            }

            ~Device() {
                if (device != 0)
                    close(device);
            }

            Device& operator=(Device&& other) {
                device = other.device;
                other.device = 0;
                return *this;
            }

            int getHandle() { return device; }

            operator bool() const {
                return device != 0;
            }
        };


        class ModeResources {
            ModeResources(const ModeResources&) = delete;
        private:
            drmModeRes* resources;
        public:
            ModeResources(): resources(nullptr) {}

            ModeResources(Device& device) {
                resources = drmModeGetResources(device.getHandle());
                if (!resources)
                    throw DRMError("Cannot get DRM resources");
            }

            ~ModeResources() {
                if (resources)
                    drmModeFreeResources(resources);
            }

            drmModeRes* getPointer() { return resources; }
        };


        class ModeConnector {
            ModeConnector(const ModeConnector&) = delete;
        private:
            drmModeConnector* connector;
        public:
            ModeConnector(): connector(nullptr) {}

            ModeConnector(Device& device, ModeResources& resources): connector(nullptr) {
                auto* resPtr = resources.getPointer();
                for (int i = 0; i < resPtr->count_connectors; ++i) {
                    connector = drmModeGetConnector(device.getHandle(), resPtr->connectors[i]);
                    if (connector->connection == DRM_MODE_CONNECTED)
                        break;
                }
                if (!connector)
                    throw DRMError("Cannot get DRM connector");
            }

            ~ModeConnector() {
                if (connector)
                    drmModeFreeConnector(connector);
            }

            ModeConnector& operator=(ModeConnector&& other) {
                connector = other.connector;
                other.connector = nullptr;
                return *this;
            }

            drmModeConnector* getPointer() { return connector; }

            const drmModeModeInfo& getMode(int index) const { return connector->modes[index]; }
        };


        class ModeEncoder {
            ModeEncoder(const ModeEncoder&) = delete;
        private:
            drmModeEncoder* encoder;
        public:
            ModeEncoder(): encoder(nullptr) {}

            ModeEncoder(Device& device, ModeConnector& connector): encoder(nullptr) {
                auto id = connector.getPointer()->encoder_id;
                if (!id)
                    throw DRMError("Cannot get DRM encoder");
                encoder = drmModeGetEncoder(device.getHandle(), id);
            }

            ~ModeEncoder() {
                if (encoder)
                    drmModeFreeEncoder(encoder);
            }

            ModeEncoder& operator=(ModeEncoder&& other) {
                encoder = other.encoder;
                other.encoder = nullptr;
                return *this;
            }


            drmModeEncoder* getPointer() { return encoder; }
        };


        class ModeCrtc {
            ModeCrtc(const ModeCrtc&) = delete;
        private:
            int device;
            uint32_t connector;
            drmModeCrtc* crtc;
        public:
            ModeCrtc(): crtc(nullptr) {}

            ModeCrtc(Device& device, ModeEncoder& encoder, ModeConnector& connector):
                device(device.getHandle()),
                connector(connector.getPointer()->connector_id),
                crtc(drmModeGetCrtc(this->device, encoder.getPointer()->crtc_id))
            {
                if (!crtc)
                    throw DRMError("Cannot get DRM crtc");
            }

            ~ModeCrtc() {
                if (crtc) {
                    drmModeSetCrtc(device, crtc->crtc_id, crtc->buffer_id, crtc->x, crtc->y, &connector, 1, &crtc->mode);
                    drmModeFreeCrtc(crtc);
                }
            }

            ModeCrtc& operator=(ModeCrtc&& other) {
                device = other.device;
                connector = other.connector;
                crtc = other.crtc;
                other.crtc = nullptr;
                return *this;
            }
        };


        class GBMDevice {
            GBMDevice(const GBMDevice&) = delete;
        private:
            struct gbm_device *device;
        public:
            GBMDevice(): device(nullptr) {}

            GBMDevice(Device& driDevice, ModeConnector& connector): device(nullptr) {
                device = gbm_create_device(driDevice.getHandle());
                if (!device)
                    throw DRMError("Cannot get GBM device");
            }

            ~GBMDevice() {
                if (device)
                    gbm_device_destroy(device);
            }

            GBMDevice& operator=(GBMDevice&& other) {
                device = other.device;
                other.device = nullptr;
                return *this;
            }

            struct gbm_device* getPointer() {
                return device;
            }
        };


        class GBMSurface {
            GBMSurface(const GBMSurface&) = delete;
        private:
            struct gbm_surface *surface;
        public:
            static const int FORMAT = GBM_FORMAT_XRGB8888;

            GBMSurface(): surface(nullptr) {}

            GBMSurface(GBMDevice& gbmDevice, const ModeConnector& connector): surface(nullptr) {
                const auto& mode = connector.getMode(0);

                surface = gbm_surface_create(
                    gbmDevice.getPointer(),
                    mode.hdisplay,
                    mode.vdisplay,
                    FORMAT,
                    GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING
                );
                if (!surface)
                    throw DRMError("Cannot create GBM surface");
            }

            ~GBMSurface() {
                if (surface)
                    gbm_surface_destroy(surface);
            }

            GBMSurface& operator=(GBMSurface&& other) {
                surface = other.surface;
                other.surface = nullptr;
                return *this;
            }

            struct gbm_surface* getPointer() {
                return surface;
            }
        };

    }
}