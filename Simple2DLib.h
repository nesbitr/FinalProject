/* Simple2D Graphics Library for C++
   (c) Katy Coe (sole proprietor) 2012-2013

   No unauthorized copying or distribution. You may re-use Simple2D in your own non-commercial projects.
   For commercial applications, you must obtain a commercial license.

   Support and documentation: www.djkaty.com/simple2d
*/

// ==================================================================================
// Configuration bits (skip over this section)
// ==================================================================================
#pragma once

// Disable wchar warnings
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

// Disable container-to-array copy warnings
#ifndef _SCL_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS
#endif

#include <intsafe.h>

// We need M_PI etc.
#define _USE_MATH_DEFINES

// Windows headers
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <WindowsX.h>
#include <ShlObj.h>

// C++ standard library
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <wchar.h>
#include <math.h>
#include <random>

// STL / Boost
#include <vector>
#include <deque>
#include <list>
#include <algorithm>

#include <boost/intrusive_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/ptr_container/ptr_map.hpp>
#include <boost/ptr_container/ptr_list.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/noncopyable.hpp>
#include <boost/functional/hash.hpp>
#include <boost/type_traits/remove_const.hpp>

// Direct2D 1.1 / DirectWrite 1.1 (remove '_1' to use Windows 7-compatible SDK headers and Direct2D 1.0)
#include <d2d1_1.h>
#include <dwrite_1.h>
#include <wincodec.h>

// Direct2D 1.1 additional requirements over Direct2D 1.0
#include <d3d11_1.h>			// Direct3D 11.1 (Direct2D 1.1 must get Direct3D surface)
#include <d2d1effects.h>		// Direct2D 1.1 ID2D1Effect support
#include <d2d1_1helper.h>		// Direct2D 1.1 helper utilities
#include <d2d1effecthelpers.h>  // Direct2D 1.1 ID2D1Effect helpers

// Other parts of Simple2D
#include "Gamepad.h"

// Using declarations
using std::string;
using std::stringstream;
using std::vector;
using std::deque;

// Provide find function for ptr_list; from: http://www.schmid.dk/wiki/index.php/Boost_Pointer_Container_Library

template <typename Ptr_container_iterator, typename Value_type>
Ptr_container_iterator find_ptr(
	Ptr_container_iterator begin, Ptr_container_iterator end, Value_type *ptr)
{
	Ptr_container_iterator i;

	for (i = begin; i != end; ++i)
		if (&(*i) == ptr) break;

	return i;
}

// Intrusive_ptr handling for COM objects
void intrusive_ptr_add_ref(IUnknown *p);
void intrusive_ptr_release(IUnknown *p);

namespace S2D
{

// Safe handling of HRESULTs
#ifndef HRDieOnFail
#define HRDieOnFail(f, s) do {						\
	HRESULT hr = (f);								\
	if (FAILED(hr))									\
	{												\
		if (strlen((s)))							\
			MessageBox(NULL, (s), "Simple2D Error", MB_OK);	\
		exit(0);									\
	}												\
} while(0)
#endif

#ifndef HRSilentDieOnFail
#define HRSilentDieOnFail(f) do {					\
	HRESULT hr = (f);								\
	if (FAILED(hr))									\
		exit(0);									\
} while(0)
#endif

#ifndef HRReturnOnFail
#define HRReturnOnFail(f, s) do {					\
	HRESULT hr = (f);								\
	if (FAILED(hr))									\
	{												\
		if (strlen((s)))							\
			MessageBox(NULL, (s), "Simple2D Error", MB_OK);	\
		return hr;									\
	}												\
} while(0)
#endif

#ifndef HRSilentReturnOnFail
#define HRSilentReturnOnFail(f) do {				\
	HRESULT hr = (f);								\
	if (FAILED(hr))									\
		return hr;									\
} while(0)
#endif

// Safe release of COM objects
template <typename Interface>
inline void SafeRelease(Interface **ppInterfaceToRelease)
{
	if (*ppInterfaceToRelease != NULL)
	{
		(*ppInterfaceToRelease)->Release();
		(*ppInterfaceToRelease) = NULL;
	}
}

// Assert macro
#ifndef Assert
#if defined(DEBUG) || defined(_DEBUG)
#define Assert(b) do {if (!(b)) {OutputDebugStringA("Assert: " #b "\n");}} while(0)
#else
#define Assert(b)
#endif
#endif

// Process pointer
#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif

// Forward references
template <typename> class TypedAnimation;
class AnimationChain;
class GenericBrush;
class Geometry;
class GradientObject;
class ImageBrushObject;
class ImageObject;
class PaintbrushObject;
class RenderingObject;
class Simple2D;

namespace S2DScene {
class Button;
struct ButtonGroupTemplate;
struct ButtonTemplate;
class CustomDraw;
class Dialog;
class Label;
class InterfaceObject;
class ObjectGroup;
class ObjectManager;
class Rectangle;
class Scene;
class Sprite;
class SpriteSheet;
class TextBox;
struct TextBoxGroupTemplate;
struct TextBoxTemplate;
template <typename, typename, typename>
class UserInterfaceItemGroup;

template <typename, typename, typename, typename>
class InterfaceItemGroupSkin;
class ISkin;
class DialogSkin;
template <typename>
class SkinnedObjectGroup;
}

// ==================================================================================
// These typedefs create aliases to make using Simple2D easier
// ==================================================================================

// Direct2D
typedef D2D1::ColorF						Colour;
typedef D2D1_MATRIX_3X2_F					Matrix;
typedef ID2D1GeometrySink *					GeometryData;

// DirectWrite
typedef boost::intrusive_ptr<IDWriteTextFormat>	TextFormat;
typedef boost::intrusive_ptr<IDWriteTextLayout>	TextLayout;

// Simple2D
#define DefaultBrush						(((GenericBrush*)0))
typedef TypedAnimation<double>				Animation;
typedef PaintbrushObject *					Paintbrush;
typedef GradientObject *					Gradient;
typedef ImageBrushObject *					ImageBrush;
typedef ImageObject *						Image;
namespace S2DScene {
typedef UserInterfaceItemGroup<Button, ButtonGroupTemplate, ButtonTemplate>	ButtonGroup;
typedef UserInterfaceItemGroup<TextBox, TextBoxGroupTemplate, TextBoxTemplate> TextBoxGroup;

typedef vector<ButtonTemplate> ButtonList;
typedef vector<TextBoxTemplate> TextBoxList;
};

// Boost
typedef std::shared_ptr<RenderingObject>	TemporaryRenderingObject;
typedef std::shared_ptr<GenericBrush>		TemporaryBrush;
typedef std::shared_ptr<PaintbrushObject>	TemporaryPaintbrush;
typedef std::shared_ptr<GradientObject>	TemporaryGradient;
typedef std::shared_ptr<ImageBrushObject>	TemporaryImageBrush;
typedef std::shared_ptr<ImageObject>		TemporaryImage;

// Legacy compatibility (<=1.07)
#ifdef SIMPLE2D_COMPAT_107
namespace S2DScene {
typedef InterfaceObject						DrawingObject;
typedef Rectangle							SceneRectangle;
typedef Label								SceneText;
typedef Sprite								SceneImage;
typedef SpriteSheet							SceneSpriteSheet;
typedef CustomDraw							SceneCustomDraw;
typedef TextBox								TextControl;
}
#endif

using namespace S2DScene;

// ==================================================================================
// Utility functions
// ==================================================================================

// Convert numbers and chars to strings easily
template <typename String>
string StringFactory(String s)
{
	stringstream ss;
	ss << s;
	return ss.str();
}

// Convert WCHAR * to string easily
string StringFactory(WCHAR *s);

// Convert std::string to WCHAR (allocates memory which should be freed by the app)
WCHAR *StringToWCHAR(string s);

// Debug output a std::string
void DebugPrint(std::string debugString);

// Output a message box from a std::string
void MessageBoxS(std::string message);

// ==================================================================================
// Pre-defined animation paths
// ==================================================================================

// Type required by animation functions
// Animation functions should return a value between -1 and 1
// which is multiplied by intervalMs and added to base to get the final result
typedef std::function<double (double)> Simple2DAnimFunc;

// Built-in animation functions
class Animations
{
public:
	static double WaitZero(double intervalPc);
	static double WaitOne(double intervalPc);
	static double Linear(double intervalPc);
	static double Sin(double intervalPc);
	static double Cos(double intervalPc);
	static double OneMinusSin(double intervalPc);
	static double OneMinusCos(double intervalPc);
	static double Tan(double intervalPc);
	static double Log(double intervalPc);
};

// ==================================================================================
// Animation helper class
// ==================================================================================

template <typename T = double>
class TypedAnimation
{
public:
	// Cycle types
	// Repeat:		 run from 0-1 and wrap around to 0 again
	// Clamp:		 run from 0-1 and hold at 1
	// Reverse:		 run from 0-1 then 1-0 and repeat

	// NOTE: Use the cropping factors to restrict the animation to use only a subset of values from 0-1, or to run backwards
	enum CycleType { Repeat, Clamp, Reverse };

private:
	double amplitude;
	int intervalMs;
	int startTime;
	int timeOffset;
	double base;
	double cropLower, cropUpper;
	CycleType cycleType;
	Simple2DAnimFunc animFunc;
	bool paused;
	double animPcAtPauseTime;
	std::function <void (void)> onStart;

public:
	// Create animation using func as the path, amplitude amp, interval milliseconds period and a base offset of b
	TypedAnimation() : onStart(nullptr) {}
	TypedAnimation(Simple2DAnimFunc func, int interval, double amp = 1.0, double b = 0, CycleType ct = Repeat, bool startPaused = true);
	TypedAnimation(Simple2DAnimFunc func, int interval, double amp, double b, double cropLower, double cropUpper, CycleType ct = Repeat, bool startPaused = true);
	TypedAnimation(T value);

	// Convert to AnimationChain silently where needed
	operator AnimationChain() const;

	// Get the current animation result
	T Get() const;

	// Get the current animation result via an implicit cast (equivalent to Get())
	operator T() const;

	// Get the current animation result + offset % (equivalent to GetAnimOffset(x))
	T operator()(double offset = 0.0) const;

	// Get the current animation result (+ offset % if specified)
	T GetAnimOffset(double offset = 0.0) const;

	// Get the current animation result (+ offset ms if specified)
	T GetAnimOffsetTime(int offset = 0);

	// Get the current animation result as if the animation was running backwards (+ offset % if specified)
	T GetAnimOffsetReversed(double offset = 0.0) const;

	// Get the current animation result as if the animation was running backwards (+ offset ms if specified)
	T GetAnimOffsetReversedTime(int offset = 0);

	// Get the animation result from a fixed position in the animation (0-1)
	T GetAnimFixed(double pos) const;

	// Get the animation result from a fixed position in time (0-interval ms)
	T GetAnimFixedTime(int pos) const;

	// Get the animation result from a fixed position in the animation as if the animation was running backwards (0-1)
	T GetAnimFixedReversed(double pos) const;

	// Get the animation result from a fixed position in the animation as if the animation was running backwards (0-interval ms)
	T GetAnimFixedReversedTime(int pos) const;

	// Get the current position in the animation (+ offset % if specified)
	double GetAnimPos(double offset = 0.0) const;

	// Get the current position in the animation (+ offset ms if specified)
	double GetAnimPosTime(int offset = 0);

	// Get the current position in the animation as if the animation was running backwards (+ offset % if specified)
	double GetAnimPosReversed(double offset = 0.0) const;

	// Get the current position in the animation as if the animation was running backwards (+ offset ms if specified)
	double GetAnimPosReversedTime(int offset);

	// Scale the specified animation position according to the function cropping factors
	double Crop(double pos) const;

	// Reset the animation timer to the beginning of a cycle
	void Reset(bool pause = false);

	// Pause/unpause animation
	void Pause(bool pause = true);

	// Return true if the animation is paused
	bool IsPaused() const;

	// Return true if the animation has finished
	bool Done() const;

	// Set animation position in percent (0-1)
	void SetPos(double pc);

	// Set animation position in milliseconds (0-interval ms)
	void SetPosTime(int ms);

	// Set animation amplitude (multiplier of animation function)
	void SetAmplitude(double amp);

	// Set animation period (in milliseconds)
	void SetInterval(int inte);

	// Set base offset
	void SetBase(double base);

	// Set animation function
	void SetFunc(Simple2DAnimFunc f);

	// Set the cycle type
	void SetCycleType(CycleType ct);

	// Set the callback function when the animation starts (and is not paused)
	void SetStartEventHandler(std::function <void (void)> func);

	// Static factory functions to create common animations
	static TypedAnimation<T> Fixed(T value);
	static TypedAnimation<T> WaitAt(double value, int interval, std::function <void (void)> startFunc = nullptr, bool startPaused = true);
	static TypedAnimation<T> FromTo(double start, double end, int interval, std::function <void (void)> startFunc = nullptr, bool startPaused = true);
	static TypedAnimation<T> FromToAndClamp(double start, double end, int interval, std::function <void (void)> startFunc = nullptr, bool startPaused = true);
	static TypedAnimation<T> FromPlus(double start, double add, int interval, std::function <void (void)> startFunc = nullptr, bool startPaused = true);
	static TypedAnimation<T> FromPlusAndClamp(double start, double add, int interval, std::function <void (void)> startFunc = nullptr, bool startPaused = true);
};

// Animation helper class
template <typename T>
TypedAnimation<T>::TypedAnimation(Simple2DAnimFunc func, int interval, double amp, double b, CycleType ct, bool startPaused)
	: animFunc(func), amplitude(amp), intervalMs(interval), base(b), cropLower(0.0f), cropUpper(1.0f), cycleType(ct), paused(startPaused),
	animPcAtPauseTime(0.0f), timeOffset(0), onStart(nullptr)
{
	Reset(startPaused);
}

template <typename T>
TypedAnimation<T>::TypedAnimation(Simple2DAnimFunc func, int interval, double amp, double b, double cL, double cU, CycleType ct, bool startPaused)
	: animFunc(func), amplitude(amp), intervalMs(interval), base(b), cropLower(cL), cropUpper(cU), cycleType(ct), paused(startPaused),
	animPcAtPauseTime(0.0f), timeOffset(0), onStart(nullptr)
{
	Reset(startPaused);
}

template <typename T>
TypedAnimation<T>::TypedAnimation(T value)
	: animFunc(Animations::WaitZero), amplitude(0), intervalMs(10), base(static_cast<double>(value)), cropLower(0.0f), cropUpper(1.0f), cycleType(CycleType::Clamp), paused(false),
	animPcAtPauseTime(0.0f), timeOffset(0), onStart(nullptr)
{
	Reset(false);
}

template <typename T>
void TypedAnimation<T>::Pause(bool pause)
{
	if (pause && !paused)
		animPcAtPauseTime = GetAnimPos();

	else if (!pause && paused)
		startTime = GetTickCount();

	paused = pause;
}

template <typename T>
bool TypedAnimation<T>::IsPaused() const
{
	return paused;
}

template <typename T>
bool TypedAnimation<T>::Done() const
{
	return (cycleType == Clamp && GetAnimPos() == 1.0f);
}

template <typename T>
void TypedAnimation<T>::Reset(bool pause)
{
	startTime = GetTickCount();
	animPcAtPauseTime = 0.0f;

	if (!pause)
		if (onStart)
			onStart();

	paused = pause;
}

template <typename T>
double TypedAnimation<T>::Crop(double pos) const
{
	if (cropUpper >= cropLower)
		return (cropUpper - cropLower) * pos + cropLower;
	else
		return cropLower - (cropLower - cropUpper) * pos;
}

template <typename T>
T TypedAnimation<T>::Get() const
{
	return GetAnimOffset();
}

template <typename T>
TypedAnimation<T>::operator T() const
{
	return Get();
}

template <typename T>
T TypedAnimation<T>::operator()(double offset) const
{
	return GetAnimOffset(offset);
}

