/******************************************************************************
 * THE OMEGA LIB PROJECT
 *-----------------------------------------------------------------------------
 * Copyright 2010-2015		Electronic Visualization Laboratory,
 *							University of Illinois at Chicago
 * Authors:
 *  Alessandro Febretti		febret@gmail.com
 *  Koosha Mirhosseini		koosha.mirhosseini@gmail.com
 *-----------------------------------------------------------------------------
 * Copyright (c) 2010-2015, Electronic Visualization Laboratory,
 * University of Illinois at Chicago
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer. Redistributions in binary
 * form must reproduce the above copyright notice, this list of conditions and
 * the following disclaimer in the documentation and/or other materials provided
 * with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE  GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *-----------------------------------------------------------------------------
 * What's in this file
 *	The DrawContext class: Contains information about the context in which
 *  drawing operations take place
 ******************************************************************************/
#ifndef __DRAWCONTEXT_H__
#define __DRAWCONTEXT_H__

#include "osystem.h"
#include "DisplayConfig.h"
#include "GpuResource.h"
#include "RenderCorrection.h"

namespace omega
{
	class RenderCorrection;

    ///////////////////////////////////////////////////////////////////////////
    //! Contains information about the current frame.
    struct FrameInfo
    {
        FrameInfo(uint64 frame, GpuContext* context): frameNum(frame), gpuContext(context) {}
        uint64 frameNum;
        GpuContext* gpuContext;
    };

    ///////////////////////////////////////////////////////////////////////////
    //! Contains information about the context in which drawing operations
    //! take place. DrawContext is a fully self-contained description of
    //! rendering operations that make up a full frame.
    struct OMEGA_API DrawContext
    {
        DrawContext();
        ~DrawContext();

        enum Eye { EyeLeft , EyeRight, EyeCyclop };
        enum Task { SceneDrawTask, OverlayDrawTask };
        uint64 frameNum; // TODO: Substitute with frameinfo
        AffineTransform3 modelview;
        Transform3 projection;
        //! The viewMin and viewMax are normalized coordinates of the view bounds
        //! on the current tile (that is, the size of the current rendered view on
        //! the current tile). These values are computed intersecting the tile
        //! position and size on the global canvas with the active camera view
        //! position and size. The view minimum and maximum bounds influence the
        //! frustum shape and pixel viewport.
        //Vector2f viewMin;
        //Vector2f viewMax;
        //! The pixel viewport coordinates of this context with respect to the
        //! owner window of the context.
        Rect viewport;
        //! The eye being rendered for this context.
        Eye eye;
        //! The current draw task.
        Task task;
        //! Information about the drawing channel associated with this context.
        //ChannelInfo* channel;
        DisplayTileConfig* tile;
        RenderTarget* drawBuffer;
        GpuContext* gpuContext;
        Renderer* renderer;
        DrawInterface* drawInterface;

        //! The camera currently rendering this context.
        Camera* camera;

        //! Used for rendering to texture for warp correction mode
        RenderCorrection* renderCorrection;

        //! Tile stack
        //! Lets cameras push/pop tiles, to support rendering with custom tile
        //! definitions
        //@{
        Queue<DisplayTileConfig*> tileStack;
        void pushTileConfig(DisplayTileConfig* newtile);
        void popTileConfig();
        //@}

        void prepare(uint64 frameNum);

        //! The drawFrame method is the 'entry point' called by the display
        //! system to render a full frame. drawFrame does all required setup
        //! operations (viewport, stereo mode etc), and calls the Renderer draw
        //! method mltiple times to draw active eyes for the scene and overlay
        //! layers. The renderer draw method in turn renders secondary cameras
        //! and performs drawing with all the active render passes.
        void drawFrame(uint64 frameNum);

        //! Updates the pixel viewport of this context, based on the actual tile
        //! viewport, active eye and stereo settings.
        void updateViewport();
        void setupInterleaver();
        void setupStereo();
        void initializeStencilInterleaver();
        void initializeQuad();
		void initializeRenderCorrection();
		void enableRenderCorrection();
		void disableRenderCorrection();

