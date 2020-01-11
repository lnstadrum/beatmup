/**
    Common rendering programs management
*/

#pragma once
#include "../gpu/program.h"
#include <map>

namespace Beatmup {

    /**
        Handles a collection of common rendering programs of predefined types and shared operations among these
        programs.
        Programs compilation and linking is done in deferred fashion, so that the first rendering pass would usually
        take much more time, but no time is spent on initializing programs that will never be used.
    */
    class RenderingPrograms {
        friend class GraphicPipeline;
    public:
        /**
            Standard rendering operations
        */
        enum class Program {
            BLEND,                  //!< default blending of a single image
            MASKED_BLEND,           //!< blending an image through a pixelwise mask
            MASKED_8BIT_BLEND,      //!< blending an image through a 8 bit pixelwise mask
            SHAPED_BLEND,           //!< shaping an image
            BLEND_EXT,              //!< default blending using a texture extension
            MASKED_BLEND_EXT,       //!< blending an image through a pixelwise mask (texture extension)
            MASKED_8BIT_BLEND_EXT,  //!< blending an image through a 8 bit pixelwise mask (texture extension)
            SHAPED_BLEND_EXT,       //!< shaping an image (texture extension)
            CUSTOM                  //!< using a user-defined program with a shared vertex shader
        };

        static const std::string
            MODELVIEW_MATRIX_ID,      //!< Modelview matrix (mapping input geometry to output) shader variable name.
            TEXTURE_COORDINATES_ID,   //!< Texture coordinates shader variable name
      
            DECLARE_TEXTURE_COORDINATES_IN_FRAG;    //!< Declaring texture coordinates in fragment shader.
            


        /**
            \brief Select and enable a common program.
            If the selected program is not yet ready, it is linked and complied.
            \param[in,out] gpu			A graphic pipeline instance
            \param[in,out] program		The selected common program to use
        */
        void enableProgram(GraphicPipeline* gpu, Program program);


        /**
            \brief Enable a user-defined program.
            The program must be linked.
            \param[in,out] gpu			A graphic pipeline instance
            \param[in,out] program		The program to use
        */
        void enableProgram(GraphicPipeline* gpu, GL::Program& program);


        /**
            \return the currently used rendering program.
        */
        GL::Program& getCurrentProgram();


        /**
            \brief Binds a mask to a masked rendering program.
            Throws an exception when a mask is about to be bound to a non-masked rendering program.
            \param[in,out] gpu		A graphic pipeline instance
            \param[in,out] mask		The mask bitmap
        */
        void bindMask(GraphicPipeline* gpu, AbstractBitmap& mask);


        /**
            Performs the blending operation
            \param[in] onScreen		If `true`, the rendering is done on screen, not into a bitmap.
        */
        void blend(bool onScreen);


        void paveBackground(GraphicPipeline* gpu, GL::TextureHandler& content, bool onScreen);


        /**
            \return default blending vertex shader to be used in a user-defined single-image blending program.
        */
        GL::VertexShader& getDefaultVertexShader(const GraphicPipeline* gpu);

    protected:
        RenderingPrograms(GraphicPipeline* gpu);
        ~RenderingPrograms();

    private:
        class Backend;
        Backend* backend;
        GL::Program* currentGlProgram;
        Program currentProgram;
        bool maskSetUp;

        std::map<Program, GL::VertexShader> vertexShaders;
        std::map<Program, GL::FragmentShader> fragmentShaders;
        std::map<Program, GL::Program> programs;

        GL::VertexShader& getVertexShader(const GraphicPipeline* gpu, Program program);
        GL::FragmentShader& getFragmentShader(const GraphicPipeline* gpu, Program program);
        GL::Program& getProgram(const GraphicPipeline* gpu, Program program);
    };
}