template <typename T>
T TypedAnimation<T>::GetAnimOffset(double offset) const
{
	return static_cast<T>(animFunc(Crop(GetAnimPos(offset))) * amplitude + base);
}

template <typename T>
T TypedAnimation<T>::GetAnimOffsetTime(int offset)
{
	return static_cast<T>(animFunc(Crop(GetAnimPosTime(offset))) * amplitude + base);
}

template <typename T>
T TypedAnimation<T>::GetAnimOffsetReversed(double offset) const
{
	return static_cast<T>(animFunc(Crop(GetAnimPosReversed(offset))) * amplitude + base);
}

template <typename T>
T TypedAnimation<T>::GetAnimOffsetReversedTime(int offset)
{
	return static_cast<T>(animFunc(Crop(GetAnimPosReversedTime(offset))) * amplitude + base);
}

template <typename T>
T TypedAnimation<T>::GetAnimFixed(double pos) const
{
	return static_cast<T>(animFunc(Crop(pos)) * amplitude + base);
}

template <typename T>
T TypedAnimation<T>::GetAnimFixedTime(int pos) const
{
	return static_cast<T>(animFunc(Crop(static_cast<double>(pos) / intervalMs)) * amplitude + base);
}

template <typename T>
T TypedAnimation<T>::GetAnimFixedReversed(double pos) const
{
	return static_cast<T>(animFunc(Crop(1 - pos)) * amplitude + base);
}

template <typename T>
T TypedAnimation<T>::GetAnimFixedReversedTime(int pos) const
{
	return static_cast<T>(animFunc(Crop(1 - static_cast<double>(pos) / intervalMs)) * amplitude + base);
}

template <typename T>
double TypedAnimation<T>::GetAnimPos(double offset) const
{
	double animPc = static_cast<double>(GetTickCount() - startTime - timeOffset) / intervalMs + animPcAtPauseTime;

	if (paused)
		animPc = animPcAtPauseTime;

	animPc += offset;

	if (cycleType == Repeat)
		animPc -= static_cast<int>(animPc);

	else if (cycleType == Clamp)
		animPc = min(animPc, 1.0f);

	else if (cycleType == Reverse)
	{
		bool reverseSection = (static_cast<int>(animPc) % 2 == 1);
		animPc -= static_cast<int>(animPc);
		if (reverseSection)
			animPc = 1 - animPc;
	}

	return animPc;
}

template <typename T>
double TypedAnimation<T>::GetAnimPosTime(int offset)
{
	int prevTimeOffset = timeOffset;

	timeOffset = offset;
	double pos = GetAnimPos();
	timeOffset = prevTimeOffset;

	return pos;
}

template <typename T>
double TypedAnimation<T>::GetAnimPosReversed(double offset) const
{
	return 1 - GetAnimPos(offset);
}

template <typename T>
double TypedAnimation<T>::GetAnimPosReversedTime(int offset)
{
	return 1 - GetAnimPosTime(offset);
}

template <typename T>
void TypedAnimation<T>::SetPos(double pc)
{
	animPcAtPauseTime = pc;
	timeOffset = 0;
	startTime = GetTickCount();
}

template <typename T>
void TypedAnimation<T>::SetPosTime(int ms)
{
	animPcAtPauseTime = 0.0f;
	timeOffset = ms;
	startTime = GetTickCount();
}

template <typename T>
void TypedAnimation<T>::SetAmplitude(double amp)    { amplitude = amp; }
template <typename T>
void TypedAnimation<T>::SetInterval(int inte)       { intervalMs = inte; }
template <typename T>
void TypedAnimation<T>::SetBase(double base)        { this->base = base; }
template <typename T>
void TypedAnimation<T>::SetFunc(Simple2DAnimFunc f) { animFunc = f; }
template <typename T>
void TypedAnimation<T>::SetCycleType(CycleType ct)  { cycleType = ct; }
template <typename T>
void TypedAnimation<T>::SetStartEventHandler(std::function <void (void)> f) { onStart = f; }

// Static factory functions to create common animations
template <typename T>
TypedAnimation<T> TypedAnimation<T>::Fixed(T value)
{
	TypedAnimation<T> a(value);
	return a;
}

template <typename T>
TypedAnimation<T> TypedAnimation<T>::WaitAt(double value, int interval, std::function <void (void)> startFunc, bool startPaused)
{
	TypedAnimation<T> a(Animations::WaitZero, interval, 0, value);
	a.SetStartEventHandler(startFunc);
	if (!startPaused)
		a.Reset();
	return a;
}

template <typename T>
TypedAnimation<T> TypedAnimation<T>::FromTo(double start, double end, int interval, std::function <void (void)> startFunc, bool startPaused)
{
	TypedAnimation<T> a(Animations::Linear, interval, (end - start), start);
	a.SetStartEventHandler(startFunc);
	if (!startPaused)
		a.Reset();
	return a;
}

template <typename T>
TypedAnimation<T> TypedAnimation<T>::FromToAndClamp(double start, double end, int interval, std::function <void (void)> startFunc, bool startPaused)
{
	TypedAnimation<T> a(Animations::Linear, interval, (end - start), start, CycleType::Clamp);
	a.SetStartEventHandler(startFunc);
	if (!startPaused)
		a.Reset();
	return a;
}

template <typename T>
TypedAnimation<T> TypedAnimation<T>::FromPlus(double start, double add, int interval, std::function <void (void)> startFunc, bool startPaused)
{
	TypedAnimation<T> a(Animations::Linear, interval, add, start);
	a.SetStartEventHandler(startFunc);
	if (!startPaused)
		a.Reset();
	return a;
}

template <typename T>
TypedAnimation<T> TypedAnimation<T>::FromPlusAndClamp(double start, double add, int interval, std::function <void (void)> startFunc, bool startPaused)
{
	TypedAnimation<T> a(Animations::Linear, interval, add, start, CycleType::Clamp);
	a.SetStartEventHandler(startFunc);
	if (!startPaused)
		a.Reset();
	return a;
}

// ==================================================================================
// Animation chains - a sequence of Animations running one after another
// ==================================================================================

class AnimationChain
{
private:
	bool done;
	bool paused;
	std::vector<Animation> animations;
	unsigned int index;
	double finalValue;
	std::function<void (void)> onDone;

	// Only Repeat and Clamp can be used. The default is Clamp.
	Animation::CycleType cycleType;

public:
	// Animation chains always start paused
	AnimationChain() : done(true), onDone(nullptr), paused(true), cycleType(Animation::Clamp), index(0) {}

	// Conversion constructor
	AnimationChain(const Animation &a);

	// Add animation
	void Add(const Animation &a);

	// Set callback function for when animation chain completes
	void SetDoneEventHandler(std::function<void (void)> doneFunc);

	// Start or unpause chain
	void Start(bool reset = false);

	// Reset animation chain (start animation again from the beginning)
	void Reset();

	// Pause animation
	void Pause(bool pause);

	// Return true if the animation is paused
	bool IsPaused();

	// Return true if animation chain is finished
	bool Done();

	// Set cycle type
	void SetCycleType(Animation::CycleType ct);

	// Move to next animation if needed
	// Should be called each frame to ensure animations don't stall between transitions
	void Update();

	// Get position in current sub-animation
	double GetAnimOffset(double offset = 0.0f);
};

// Allow implicit casting of TypedAnimation<double> to AnimationChain
template <typename T>
TypedAnimation<T>::operator AnimationChain() const
{
	throw;
}

// ==================================================================================
// Base class for all device-dependent rendering objects
// (or device-independent resources we want to discard before the application ends,
// for example temporary geometry)
// ==================================================================================

class RenderingObject
{
protected:
	// Object hash
	size_t hash;

	// The rendering class
	Simple2D *renderer;

	// The Direct2D object to wrap
	boost::intrusive_ptr<ID2D1Resource> resource;

	// Just-In-Time object creation
	virtual void Create() = 0;

protected:
	// Constructor
	RenderingObject();
	RenderingObject(ID2D1Resource *res);

#ifdef SIMPLE2D_COMPAT_107
	RenderingObject(Simple2D *r) : renderer(r), resource(NULL), hash(0) {}
	RenderingObject(Simple2D *r, ID2D1Resource *res) : renderer(r), resource(res, false), hash(0) {}
#endif

public:
	// Get pointer to encapsulated object
	ID2D1Resource *Get() { if (!resource.get()) Create(); return resource.get(); }

	// Get pointer to encapsulated object if it has already been created only, otherwise null
	ID2D1Resource *GetIfCreated() { return resource.get(); }

	// Invalidate the object (will also call SafeRelease on the Interface)
	void Invalidate() { resource.reset(); }

	// Hashing for containers
	size_t &GetHash() { if (!hash) hash = CreateHash(); return hash; }

	virtual size_t CreateHash() const = 0;
};

class RenderingObjectNonShareable : public RenderingObject, boost::noncopyable
{
protected:
	// Constructors
	RenderingObjectNonShareable() : RenderingObject() {}
#ifdef SIMPLE2D_COMPAT_107
	RenderingObjectNonShareable(Simple2D *r) : RenderingObject(r) {}
	RenderingObjectNonShareable(Simple2D *r, ID2D1Resource *res) : RenderingObject(r, res) {}
#endif
	RenderingObjectNonShareable(ID2D1Resource *res) : RenderingObject(res) {}
};

class RenderingObjectShareable : public RenderingObject
{
protected:
	// Constructors
	RenderingObjectShareable() : RenderingObject() {}
#ifdef SIMPLE2D_COMPAT_107
	RenderingObjectShareable(Simple2D *r) : RenderingObject(r) {}
	RenderingObjectShareable(Simple2D *r, ID2D1Resource *res) : RenderingObject(r, res) {}
#endif
	RenderingObjectShareable(ID2D1Resource *res) : RenderingObject(res) {}

	// Copy constructor
	RenderingObjectShareable(RenderingObjectShareable const &o) : RenderingObject()
	{
		resource = boost::intrusive_ptr<ID2D1Resource>(o.resource);
	}
};

// ==================================================================================
// Bitmap helper class
// ==================================================================================

// NOTE: No memory management is required

class ImageObject : public RenderingObjectNonShareable
{
	int resourceNameInt;
	int resourceTypeInt;
	char *resourceName;
	char *resourceType;
	int w;
	int h;
	D2D1_PIXEL_FORMAT pixelFormat;
	D2D1_BITMAP_OPTIONS bitmapOptions;

	wchar_t *resourceFile;

public:
	// Constructors
	ImageObject()
		: RenderingObjectNonShareable(), resourceName(NULL), resourceType(NULL), resourceFile(NULL), resourceNameInt(-1), resourceTypeInt(-1), w(-1), h(-1), pixelFormat(D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_UNKNOWN)) {}
#ifdef SIMPLE2D_COMPAT_107
	ImageObject(Simple2D *r)
		: RenderingObjectNonShareable(r), resourceName(NULL), resourceType(NULL), resourceFile(NULL), resourceNameInt(-1), resourceTypeInt(-1), w(-1), h(-1), pixelFormat(D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_UNKNOWN)) {}
	ImageObject(Simple2D *r, ID2D1Bitmap1 *b)
		: RenderingObjectNonShareable(r, b), resourceName(NULL), resourceType(NULL), resourceFile(NULL), resourceNameInt(-1), resourceTypeInt(-1), w(-1), h(-1), pixelFormat(D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_UNKNOWN)) {}
	ImageObject(Simple2D *r, char const *resName, char const *resType)
		: RenderingObjectNonShareable(r, NULL), resourceFile(NULL), resourceNameInt(-1), resourceTypeInt(-1), w(-1), h(-1), pixelFormat(D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_UNKNOWN))
	{
		resourceName = new char [strlen(resName) + 1];
		resourceType = new char [strlen(resType) + 1];
		strcpy(resourceName, resName);
		strcpy(resourceType, resType);
	}
	ImageObject(Simple2D *r, int const resName, char const *resType)
		: RenderingObjectNonShareable(r, NULL), resourceName(NULL), resourceFile(NULL), resourceNameInt(resName), resourceTypeInt(-1), w(-1), h(-1), pixelFormat(D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_UNKNOWN))
	{
		resourceType = new char [strlen(resType) + 1];
		strcpy(resourceType, resType);
	}
	ImageObject(Simple2D *r, char const *resName, int const resType)
		: RenderingObjectNonShareable(r, NULL), resourceType(NULL), resourceFile(NULL), resourceNameInt(-1), resourceTypeInt(resType), w(-1), h(-1), pixelFormat(D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_UNKNOWN))
	{
		resourceName = new char [strlen(resName) + 1];
		strcpy(resourceName, resName);
	}
	ImageObject(Simple2D *r, int const resName, int const resType)
		: RenderingObjectNonShareable(r, NULL), resourceName(NULL), resourceType(NULL), resourceFile(NULL), resourceNameInt(resName), resourceTypeInt(resType), w(-1), h(-1), pixelFormat(D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_UNKNOWN)) {}

	ImageObject(Simple2D *r, PCWSTR resFile)
		: RenderingObjectNonShareable(r, NULL), resourceName(NULL), resourceType(NULL), resourceNameInt(-1), resourceTypeInt(-1), w(-1), h(-1), pixelFormat(D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_UNKNOWN))
	{
		resourceFile = new wchar_t [wcslen(resFile) + 1];
		wcscpy(resourceFile, resFile);
	}

	ImageObject(Simple2D *r, int w, int h, D2D1_PIXEL_FORMAT pixelFormat, D2D1_BITMAP_OPTIONS bitmapOptions = D2D1_BITMAP_OPTIONS_NONE)
		: RenderingObjectNonShareable(r, NULL), resourceName(NULL), resourceType(NULL), resourceFile(NULL), resourceNameInt(-1), resourceTypeInt(-1), w(w), h(h), pixelFormat(pixelFormat), bitmapOptions(bitmapOptions) {}
