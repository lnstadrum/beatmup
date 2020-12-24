/*
    Beatmup image and signal processing library
    Copyright (C) 2019, lnstadrum

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

#include "../bitmap/content_lock.h"
#include "../gpu/pipeline.h"
#include "../gpu/program.h"

namespace Beatmup {
    /**
        Stores the rendering context: current program, current mapping in the scene space, common rendering properties,
        locked bitmaps, etc. Provides a set of tools and shortcuts to access rendering programs routines.
    */
    class RenderingContext : public BitmapContentLock {
        friend class SceneRenderer;
    public:
        class EventListener {
        public:
            virtual void onRenderingStart() = 0;
        };

    private:
        EventListener* eventListener;
        AffineMapping mapping;

        ImageResolution outputResolution;
        const float outputWidth;
        const bool renderingOnScreen;

        GraphicPipeline& gpu;
        RenderingContext(GraphicPipeline& gpu, EventListener* eventListener, const ImageResolution& outputResolution, const float refWidth, const bool renderingOnScreen);

    public:
        void lockBitmap(AbstractBitmap* bitmap);
        void unlockBitmap(AbstractBitmap* bitmap);
        GraphicPipeline& getGpu() { return gpu; }

        const ImageResolution& getOutputResolution() const { return outputResolution; }

        /**
            Initiates the rendering operation.
        */
        void blend();

        void enableProgram(GL::RenderingPrograms::Operation operation);
        GL::Program& getProgram();

        void bindMask(AbstractBitmap& mask);

        void setMapping(const AffineMapping& mapping) { this->mapping = mapping; }
        const AffineMapping& getMapping() const { return mapping; }

        const float getOutputWidth() const { return outputWidth; }
        const bool isRenderingOnScreen() const { return renderingOnScreen; }
    };

}
