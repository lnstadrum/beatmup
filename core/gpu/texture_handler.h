/*
	A texture stored in GPU memory
*/
#pragma once
#include "../basic_types.h"
#include "recycle_bin.h"

namespace Beatmup {
	class GraphicPipeline;

	namespace GL {
		class TextureHandler : public Beatmup::Object {
			friend class ::Beatmup::GraphicPipeline;

		public:
			/**
				Texture format, specifies how the texture should be interpreted on the shader side
			*/
			enum TextureFormat {
				Rx8,
				RGBx8,
				RGBAx8,
				Rx32f,
				RGBx32f,
				RGBAx32f,
				OES_Ext			//!< external EGL image
			};

			static const char* textureFormatToString(const TextureFormat&);

		protected:
			glhandle textureHandle;

			TextureHandler();

			/**
				Prepares (eventually uploads) texture data on GPU.
				Called only by the environment managing thread.
			*/
			virtual void prepare(GraphicPipeline& gpu);

			/**
				Forces disposing the texture data, e.g. when it is not used any more
			*/
			void invalidate(RecycleBin&);

		private:
			unsigned int groupIndex;

		public:
			~TextureHandler();

			/**
				Width of the texture in pixels
			*/
			virtual const int getWidth() const = 0;

			/**
				Height of the texture in pixels
			*/
			virtual const int getHeight() const = 0;

			/**
				Depth of the texture in pixels
			*/
			virtual const int getDepth() const = 0;

			/**
				Aspect ratio of the texture.
			*/
			float getAspectRatio() const { return (float)getWidth() / getHeight(); }
			
			/**
				Inverse of the aspect ratio of the texture.
			*/
			float getInvAspectRatio() const { return (float)getHeight() / getWidth(); }

			/**
				Returns the texture format specifying how the shader must interpret the data
			*/
			virtual const TextureFormat getTextureFormat() const = 0;

			const int getNumberOfChannels() const;

			/**
				Returns `true` if the texture handle points to a valid texture
			*/
			bool hasValidHandle() const { return textureHandle > 0; }

			/**
				Assigns the texture handler to a new group
			*/
			void assignToNewGroup();

			/**
				Assigns the texture handler to a group of another texture handler
			*/
			void assignGroupFrom(const TextureHandler&);

			/**
				Checks whether the texture handler is assigned to the same group as another texture handler
			*/
			bool isOfSameGroup(const TextureHandler&) const;

			/**
				Checks whether the texture handler is assigned to any group
			*/
			bool isAssignedToGroup() const;
		};
	}
}