#endif
	ImageObject(ID2D1Bitmap1 *b)
		: RenderingObjectNonShareable(b), resourceName(NULL), resourceType(NULL), resourceFile(NULL), resourceNameInt(-1), resourceTypeInt(-1), w(-1), h(-1), pixelFormat(D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_UNKNOWN)) {}
	ImageObject(char const *resName, char const *resType)
		: RenderingObjectNonShareable(), resourceFile(NULL), resourceNameInt(-1), resourceTypeInt(-1), w(-1), h(-1), pixelFormat(D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_UNKNOWN))
	{
		resourceName = new char [strlen(resName) + 1];
		resourceType = new char [strlen(resType) + 1];
		strcpy(resourceName, resName);
		strcpy(resourceType, resType);
	}
	ImageObject(int const resName, char const *resType)
		: RenderingObjectNonShareable(), resourceName(NULL), resourceFile(NULL), resourceNameInt(resName), resourceTypeInt(-1), w(-1), h(-1), pixelFormat(D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_UNKNOWN))
	{
		resourceType = new char [strlen(resType) + 1];
		strcpy(resourceType, resType);
	}
	ImageObject(char const *resName, int const resType)
		: RenderingObjectNonShareable(), resourceType(NULL), resourceFile(NULL), resourceNameInt(-1), resourceTypeInt(resType), w(-1), h(-1), pixelFormat(D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_UNKNOWN))
	{
		resourceName = new char [strlen(resName) + 1];
		strcpy(resourceName, resName);
	}
	ImageObject(int const resName, int const resType)
		: RenderingObjectNonShareable(), resourceName(NULL), resourceType(NULL), resourceFile(NULL), resourceNameInt(resName), resourceTypeInt(resType), w(-1), h(-1), pixelFormat(D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_UNKNOWN)) {}

	ImageObject(PCWSTR resFile)
		: RenderingObjectNonShareable(), resourceName(NULL), resourceType(NULL), resourceNameInt(-1), resourceTypeInt(-1), w(-1), h(-1), pixelFormat(D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_UNKNOWN))
	{
		resourceFile = new wchar_t [wcslen(resFile) + 1];
		wcscpy(resourceFile, resFile);
	}

	ImageObject(int w, int h, D2D1_PIXEL_FORMAT pixelFormat, D2D1_BITMAP_OPTIONS bitmapOptions = D2D1_BITMAP_OPTIONS_NONE)
		: RenderingObjectNonShareable(), resourceName(NULL), resourceType(NULL), resourceFile(NULL), resourceNameInt(-1), resourceTypeInt(-1), w(w), h(h), pixelFormat(pixelFormat), bitmapOptions(bitmapOptions) {}

	~ImageObject()
	{
		if (resourceName) delete [] resourceName;
		if (resourceType) delete [] resourceType;
		if (resourceFile) delete [] resourceFile;
	}

	// Get pointer to encapsulated image
	ID2D1Bitmap1 *GetImage() { if (!resource) Create(); return static_cast<ID2D1Bitmap1 *>(Get()); }

	// Hashing for containers
	size_t CreateHash() const
	{
		size_t seed = 0;

		if (resourceName)
			boost::hash_combine(seed, resourceName);
		if (resourceType)
			boost::hash_combine(seed, resourceType);
		if (resourceNameInt != -1)
			boost::hash_combine(seed, resourceNameInt);
		if (resourceTypeInt != -1)
			boost::hash_combine(seed, resourceTypeInt);
		if (resourceFile)
			boost::hash_combine(seed, resourceFile);

		return seed;
	}

	// Implementation
	void Create();

	// Drawing functions

	// Draw entire bitmap at (x,y) on screen
	void Draw(int x, int y, float opacity = 1.0f, float rotation = 0.f);

	// Draw entire bitmap scaled to fit (x1,y1)->(x2,y2) on screen
	void Draw(int x1, int y1, int x2, int y2, float opacity = 1.0f, float rotation = 0.f);

	// Draw entire bitmap scaled to fit (x,y)->(x+w,y+h) on screen
	void DrawWH(int x, int y, int w, int h, float opacity = 1.0f, float rotation = 0.f);

	// Draw portion of bitmap (srcX,srcY)->(srcX + w, srcY + h) unscaled at (x,y) on screen
	void DrawPartWH(int x, int y, int srcX, int srcY, int w, int h, float opacity = 1.0f, float rotation = 0.f);

	// Draw portion of bitmap (srcX,srcY)->(srcX+srcW,srcY+srcH) scaled to fit (x,y)->(x+w,y+h) on screen
	void DrawPartWH(int x, int y, int w, int h, int srcX, int srcY, int srcW, int srcH, float opacity = 1.0f, float rotation = 0.f);

	// Draw portion of bitmap (srcX1,srcY1)->(srcX2,srcY2) scaled to fit (x1,y1)->(x2,y2) on screen
	void DrawPart(int x1, int y1, int x2, int y2, int srcX1, int srcY1, int srcX2, int srcY2, float opacity = 1.0f, float rotation = 0.f);
};

// ==================================================================================
// Abstract brush class
// ==================================================================================

// Brush alignment types
enum AlignmentType { Horizontal, Vertical, Diagonal, Auto, Custom };

class GenericBrush : public RenderingObjectNonShareable
{
public:
	// Constructors
	GenericBrush() : RenderingObjectNonShareable() {}
	GenericBrush(ID2D1Brush *b) : RenderingObjectNonShareable(b) {}

#ifdef SIMPLE2D_COMPAT_107
	GenericBrush(Simple2D *r) : RenderingObjectNonShareable(r) {}
	GenericBrush(Simple2D *r, ID2D1Brush *b) : RenderingObjectNonShareable(r, b) {}
#endif

	// Get pointer to encapsulated brush
	ID2D1Brush *GetBrush() { return static_cast<ID2D1Brush *>(Get()); }

	// Set brush opacity
	void SetOpacity(FLOAT opacity) { GetBrush()->SetOpacity(opacity); }

	// Set brush transform
	void SetTransform(Matrix &m) { SetTransform(&m); }
	void SetTransform(Matrix *m) { GetBrush()->SetTransform(m); }

	// Prepare the brush for rendering (for example, setting the gradient positions)
	virtual void Prepare(int x1, int y1, int x2, int y2) = 0;
};

// ==================================================================================
// Solid colour paintbrush helper class
// ==================================================================================

// NOTE: No memory management is required

class PaintbrushObject : public GenericBrush
{
private:
	// Brush colour
	D2D1_COLOR_F colour;

public:
	// Constructors
	PaintbrushObject(ID2D1SolidColorBrush *b);
	PaintbrushObject(D2D1_COLOR_F &col);
	PaintbrushObject(D2D1::ColorF::Enum col);

#ifdef SIMPLE2D_COMPAT_107
	PaintbrushObject(Simple2D *r, ID2D1SolidColorBrush *b) : GenericBrush(r, b) {}
	PaintbrushObject(Simple2D *r, D2D1_COLOR_F &col) : GenericBrush(r), colour(col) {}
	PaintbrushObject(Simple2D *r, D2D1::ColorF::Enum col) : GenericBrush(r), colour(D2D1::ColorF(col)) {}
#endif

	// Hashing for containers
	size_t CreateHash() const
	{
		std::size_t seed = 0;
		boost::hash_combine(seed, colour.a);
		boost::hash_combine(seed, colour.b);
		boost::hash_combine(seed, colour.g);
		boost::hash_combine(seed, colour.r);
		return seed;
	}

	// Implementation
	void Create();
	void Prepare(int x1, int y1, int x2, int y2) {}
};

// ==================================================================================
// Linear Gradient Paintbrush helper class
// ==================================================================================

// NOTE: No memory management is required

class GradientObject : public GenericBrush
{
private:
	// The gradient type
	AlignmentType gradientType;

	// The start and end colours
	D2D1_COLOR_F start, end;

	// Extend mode
	D2D1_EXTEND_MODE extendMode;

public:
	// Set the start and end points of the gradient relative to the render target
	// (these are called automatically for you if you pass an object of this class to
	// one of the Simple2D drawing functions, to align the gradient to what you are
	// drawing)
	void SetPoints(int x1, int y1, int x2, int y2);
	void SetPointsWH(int x1, int y1, int w, int h);
	void SetPointsUsingAlignmentType(int x1, int y1, int x2, int y2);
	void SetPointsUsingAlignmentTypeWH(int x1, int y1, int w, int h);

	// Constructors
	GradientObject() : gradientType(Vertical), extendMode(D2D1_EXTEND_MODE_CLAMP), GenericBrush() {}
	GradientObject(ID2D1LinearGradientBrush *b, AlignmentType gt = Vertical, D2D1_EXTEND_MODE extendMode = D2D1_EXTEND_MODE_CLAMP);
	GradientObject(D2D1_COLOR_F &start, D2D1_COLOR_F &end, AlignmentType gt = Vertical, D2D1_EXTEND_MODE extendMode = D2D1_EXTEND_MODE_CLAMP);
	GradientObject(D2D1::ColorF::Enum start, D2D1_COLOR_F &end, AlignmentType gt = Vertical, D2D1_EXTEND_MODE extendMode = D2D1_EXTEND_MODE_CLAMP);
	GradientObject(D2D1_COLOR_F &start, D2D1::ColorF::Enum end, AlignmentType gt = Vertical, D2D1_EXTEND_MODE extendMode = D2D1_EXTEND_MODE_CLAMP);
	GradientObject(D2D1::ColorF::Enum start, D2D1::ColorF::Enum end, AlignmentType gt = Vertical, D2D1_EXTEND_MODE extendMode = D2D1_EXTEND_MODE_CLAMP);

private:
	GradientObject(ID2D1LinearGradientBrush *b, D2D1_COLOR_F &start, D2D1_COLOR_F &end, AlignmentType gt, D2D1_EXTEND_MODE extendMode);

public:
#ifdef SIMPLE2D_COMPAT_107
	GradientObject(Simple2D *r, ID2D1LinearGradientBrush *b, AlignmentType gt = Vertical, D2D1_EXTEND_MODE extendMode = D2D1_EXTEND_MODE_CLAMP)
		: GenericBrush(r, b), gradientType(gt), extendMode(extendMode) {}
	GradientObject(Simple2D *r, D2D1_COLOR_F &start, D2D1_COLOR_F &end, AlignmentType gt = Vertical, D2D1_EXTEND_MODE extendMode = D2D1_EXTEND_MODE_CLAMP)
		: GenericBrush(r, NULL), start(start), end(end), gradientType(gt), extendMode(extendMode) {}
	GradientObject(Simple2D *r, D2D1::ColorF::Enum start, D2D1_COLOR_F &end, AlignmentType gt = Vertical, D2D1_EXTEND_MODE extendMode = D2D1_EXTEND_MODE_CLAMP)
		: GenericBrush(r, NULL), start(D2D1::ColorF(start)), end(end), gradientType(gt), extendMode(extendMode) {}
	GradientObject(Simple2D *r, D2D1_COLOR_F &start, D2D1::ColorF::Enum end, AlignmentType gt = Vertical, D2D1_EXTEND_MODE extendMode = D2D1_EXTEND_MODE_CLAMP)
		: GenericBrush(r, NULL), start(start), end(D2D1::ColorF(end)), gradientType(gt), extendMode(extendMode) {}
	GradientObject(Simple2D *r, D2D1::ColorF::Enum start, D2D1::ColorF::Enum end, AlignmentType gt = Vertical, D2D1_EXTEND_MODE extendMode = D2D1_EXTEND_MODE_CLAMP)
		: GenericBrush(r, NULL), start(D2D1::ColorF(start)), end(D2D1::ColorF(end)), gradientType(gt), extendMode(extendMode) {}
#endif

	// Change gradient type
	void SetAlignmentType(AlignmentType gt) { gradientType = gt; }

	// Hashing for containers
	size_t CreateHash() const
	{
		std::size_t seed = 0;
		boost::hash_combine(seed, start.a);
		boost::hash_combine(seed, start.b);
		boost::hash_combine(seed, start.g);
		boost::hash_combine(seed, start.r);
		boost::hash_combine(seed, end.a);
		boost::hash_combine(seed, end.b);
		boost::hash_combine(seed, end.g);
		boost::hash_combine(seed, end.r);
		return seed;
	}

	// Implementation
	virtual void Create();
	virtual void Prepare(int x1, int y1, int x2, int y2) { if (!resource.get()) Create(); SetPointsUsingAlignmentType(x1, y1, x2, y2); }
};

// ==================================================================================
// Bitmap brush helper class
// ==================================================================================

// NOTE: No memory management is required

class ImageBrushObject : public GenericBrush
{
private:
	// Pointer to bitmap to use
	Image bitmap;

	// Extend modes
	D2D1_BITMAP_BRUSH_PROPERTIES properties;

	// Alignment type
	AlignmentType alignment;

public:
	// Constructors
	ImageBrushObject() : GenericBrush(), bitmap(NULL), alignment(Custom) {}

#ifdef SIMPLE2D_COMPAT_107
	ImageBrushObject(Simple2D *r, Image image, AlignmentType at = Auto,
		D2D1_EXTEND_MODE extendModeX = D2D1_EXTEND_MODE_CLAMP, D2D1_EXTEND_MODE extendModeY = D2D1_EXTEND_MODE_CLAMP)
		: GenericBrush(), bitmap(image), alignment(at) {
			properties.extendModeX = extendModeX;
			properties.extendModeY = extendModeY;
			properties.interpolationMode = D2D1_BITMAP_INTERPOLATION_MODE_LINEAR;
	}
#endif
	ImageBrushObject(Image image, AlignmentType at = Auto,
		D2D1_EXTEND_MODE extendModeX = D2D1_EXTEND_MODE_CLAMP, D2D1_EXTEND_MODE extendModeY = D2D1_EXTEND_MODE_CLAMP)
		: GenericBrush(), bitmap(image), alignment(at) {
			properties.extendModeX = extendModeX;
			properties.extendModeY = extendModeY;
			properties.interpolationMode = D2D1_BITMAP_INTERPOLATION_MODE_LINEAR;
	}

	// Hashing for containers
	size_t CreateHash() const
	{
		return bitmap->GetHash() + 1;
	}

	// Implementation
	virtual void Create();
	void Prepare(int x1, int y1, int x2, int y2) { if (!resource.get()) Create(); if (alignment == Auto) SetTransform(D2D1::Matrix3x2F::Translation(static_cast<FLOAT>(x1), static_cast<FLOAT>(y1))); }
};

// ==================================================================================
// Geometry helper class
// ==================================================================================

// NOTE: No memory management is needed

// Relative drawing start co-ordinates
enum GeometryDrawStart { Default, TopLeft, Center, Assigned };

// A transformation point
enum GeometryTransformPoint { PointCenter, PointTopLeft, PointTopRight, PointBottomLeft, PointBottomRight };

class Geometry : public RenderingObjectShareable
{
public:
	// Fill types
	enum FillType { Filled, Hollow };

	// Figure fill types
	enum FigureFillType { Winding, Alternate };

	// Path types
	enum PathType { Open, Closed };

private:
	// Temporary geometry sink
	ID2D1GeometrySink *sink;

	// The fill type
	FillType fillType;

	// The default relative drawing start co-ordinate type
	GeometryDrawStart relativeDrawPos;

	// The default stroke width
	FLOAT defaultStrokeWidth;

	// The default stroke style
	ID2D1StrokeStyle *defaultStrokeStyle;

	// The default opacity brush
	ID2D1Brush *defaultOpacityBrush;

	// True to auto-adjust brush to have the same transform as the geometry
	bool autoAdjustBrush;

	// The current transform
	Matrix transform;

	// Calculate a geometric transform (return object is not memory-managed)
	ID2D1TransformedGeometry *internalTransform(Matrix &transform = D2D1::Matrix3x2F::Identity(), GeometryDrawStart relPos = Assigned, FLOAT strokeWidth = -1.0f, ID2D1StrokeStyle *strokeStyle = NULL);

public:
	// Constructors
	Geometry()
		: RenderingObjectShareable(), sink(NULL), fillType(Filled), relativeDrawPos(Default), autoAdjustBrush(false),
		  defaultStrokeWidth(1.0f), defaultStrokeStyle(NULL), defaultOpacityBrush(NULL), transform(D2D1::Matrix3x2F::Identity()) {}

#ifdef SIMPLE2D_COMPAT_107
	Geometry(Simple2D *r)
		: RenderingObjectShareable(), sink(NULL), fillType(Filled), relativeDrawPos(Default), autoAdjustBrush(false),
		  defaultStrokeWidth(1.0f), defaultStrokeStyle(NULL), defaultOpacityBrush(NULL), transform(D2D1::Matrix3x2F::Identity()) {}
	Geometry(Simple2D *r, ID2D1Geometry *g, FillType ft = Filled, GeometryDrawStart ds = Default, bool autoAdjustBrush = false)
		: RenderingObjectShareable(g), sink(NULL), fillType(ft), relativeDrawPos(ds), autoAdjustBrush(autoAdjustBrush),
		  defaultStrokeWidth(1.0f), defaultStrokeStyle(NULL), defaultOpacityBrush(NULL), transform(D2D1::Matrix3x2F::Identity()) {}
#endif
	Geometry(ID2D1Geometry *g, FillType ft = Filled, GeometryDrawStart ds = Default, bool autoAdjustBrush = false)
		: RenderingObjectShareable(g), sink(NULL), fillType(ft), relativeDrawPos(ds), autoAdjustBrush(autoAdjustBrush),
		  defaultStrokeWidth(1.0f), defaultStrokeStyle(NULL), defaultOpacityBrush(NULL), transform(D2D1::Matrix3x2F::Identity()) {}

	// Get pointer to encapsulated geometry
	ID2D1Geometry *GetOriginalGeometry() { return static_cast<ID2D1Geometry *>(Get()); }

	// Get new geometry object with current transform applied
	Geometry GetGeometry(Matrix &m = D2D1::Matrix3x2F::Identity());

	// Clear current transform
	void ClearTransform() { transform = None(); }

	// Set current transform
	void SetTransform(Matrix &m) { transform = m; }

