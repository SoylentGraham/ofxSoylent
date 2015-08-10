#pragma once

#include <SoyEvent.h>
#include <SoyMath.h>
#include <SoyPixels.h>


#if defined(TARGET_ANDROID) || defined(TARGET_IOS)
	#define OPENGL_ES_3		//	need 3 for FBO's
#elif defined(TARGET_OSX)
	#define OPENGL_CORE_3	//	need 3 for VBA's
#elif defined(TARGET_WINDOWS)
	#define OPENGL_CORE_1
#endif

//#define GL_NONE				GL_NO_ERROR	//	declared in GLES3

#if defined(TARGET_ANDROID) && defined(OPENGL_ES_3)
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

//	include EOS in header
#define GL_GLEXT_PROTOTYPES
#define glBindVertexArray	glBindVertexArrayOES
#define glGenVertexArrays	glGenVertexArraysOES
#include <GLES/glext.h>	//	need for EOS

#endif


#if defined(TARGET_ANDROID) && defined(OPENGL_ES_2)
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES/glext.h>	//	need for EOS
#endif

#if defined(TARGET_OSX) && defined(OPENGL_CORE_3)
#include <Opengl/gl3.h>
#include <Opengl/gl3ext.h>
#endif

#if defined(TARGET_IOS) && defined(OPENGL_ES_3)
#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES3/glext.h>
#endif

#if defined(TARGET_IOS) && defined(OPENGL_ES_2)
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#endif

#if defined(TARGET_WINDOWS) && defined(OPENGL_CORE_1)
#if !defined(GLEW_STATIC)
#error expected GLEW_STATIC to be defined
#endif
#pragma comment(lib,"opengl32.lib")
#include <GL/glew.h>
#endif

#define GL_ASSET_INVALID	0
#define GL_UNIFORM_INVALID	-1



namespace Opengl
{
	class TFboMeta;
	class TUniform;
	class TAsset;
	class TShader;
	class TShaderState;
	class TTexture;
	class TTextureUploadParams;
	class TFbo;
	class TGeoQuad;
	class TShaderEosBlit;
	class TGeometry;
	class TGeometryVertex;
	class TGeometryVertexElement;
	class TContext;
	
	
	// It probably isn't worth keeping these shared here, each user
	// should just duplicate them.
	extern const char * externalFragmentShaderSource;
	extern const char * textureFragmentShaderSource;
	extern const char * identityVertexShaderSource;
	extern const char * untexturedFragmentShaderSource;
	
	extern const char * VertexColorVertexShaderSrc;
	extern const char * VertexColorSkinned1VertexShaderSrc;
	extern const char * VertexColorFragmentShaderSrc;
	
	extern const char * SingleTextureVertexShaderSrc;
	extern const char * SingleTextureSkinned1VertexShaderSrc;
	extern const char * SingleTextureFragmentShaderSrc;
	
	extern const char * LightMappedVertexShaderSrc;
	extern const char * LightMappedSkinned1VertexShaderSrc;
	extern const char * LightMappedFragmentShaderSrc;
	
	extern const char * ReflectionMappedVertexShaderSrc;
	extern const char * ReflectionMappedSkinned1VertexShaderSrc;
	extern const char * ReflectionMappedFragmentShaderSrc;
	

#define Opengl_IsOkay()			Opengl::IsOkay(__func__)
#define Opengl_IsOkayFlush()	Opengl::IsOkay( std::string(__func__)+ " flush", false )

	bool			IsOkay(const char* Context,bool ThrowException=true);
	inline bool		IsOkay(const std::string& Context,bool ThrowException=true)	{	return IsOkay( Context.c_str(), ThrowException );	}
	std::string		GetEnumString(GLenum Type);

	GLenum	GetUploadPixelFormat(const TTexture& Texture,SoyPixelsFormat::Type Format,bool AllowConversion);
	GLenum	GetNewTexturePixelFormat(SoyPixelsFormat::Type Format);
	GLenum	GetDownloadPixelFormat(const TTexture& Texture,SoyPixelsFormat::Type& PixelFormat);
	SoyPixelsFormat::Type	GetDownloadPixelFormat(GLenum Format);

	//	helpers
	void	ClearColour(Soy::TRgb Colour,float Alpha=1);
	void	ClearDepth();
	void	ClearStencil();
	void	SetViewport(Soy::Rectf Viewport);
};





class Opengl::TFboMeta
{
public:
	TFboMeta()	{}
	TFboMeta(const std::string& Name,size_t Width,size_t Height) :
		mName	( Name ),
		mSize	( Width, Height )
	{
	}
	