        DisplayTileConfig::StereoMode getCurrentStereoMode() const;
        DisplayTileConfig::CorrectionMode getCurrentCorrectionMode() const;

        //! Utility method: returns true if side by side stereo is enabled
        //! in this context.
        //! @remarks Side by side is enabled if tile stereo mode is side by side,
        //! if tile mode is default and the global mode is side by side and the
        //! global mono force mode flag is disabled.
        bool isSideBySideStereoEnabled();

        //! Utility method: returns true if post-draw corrections were requested
        //! for this context.
        //! @remarks Post draw corrections are enabled if either a warp mesh
        //! and/or an edge blend texture has been defined for the tile.
        bool hasRenderCorrection();

		//! Utility method: returns true if post-draw corrections are enabled
		//! for this context.
		//! @remarks Post draw corrections are enabled if either a warp mesh
		//! and/or an edge blend texture has been defined for the tile.
		bool isRenderCorrectionEnabled();

        //! Stencil initialization value. If = 1, stencil has been initialized
        //! if = 0, stencil will be initialized this frame. If = -N, stencil
        //! will be initialized in N frames. The frame delay is useful to make
        //! sure OS windows and frame buffers have been updated before a stencil
        //! mask update.
        short stencilInitialized;
        short quadInitialized;
        int stencilMaskWidth;
        int stencilMaskHeight;

        //! Updates the viewport based on the view size and position an the size
        //! of the overall canvas
        //void updateViewBounds(const Vector2i& viewPos, const Vector2i& viewSize);

        //! Updates the modelview and projection matrices based on head / view
        //! transform and eye separation. Crrent eye is read from context.
        void updateTransforms(
            const AffineTransform3& head,
            const AffineTransform3& view,
            float eyeSeparation,
            float nearZ,
            float farZ);

        //! Return true if this draw context is supposed to draw something for
        //! the specified view rectangle
        //bool overlapsView(
        //    const Vector2i& viewPos,
        //    const Vector2i& viewSize) const;
    };

    ///////////////////////////////////////////////////////////////////////////
    inline DisplayTileConfig::CorrectionMode DrawContext::getCurrentCorrectionMode() const
    {
        return tile->correctionMode;
    }

    ///////////////////////////////////////////////////////////////////////////
    inline bool DrawContext::isSideBySideStereoEnabled()
    {
        DisplayConfig& dcfg = tile->displayConfig;
        if(tile->stereoMode == DisplayTileConfig::SideBySide ||
            (tile->stereoMode == DisplayTileConfig::Default &&
            dcfg.stereoMode == DisplayTileConfig::SideBySide))
        {
            return !dcfg.forceMono;
        }
        return false;
    }

    ///////////////////////////////////////////////////////////////////////////
    inline bool DrawContext::hasRenderCorrection()
    {
        if(tile->correctionMode != DisplayTileConfig::Passthru)
        {
            return true;
        }
        return false;
    }

	///////////////////////////////////////////////////////////////////////////
	inline bool DrawContext::isRenderCorrectionEnabled()
	{
		if (hasRenderCorrection() && renderCorrection != NULL )
		{
			return renderCorrection->isEnabled();
		}
		return false;
	}

    // NOTE: would like to have this in GpuContext.h but can't since it uses
    // DrawContext (and even as a template it needs to be fully compiled when on
    // Visual Studio)
    ///////////////////////////////////////////////////////////////////////////
    //! A template for accessing gpu resources on multiple contexts.
    template<typename T> class GpuRef
    {
    public:
        GpuRef()
        {
            memset(myStamps, 0, sizeof(myStamps));
        }
        Ref<T>& operator()(const GpuContext& context)
        {
            return myObjects[context.getId()];
        }
        Ref<T>& operator()(const DrawContext& context)
        {
            return myObjects[context.gpuContext->getId()];
        }
        double& stamp(const GpuContext& context)
        {
            return myStamps[context.getId()];
        }
        double& stamp(const DrawContext& context)
        {
            return myStamps[context.gpuContext->getId()];
        }
    private:
        Ref<T> myObjects[GpuContext::MaxContexts];
        double myStamps[GpuContext::MaxContexts];
    };

}; // namespace omega

#endif