	// Get current transform
	Matrix &GetTransform() { return transform; }

	// Set stroke width
	void SetStrokeWidth(FLOAT w) { defaultStrokeWidth = w; }

	// Set stroke style
	void SetStrokeStyle(ID2D1StrokeStyle *s) { defaultStrokeStyle = s; }

	// Set opacity brush
	void SetOpacityBrush(ID2D1Brush *b) { defaultOpacityBrush = b; }

	// Set whether to auto-adjust the brush to match the geometry's transform
	void SetAutoAdjustBrushTransform(bool aab) { autoAdjustBrush = aab; }

	// Hashing for containers
	size_t CreateHash() const
	{
		return static_cast<size_t>(rand());
	}

	// Implementation
	void Create() {}

	// Open and close geometry sink
	ID2D1GeometrySink *OpenSink();
	void CloseSink();

	// Get a rotation matrix around the specified point
	Matrix Rotate(float angle, GeometryTransformPoint point);
	Matrix Rotate(float angle, int x, int y);

	// Get a scaling matrix on the specified point
	Matrix Scale(float scaleX, float scaleY, GeometryTransformPoint point);
	Matrix Scale(float scaleX, float scaleY, int x, int y);

	// Get a skewing matrix on the specified point
	Matrix Skew(float angleX, float angleY, GeometryTransformPoint point);
	Matrix Skew(float angleX, float angleY, int x, int y);

	// Get a translation matrix
	Matrix Move(float x, float y);
	Matrix Move(int x, int y);

	// Get the identity matrix
	Matrix None();

	// Get bounding box of geometry
	D2D1_RECT_F GetOriginalBounds(Matrix &transform = D2D1::Matrix3x2F::Identity(), FLOAT strokeWidth = -1.0f, ID2D1StrokeStyle *strokeStyle = NULL);
	D2D1_RECT_F GetBounds(Matrix &transform = D2D1::Matrix3x2F::Identity(), FLOAT strokeWidth = -1.0f, ID2D1StrokeStyle *strokeStyle = NULL);

	// Get the length of the geometry (rolled out as a single line)
	float GetOriginalLength(Matrix &transform);
	float GetOriginalLength(Matrix *transform = NULL);
	float GetLength(Matrix &transform);
	float GetLength(Matrix *transform = NULL);

	// Returns information about geometry intersection
	D2D1_GEOMETRY_RELATION GetIntersection(Geometry &o);

	// Returns true if the geometry contains the specified point
	bool ContainsPoint(D2D1_POINT_2F point);
	bool ContainsPoint(float x, float y);
	bool ContainsPoint(int x, int y);

	// Returns the intersection of two pieces of geometry, with the supplied geometry optionally transformed first
	Geometry GetIntersectedGeometry(Geometry &o, Matrix &m = D2D1::Matrix3x2F::Identity());

	// Drawing functions
	void Draw(Matrix &transform = D2D1::Matrix3x2F::Identity(), GeometryDrawStart relPos = Assigned, GenericBrush *brush = NULL, FLOAT strokeWidth = -1.0f, ID2D1StrokeStyle *strokeStyle = NULL);
	void Draw(Matrix &transform, GeometryDrawStart relPos, ID2D1Brush *brush, FLOAT strokeWidth = -1.0f, ID2D1StrokeStyle *strokeStyle = NULL);
	void Draw(Matrix &transform, GeometryDrawStart relPos, TemporaryBrush brush, FLOAT strokeWidth = -1.0f, ID2D1StrokeStyle *strokeStyle = NULL);
	void Draw(int x, int y, GeometryDrawStart relPos = Assigned, GenericBrush *brush = NULL, FLOAT strokeWidth = -1.0f, ID2D1StrokeStyle *strokeStyle = NULL);
	void Draw(int x, int y, GeometryDrawStart relPos, ID2D1Brush *brush, FLOAT strokeWidth = -1.0f, ID2D1StrokeStyle *strokeStyle = NULL);
	void Draw(int x, int y, GeometryDrawStart relPos, TemporaryBrush brush, FLOAT strokeWidth = -1.0f, ID2D1StrokeStyle *strokeStyle = NULL);

	void Fill(Matrix &transform = D2D1::Matrix3x2F::Identity(), GeometryDrawStart relPos = Assigned, GenericBrush *brush = NULL, ID2D1Brush *opacityBrush = NULL);
	void Fill(Matrix &transform, GeometryDrawStart relPos, ID2D1Brush *brush, ID2D1Brush *opacityBrush = NULL);
	void Fill(Matrix &transform, GeometryDrawStart relPos, TemporaryBrush brush, ID2D1Brush *opacityBrush = NULL);
	void Fill(int x, int y, GeometryDrawStart relPos = Assigned, GenericBrush *brush = NULL, ID2D1Brush *opacityBrush = NULL);
	void Fill(int x, int y, GeometryDrawStart relPos, ID2D1Brush *brush, ID2D1Brush *opacityBrush = NULL);
	void Fill(int x, int y, GeometryDrawStart relPos, TemporaryBrush brush, ID2D1Brush *opacityBrush = NULL);
};

// ==================================================================================
// Simple2D Interface
// ==================================================================================

enum ResizeBehaviour { RB_Center, RB_Stretch, RB_Resize, RB_Zoom };

// Struct to store application startup data
struct Simple2DStartupInfo
{
	// Manufacturer name
	string ManufacturerName;

	// Application name
	string AppName;

	// Application version
	string Version;

	// Initial render target size
	int ResolutionX;
	int ResolutionY;

	// Minimum supported Direct3D feature level where Direct3D is used
	double MinimumFeatureLevel;

	// Enable Direct3D rendering pipeline access
	bool Enable3D;

	// Icon resource ID for the application window (0 for default)
	int IconResource;

	// Name for the application window
	string WindowName;

	// Window resize behaviour
	ResizeBehaviour ResizeBehaviourType;

	// Direct2D background colour clear if clearing is enabled
	D2D1_COLOR_F BackgroundColour;

	// Enable Direct2D render target clear at start of each frame
	bool EnableClear;

	// Enable/disable full-screen operation on user demand
	bool EnableFullScreen;

	// Start application full-screen or windowed
	bool FullScreen;

	// Enable/disable full-screen display mode switching
	bool EnableModeSwitch;

	// Enable/disable window resizing
	bool ResizableWindow;

	// The minimum window size
	int MinimumWindowSizeX;
	int MinimumWindowSizeY;

	// Enable 4x multi-sampling where Direct3D is used
	bool EnableMSAA;

	// Vertical sync clamp (0 = no clamp, >0 = v-sync multiple)
	int VSyncClamp;

	// Convenience constructor
	Simple2DStartupInfo(
		string manufacturerName = "",
		string appName = "",
		string version = "",
		int resolutionX = 640,
		int resolutionY = 480,
		double minimumFeatureLevel = 9.1,
		bool enable3D = false,
		int iconResource = 0,
		string windowName = "Simple2D Application",
		ResizeBehaviour resizeBehaviourType = ResizeBehaviour::RB_Center,
		D2D1::ColorF backgroundColour = D2D1::ColorF(D2D1::ColorF::White),
		bool enableClear = true,
		bool enableFullScreen = true,
		bool fullScreen = false,
		bool enableModeSwitch = false,
		bool resizableWindow = false,
		int minimumWindowSizeX = 0,
		int minimumWindowSizeY = 0,
		bool enableMSAA = false,
		int vSyncClamp = 0)
		:
		ManufacturerName(manufacturerName), AppName(appName), Version(version),
		ResolutionX(resolutionX), ResolutionY(resolutionY),
		MinimumFeatureLevel(minimumFeatureLevel),
		Enable3D(enable3D),
		IconResource(iconResource), WindowName(windowName),
		ResizeBehaviourType(resizeBehaviourType),
		BackgroundColour(backgroundColour),
		EnableClear(enableClear),
		EnableFullScreen(enableFullScreen), FullScreen(fullScreen),
		EnableModeSwitch(enableModeSwitch),
		ResizableWindow(resizableWindow),
		MinimumWindowSizeX(minimumWindowSizeX), MinimumWindowSizeY(minimumWindowSizeY),
		EnableMSAA(enableMSAA),
		VSyncClamp(vSyncClamp)
	{}
};

class Simple2D
{
	friend class Geometry;

public:
	// Constructor and destructor
	Simple2D();
	Simple2D(Simple2DStartupInfo);
	~Simple2D();

	// Pointer to singleton
	static Simple2D *App;

	// Entry point (call this from Simple2DStart() after creating your application object)
	void Run();

	// Set the window title (after application window is opened)
	void SetWindowTitle(LPCSTR title);

	// Set resize behaviour
	void SetResizeBehaviour(ResizeBehaviour);

	// Set background colour
	void SetBackgroundColour(D2D1_COLOR_F &);
	void SetBackgroundColour(D2D1::ColorF::Enum);

	// Enable/disable per-frame surface clear
	void SetEnableClear(bool);

	// Enable/disable FPS counter in window title
	void SetShowFps(bool);

	// Set when the active Scene (if any) should be rendered
	void SetRenderSceneAfter(bool);

	// Set the V-sync clamp (0 = no clamp, >0 = v-sync multiple)
	void SetVSyncClamp(int);

	// Return true if the application window or the full-screen render target has focus
	bool HasFocus();

	// Get window handle
	HWND GetWindow();

	// Get the render target aspect ratio
	float GetAspectRatio() const;

	// Set the active scene
	void ClearScene();
	void SetScene(Scene &scene, bool resetAnimations = true);
	Scene *GetScene();

	// Get the overlay scene
	Scene &GetOverlay();

	// Set focus object
	// NOTE: Should not be used directly. Use SetFocus() on a control instance instead.
	void SetFocusObject(InterfaceObject &focus, bool set = true);
	void SetFocusObject(InterfaceObject *focus, bool set = true);

	// Get focus object
	InterfaceObject *GetFocusObject();

	// Make solid brushes
	Paintbrush MakeBrush(D2D1_COLOR_F &);
	Paintbrush MakeBrush(D2D1::ColorF::Enum);

	// Make linear gradient brushes
	Gradient MakeBrush(D2D1_COLOR_F &start, D2D1_COLOR_F &end, AlignmentType = Vertical, D2D1_EXTEND_MODE extendMode = D2D1_EXTEND_MODE_CLAMP, bool managed = true);
	Gradient MakeBrush(D2D1::ColorF::Enum start, D2D1_COLOR_F &end, AlignmentType = Vertical, D2D1_EXTEND_MODE extendMode = D2D1_EXTEND_MODE_CLAMP, bool managed = true);
	Gradient MakeBrush(D2D1_COLOR_F &start, D2D1::ColorF::Enum end, AlignmentType = Vertical, D2D1_EXTEND_MODE extendMode = D2D1_EXTEND_MODE_CLAMP, bool managed = true);
	Gradient MakeBrush(D2D1::ColorF::Enum start, D2D1::ColorF::Enum end, AlignmentType = Vertical, D2D1_EXTEND_MODE extendMode = D2D1_EXTEND_MODE_CLAMP, bool managed = true);

	// Make bitmap brushes
	ImageBrush MakeBrush(Image image, AlignmentType alignment = Auto, D2D1_EXTEND_MODE extendModeX = D2D1_EXTEND_MODE_CLAMP, D2D1_EXTEND_MODE extendModeY = D2D1_EXTEND_MODE_CLAMP, bool managed = true);

	// Load a bitmap
	Image MakeImage(char const *resourceName, char const *resourceType, bool managed = true);
	Image MakeImage(int const resourceName, char const *resourceType, bool managed = true);
	Image MakeImage(char const *resourceName, int const resourceType, bool managed = true);
	Image MakeImage(int const resourceName, int const resourceType, bool managed = true);
	Image MakeImage(PCWSTR resourceFile, bool managed = true);

