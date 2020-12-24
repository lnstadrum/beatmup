/*
    Beatmup image and signal processing library
    Copyright (C) 2020, lnstadrum

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

#include "model.h"
#include "inference_task.h"
#include "../bitmap/abstract_bitmap.h"
#include "softmax.h"
#include <vector>
#include <initializer_list>


namespace Beatmup {
    namespace NNets {
        /**
            Image classifier base class.
            Combines a runnable InferenceTask with a Model assuming Conv2D and Softmax are its first and last operations respectively.
        */
        class Classifier : public Model, public InferenceTask {
        protected:
            Context& context;

        public:
            /**
                Creates a Classifier instance.
                \param context      A context instance Classifier will use to store its internal data
                \param data         A ChunkCollection to access to the classifier model data
            */
            Classifier(Context& context, ChunkCollection& data);
            ~Classifier();

            /**
                Classifies an image (blocking).
                The very first call includes the model preparation and might be slow as hell. Subsequent calls only run the inference and are likely
                much faster.
                \param[in] input    The input image
                \return a vector of probabilities per class.
            */
            const std::vector<float>& operator()(AbstractBitmap& input);

            /**
                Initiates the classification of a given image.
                The call is non-blocking.
                \param[in] input    The input image
                \return a job corresponding to the submitted task.
            */
            Job start(AbstractBitmap& input);

            /**
                Returns the last classification results.
                \return a vector of probabilities per class.
            */
            inline const std::vector<float>& getProbabilities() const {
                return static_cast<const Softmax&>(getLastOperation()).getProbabilities();
            }
        };
    }
}