	std::string		mName;
	vec2x<size_t>	mSize;
};




class Opengl::TUniform
{
public:
	TUniform(const std::string& Name=std::string()) :
		mIndex		( GL_UNIFORM_INVALID ),
		mType		( GL_ASSET_INVALID ),
		mArraySize	( 0 ),
		mName		( Name )
	{
	}
	
	bool		IsValid() const	{	return mIndex != GL_UNIFORM_INVALID;	}
	bool		operator==(const std::string& Name) const	{	return mName == Name;	}
	
	std::string	mName;
	GLenum		mType;
	GLsizei		mArraySize;	//	for arrays of mType
	GLint		mIndex;		//	attrib index
};
namespace Opengl
{
std::ostream& operator<<(std::ostream &out,const Opengl::TUniform& in);
}


class Opengl::TGeometryVertexElement : public TUniform
{
public:
	size_t		mElementDataSize;
	bool		mNormalised;
};

class Opengl::TGeometryVertex
{
public:
	size_t							GetDataSize() const;	//	size of vertex struct
	size_t							GetOffset(size_t ElementIndex) const;
	size_t							GetStride(size_t ElementIndex) const;

	void							EnableAttribs(bool Enable=true) const;
	void							DisableAttribs() const				{	EnableAttribs(false);	}
	
public:
	Array<TGeometryVertexElement>	mElements;
};


//	gr: for now, POD, no virtuals...
class Opengl::TAsset
{
public:
	TAsset() :
	mName	( GL_ASSET_INVALID )
	{
	}
	explicit TAsset(GLuint Name) :
	mName	( Name )
	{
	}
	
	bool	IsValid() const		{	return mName != GL_ASSET_INVALID;	}
	
	GLuint	mName;
};

//	clever class which does the binding, auto texture mapping, and unbinding
//	why? so we can use const TShaders and share them across threads
class Opengl::TShaderState
{
public:
	TShaderState(const TShader& Shader);
	~TShaderState();
	
	bool	SetUniform(const char* Name,const float& v);
	bool	SetUniform(const char* Name,const vec2f& v);
	bool	SetUniform(const char* Name,const vec4f& v);
	bool	SetUniform(const char* Name,const TTexture& Texture);	//	special case which tracks how many textures are bound
	void	BindTexture(size_t TextureIndex,TTexture Texture);	//	use to unbind too
	
public:
	const TShader&	mShader;
	size_t			mTextureBindCount;
};

class Opengl::TShader
{
public:
	TShader(const std::string& vertexSrc,const std::string& fragmentSrc,const TGeometryVertex& Vertex,const std::string& ShaderName,Opengl::TContext& Context);
	~TShader();
	
	TShaderState	Bind();	//	let this go out of scope to unbind
	TUniform		GetUniform(const char* Name) const
	{
		auto* Uniform = mUniforms.Find( Name );
		return Uniform ? *Uniform : TUniform();
	}
	TUniform		GetAttribute(const char* Name) const
	{
		auto* Uniform = mAttributes.Find( Name );
		return Uniform ? *Uniform : TUniform();
	}

public:
	TAsset			mProgram;
	TAsset			mVertexShader;
	TAsset			mFragmentShader;
	
	Array<TUniform>	mUniforms;
	Array<TUniform>	mAttributes;
};



class Opengl::TGeometry
{
public:
	TGeometry(const ArrayBridge<uint8>&& Data,const ArrayBridge<GLshort>&& Indexes,const Opengl::TGeometryVertex& Vertex,TContext& Context);
	~TGeometry();

	void	Draw() const;
	bool	IsValid() const;
	
public:
	TGeometryVertex	mVertexDescription;	//	for attrib binding info
	GLuint			mVertexArrayObject;
	GLuint			mVertexBuffer;
	GLsizei			mVertexCount;
	GLuint			mIndexBuffer;
	GLsizei			mIndexCount;
	GLenum			mIndexType;		//	short/int etc data stored in index buffer
};


class Opengl::TTextureUploadParams
{
public:
	TTextureUploadParams() :
		mStretch				( false ),
		mAllowCpuConversion		( true ),
		mAllowOpenglConversion	( true ),
		mAllowClientStorage		( false )		//	currently unstable on texture release?
	{
	};
	
	bool	mStretch;				//	if smaller, should we stretch (subimage vs teximage)
	bool	mAllowCpuConversion;
	bool	mAllowOpenglConversion;
	bool	mAllowClientStorage;
};