	// Create an empty bitmap of the specified size using the Direct2D device context or render target's DPI
	Image MakeEmptyImage(int w, int h, D2D1_PIXEL_FORMAT pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED), D2D1_BITMAP_OPTIONS options = D2D1_BITMAP_OPTIONS_NONE, bool managed = false);

	// Set the current brush
	void SetBrush(ID2D1Brush *);
	void SetBrush(GenericBrush *);
	void SetBrush(TemporaryBrush);

	// Make a brush of the selected colour if none exists and set it as the current brush
	void SetBrush(D2D1_COLOR_F &col);
	void SetBrush(D2D1::ColorF::Enum col);

	// Make DirectWrite text format
	TextFormat MakeTextFormat(WCHAR *, FLOAT,
		DWRITE_TEXT_ALIGNMENT = DWRITE_TEXT_ALIGNMENT_LEADING,
		DWRITE_FONT_WEIGHT = DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE = DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH = DWRITE_FONT_STRETCH_NORMAL);

	// Make DirectWrite text layout
	TextLayout MakeTextLayout(const WCHAR *text, TextFormat format, int boundX = -1, int boundY = -1);
	TextLayout MakeTextLayout(string text, TextFormat format, int boundX = -1, int boundY = -1);

	// Get number of characters in text from its TextLayout
	int TextLength(TextLayout layout);

	// Get pixel width of individual character or all characters if which == -1
	int TextWidth(TextLayout layout, int which = -1);

	// Get pixel height of text
	int TextHeight(TextLayout layout);

	// Set the current text format
	void SetTextFormat(TextFormat );

	// Create an ellipse geometry (or circle if ry == -1) (centered at origin if x,y omitted)
	Geometry MakeEllipseGeometry(int x, int y, int rx, int ry = -1);
	Geometry MakeEllipseGeometry(int rx, int ry = -1);

	// Create a rectangle geometry
	Geometry MakeRectangleGeometry(int w, int h);
	Geometry MakeRectangleGeometry(int x1, int y1, int x2, int y2);
	Geometry MakeRectangleGeometry(D2D1_RECT_F r);
	Geometry MakeRectangleGeometryWH(int x1, int y1, int w, int h);

	// Start to create a path geometry
	ID2D1GeometrySink *StartCreatePath(int x, int y, Geometry::FillType = Geometry::Filled, GeometryDrawStart = Default, Geometry::FigureFillType = Geometry::Winding);

	// Finish creating a path geometry
	Geometry EndCreatePath(Geometry::PathType = Geometry::Closed);

	// Make line geometry
	Geometry MakeLineGeometry(D2D1_POINT_2F *points, int numPoints, Geometry::PathType = Geometry::Closed, Geometry::FillType = Geometry::Filled, GeometryDrawStart = Default, Geometry::FigureFillType = Geometry::Winding);

	template <typename Container>
	Geometry MakeLineGeometry(Container points, Geometry::PathType pt = Geometry::Closed, Geometry::FillType ft = Geometry::Filled, GeometryDrawStart rp = Default, Geometry::FigureFillType fft = Geometry::Winding)
	{
		GeometryData gd = StartCreatePath(static_cast<int>(points[0].x), static_cast<int>(points[0].y), ft, rp, fft);
		gd->AddLines(&(*points.begin()), points.size() - 1);
		return EndCreatePath(pt);
	}

	// Test two geometries for collisions
	D2D1_GEOMETRY_RELATION GeometryCollision(Geometry &o1, Geometry &o2, Matrix &m1 = D2D1::Matrix3x2F::Identity(), Matrix &m2 = D2D1::Matrix3x2F::Identity());

	// Drawing functions

	// Draw a line from (x1,y1) to (x2,y2) using the specified brush, or the current brush if not specified
	// The last two argumetns are the stroke width (optional) and stroke style (optional)
	void Line(int x1, int y1, int x2, int y2, GenericBrush *brush = NULL, FLOAT = 1.0f, ID2D1StrokeStyle * = NULL);
	void Line(int x1, int y1, int x2, int y2, ID2D1Brush *, FLOAT = 1.0f, ID2D1StrokeStyle * = NULL);
	void Line(int x1, int y1, int x2, int y2, TemporaryBrush brush, FLOAT = 1.0f, ID2D1StrokeStyle * = NULL);
	void Line(int x1, int y1, int x2, int y2, D2D1_COLOR_F &col, FLOAT strokeWidth = 1.0f, ID2D1StrokeStyle *strokeStyle = NULL);
	void Line(int x1, int y1, int x2, int y2, D2D1::ColorF::Enum col, FLOAT strokeWidth = 1.0f, ID2D1StrokeStyle *strokeStyle = NULL);
	void Line(int x1, int y1, int x2, int y2, FLOAT = 1.0f, ID2D1StrokeStyle * = NULL);

	// Draw a line from (x1,y1) of dimensions (w,h)
	void LineWH(int x1, int y1, int w, int h, GenericBrush *brush = NULL, FLOAT = 1.0f, ID2D1StrokeStyle * = NULL);
	void LineWH(int x1, int y1, int w, int h, ID2D1Brush *, FLOAT = 1.0f, ID2D1StrokeStyle * = NULL);
	void LineWH(int x1, int y1, int w, int h, TemporaryBrush brush, FLOAT = 1.0f, ID2D1StrokeStyle * = NULL);
	void LineWH(int x1, int y1, int w, int h, D2D1_COLOR_F &col, FLOAT strokeWidth = 1.0f, ID2D1StrokeStyle *strokeStyle = NULL);
	void LineWH(int x1, int y1, int w, int h, D2D1::ColorF::Enum col, FLOAT strokeWidth = 1.0f, ID2D1StrokeStyle *strokeStyle = NULL);
	void LineWH(int x1, int y1, int w, int h, FLOAT = 1.0f, ID2D1StrokeStyle * = NULL);

	// Draw a rectangle with upper-left corner (x1,y1) and lower-right corner (x2,y2) using the specified brush,
	// or the current brush if not specified
	void DrawRectangle(int x1, int y1, int x2, int y2, GenericBrush *brush = NULL);
	void DrawRectangle(int x1, int y1, int x2, int y2, ID2D1Brush *);
	void DrawRectangle(int x1, int y1, int x2, int y2, TemporaryBrush brush);
	void DrawRectangle(int x1, int y1, int x2, int y2, D2D1_COLOR_F &col);
	void DrawRectangle(int x1, int y1, int x2, int y2, D2D1::ColorF::Enum);
	void FillRectangle(int x1, int y1, int x2, int y2, GenericBrush *brush = NULL);
	void FillRectangle(int x1, int y1, int x2, int y2, ID2D1Brush *);
	void FillRectangle(int x1, int y1, int x2, int y2, TemporaryBrush brush);
	void FillRectangle(int x1, int y1, int x2, int y2, D2D1_COLOR_F &col);
	void FillRectangle(int x1, int y1, int x2, int y2, D2D1::ColorF::Enum);

	// Draw a rectangle with upper-left corner (x1,y1) with dimensions (w,h)
	void DrawRectangleWH(int x1, int y1, int w, int h, GenericBrush *brush = NULL);
	void DrawRectangleWH(int x1, int y1, int w, int h, ID2D1Brush *);
	void DrawRectangleWH(int x1, int y1, int w, int h, TemporaryBrush brush);
	void DrawRectangleWH(int x1, int y1, int w, int h, D2D1_COLOR_F &col);
	void DrawRectangleWH(int x1, int y1, int w, int h, D2D1::ColorF::Enum);
	void FillRectangleWH(int x1, int y1, int w, int h, GenericBrush *brush = NULL);
	void FillRectangleWH(int x1, int y1, int w, int h, ID2D1Brush *);
	void FillRectangleWH(int x1, int y1, int w, int h, TemporaryBrush brush);
	void FillRectangleWH(int x1, int y1, int w, int h, D2D1_COLOR_F &col);
	void FillRectangleWH(int x1, int y1, int w, int h, D2D1::ColorF::Enum);

	// Rounded rectangles (same parameters above, with two extras for the horizontal and vertical rounding radii)
	void DrawRoundedRectangle(int x1, int y1, int x2, int y2, int hr, int vr, GenericBrush *brush = NULL);
	void DrawRoundedRectangle(int x1, int y1, int x2, int y2, int hr, int vr, ID2D1Brush *);
	void DrawRoundedRectangle(int x1, int y1, int x2, int y2, int hr, int vr, TemporaryBrush brush);
	void DrawRoundedRectangle(int x1, int y1, int x2, int y2, int hr, int vr, D2D1_COLOR_F &col);
	void DrawRoundedRectangle(int x1, int y1, int x2, int y2, int hr, int vr, D2D1::ColorF::Enum);
	void FillRoundedRectangle(int x1, int y1, int x2, int y2, int hr, int vr, GenericBrush *brush = NULL);
	void FillRoundedRectangle(int x1, int y1, int x2, int y2, int hr, int vr, ID2D1Brush *);
	void FillRoundedRectangle(int x1, int y1, int x2, int y2, int hr, int vr, TemporaryBrush brush);
	void FillRoundedRectangle(int x1, int y1, int x2, int y2, int hr, int vr, D2D1_COLOR_F &col);
	void FillRoundedRectangle(int x1, int y1, int x2, int y2, int hr, int vr, D2D1::ColorF::Enum);

	void DrawRoundedRectangleWH(int x1, int y1, int w, int h, int hr, int vr, GenericBrush *brush = NULL);
	void DrawRoundedRectangleWH(int x1, int y1, int w, int h, int hr, int vr, ID2D1Brush *);
	void DrawRoundedRectangleWH(int x1, int y1, int w, int h, int hr, int vr, TemporaryBrush brush);
	void DrawRoundedRectangleWH(int x1, int y1, int w, int h, int hr, int vr, D2D1_COLOR_F &col);
	void DrawRoundedRectangleWH(int x1, int y1, int w, int h, int hr, int vr, D2D1::ColorF::Enum);
	void FillRoundedRectangleWH(int x1, int y1, int w, int h, int hr, int vr, GenericBrush *brush = NULL);
	void FillRoundedRectangleWH(int x1, int y1, int w, int h, int hr, int vr, ID2D1Brush *);
	void FillRoundedRectangleWH(int x1, int y1, int w, int h, int hr, int vr, TemporaryBrush brush);
	void FillRoundedRectangleWH(int x1, int y1, int w, int h, int hr, int vr, D2D1_COLOR_F &col);
	void FillRoundedRectangleWH(int x1, int y1, int w, int h, int hr, int vr, D2D1::ColorF::Enum);

	// Draw text
	void Text(int x, int y, string text, WCHAR *font, FLOAT size, GenericBrush *brush = NULL, DWRITE_TEXT_ALIGNMENT alignment = DWRITE_TEXT_ALIGNMENT_LEADING,
		DWRITE_FONT_WEIGHT weight = DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE style = DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH stretch = DWRITE_FONT_STRETCH_NORMAL, int w = -1, int h = -1);

	void Text(int x, int y, string text, WCHAR *font, FLOAT size, ID2D1Brush *brush, DWRITE_TEXT_ALIGNMENT alignment = DWRITE_TEXT_ALIGNMENT_LEADING,
		DWRITE_FONT_WEIGHT weight = DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE style = DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH stretch = DWRITE_FONT_STRETCH_NORMAL, int w = -1, int h = -1);

	void Text(int x, int y, string text, WCHAR *font, FLOAT size, TemporaryBrush brush, DWRITE_TEXT_ALIGNMENT alignment = DWRITE_TEXT_ALIGNMENT_LEADING,
		DWRITE_FONT_WEIGHT weight = DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE style = DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH stretch = DWRITE_FONT_STRETCH_NORMAL, int w = -1, int h = -1);

	void Text(int x, int y, string text, WCHAR *font, FLOAT size, D2D1_COLOR_F &col, DWRITE_TEXT_ALIGNMENT alignment = DWRITE_TEXT_ALIGNMENT_LEADING,
		DWRITE_FONT_WEIGHT weight = DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE style = DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH stretch = DWRITE_FONT_STRETCH_NORMAL, int w = -1, int h = -1);

	void Text(int x, int y, string text, WCHAR *font, FLOAT size, D2D1::ColorF::Enum col, DWRITE_TEXT_ALIGNMENT alignment = DWRITE_TEXT_ALIGNMENT_LEADING,
		DWRITE_FONT_WEIGHT weight = DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE style = DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH stretch = DWRITE_FONT_STRETCH_NORMAL, int w = -1, int h = -1);

	void Text(int x, int y, string text, GenericBrush *brush = NULL, TextFormat format = NULL, int w = -1, int h = -1);
	void Text(int x, int y, string text, ID2D1Brush *brush, TextFormat format = NULL, int w = -1, int h = -1);
	void Text(int x, int y, string text, TemporaryBrush brush, TextFormat format = NULL, int w = -1, int h = -1);
	void Text(int x, int y, string text, D2D1_COLOR_F &col, TextFormat format = NULL, int w = -1, int h = -1);
	void Text(int x, int y, string text, D2D1::ColorF::Enum col, TextFormat format = NULL,int w = -1, int h = -1);

	// Math and animation functions

	// Return the amount of time elapsed during the last frame in seconds
	float GetLastFrameTime() const;

	// Animation: calculate how many pixels to move an object on the current frame such that it
	// moves a total of x pixels per second
	float LinearMovement(float x) const;

	// Generate a random number (integer)
	static int Random(int min, int max);

	// Generate a random number (float)
	static float Random(float min, float max);

	// Clamp a value between two values (min(max(v, a1), a2))
	template <typename T>
	T Clamp(T const &v, T const &a1, T const &a2) const
	{
		return (v < a1? a1 : (v > a2? a2 : v));
	}

	// Public fields

	// Storage path
	string DataPath;

	// Top-left co-ordinate of render target relative to application window
	int RenderTargetX, RenderTargetY;

	// Scaled/stretched size of render target as shown on the user's display
	int RenderTargetW, RenderTargetH;

	// Resolution of actual render target surface in pixels
	int ResolutionX, ResolutionY;

	// Size of application client area in pixels (might not be the same as the render target)
	int ClientW, ClientH;

	// User-defined minimum window size
	int MinWindowSizeX, MinWindowSizeY;

	// Current (default) brush (used in drawing functions when no specific brush is given in the parameters)
	GenericBrush *CurrentBrush;

	// Current (default) text format (used in text functions when no specific text format is given in the parameters)
	TextFormat CurrentTextFormat;

	// The overlay scene
	std::unique_ptr<Scene> Overlay;

	// The render target (window surface). You can use this to access features of Direct2D
	// not provided by Simple2D.
	ID2D1DeviceContext    *Screen;		// Direct2D 1.1

	// ========================================================
	// For Direct2D 1.0
	// ========================================================

	// Direct2D factory access (remove '1' for Direct2D 1.0)
	ID2D1Factory1 *Direct2D;

	// DirectWrite factory access
    IDWriteFactory1 *TextFactory;

	// Windows Imaging Component factory access
	IWICImagingFactory2 *ImageFactory;

	// ========================================================
	// Also required for Direct2D 1.1
	// ========================================================

	// Direct3D device
	ID3D11Device1 *Direct3D;

	// Direct3D device context
	ID3D11DeviceContext1 *Screen3D;

	// Direct2D device
	ID2D1Device *Direct2DDevice;

	// DXGI swap chain (currently active, points to either the windowed or full-screen swap chain - no reference count held)
	IDXGISwapChain1 *DXGISwapChain;

	IDXGISwapChain1 *DXGISwapChainWindowed;
	IDXGISwapChain1 *DXGISwapChainFullScreen;

	// Direct2D target rendering bitmap
	// (linked to DXGI back buffer which is linked to Direct3D pipeline)
	ID2D1Bitmap1 *Direct2DBackBuffer;

	// Direct3D render target view
	ID3D11RenderTargetView *RenderTargetView;

	// Direct3D depth/stencil view
	ID3D11DepthStencilView *DepthStencilView;

protected:
	// Name of the application window
	wchar_t *WindowName;

	// Enables/disables clearing the surface to the background colour between each frame
	bool EnableClear;

	// Enable full-screen switch via Alt+Enter
	bool EnableFullScreen;

	// Start in full-screen
	bool FullScreen;

	// Enable full-screen display mode switching
	bool EnableModeSwitch;

	// Background colour
	D2D1_COLOR_F BackgroundColour;

	// Allow window to be re-sized
	bool ResizableWindow;

	// Enable showing the FPS counter in the window title
	bool ShowFps;

	// Performance count of when physics objects were last updated (used for animation calculations)
	__int64 LastUpdateTime64;
	__int64 PerformanceFrequency;

	// Performance counter counts between the last two frames
	__int64 LastFrameTime;

	// DirectX / DXGI
	
	// Enable Direct3D code paths (also causes BeginDraw() and EndDraw() NOT to be called automatically)
	bool Enable3D;

	// Enable 4x MSAA
	bool EnableMSAA;

// ==================================================================================
// Optional functions you can define in your own class
// ==================================================================================
protected:
	// Creates all the resources not dependent on the render target once at the
	// start of your application (when Run() is called)
	virtual bool SetupInitialResources() { return true; }

	// Initialize your program once at the start of your application, after
	// SetupInitialResources() and SetupResources() have run
	virtual void SetupApplication() {}

	// Creates all the graphics resources (like paintbrushes) for your application.
	// that are dependent on the render target. This is called every time the
	// render target needs to be re-built so don't setup one-time initialization
	// resources here
	// Return true on success, false on failure
	virtual bool SetupResources() { return true; }

	// Releases the resources created in SetupResources.
	// NOTE: If you use built-in Simple2D functions like MakeBrush, releasing the
	// resources is done for you automatically and you don't need to use this
	virtual void ReleaseResources() {}

	// Release the resources created in SetupInitialResources
	// NOTE: If you use built-in Simple2D functions like MakeTextFormat, releasing the
	// resources is done for you automatically and you don't need to use this
	virtual void ReleaseInitialResources() {}

	// Draws the current scene
	virtual void DrawScene() {}

	// Updates any animation or physics objects in your scene between each frame
	// If you are not using animation, you don't need to use this
	virtual void UpdateObjects() {}

	// Respond to keyboard input (character pressed)
	virtual bool OnKeyCharacter(int, int, bool, bool) { return false; }
	
	// Respond to keyboard input (key down)
	virtual bool OnKeyDown(int, int, bool) { return false; }

	// Respond to keyboard input (key up)
	virtual bool OnKeyUp(int, int) { return false; }

	// Respond to mouse input
	virtual bool OnMouseMove(int x, int y, WPARAM) { return false; }
	virtual bool OnMouseButton(UINT button, int x, int y, WPARAM) { return false; }

	// Respond to render target resizing
	// You don't need to manage resources here, this notification callback will be
	// invoked after the render target has been resized and resources re-created.
	// Only called when ResizeBehaviour == RB_Resize
	virtual void OnResize(int width, int height) {}

	// Extension to Windows message loop
	virtual bool OnWindowsMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) { return false; }

