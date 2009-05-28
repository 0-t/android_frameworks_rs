

ContextBindRootScript {
	param RsScript sampler
	}

ContextBindProgramFragmentStore {
	param RsProgramFragmentStore pgm
	}

ContextBindProgramFragment {
	param RsProgramFragment pgm
	}

ContextBindProgramVertex {
	param RsProgramVertex pgm
	}


ElementBegin {
}

ElementAddPredefined {
	param RsElementPredefined predef
	}

ElementAdd {
	param RsDataKind dataKind
	param RsDataType dataType
	param bool isNormalized
	param size_t bits
	}

ElementCreate {
	ret RsElement
	}

ElementGetPredefined {
	param RsElementPredefined predef
	ret RsElement
	}

ElementDestroy {
	param RsElement ve
	}

TypeBegin {
	param RsElement type
	}

TypeAdd {
	param RsDimension dim
	param size_t value
	}

TypeCreate {
	ret RsType
	}

TypeDestroy {
	param RsType p
	}

AllocationCreateTyped {
	param RsType type
	ret RsAllocation
	}

AllocationCreatePredefSized {
	param RsElementPredefined predef
	param size_t count
	ret RsAllocation
	}

AllocationCreateSized {
	param RsElement e
	param size_t count
	ret RsAllocation
	}

AllocationCreateFromFile {
	param const char *file
	param bool genMips
	ret RsAllocation
	}

AllocationCreateFromBitmap {
	param uint32_t width
	param uint32_t height
	param RsElementPredefined dstFmt
	param RsElementPredefined srcFmt
	param bool genMips
	param const void * data
	ret RsAllocation
	}


AllocationUploadToTexture {
	param RsAllocation alloc
	param uint32_t baseMipLevel
	}

AllocationUploadToBufferObject {
	param RsAllocation alloc
	}

AllocationDestroy {
	param RsAllocation alloc
	}


AllocationData {
	param RsAllocation va
	param const void * data
	} 

Allocation1DSubData {
	param RsAllocation va
	param uint32_t xoff
	param uint32_t count
	param const void *data
	} 

Allocation2DSubData {
	param RsAllocation va
	param uint32_t xoff
	param uint32_t yoff
	param uint32_t w
	param uint32_t h
	param const void *data
	}


Adapter1DCreate {
	ret RsAdapter1D
	}

Adapter1DBindAllocation {
	param RsAdapter1D adapt
	param RsAllocation alloc
	}

Adapter1DDestroy {
	param RsAdapter1D adapter
	}

Adapter1DSetConstraint {
	param RsAdapter1D adapter
	param RsDimension dim
	param uint32_t value
	}

Adapter1DData {
	param RsAdapter1D adapter
	param const void * data
	} 

Adapter1DSubData {
	param RsAdapter1D adapter
	param uint32_t xoff
	param uint32_t count
	param const void *data
	} 

Adapter2DCreate {
	ret RsAdapter2D
	}

Adapter2DBindAllocation {
	param RsAdapter2D adapt
	param RsAllocation alloc
	}

Adapter2DDestroy {
	param RsAdapter2D adapter
	}

Adapter2DSetConstraint {
	param RsAdapter2D adapter
	param RsDimension dim
	param uint32_t value
	}

Adapter2DData {
	param RsAdapter2D adapter
	param const void *data
	} 

Adapter2DSubData {
	param RsAdapter2D adapter
	param uint32_t xoff
	param uint32_t yoff
	param uint32_t w
	param uint32_t h
	param const void *data
	}

SamplerBegin {
	}

SamplerSet {
	param RsSamplerParam p
	param RsSamplerValue value
	}

SamplerCreate {
	ret RsSampler
	}

SamplerDestroy {
	param RsSampler s
	}

TriangleMeshBegin {
	param RsElement vertex
	param RsElement index
	}

TriangleMeshAddVertex {
	param const void *vtx
	}

TriangleMeshAddTriangle {
	param uint32_t idx1
	param uint32_t idx2
	param uint32_t idx3
	}

TriangleMeshCreate {
	ret RsTriangleMesh
	}

TriangleMeshDestroy {
	param RsTriangleMesh mesh
	}

TriangleMeshRender {
	param RsTriangleMesh vtm
	}

TriangleMeshRenderRange {
	param RsTriangleMesh vtm
	param uint32_t start
	param uint32_t count
	}

ScriptDestroy {
	param RsScript script
	}

ScriptBindAllocation {
	param RsScript vtm
	param RsAllocation va
	param uint32_t slot
	}


ScriptCBegin {
	}

ScriptCSetClearColor {
	param float r
	param float g
	param float b
	param float a
	}

ScriptCSetClearDepth {
	param float depth
	}

ScriptCSetClearStencil {
	param uint32_t stencil
	}

ScriptCAddType {
	param RsType type
	}

ScriptCSetRoot {
	param bool isRoot
	}

ScriptCSetOrtho {
	param bool isOrtho
	}

ScriptCSetScript {
	param void * accScript
	param void * codePtr
	}

ScriptCCreate {
	ret RsScript
	}


ProgramFragmentStoreBegin {
	param RsElement in
	param RsElement out
	}

ProgramFragmentStoreColorMask {
	param bool r
	param bool g
	param bool b
	param bool a
	}

ProgramFragmentStoreBlendFunc {
	param RsBlendSrcFunc srcFunc
	param RsBlendDstFunc destFunc
	}

ProgramFragmentStoreDepthMask {
	param bool enable
}

ProgramFragmentStoreDither {
	param bool enable
}

ProgramFragmentStoreDepthFunc {
	param RsDepthFunc func
}

ProgramFragmentStoreCreate {
	ret RsProgramFragmentStore
	}



ProgramFragmentBegin {
	param RsElement in
	param RsElement out
	}

ProgramFragmentBindTexture {
	param RsProgramFragment pf
	param uint32_t slot
	param RsAllocation a
	}

ProgramFragmentBindSampler {
	param RsProgramFragment pf
	param uint32_t slot
	param RsSampler s
	}

ProgramFragmentSetType {
	param uint32_t slot
	param RsType t
	}

ProgramFragmentSetEnvMode {
	param uint32_t slot
	param RsTexEnvMode env
	}

ProgramFragmentSetTexEnable {
	param uint32_t slot
	param bool enable
	}

ProgramFragmentCreate {
	ret RsProgramFragment
	}



ProgramVertexBegin {
	param RsElement in
	param RsElement out
	}

ProgramVertexCreate {
	ret RsProgramVertex
	}

ProgramVertexBindAllocation {
	param RsProgramVertex vpgm
	param uint32_t slot
	param RsAllocation constants
	}

ProgramVertexSetType {
	param uint32_t slot
	param RsType constants
	}

ProgramVertexSetCameraMode {
	param bool ortho
	}

ProgramVertexSetTextureMatrixEnable {
	param bool enable
	}

ProgramVertexSetModelMatrixEnable {
	param bool enable
	}