class Opengl::TTexture
{
public:
	//	invalid
	explicit TTexture() :
		mAutoRelease	( false ),
		mType			( GL_TEXTURE_2D )
	{
	}
	TTexture(TTexture&& Move)			{	*this = std::move(Move);	}
	TTexture(const TTexture& Reference)	{	*this = Reference;	}
	explicit TTexture(SoyPixelsMeta Meta,GLenum Type);	//	alloc
	
	//	reference from external
	TTexture(void* TexturePtr,const SoyPixelsMeta& Meta,GLenum Type) :
	mMeta			( Meta ),
	mType			( Type ),
	mAutoRelease	( false ),
#if defined(TARGET_ANDROID)
	mTexture		( reinterpret_cast<GLuint>(TexturePtr) )
#elif defined(TARGET_OSX)
	mTexture		( static_cast<GLuint>(reinterpret_cast<GLuint64>(TexturePtr)) )
#elif defined(TARGET_IOS)
	mTexture		( static_cast<GLuint>( reinterpret_cast<intptr_t>(TexturePtr) ) )
#elif defined(TARGET_WINDOWS)
	mTexture		( static_cast<GLuint>(reinterpret_cast<intptr_t>(TexturePtr)) )
#endif
	{
	}
	
	TTexture(GLuint TextureName,const SoyPixelsMeta& Meta,GLenum Type) :
		mMeta			( Meta ),
		mType			( Type ),
		mAutoRelease	( false ),
		mTexture		( TextureName )
	{
	}
	
	~TTexture()
	{
		if ( mAutoRelease )
			Delete();
	}
	
	
	TTexture& operator=(const TTexture& Weak)
	{
		if ( this != &Weak )
		{
			mAutoRelease = false;
			mTexture = Weak.mTexture;
			mMeta = Weak.mMeta;
			mType = Weak.mType;
			mClientBuffer = Weak.mClientBuffer;
		}
		return *this;
	}

	TTexture& operator=(TTexture&& Move)
	{
		if ( this != &Move )
		{
			mAutoRelease = Move.mAutoRelease;
			mTexture = Move.mTexture;
			mMeta = Move.mMeta;
			mType = Move.mType;
			mClientBuffer = Move.mClientBuffer;
			
			//	stolen the resource
			Move.mAutoRelease = false;
		}
		return *this;
	}

	SoyPixelsMeta		GetInternalMeta(GLenum& Type);	//	read meta from opengl

	size_t				GetWidth() const	{	return mMeta.GetWidth();	}
	size_t				GetHeight() const	{	return mMeta.GetHeight();	}
	SoyPixelsFormat::Type	GetFormat() const	{	return mMeta.GetFormat();	}

	bool				Bind();
	void				Unbind();
	bool				IsValid() const;
	void				Delete();
	void				Copy(const SoyPixelsImpl& Pixels,TTextureUploadParams Params=TTextureUploadParams());
	void				Read(SoyPixelsImpl& Pixels);
	
public:
	bool				mAutoRelease;
	std::shared_ptr<SoyPixelsImpl>	mClientBuffer;	//	for CPU-buffered textures, it's kept here. ownership should go with mAutoRelease, but shared_ptr maybe takes care of that?
	TAsset				mTexture;
	SoyPixelsMeta		mMeta;
	GLenum				mType;		//	GL_TEXTURE_2D by default. gr: "type" may be the wrong nomenclature here
};

class Opengl::TFbo
{
public:
	TFbo(TTexture Texture);
	~TFbo();
	
	bool		IsValid() const	{	return mFbo.IsValid() && mTarget.IsValid();	}
	bool		Bind();
	void		Unbind();
	void		InvalidateContent();
	void		CheckStatus();
	
	void		Delete(Opengl::TContext& Context);	//	deffered delete
	void		Delete();
	
	size_t		GetAlphaBits() const;
	
	TAsset		mFbo;
	TTexture	mTarget;
};
/*
class Opengl::TGeoQuad
{
public:
	TGeoQuad();
	~TGeoQuad();
	
	bool		IsValid() const	{	return mGeometry.IsValid();	}
	
	TGeometry	mGeometry;
	
	bool		Render();
};


class Opengl::TShaderEosBlit
{
public:
	TShaderEosBlit();
	~TShaderEosBlit();
	
	bool		IsValid() const	{	return mProgram.IsValid();	}
	
	Opengl::TShader	mProgram;
};
*/