// ==================================================================================
// This section should be ignored
// ==================================================================================
private:
	// Register the window class and call methods for instantiating drawing resources
	HRESULT Initialize(string windowName, int iconResource);

	// Process and dispatch messages
	void RunMessageLoop();

	// Initialize device-independent resources
	HRESULT CreateDeviceIndependentResources();

	// Initialize device-dependent resources
	HRESULT CreateDeviceResources();

	// Initialize Direct3D device-dependent resources (for 3D applications only)
	HRESULT CreateDirect3DResources();

	// Release device-dependent resource
	void DiscardDeviceResources();

	// Release Direct3D device-dependent resources
	void DiscardDirect3DResources();

	// Draw content
	HRESULT onRender();

	// Resize the render target
	void OnWindowResize(UINT width, UINT height);

	// The windows procedure
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	// Direct2D
	HWND m_hwnd_app;
	HWND m_hwnd_rt_windowed;
	HWND m_hwnd_rt_fullscreen;
	bool killbit;
	static bool hasFocus;
	int vSyncClamp;
	ResizeBehaviour resizeBehaviour;

	// DirectX / DXGI

	// Minimum feature level the graphics card must support
	D3D_FEATURE_LEVEL minFeatureLevel;

	// Direct3D depth/stencil buffer
	ID3D11Texture2D *depthStencilBuffer;

	// Direct2D interface objects
	
	// Map of objects
	typedef boost::ptr_map<size_t, RenderingObject> renderingObjectMap;
	renderingObjectMap RenderingObjects;

	// Where in the rendering chain to render the scene (before or after user's Draw())
	bool renderSceneAfter;

	// Path object currently being created
	Geometry pathToCreate;

	// The active scene
	Scene *scene;

	// The interface element with focus
	InterfaceObject *focusObject;

	// Random number generator
	static std::mt19937 RandomEngine;

	// Add or re-use a rendering object in the map
	// Note this is just for re-use, not memory management
	template <typename T>
	T *addRenderingObject(T *resource, bool managed = true)
	{
		if (managed)
		{
			auto i = RenderingObjects.find(resource->GetHash());
			if (i != RenderingObjects.end())
			{
				delete resource;
				return dynamic_cast<T *>(i->second);
			}

			RenderingObjects.insert(resource->GetHash(), resource);
		}
		return resource;
	}
	
	// FPS monitor
	void ShowFPS(int);
};

// Types required by scene and interface objects
typedef std::function<bool (int, int, bool, bool)> Simple2DKeyCharFunc;
typedef std::function<bool (int, int, bool)> Simple2DKeyDownFunc;
typedef std::function<bool (int, int)> Simple2DKeyUpFunc;
typedef std::function<bool (int, int, WPARAM)> Simple2DMouseMoveFunc;
typedef std::function<bool (UINT, int, int, WPARAM)> Simple2DMouseButtonFunc;
typedef std::function<void (void)> Simple2DMouseHoverFunc;

typedef std::function<void (void)> Simple2DVoidCallback;

// ==================================================================================
// Scene management
// ==================================================================================

namespace S2DScene
{

// List of possible animation binding variables
typedef enum { BindNone, BindU, BindV, BindX, BindY, BindZ, BindWidth, BindHeight, BindR, BindG, BindB, BindAlpha, BindAlpha2, BindSize, BindRotation, BindRotX, BindRotY, BindRotZ, BindX2, BindY2, BindWidth2, BindHeight2, BindBaseX, BindBaseY } SceneObjectBindings;
typedef enum { SODB_Destroy, SODB_Release } SceneObjectDeleteBehaviour;

// Single scene object
class SceneObject
{
private:
	// The scene or object group which owns this object
	ObjectManager *owner;

	// Map of binding variables to animations, if any (set by the user)
	std::map<SceneObjectBindings, AnimationChain> animations;

	// The master animation (when this one ends, the object will be removed from the scene or disabled automatically)
	SceneObjectBindings masterAnimation;

	// The behaviour for this object when it is to be removed from the scene
	SceneObjectDeleteBehaviour deleteBehaviour;

	// Set to true if the object is marked for removal from the scene / to be disabled
	bool done;

protected:
	// Application pointer
	Simple2D *renderer;

	// Cannot instantiate object directly
	SceneObject(Simple2D *r = nullptr)
		: renderer(Simple2D::App), owner(nullptr), masterAnimation(BindNone), done(false), visible(true), deleteBehaviour(SODB_Destroy) {}

	// Set to false to prevent the object being rendered
	bool visible;

	// Get bindings
	virtual double *getBinding(SceneObjectBindings b) { return (double *)0; }

	// Optional overrides for when the object is made visible or invisible
	virtual void doOn() {}
	virtual void doOff() {}

	// Optional override for default update function
	virtual void doUpdate() {}

	// Optional override for drawing function
	virtual void doDraw() {}

public:
	// Bind a variable to an animation, optionally setting it to the master animation
	void Bind(SceneObjectBindings bindname, AnimationChain &anim, bool master = false);
	void Bind(SceneObjectBindings bindname, Animation &anim, bool master = false);
	void Unbind(SceneObjectBindings bindname);

	// Get/set the absolute value of a bound variable
	double &operator[](SceneObjectBindings bindname);

	// Set master animation
	void SetMasterAnimation(SceneObjectBindings bindname = BindNone);

	// Set done behaviour
	void SetDeleteBehaviour(SceneObjectDeleteBehaviour sodb);

	// Get done behaviour
	SceneObjectDeleteBehaviour GetDeleteBehaviour();

	// Set object owner
	void SetOwner(ObjectManager *parent = nullptr);
	void SetOwner(ObjectManager &parent);

	// Get object owner
	ObjectManager *GetOwner();

	// Reset all animations to their starting points
	virtual void ResetAnimations();

	// Drawing function
	void Draw();

	// Update function
	void Update();

	// Set visible/invisible
	void On();
	void Off();

	// Return true if the object is currently visible
	bool IsOn();

	// Return true if object is marked for removal from the scene / to be disabled
	bool Done();

	// Set the object as done (for removal)
	void SetDone() { done = true; }

	// Clone
	virtual SceneObject *clone() const = 0;
};

// Scene Object: Rectangle
class Rectangle : public SceneObject
{
private:
	double x, y, w, h;
	double alpha;
	GenericBrush *brush;

protected:
	double *getBinding(SceneObjectBindings b) override;
	void doDraw() override;

public:
#ifdef SIMPLE2D_COMPAT_107
	Rectangle(Simple2D *r, int x, int y, int w, int h, GenericBrush *b);
#endif
	Rectangle(int x, int y, int w, int h, GenericBrush *b);

	virtual Rectangle *clone() const override { return new Rectangle(*this); }
};

// Scene object: Text
class Label : public SceneObject
{
private:
	double x, y;
	double alpha;
	int w, h;
	string text;
	GenericBrush *brush;
	TextFormat format;

protected:
	double *getBinding(SceneObjectBindings b) override;
	void doDraw() override;

public:
#ifdef SIMPLE2D_COMPAT_107
	Label(Simple2D *r, int x, int y, string text, GenericBrush *b, TextFormat fmt, int w = -1, int h = -1);
#endif
	Label(int x, int y, string text, GenericBrush *b, TextFormat fmt, int w = -1, int h = -1);

	// Set label text
	void SetText(string text) { this->text = text; }

	virtual Label *clone() const override { return new Label(*this); }
};

// Scene Object: Image
class Sprite : public SceneObject
{
protected:
	double x, y, w, h;
	double sx, sy, sw, sh;
	double alpha, rotation;

	ImageObject *image;

protected:
	virtual double *getBinding(SceneObjectBindings b) override;
	virtual void doDraw() override;

public:
#ifdef SIMPLE2D_COMPAT_107
	Sprite(Simple2D *r, ImageObject *img, int x = 0, int y = 0, int w = -1, int h = -1);
#endif
	Sprite(ImageObject *img, int x = 0, int y = 0, int w = -1, int h = -1);

	// Set render rectangle
	void SetRectWH(int x, int y, int w, int h);

	// Set source rectangle
	void SetSourceRectWH(int x, int y, int w, int h);

	// Clone
	virtual Sprite *clone() const override { return new Sprite(*this); }
};

// Scene object: animated sprite sheet
class SpriteSheet : public Sprite
{
private:
	int rows, cols;
	int selectedImage;
	int frames;

	bool useAnimation, animationChanged;
	AnimationChain spriteSelector;

protected:
	// Drawing function
	virtual void doDraw() override;

public:
#ifdef SIMPLE2D_COMPAT_107
	SpriteSheet(Simple2D *r, ImageObject *img, int col = 1, int row = 1, int item = 0);
#endif
	SpriteSheet(ImageObject *img, int col = 1, int row = 1, int item = 0, int frames = -1);

	// Set sprite to plot
	void SetSprite(int s);

	// Select to plot sprite based on animation result
	void SetSpriteFromAnimation(bool use = true);

	// Set animation function to control which sprite is used from the sheet
	// Returns values from 0.0 to 1.0 which are scaled to the available number of sprites
	void SetAnimation(AnimationChain &a);
	void SetAnimation(Animation &a, Animation::CycleType ct = Animation::Clamp);

	// Clone
	virtual SpriteSheet *clone() const override { return new SpriteSheet(*this); }
};

// Scene object: Custom items. Cannot bind any animations to this, it is for rendering a group of static items simply.
class CustomDraw : public SceneObject
{
private:
	// User-defined drawing function for an instance of the object
	Simple2DVoidCallback onDraw;

public:
	// Constructors/clone
#ifdef SIMPLE2D_COMPAT_107
	CustomDraw(Simple2D *r, Simple2DVoidCallback drawFunc) : SceneObject(), onDraw(drawFunc) {}
#endif
	CustomDraw(Simple2DVoidCallback drawFunc) : SceneObject(), onDraw(drawFunc) {}

protected:
	// Call user drawing function
	void doDraw() override { onDraw(); }

public:
	// Clone
	virtual CustomDraw *clone() const override { return new CustomDraw(*this); }
};
	
// ==================================================================================
// Base class for user interface objects
// ==================================================================================

// WM_COMMAND accelerators
enum InterfaceObjectWindowsCommands { IOWC_CUT = 1, IOWC_COPY, IOWC_PASTE };

// Scene object: Generic user interface object
// (can be used for callback-only behaviour, but should prefer Scene's callbacks unless you need to capture within a bounding box)
class InterfaceObject : public SceneObject
{
private:
	// User-defined functions to call on specific mouse and keyboard events
	Simple2DMouseMoveFunc moveCallback;
	Simple2DMouseHoverFunc hoverCallback;
	Simple2DMouseHoverFunc unhoverCallback;
	Simple2DMouseButtonFunc buttonCallback;
	Simple2DKeyCharFunc keyCharCallback;
	Simple2DKeyDownFunc keyDownCallback;
	Simple2DKeyUpFunc keyUpCallback;

protected:
	// True if the mouse is currently over the object (only updated if the object is enabled)
	bool mouseOver;

	// True if the control just got the input focus (we wait for the user to release all keys before allowing input)
	bool justGotFocus;

	// Interface
	void doOn() override;
	void doOff() override;

	virtual bool doOnKeyCharacter(int, int, bool, bool) { return false; }
	virtual bool doOnKeyDown(int, int, bool) { return false; }
	virtual bool doOnKeyUp(int, int) { return false; }
	virtual bool doOnMouseMove(int, int, WPARAM) { return false; }
	virtual bool doOnMouseButton(UINT, int, int, WPARAM) { return false; }
	virtual void doOnMouseHover(int, int, WPARAM) {}
	virtual void doOnMouseUnhover(int, int, WPARAM) {}
	virtual bool doOnWindowsMessage(HWND, UINT, WPARAM,LPARAM) { return false; }

public:
	// Bounding box of object
	int x;
	int y;
	int w;
	int h;

	// Interface
	bool OnKeyCharacter(int key, int rc, bool prev, bool trans);
	bool OnKeyDown(int key, int rc, bool prev);
	bool OnKeyUp(int key, int rc);
	bool OnMouseMove(int x, int y, WPARAM keys);
	bool OnMouseButton(UINT button, int x, int y, WPARAM keys);
	bool OnWindowsMessage(HWND, UINT, WPARAM, LPARAM);

	// Template interface
	int GetWidth() { return w; }
	int GetHeight() { return h; }

	// Give and remove input focus
	void SetFocus(bool focus = true);

	// True if the control has the input focus
	bool HasFocus();

	// Legacy constructors (for B/C with <=1.07)
#ifdef SIMPLE2D_COMPAT_107
	InterfaceObject(Simple2D *r, int x, int y, int w, int h, Simple2DMouseButtonFunc bf);
	InterfaceObject(Simple2D *r, int x, int y, int w, int h, Simple2DMouseHoverFunc hf, Simple2DMouseHoverFunc uhf, Simple2DMouseButtonFunc bf);
	InterfaceObject(Simple2D *r, Simple2DKeyCharFunc cf, Simple2DKeyDownFunc df = NULL, Simple2DKeyUpFunc uf = NULL);
#endif

	// Constructors
	InterfaceObject(int x = 0, int y = 0, int w = 0, int h = 0);
	InterfaceObject(int x, int y, int w, int h, Simple2DMouseButtonFunc bf);
	InterfaceObject(int x, int y, int w, int h, Simple2DMouseHoverFunc hf, Simple2DMouseHoverFunc uhf, Simple2DMouseButtonFunc bf);
	InterfaceObject(Simple2DKeyCharFunc cf, Simple2DKeyDownFunc df = NULL, Simple2DKeyUpFunc uf = NULL);

	InterfaceObject(int x, int y, int w, int h,
							 Simple2DMouseHoverFunc hf, Simple2DMouseHoverFunc uhf, Simple2DMouseButtonFunc bf,
							 Simple2DKeyCharFunc cf, Simple2DKeyDownFunc df, Simple2DKeyUpFunc uf);
public:
	// Clone
	virtual InterfaceObject *clone() const override { return new InterfaceObject(*this); }
};

// ==================================================================================
// Interface for tabbable objects
// ==================================================================================

class ITabbedItem
{
public:
	virtual ~ITabbedItem() {}
};

class ITabbedObjectGroup
{
public:
	virtual ~ITabbedObjectGroup() {}

	virtual ITabbedItem *FindTabbedItem(bool forwards = true) = 0;
};

// ==================================================================================
// Text input control class
// ==================================================================================

// NOTE: No memory management is needed

enum TextBoxCaptionPosition { TBCP_Left, TBCP_Above };

enum TextBoxStateAction { TBSA_Add, TBSA_DeleteBack, TBSA_DeleteForward };
struct TextBoxState
{
	TextBoxStateAction thisAction;
	bool seal;

	unsigned int charStart;
	unsigned int caret;
	string text;
};

class TextBox : public InterfaceObject, public ITabbedItem
{
protected:
	// Text format for control
	TextFormat format;

	// Text layout for control (generated and destroyed on the fly to calculate character widths)
	TextLayout layout;

	// Brush to paint text with
	GenericBrush *textBrush;

	// Brush to paint caret with
	GenericBrush *caretBrush;

	// Brush to paint selected text with
	GenericBrush *selTextBrush;

	// Brush to paint background of selected text with
	GenericBrush *selBoxBrush;

	// Offset to add to caret y co-ordinate
	int caretYoffset;

	// Offset to subtract from caret height
	int caretYheightOffset;

	// The maximum number of characters that can be entered
	size_t maxlen;

	// Canvas X and Y expansion from control bounding box
	int canvasXoffset, canvasYoffset;

	// Canvas X and Y rounded corner radii
	int canvasXradius, canvasYradius;

	// Canvas edge brush
	GenericBrush *canvasOutlineBrush;

	// Canvas fill brush
	GenericBrush *canvasFillBrush;

	// Caption brush
	GenericBrush *captionBrush;

	// Caption text format
	TextFormat captionFormat;

	// Caption location
	TextBoxCaptionPosition captionPosition;

	// Caption pixel gap between text box
	int captionGap;

private:
	// Set if CTRL is currently held down
	bool ctrlPressed;

	// Set if SHIFT is currently held down
	bool shiftPressed;

	// The character to render the text from if it's too big to fit in the box (bounded by the caret)
	unsigned int charStart;

	// Selection point (the other end of the selection is the caret position)
	// or string::npos for no selection point
	unsigned int selectionPoint;

	// Delete the current selection if there is one and place the caret appropriately
	void deleteSelection();

	// The undo history
	std::list<TextBoxState> undoHistory;

	typedef std::list<TextBoxState>::iterator UndoIterator;

	// How far into the undo history we are
	// (points to the last undone item, or the end iterator
	// if the undo history is empty or nothing has been undone)
	UndoIterator undoPos;

	// Add (or potentially modify) a text event to the undo buffer
	void push(TextBoxStateAction action, bool forceSeal = false);

	// Undo the last action if possible
	void undo();

	// Redo the last undone action and move forward in the history
	void redo();

protected:
	// Interface
	virtual void doOn() override;
	virtual void doOff() override;
	virtual void doDraw() override;
	virtual bool doOnKeyUp(int key, int rc) override;
	virtual bool doOnKeyDown(int key, int rc, bool prev) override;
	virtual bool doOnKeyCharacter(int key, int rc, bool prev, bool trans) override;
	virtual bool doOnMouseButton(UINT, int, int, WPARAM) override;
	virtual bool doOnWindowsMessage(HWND, UINT, WPARAM, LPARAM) override;

public:
	// The field caption
	string Caption;

	// The string currently in the text field
	string Text;

	// The position of the caret (in characters) from the start of the text
	unsigned int Caret;

	// Set to true to hide the displayed text
	bool IsPassword;

	// True if the user just pressed Enter
	bool ReturnPressed;

	// Constructors
	TextBox() {}
#ifdef SIMPLE2D_COMPAT_107
	TextBox(Simple2D *r) : ReturnPressed(false), InterfaceObject() {}
#endif
	TextBox(int x, int y, int w, int h, int l, string initialText, TextFormat fmt,
		GenericBrush *tb, GenericBrush *caret, int cYo, int cYho, Simple2D *r = nullptr)
		: InterfaceObject(x, y, w, h),
		Text(initialText), Caret(initialText.length()), format(fmt),
		textBrush(tb), caretBrush(caret), caretYoffset(cYo), caretYheightOffset(cYho),
		selTextBrush(nullptr), selBoxBrush(nullptr),
		maxlen(l), ReturnPressed(false), IsPassword(false),
		Caption(""), captionBrush(nullptr), captionFormat(nullptr), captionGap(0), captionPosition(TBCP_Left),
		shiftPressed(false), ctrlPressed(false), selectionPoint(string::npos), charStart(0),
		canvasXoffset(0), canvasYoffset(0), canvasXradius(0), canvasYradius(0),
		canvasOutlineBrush(nullptr), canvasFillBrush(nullptr)
	{}

	// Set canvas settings
	void SetCanvas(int xo, int yo, int xr, int yr, GenericBrush *ob, GenericBrush *fb);

	// Set caption settings
	void SetCaption(string text, TextFormat format, GenericBrush *brush, TextBoxCaptionPosition position = TBCP_Left, int gap = 0);

	// Set selection area brushes
	void SetSelectionBrushes(GenericBrush *stb, GenericBrush *sbb);

	// Get the text in various formats
	const char *TextAsChar() { return Text.c_str(); }
	int TextAsInt()          { return static_cast<int>(atoi(TextAsChar()));    }
	float TextAsFloat()      { return static_cast<float>(atof(TextAsChar()));  }
	double TextAsDouble()    { return static_cast<double>(atof(TextAsChar())); }

public:
	// Clone
	virtual TextBox *clone() const override { return new TextBox(*this); }
};

// ==================================================================================
// Mouse-activated button class
// ==================================================================================

// NOTE: No memory management is needed

class Button : public InterfaceObject
{
private:
	// The button text
	string text;

	// True if the button responds to hovers and mouse clicks, false for rendering only
	bool active;

	// Brushes for when the button is hovered or unhovered
	GenericBrush *noHoverBrush;
	GenericBrush *hoverBrush;

	// Current brush
	GenericBrush *buttonBrush;

	// Brush for text
	GenericBrush *textBrush;

	// Text font format, size etc.
	TextFormat textFormat;
	DWRITE_TEXT_METRICS metrics;

	// User-defined function to call when the button is left-clicked
	std::function<void (Button &)> onClick;

protected:
	virtual void doDraw() override;
	virtual void doOnMouseUnhover(int, int, WPARAM) override;
	virtual void doOnMouseHover(int, int, WPARAM) override;
	virtual bool doOnMouseButton(UINT button, int, int, WPARAM) override;

public:
	// The radii of the button corners (rounded rectangle)
	int rx;
	int ry;

	// Constructors
	Button() : onClick(nullptr) {}
	Button(int x, int y, int w, int h, int rx, int ry, string text, GenericBrush *nhb, GenericBrush *hb,
			TextFormat fmt, GenericBrush *tb,
			std::function<void (Button &)> clickFunc, bool active = true)
			: rx(rx), ry(ry), noHoverBrush(nhb), hoverBrush(hb), textFormat(fmt), textBrush(tb), buttonBrush(nhb), onClick(clickFunc),
			  active(active), InterfaceObject(x, y, w, h)
	{
		buttonBrush = noHoverBrush;
		SetText(text);
	}

	// Legacy constructors
#ifdef SIMPLE2D_COMPAT_107
	Button(Simple2D *r, int x, int y, int w, int h, int rx, int ry, string text, GenericBrush *nhb, GenericBrush *hb,
			TextFormat fmt, GenericBrush *tb,
			std::function<void (Button &)> clickFunc, bool active = true)
			: rx(rx), ry(ry), noHoverBrush(nhb), hoverBrush(hb), textFormat(fmt), textBrush(tb), buttonBrush(nhb), onClick(clickFunc),
			  active(active), InterfaceObject(x, y, w, h)
	{
		buttonBrush = noHoverBrush;
		SetText(text);
	}
#endif

	// Set the button text
	void SetText(string);

	// Change the active flag (the button will be rendered if inactive but will not respond to mouse events)
	void SetActive(bool a) { active = a; }

	// Change the active brush temporarily (until next mouse event)
	void SetBrush(GenericBrush *b) { buttonBrush = b; }

public:
	// Clone
	virtual Button *clone() const override { return new Button(*this); }
};

// ==================================================================================
// Slider class
// ==================================================================================

// NOTE: No memory management is needed

class Slider : public InterfaceObject
{
private:
	// The slider value (user specified lower and upper limits)
	int value;
	int valueMin;
	int valueMax;

	// Brushes for the bar and the slider
	GenericBrush *barBrush;
	GenericBrush *sliderBrush;

	// User-defined function to call when the slider value is changed (via click or drag)
	std::function<void (Slider &)> onChange;

	// Update slider value on mouse click/drag
	void update(int x);

protected:
	// Interface
	virtual void doDraw() override;
	virtual bool doOnMouseMove(int, int, WPARAM) override;
	virtual bool doOnMouseButton(UINT button, int, int, WPARAM) override;

public:
	// Constructors
	Slider() {}

#ifdef SIMPLE2D_COMPAT_107
	Slider(Simple2D *r, int x, int y, int w, int h, int startValue, int lower, int upper, GenericBrush *bb, GenericBrush *sb,
			std::function<void (Slider &)> changeFunc)
			: InterfaceObject(x, y, w, h), barBrush(bb), sliderBrush(sb),
			  value(startValue), valueMin(lower), valueMax(upper), onChange(changeFunc) {}
#endif
	Slider(int x, int y, int w, int h, int startValue, int lower, int upper, GenericBrush *bb, GenericBrush *sb,
			std::function<void (Slider &)> changeFunc)
			: InterfaceObject(x, y, w, h), barBrush(bb), sliderBrush(sb),
			  value(startValue), valueMin(lower), valueMax(upper), onChange(changeFunc) {}

	// Get slider value
	int GetValue();

	// Set slider value
	int SetValue(int v);

public:
	// Clone
	virtual Slider *clone() const override { return new Slider(*this); }
};

// ==================================================================================
// Checkbox class
// ==================================================================================

// NOTE: No memory management is needed

class CheckBox : public InterfaceObject
{
private:
	// Check box set/clear state
	bool checked;

	// Brushes for the check box border and check mark
	GenericBrush *boxBrush;
	GenericBrush *tickBrush;

	// The annotation text
	string text;
	
	// Brush for annotation text
	GenericBrush *textBrush;

	// Text format
	TextFormat textFormat;
	DWRITE_TEXT_METRICS metrics;

	// User-defined function for when the check box state changes
	std::function<void (CheckBox &)> onChange;

protected:
	// Interface
	virtual void doDraw() override;
	virtual bool doOnMouseButton(UINT, int, int, WPARAM) override;

public:
	// Constructors
	CheckBox() {}
	CheckBox(int x, int y, int size, bool state, string text, TextFormat format,
		GenericBrush *bb, GenericBrush *tb, GenericBrush *xb,
		std::function<void (CheckBox &)> changeFunc = NULL)
		: InterfaceObject(x, y, size, size), checked(state), text(text), textFormat(format),
		boxBrush(bb), tickBrush(tb), textBrush(xb), onChange(changeFunc)
	{
		TextLayout layout = renderer->MakeTextLayout(text, textFormat, renderer->ResolutionX, renderer->ResolutionY);
		layout->GetMetrics(&metrics);
	}

	// Get checkbox state
	bool IsChecked() { return checked; }

	// Set checkbox state
	void SetChecked(bool state) { checked = state; }

public:
	// Clone
	virtual CheckBox *clone() const override { return new CheckBox(*this); }
};

// ==================================================================================
// User interface item group - useful for managing interfaces
// ==================================================================================

// NOTE: No memory management is needed

struct TextBoxTemplate {
	string caption;
	string initialText;
	size_t maxLength;
	bool isPassword;

	TextBoxTemplate(string initialText = "", size_t maxLength = 100000, string caption = "", bool isPassword = false)
		: caption(caption), initialText(initialText), maxLength(maxLength), isPassword(isPassword) {}
};

struct TextBoxGroupTemplate {
	int width;
	int height;

	TextFormat textFormat;
	GenericBrush *textBrush;
	GenericBrush *caretBrush;
	GenericBrush *selTextBrush;
	GenericBrush *selBoxBrush;
	int caretYoffset;
	int caretYheightOffset;
	
	int canvasXoffset, canvasYoffset;
	int canvasXradius, canvasYradius;

	GenericBrush *canvasOutlineBrush;
	GenericBrush *canvasFillBrush;

	TextFormat captionFormat;
	GenericBrush *captionBrush;
	TextBoxCaptionPosition captionPosition;
	int captionGap;

	TextBoxGroupTemplate() {}
	TextBoxGroupTemplate(int w, int h, TextFormat fmt, GenericBrush *tb, GenericBrush *cb,
		GenericBrush *stb, GenericBrush *sbb, int cYo, int cYho,
		int canvasXo, int canvasYo, int canvasXr, int canvasYr, GenericBrush *cob, GenericBrush *cfb,
		TextFormat captionFormat, GenericBrush *captionBrush, TextBoxCaptionPosition position, int captionGap)
		: width(w), height(h), textFormat(fmt), textBrush(tb), caretBrush(cb),
		selTextBrush(stb), selBoxBrush(sbb),
		caretYoffset(cYo), caretYheightOffset(cYho),
		canvasXoffset(canvasXo), canvasYoffset(canvasYo), canvasXradius(canvasXr), canvasYradius(canvasYr),
		canvasOutlineBrush(cob), canvasFillBrush(cfb),
		captionFormat(captionFormat), captionBrush(captionBrush), captionPosition(position), captionGap(captionGap) {}

	TextBox *Generate(int x, int y, TextBoxTemplate tt)
	{
		TextBox *textBox = new TextBox(x, y, width, height, tt.maxLength, tt.initialText, textFormat, textBrush, caretBrush, caretYoffset, caretYheightOffset);
		textBox->SetCanvas(canvasXoffset, canvasYoffset, canvasXradius, canvasYradius, canvasOutlineBrush, canvasFillBrush);
		textBox->SetCaption(tt.caption, captionFormat, captionBrush, captionPosition, captionGap);
		textBox->SetSelectionBrushes(selTextBrush, selBoxBrush);
		textBox->IsPassword = tt.isPassword;
		return textBox;
	}
};

struct ButtonTemplate {
	string text;
	std::function<void (Button &)> onClick;

	ButtonTemplate(string text = "", std::function<void (Button &)> onClick = nullptr) : text(text), onClick(onClick) {}
};

struct ButtonGroupTemplate {
	bool active;

	int bw;
	int bh;
	int rx;
	int ry;

	GenericBrush *noHoverBrush;
	GenericBrush *hoverBrush;

	TextFormat textFormat;
	GenericBrush *textBrush;

	ButtonGroupTemplate() {}
	ButtonGroupTemplate(int bw, int bh, int rx, int ry, GenericBrush *nhb, GenericBrush *hb, TextFormat fmt, GenericBrush *tb, bool active = true)
		: bw(bw), bh(bh), rx(rx), ry(ry), noHoverBrush(nhb), hoverBrush(hb), textFormat(fmt), textBrush(tb), active(active) {}

	Button *Generate(int x, int y, ButtonTemplate bt)
	{
		return new Button(x, y, bw, bh, rx, ry, bt.text, noHoverBrush, hoverBrush, textFormat, textBrush, bt.onClick, active);
	}
};

template <typename InterfaceItem, typename InterfaceGroupTemplate, typename InterfaceItemTemplate>
class UserInterfaceItemGroup
{
	boost::ptr_vector<InterfaceItem> items;
	Simple2D *renderer;

public:
	UserInterfaceItemGroup() {}

	void AddItem(InterfaceItem &item);
	vector<InterfaceItem *> AddItemColumn(int x, int y, InterfaceGroupTemplate gt, InterfaceItemTemplate items[], int numItems, int pixelGap);
	vector<InterfaceItem *> AddItemColumn(int x, int y, InterfaceGroupTemplate gt, vector<InterfaceItemTemplate> items, int pixelGap);
	vector<InterfaceItem *> AddItemRow(int x, int y, InterfaceGroupTemplate gt, InterfaceItemTemplate items[], int numItems, int pixelGap);
	vector<InterfaceItem *> AddItemRow(int x, int y, InterfaceGroupTemplate gt, vector<InterfaceItemTemplate> items, int pixelGap);

	boost::ptr_vector<InterfaceItem> *MoveItems() { return items.release().release(); }
};

template <typename InterfaceItem, typename IGT, typename IIT>
void UserInterfaceItemGroup<InterfaceItem, IGT, IIT>::AddItem(InterfaceItem &item)
{
	items.push_back(&item);
}

template <typename II, typename IGT, typename IIT>
vector<II *> UserInterfaceItemGroup<II, IGT, IIT>::AddItemColumn(int x, int y, IGT gt, IIT items[], int numItems, int pixelGap)
{
	vector<II *> ids;
	ids.reserve(numItems);

	int yOffset = 0;

	for (int i = 0; i < numItems; i++)
	{
		II *o = gt.Generate(x, y + yOffset, items[i]);
		this->items.push_back(o);

		ids.push_back(o);

		yOffset += o->GetHeight() + pixelGap;
	}

	return ids;
}

template <typename II, typename IGT, typename IIT>
vector<II *> UserInterfaceItemGroup<II, IGT, IIT>::AddItemColumn(int x, int y, IGT gt, vector<IIT> items, int pixelGap)
{
	IIT *itemArray = new IIT[items.size()];
	std::copy(items.begin(), items.end(), itemArray);

	vector<II *> ids = AddItemColumn(x, y, gt, itemArray, items.size(), pixelGap);

	delete [] itemArray;

	return ids;
}

template <typename II, typename IGT, typename IIT>
vector<II *> UserInterfaceItemGroup<II, IGT, IIT>::AddItemRow(int x, int y, IGT gt, IIT items[], int numItems, int pixelGap)
{
	vector<II *> ids;
	ids.reserve(numItems);

	int xOffset = 0;

	for (int i = 0; i < numItems; i++)
	{
		II *o = gt.Generate(x + xOffset, y, items[i]);
		this->items.push_back(o);

		ids.push_back(o);

		xOffset += o->GetWidth() + pixelGap;
	}

	return ids;
}

template <typename II, typename IGT, typename IIT>
vector<II *> UserInterfaceItemGroup<II, IGT, IIT>::AddItemRow(int x, int y, IGT gt, vector<IIT> items, int pixelGap)
{
	IIT *itemArray = new IIT[items.size()];
	std::copy(items.begin(), items.end(), itemArray);

	vector<II *> ids = AddItemRow(x, y, gt, itemArray, items.size(), pixelGap);

	delete [] itemArray;

	return ids;
}

// ==================================================================================
// Scene object management
// ==================================================================================

class ObjectManager
{
protected:
	// Objects to manage
	boost::ptr_list<SceneObject> objects;

	// Instantiation only by derived classes
	ObjectManager() {}

	// Clone everything, preserving bindings
	ObjectManager(ObjectManager const &o)
	{
		for (auto it = o.objects.begin(); it != o.objects.end(); ++it)
		{
			auto cloned = it->clone();
			cloned->SetDeleteBehaviour(SODB_Destroy);
			cloned->SetOwner(this);

			objects.push_back(cloned);
		}
	}

	// Destruct the pointer list without freeing memory that shouldn't be freed
	~ObjectManager()
	{
		while (objects.size() > 0)
		{
			auto &obj = *objects.begin();

			auto *iobj = dynamic_cast<InterfaceObject *>(&obj);
			if (iobj)
				iobj->SetFocus(false);

			if (obj.GetDeleteBehaviour() == SODB_Destroy)
				objects.erase(objects.begin());
			else if (obj.GetDeleteBehaviour() == SODB_Release)
				objects.release(objects.begin()).release();
		}
	}

	// Dispatch mouse events to all child objects
	bool DispatchOnMouseMove(int, int, WPARAM);
	bool DispatchOnMouseButton(UINT, int, int, WPARAM);

public:
	// Add(): adds an object, cloning it if it's a reference, takes ownership of the object, destroys the object when removed from scene
	// AddRef(): adds an object (pointer or reference), does not clone, does not take ownership, does not destroy the object when removed from scene

	// Add an object - note, once the object is added, Scene takes over memory management (de-allocation)
	// so don't call delete on pointers to added objects
	// Note: this is only used for types of SceneObject * and derived types, it doesn't need to be a template but
	// we set it as one so that pointers do not get passed to the SceneObject & reference template below
	// (it does however eliminate the need for casts on eg. Sprite *i = static_cast<Sprite *>(scene.Add(new Sprite(...))); )
	template <typename SO>
	SO *Add(SO *obj)
	{
		typedef boost::remove_const<SO>::type T;

		obj->SetDeleteBehaviour(SODB_Destroy);
		obj->SetOwner(this);

		objects.push_back(const_cast<T *>(obj));
		return obj;
	}

	template <typename SO>
	SO *AddBehind(SO *obj)
	{
		typedef boost::remove_const<SO>::type T;

		obj->SetDeleteBehaviour(SODB_Destroy);
		obj->SetOwner(this);

		objects.push_front(const_cast<T *>(obj));
		return obj;
	}

	// Add an object that's an rvalue reference or other local temporary in the calling scope
	// Clones the object onto the heap and returns its pointer
	template <typename SO>
	SO *Add(SO &obj)
	{
		typedef boost::remove_const<SO>::type T;

		T *objClone = new T(obj);

		objClone->SetDeleteBehaviour(SODB_Destroy);
		objClone->SetOwner(this);

		objects.push_back(objClone);
		return objClone;
	}

	template <typename SO>
	SO *AddBehind(SO &obj)
	{
		typedef boost::remove_const<SO>::type T;

		T *objClone = new T(obj);
		objClone->SetDeleteBehaviour(SODB_Destroy);
		objClone->SetOwner(this);

		objects.push_front(objClone);
		return objClone;
	}

	// Add unmanaged objects (these won't be destroyed when removed from the scene)
	template <typename SO>
	SO *AddRef(SO *obj)
	{
		typedef boost::remove_const<SO>::type T;

		obj->SetDeleteBehaviour(SODB_Release);
		obj->SetOwner(this);

		objects.push_back(const_cast<T *>(obj));
		return obj;
	}

	template <typename SO>
	SO *AddBehindRef(SO *obj)
	{
		typedef boost::remove_const<SO>::type T;

		obj->SetDeleteBehaviour(SODB_Release);
		obj->SetOwner(this);

		objects.push_front(const_cast<T *>(obj));
		return obj;
	}

	template <typename SO>
	SO *AddRef(SO &obj)
	{
		typedef boost::remove_const<SO>::type T;

		obj.SetDeleteBehaviour(SODB_Release);
		obj.SetOwner(this);

		objects.push_back(const_cast<T *>(&obj));
		return &obj;
	}

	template <typename SO>
	SO *AddBehindRef(SO &obj)
	{
		typedef boost::remove_const<SO>::type T;

		obj.SetDeleteBehaviour(SODB_Release);
		obj.SetOwner(this);

		objects.push_front(const_cast<T *>(&obj));
		return &obj;
	}

	// Add a group of InterfaceObjects
	// WARNING: Ownership of the pointers is transferred from the group template object to the scene (>=1.08)
	template <typename II, typename IIT, typename IGT>
	vector<II *> Add(UserInterfaceItemGroup<II, IIT, IGT> *objGroup)
	{
		vector<II *> pointers;

		auto &objs = *objGroup->MoveItems();

		while (objs.size() > 0)
		{
			objects.transfer(objects.end(), objs.begin(), objs);
			auto it = objects.end();
			--it;

			auto *obj = dynamic_cast<II *>(&(*it));

			obj->SetDeleteBehaviour(SODB_Destroy);
			obj->SetOwner(this);

			pointers.push_back(obj);
		}
		return pointers;
	}

	template <typename II, typename IIT, typename IGT>
	vector<II *> Add(UserInterfaceItemGroup<II, IIT, IGT> &objGroup)
	{
		vector<II *> pointers;

		auto &objs = *objGroup.MoveItems();

		while (objs.size() > 0)
		{
			objects.transfer(objects.end(), objs.begin(), objs);
			auto it = objects.end();
			--it;

			auto *obj = dynamic_cast<II *>(&(*it));

			obj->SetDeleteBehaviour(SODB_Destroy);
			obj->SetOwner(this);

			pointers.push_back(obj);
		}
		return pointers;
	}

	// Add a custom drawing function (helper function)
	virtual CustomDraw *AddDrawing(Simple2DVoidCallback drawFunc)
	{
		return Add(new CustomDraw(drawFunc));
	}

	virtual CustomDraw *AddDrawingBehind(Simple2DVoidCallback drawFunc)
	{
		return AddBehind(new CustomDraw(drawFunc));
	}

	// Remove and optionally destroy an object
	void Remove(SceneObject &obj)
	{
		auto it = find_ptr(objects.begin(), objects.end(), &obj);

		if (it != objects.end())
		{
			auto obj = dynamic_cast<InterfaceObject *>(&(*it));
			if (obj)
				obj->SetFocus(false);

			if (it->GetDeleteBehaviour() == SODB_Destroy)
				objects.erase(it);
			else if (it->GetDeleteBehaviour() == SODB_Release)
				objects.release(it).release()->SetOwner();
		}
	}

	void Remove(SceneObject *obj)
	{
		auto it = find_ptr(objects.begin(), objects.end(), obj);

		if (it != objects.end())
		{
			auto obj = dynamic_cast<InterfaceObject *>(&(*it));
			if (obj)
				obj->SetFocus(false);

			if (it->GetDeleteBehaviour() == SODB_Destroy)
				objects.erase(it);
			else if (it->GetDeleteBehaviour() == SODB_Release)
				objects.release(it).release()->SetOwner();
		}
	}

	// Return the objects in the collection
	boost::ptr_list<SceneObject> *GetObjects()
	{
		return &objects;
	}
};

// ==================================================================================
// Object Group (a group of objects that are handled as one)
// ==================================================================================

class ObjectGroup : public SceneObject, public ObjectManager, public ITabbedObjectGroup
{
private:
	// Upper left corner co-ordinates relative to which the objects will be rendered
	double baseX, baseY;

	// Relative position (animated) to left-hand corner
	double x, y;

	// Opacity of the objects
	double alpha;

protected:
	double *getBinding(SceneObjectBindings b) override;

	virtual void doDraw() override;
	virtual void doUpdate() override;

public:
	ObjectGroup(int x = 0, int y = 0, double a = 1);

	void Clear();
	void ResetAnimations() override;

	ITabbedItem *FindTabbedItem(bool forwards) override;

	virtual ObjectGroup *clone() const override { return new ObjectGroup(*this); }
};

// ==================================================================================
// A single scene
// ==================================================================================

// You can create and store as many scenes as you want and register one at a time with Simple2D as the current scene
// The idea is to pass in all the SceneObjects you want to use, Scene will clone them so they can go out of scope in the
// calling function (and don't need to be dynamically allocated). A pointer to the actual SceneObject is returned if you
// wish to make optional modifications. Calling Remove() with the pointer removes the SceneObject from the scene and
// frees any memory associated with it - therefore you cannot re-add the object after removing it because the pointer
// becomes invalid. To have objects you can re-add, make class/global scope SceneObjects that don't go out of scope
// during the lifetime of your application. Alternatively, toggle an object's visibility by calling On() or Off() on
// a SceneObject instance.

class Scene : public ObjectManager, public boost::noncopyable
{
private:
	// List of objects marked for removal on next update
	vector<SceneObject *> markedForRemoval;

	// User-defined functions to call on specific mouse and keyboard events
	Simple2DMouseMoveFunc moveCallback;
	Simple2DMouseButtonFunc buttonCallback;
	Simple2DKeyCharFunc keyCharCallback;
	Simple2DKeyDownFunc keyDownCallback;
	Simple2DKeyUpFunc keyUpCallback;

	// User-defined functions for other events
	Simple2DVoidCallback onActivateCallback;
	Simple2DVoidCallback onDeactivateCallback;

public:
	// Constructor
	Scene() :	moveCallback(nullptr), buttonCallback(nullptr),
				keyCharCallback(nullptr), keyDownCallback(nullptr), keyUpCallback(nullptr),
				onActivateCallback(nullptr), onDeactivateCallback(nullptr) {}

	// Clear the whole scene (but not until the next frame in case we are processing in another thread,
	// and don't clear any objects we add to the scene in the same frame after the call to Clear!)
	void Clear();

	// Reset the animations for each object
	void ResetAnimations();

	// Update everything in the scene
	void Update();

	// Draw everything in the scene
	void Draw();

	// Set event handlers
	void SetMouseMoveEventHandler(Simple2DMouseMoveFunc f) { moveCallback = f; }
	void SetMouseButtonEventHandler(Simple2DMouseButtonFunc f) { buttonCallback = f; }
	void SetKeyCharEventHandler(Simple2DKeyCharFunc f) { keyCharCallback = f; }
	void SetKeyDownEventHandler(Simple2DKeyDownFunc f) { keyDownCallback = f; }
	void SetKeyUpEventHandler(Simple2DKeyUpFunc f) { keyUpCallback = f; }

	void SetOnActivateEventHandler(Simple2DVoidCallback f) { onActivateCallback = f; }
	void SetOnDeactivateEventHandler(Simple2DVoidCallback f) { onDeactivateCallback = f; }

	// Called from the Windows message pump when an event occurs
	bool OnKeyCharacter(int key, int rc, bool prev, bool trans);
	bool OnKeyDown(int key, int rc, bool prev);
	bool OnKeyUp(int key, int rc);
	bool OnMouseMove(int x, int y, WPARAM keys);
	bool OnMouseButton(UINT button, int x, int y, WPARAM keys);

	// Called by Simple2D when the scene is activated or de-activated
	void OnActivate()   { if (onActivateCallback) onActivateCallback(); }
	void OnDeactivate() { if (onDeactivateCallback) onDeactivateCallback(); }
};

// ==================================================================================
// Skin code. WARNING: EARLY DEVELOPMENT, SUBJECT TO DRASTIC CHANGES!
// This code will be documented when it becomes more stable.
// ==================================================================================

// Template to skin one particular type of InterfaceObject
enum InterfaceGroupLayout { IGL_Row, IGL_Column };

template <typename InterfaceGroupTemplate, typename InterfaceItemTemplate, typename InterfaceItem,
	typename InterfaceItemGroup = UserInterfaceItemGroup<InterfaceItem, InterfaceGroupTemplate, InterfaceItemTemplate> >
class InterfaceItemGroupSkin
{
private:
	ObjectGroup *user;

protected:
	int w;
	int h;

	InterfaceGroupLayout order;

public:
	InterfaceItemGroupSkin() : user(nullptr) {}
	InterfaceItemGroupSkin(ObjectGroup &user)
	{
		this->user = &user;

		w = static_cast<int>(user[BindWidth]);
		h = static_cast<int>(user[BindHeight]);
	}

	virtual int GetWidth() = 0;
	virtual int GetHeight() = 0;
	virtual int GetGap(int n) = 0;
	virtual int GetFirstX(int n) = 0;
	virtual int GetFirstY(int n) = 0;
	virtual InterfaceGroupTemplate GetTemplate() = 0;

	vector<InterfaceItem *> GenerateObjects(std::vector<InterfaceItemTemplate> &items)
	{
		InterfaceGroupTemplate igt = GetTemplate();
		InterfaceItemGroup iig;

		if (order == IGL_Column)
			iig.AddItemColumn(GetFirstX(items.size()), GetFirstY(items.size()), igt, items, GetGap(items.size()));
		else if (order == IGL_Row)
			iig.AddItemRow(GetFirstX(items.size()), GetFirstY(items.size()), igt, items, GetGap(items.size()));

		return user->Add(iig);
	}
};

// Shortcut class names
typedef InterfaceItemGroupSkin<ButtonGroupTemplate, ButtonTemplate, Button> ButtonListSkin;
typedef InterfaceItemGroupSkin<TextBoxGroupTemplate, TextBoxTemplate, TextBox> TextBoxListSkin;

// Base skin class. All objects which have a skin should include this as a member
class ISkin
{
public:
	virtual void Configure(ObjectGroup &user) = 0;
	virtual void Prepare(ObjectGroup &user) = 0;
};

// The skinnable fields and InterfaceItem skins used in a skinned dialog box
class DialogSkin : public ISkin
{
protected:
	Label *errorLabel;

public:
	std::string HeaderLabel;
	std::unique_ptr<ButtonListSkin> ButtonSkin;
	std::unique_ptr<TextBoxListSkin> TextBoxSkin;

	void SetErrorText(string errorText)
	{
		errorLabel->SetText(errorText);
	}
};

// An ObjectGroup with a skin
template <typename SkinBase = ISkin>
class SkinnedObjectGroup : public ObjectGroup
{
protected:
	double w;
	double h;

	SkinBase *skin;

	double *getBinding(SceneObjectBindings b) override
	{
		double *parentBinding = ObjectGroup::getBinding(b);

		if (parentBinding)
			return parentBinding;

		switch (b) {
			case BindWidth: return &this->w;
			case BindHeight: return &this->h;
		}
		return (double *)0;
	}

public:
	SkinnedObjectGroup() : skin(nullptr) {}
	SkinnedObjectGroup(int x, int y, int w, int h) : ObjectGroup(x, y), w(w), h(h), skin(nullptr) {}

	~SkinnedObjectGroup()
	{
		if (skin != nullptr)
			delete skin;
	}

public:
	void SetSkin(SkinBase *skin)
	{
		if (this->skin)
			delete this->skin;

		this->skin = skin;
		this->skin->Configure(*this);
		updateLayout();
		skin->Prepare(*this);
	}

protected:
	virtual void updateLayout() {}
};

// Objects with skins

// Dialog Box
class Dialog : public SkinnedObjectGroup<DialogSkin>
{
private:
	vector<TextBox *> activeTextBoxes;

protected:
	virtual void updateLayout() override;

public:
	// The title of the dialog box
	std::string TopLabel;

	// All of the text boxes in the dialog box
	TextBoxList TextBoxes;

	// All of the buttons in the dialog box
	ButtonList ButtonBar;

	Dialog() {}
	Dialog(int x, int y, int w, int h) : SkinnedObjectGroup(x, y, w, h) {}

	vector<TextBox *> &GetTextBoxes() { return activeTextBoxes; }

	void SetErrorText(string errorText) { skin->SetErrorText(errorText); }

	virtual Dialog *clone() const override { 
		return new Dialog(*this);
	}
};
// End Scene namespace
}

// End Simple2D namespace
}
